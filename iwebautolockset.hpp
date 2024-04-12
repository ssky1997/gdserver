
#ifndef __GNET_IWEBAUTOLOCKSET_HPP
#define __GNET_IWEBAUTOLOCKSET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "timer.h"

namespace GNET
{

class IWebAutolockSet : public GNET::Protocol
{
	#include "iwebautolockset"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("iwebautolockset: receive. tid=%d userid=%d locktime=%d",tid,userid,locktime);
		// locktimeֵ���壺0: ����, -1: �Ӳ���ʱ��, >0: ��������
		if (locktime < -1) locktime = -1;
		
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());

		UserInfo* userinfo = UserContainer::GetInstance().FindUser(userid);
		if (NULL==userinfo || !userinfo->rolelist.IsRoleListInitialed())
		{
			// ��Ҳ����߻�δ����User���ݣ�ִ�����߲���
			// ����Ĳ���Ӧ��GMSetLocktime����Ϊ����һ��
			std::vector<GPair> autolockdiff;
			autolockdiff.push_back(GPair(LOCKSET_TIME_GM, locktime==0 ? -1 : Timer::GetTime()));
			autolockdiff.push_back(GPair(LOCKTIME_GM, locktime));

			DBAutolockSetOfflineArg arg;
			arg.userid = userid;
			arg.autolockdiff.swap(autolockdiff);

			DBAutolockSetOffline* rpc = (DBAutolockSetOffline*) Rpc::Call(RPC_DBAUTOLOCKSETOFFLINE,
					arg);
			rpc->save_sid = sid;
			rpc->save_tid = tid;

			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
		else
		{
			userinfo->GMSetLocktime(locktime, /*force=*/ true);

			DBAutolockSetArg arg;
			arg.userid = userid;
			arg.autolock = userinfo->autolock.GetList();

			DBAutolockSet * rpc = (DBAutolockSet*) Rpc::Call( RPC_DBAUTOLOCKSET,arg);
			rpc->reason = DBAutolockSet::REASON_IWEB;
			rpc->save_timeout = 0;
			rpc->save_dstroleid = userinfo->roleid;
			rpc->save_sid = sid;
			rpc->save_localsid = 0;
			rpc->save_tid = tid;

			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
		
	}
};

};

#endif
