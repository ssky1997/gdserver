
#ifndef __GNET_ACKICKOUTUSER_HPP
#define __GNET_ACKICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmkickoutuser.hpp"
#include "gdeliveryserver.hpp"
#include "gauthclient.hpp"
#include "dbforbiduser.hrp"
#include "acforbidcheater.hpp"

namespace GNET
{

class ACKickoutUser : public GNET::Protocol
{
	#include "ackickoutuser"
	
	void KickoutLocalUser(int userid)
	{
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
		if(NULL != pinfo) {
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, KickoutUser(userid, pinfo->localsid, (forbid_time != -1) ? ERR_ACKICKOUT : 0));
			UserContainer::GetInstance().UserLogout(pinfo, KICKOUT_LOCAL, true);
		}
	}
	
	int SendAUForbidUser(int userid)
	{
		int ret = ERR_COMMUNICATION;
		
		if(idtype == 0) 
		{
			//add user to forbidlogin map
			if( forbid_time > 1 ) 
			{
				GRoleForbid forbid(Forbid::FBD_FORBID_LOGIN, forbid_time, time(NULL), reason); 
				ForbidLogin::GetInstance().SetForbidLoginIfLonger( userid, forbid );
			}
			
			// send to Auth,let AU send kickoutuser command
			if( forbid_time >= 0 ) 
			{ 
				if(GAuthClient::GetInstance()->GetVersion() == 1) 
				{
					//����AU
					ACForbidCheater acforbid;
					acforbid.userid = userid;
					acforbid.time = forbid_time;
					acforbid.operation = (forbid_time > 1 ? 0 : 1);
					acforbid.reason.swap(reason);

					if(GAuthClient::GetInstance()->SendProtocol(acforbid)) 
					{
						ret = ERR_SUCCESS;
					} 
				} 
				else 
				{
					GMKickoutUser gmkou;
					gmkou.gmroleid = gmuserid;
					gmkou.kickuserid = userid;
					gmkou.forbid_time = forbid_time;
					gmkou.reason.swap(reason);

					if( GAuthClient::GetInstance()->SendProtocol(gmkou) ) 
					{
						ret = ERR_SUCCESS;
					}
				}

			}
		} 
		else if(forbid_time!=-1) 
		{
			char oper = forbid_time > 1 ? 1 : 2; // 1 ���  2 ���
			char source = 2;
			/*gamedbd����source�жϷ���Ƿ����������(1)������ͨ���(2)
			 *����AU���ַ�������Է���һ��ǿͷ���sourceΪ1��ʾ�����������
			 *����AU�����ַ����Դ��sourceΪ2��ʾ��ͨ���*/
			if(GAuthClient::GetInstance()->GetVersion() == 1)
				source = 1;

			DBForbidUser *rpc = (DBForbidUser *)Rpc::Call(RPC_DBFORBIDUSER, ForbidUserArg(oper, gmuserid, source, userid, forbid_time, reason));

			if( GameDBClient::GetInstance()->SendProtocol( rpc ) ) 
			{
				ret = ERR_SUCCESS;
			}
		}
		
		return ret;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager == CentralDeliveryClient::GetInstance()) 
		{
			//�����Ϣ�ǿ��ת�������ģ����ڿ���Ѿ�����������ʱ��userid��Ȼ���˺�ID
			//�������û��߳�
			if(forbid_time != 1) KickoutLocalUser(userid);
			//֪ͨAU��Ӧ��������Ҫ��GM֪ͨ����������ΪGM��ɫ��ʱ�ٿ����
			SendAUForbidUser(userid); 
		} 
		else //if(mananger == GDeliveryserver::GetInstance())
		{
			//�����Ϣ���Ա���, ������ԭ����Ҳ�����ǿ��
			bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();

			STAT_MIN5("ACKickoutUser", 1);
			LOG_TRACE("ACKickoutUser: GM=%d,idtype=%d,userid=%d,forbid_time=%d,reason_size=%d",gmuserid,idtype,userid,forbid_time,reason.size());

			if( idtype != 0 ) // byroleid
			{
				int uid = UidConverter::Instance().Roleid2Uid(userid);
				if(!uid)
				{
					Roleid2Uid::LegacyFetch(manager, sid, this->Clone(), userid);
					return;
				}
				userid = uid; //��ʱ��userid�Ѿ����������˺�ID
			}

			//�������˺��߳�
			if(forbid_time != 1) KickoutLocalUser(userid);
			
			if(!is_central) //ԭ�������������߼�
			{
				//֪ͨAU��Ӧ����֪ͨGM������
				int ret = SendAUForbidUser(userid); 
				manager->Send(sid, GMKickoutUser_Re(ret, gmuserid, 0, userid));
			}
			else //���
			{
				UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);

				//����Э��ת����ԭ��
				if(pinfo && pinfo->src_zoneid && CentralDeliveryServer::GetInstance()->DispatchProtocol(pinfo->src_zoneid, this))
				{
					manager->Send(sid, GMKickoutUser_Re(ERR_SUCCESS, gmuserid, 0, userid));
				}
				else
				{
					manager->Send(sid, GMKickoutUser_Re(ERR_COMMUNICATION, gmuserid, 0, userid));
				}
			}
		}
	}
};

};

#endif
