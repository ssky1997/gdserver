#include "gproviderserver.hpp"
#include "dbuniquedataload.hrp"
#include "dbuniquedatasave.hrp"
#include "uniquedatamodifynotice.hpp"
#include "uniquedatamodifybroadcast.hpp"
#include "uniquedatasynch.hpp"
#include "uniquedataserver.h"
#include "localmacro.h"
#include "maplinkserver.h"

namespace GNET
{

template<typename vt>
bool MergeUniqueData(Octets& localdata,Octets& newdata,Octets& olddata)	
{
	vt localval,newval,oldval;
	

	if(localdata.size() == sizeof(vt) &&
	   newdata.size() == sizeof(vt) &&
	   olddata.size() == sizeof(vt) )
	{
		localval = *((vt*)localdata.begin());
		newval = *((vt*)newdata.begin());
		oldval = *((vt*)olddata.begin());
	}
	else
	{
		newdata = localdata;  // �����쳣��server����Ϊ׼ͬ��
		return false;
	}

	localval += (newval - oldval);
	*((vt*)localdata.begin()) = localval;
	newdata = localdata;
	return true;
}

// newdata�ᱻ�ı��delivery������ֵ
int UniqueDataServer::SGUniqueData::SGUniqueDataElem::modify(int vtype,Octets& newdata,Octets& olddata,bool exclusive,int& ver,bool timeout)
{
	if(elem.vtype && elem.vtype != vtype)
	{
		newdata = elem.value;
		return ERR_UNIQUE_TYPE_INVALID;
	}

	if(elem.version != ver && (exclusive || timeout)) // ��ʱ�Ͷ�ռ���԰汾���ϸ�Ҫ��
	{
		newdata = elem.value; 
		ver = elem.version;
		return ERR_UNIQUE_VERSION_FAIL;
	}
	
	ver = elem.version;

	if(elem.vtype && olddata != elem.value) // �ѳ�ʼ����δͬ������
	{
		if(exclusive)
		{
			newdata = elem.value;
			return ERR_UNIQUE_CLASH;
		}

		switch(elem.vtype)
		{
			case UDT_INT:
				{
					if(!MergeUniqueData<int>(elem.value, newdata, olddata))
						return ERR_UNIQUE_TYPE_INVALID;
				}
				break;
			case UDT_DOUBLE:
				{
					if(!MergeUniqueData<double>(elem.value, newdata, olddata))
						return ERR_UNIQUE_TYPE_INVALID;
				}
				break;
			default:
				return ERR_UNIQUE_TYPE_INVALID;
		}

	}
	else
	{
		elem.value = newdata;
	}

	elem.vtype = vtype;
	ver = ++elem.version;
	wflag = true;

	return ERR_SUCCESS;
}

bool UniqueDataServer::Initialize()
{
	IntervalTimer::AddTimer(this,30); // 30s
	return true;
}

bool UniqueDataServer::Update()
{
	if(_initialized)
	{
		Thread::Mutex::Scoped l(lock);
		SyncModifyToDB();
		SyncModifyToClient();
	}

	return true;
}

void UniqueDataServer::OnDBLoad(GUniqueDataElemNodeVector & values, bool finish)
{
	if(!_initialized)
	{
		Thread::Mutex::Scoped l(lock);
	
		_data.load(values);

		_initialized = finish;

		if(_initialized)
		{
			SyncAllToGS();
		}
	}
}

void UniqueDataServer::OnGSConnect(Protocol::Manager * manager, int sid)
{
	if(!_initialized) return;
	Thread::Mutex::Scoped l(lock);
	SyncAllToGS(sid);
}


void UniqueDataServer::ModifyUniqueData(int worldtag,int key, int vtype, Octets& val,Octets& oldval, bool excl, bool broadcast, int sid, int version,bool timeout)
{
	if(!_initialized) 
	{
		Log::log(LOG_ERR,"ModifyUniqueData,Uninitialized, worldtag=%d,key=%d,vtype=%d,excl=%d,\n", worldtag, key, vtype, excl?1:0);
		val.dump(); oldval.dump();
		return;
	}

	Thread::Mutex::Scoped l(lock);

	int retcode = _data.elems[key].modify(vtype,val,oldval,excl,version,timeout);
	
	SyncModifyToGS(worldtag, key, vtype, val, oldval, excl, retcode, version, ERR_SUCCESS == retcode ? -1 : sid);
	Log::log(LOG_NOTICE,"ModifyUniqueData, Modify, worldtag=%d,key=%d,vtype=%d,excl=%d retcode=%d\n",worldtag, key, vtype, excl?1:0,retcode);

	if( ERR_SUCCESS == retcode && broadcast )
	{
		_data.elems[key].broadcast = true;
	}
	else if(ERR_SUCCESS != retcode)
	{
		Log::log(LOG_ERR,"ModifyUniqueData, Modify fail, worldtag=%d,key=%d,vtype=%d,excl=%d retcode=%d\n",worldtag, key, vtype, excl?1:0,retcode);
	}

}

void UniqueDataServer::InitGSData(int worldtag, int sid)
{
	if(!_initialized) 
	{
		Log::log(LOG_NOTICE,"InitGSData,Uninitialized, worldtag=%d ,sid=%d\n", worldtag, sid);
		UniqueDataSynch proto(-1);
		GProviderServer::GetInstance()->Send(sid, proto);
		return ;
	}

	Thread::Mutex::Scoped l(lock);
	SyncAllToGS(sid);
	Log::log(LOG_NOTICE,"InitGSData, Success, worldtag=%d ,sid=%d\n", worldtag, sid);
}

void UniqueDataServer::SyncModifyToGS(int worldtag,int key, int vtype, Octets& val,Octets& oldval, bool excl,int retcode,int version ,int sid)
{
	if(sid == -1)
		GProviderServer::GetInstance()->BroadcastProtocol(UniqueDataModifyNotice(worldtag,key,vtype,val,oldval,excl,retcode,version));
	else
		GProviderServer::GetInstance()->Send(sid, UniqueDataModifyNotice(worldtag,key,vtype,val,oldval,excl,retcode,version));
}

void UniqueDataServer::SyncAllToGS(int sid)
{
	UniqueDataSynch proto(char(0));
	SGUniqueData::GUHandle handle(-1);
	
	while(!proto.finish)
	{	
		proto.finish = _data.save(proto.values, SGUniqueData::SAVE_TO_GS, handle);

		if(sid == -1)
			GProviderServer::GetInstance()->BroadcastProtocol(proto);
		else
			GProviderServer::GetInstance()->Send(sid, proto);

		proto.values.clear();
	}
}

void UniqueDataServer::SyncModifyToDB()
{
	GUniqueDataElemNodeVector values;
	SGUniqueData::GUHandle handle(-1);
	bool finish = false;

	while(!finish)
	{	
		finish = _data.save(values, SGUniqueData::SAVE_TO_DB, handle) || (0 == values.size());
		
		if(values.size())
		{
			GameDBClient::GetInstance()->SendProtocol(Rpc::Call(RPC_DBUNIQUEDATASAVE, DBUniqueDataSaveArg(values)));
			values.clear();
		}
	}
}

void UniqueDataServer::SyncModifyToClient()
{
	static const int DELAY_CSTICK = 10; // 5min

	if(_clientsynctick > 0) 
	{
		--_clientsynctick;
		return;
	}

	UniqueDataModifyBroadcast proto(-1);
	_data.save(proto.values, SGUniqueData::SAVE_TO_CL, proto.handle);

	if(proto.values.size())
	{
		LinkServer::GetInstance().BroadcastProtocol(&proto);
		_clientsynctick = DELAY_CSTICK;
	}
}

void UniqueDataServer::OnDBConnect(Protocol::Manager * manager, int sid)
{
	if(!_initialized)
		manager->Send(sid, Rpc::Call(RPC_DBUNIQUEDATALOAD, DBUniqueDataLoadArg()));	
}

}
