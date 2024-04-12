
#ifndef __GNET_PLAYERLOGIN_RE_HPP
#define __GNET_PLAYERLOGIN_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gauthclient.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"
#include "playeroffline.hpp"

#include "gauthclient.hpp"
#include "accountingrequest.hpp"
#include "getrolebase.hrp"

#include "gamedbclient.hpp"
#include "announcegm.hpp"
#include "queryrewardtype_re.hpp"
#include "postoffice.h"
#include "chatroom.h"
#include "mapforbid.h"
#include "mapuser.h"
#include "announceserverattribute.hpp"
#include "maplinkserver.h"
#include "gamemaster.h"
#include "mappasswd.h"

#include "remoteloginquery.hpp"
#include "centraldeliveryserver.hpp"
#include "crosssystem.h"

namespace GNET
{

class PlayerLogin_Re : public GNET::Protocol
{
	#include "playerlogin_re"
	void ResetRequest(int userid, int linksid, int localsid);

	/*void SendServerAttr(Manager::Session::ID linksid)
	{
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		unsigned int _load=(unsigned int)( (double)(UserContainer().GetInstance().Size()*200)/
			   (double)(UserContainer().GetInstance().GetFakePlayerLimit())	);
		if ( _load>200 ) _load=200;
		dsm->serverAttr.SetLoad((unsigned char)_load);
		LinkServer::GetInstance().BroadcastProtocol(AnnounceServerAttribute(dsm->serverAttr.GetAttr(),
			dsm->freecreatime, dsm->serverAttr.GetExpRate()));
	}*/

	static void RealLogin (int roleid, UserInfo* pinfo)
	{
		int rewardmask = Passwd::GetInstance().GetUserReward(pinfo->userid);
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, QueryRewardType_Re(roleid, pinfo->rewardtype, pinfo->rewardtype2, pinfo->rewarddata, rewardmask));
		
		PlayerInfo* role = UserContainer::GetInstance().FindRole(roleid);
		if(!role) return;
		
		if(pinfo->gmstatus & GMSTATE_ACTIVE) 
		{
			MasterContainer::Instance().Insert(pinfo->userid, roleid, pinfo->linksid, pinfo->localsid, pinfo->privileges);
			
			AnnounceGM agm;
			agm.roleid = roleid;
			agm.auth = pinfo->privileges;
			GProviderServer::GetInstance()->DispatchProtocol( pinfo->gameid, agm );
		}
		
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		dsm->rbcache.Lock();
		GRoleBase* grb = dsm->rbcache.GetDirectly(roleid);
		if(grb == NULL) 
		{
			GetRoleBase* rpc = (GetRoleBase*)Rpc::Call(RPC_GETROLEBASE, RoleId(roleid));
			rpc->need_send2client = false;
			GameDBClient::GetInstance()->SendProtocol(rpc);
		} 
		else 
		{
			role->name=grb->name;
			role->cls=grb->cls;
			UserContainer::GetInstance().InsertName( grb->name, grb->id );
		}
		dsm->rbcache.UnLock();
		
		//update player's mailbox
		PostOffice::GetInstance().OnRoleOnline( roleid, pinfo->linksid, pinfo->localsid );
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("gdelivery::PlayerLogin_Re from gs,roleid=%d,result=%d\n",roleid,result);

		int userid = UidConverter::Instance().Roleid2Uid(roleid);
		if (result==ERR_SUCCESS)
			RoomManager::GetInstance()->ResetRole(roleid);
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		ForbiddenUsers::GetInstance().Pop(userid);

		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		if (NULL == pinfo)
		{
			/* send playeroffline to game server, src_provider_id is linkserver ID */
			if (manager->Send(sid,PlayerOffline(roleid,src_provider_id,localsid))) {
				ForbiddenUsers::GetInstance().Push(userid,roleid,_STATUS_ONLINE);
			}
			return;
		}
		/* if login success, set information in UserInfo struct*/
		/*if (result==ERR_SUCCESS)
		{
			int rewardmask = Passwd::GetInstance().GetUserReward(userid);
			manager->Send(sid, QueryRewardType_Re(roleid,pinfo->rewardtype,pinfo->rewardtype2,pinfo->rewarddata,rewardmask));
			PlayerInfo* role = UserContainer::GetInstance().FindRole(roleid);
			if(!role)
				return;
                        if(pinfo->gmstatus & GMSTATE_ACTIVE)
			{
                                MasterContainer::Instance().Insert(userid, roleid, pinfo->linksid, pinfo->localsid, pinfo->privileges);
				AnnounceGM agm;
				agm.roleid = roleid;
				agm.auth = pinfo->privileges;
				GProviderServer::GetInstance()->DispatchProtocol( pinfo->gameid, agm );
			}

			dsm->rbcache.Lock();
			GRoleBase* grb=dsm->rbcache.GetDirectly(roleid);
			if (grb==NULL)
			{
				GetRoleBase* rpc = (GetRoleBase*) Rpc::Call(RPC_GETROLEBASE,RoleId(roleid));
				rpc->need_send2client = false;
				GameDBClient::GetInstance()->SendProtocol(rpc);
			}
			else
			{
				role->name=grb->name;
				role->cls=grb->cls;
				UserContainer::GetInstance().InsertName( grb->name, grb->id );
			}
			dsm->rbcache.UnLock();
			//update player's mailbox
			PostOffice::GetInstance().OnRoleOnline( roleid,pinfo->linksid,pinfo->localsid );
		}
		// 2 for max player | 5 ro max instance
		else if (2 == result || 5 == result )
		{
			pinfo->gameid = _GAMESERVER_ID_INVALID;
			ResetRequest(userid,pinfo->linksid,pinfo->localsid);	// prepare to reset player position
			return; // cutoff login_re for PlayerPositionResetRqst
		}
		else
			pinfo->gameid = _GAMESERVER_ID_INVALID; */
		
		bool is_central = dsm->IsCentralDS();
		if(!is_central) //ԭ���յ���Э��
		{
			//��Ϊ������½ԭ��flag == 0���ʹӿ���ص�ԭ��flag == CENTRALDS_TO_DS
			//�����������result == ERR_SUCCESSʱ����Ҫ�������к����ĵ�¼����
			if(result == ERR_SUCCESS && (flag == 0 || flag == CENTRALDS_TO_DS) )
			{
				RealLogin(roleid, pinfo); //�������е�¼Role�ĺ�������
			}
			else if((2 == result || 5 == result) && flag == 0) //����player��λ�ã����ܳ�����������½ԭ��flag == 0ʱ���ӿ���ص�ԭ�����ᷢ��
			{
				pinfo->gameid = _GAMESERVER_ID_INVALID;
				ResetRequest(userid,pinfo->linksid,pinfo->localsid);	// prepare to reset player position
				return; // cutoff login_re for PlayerPositionResetRqst
			}
			else 
			{
				pinfo->gameid = _GAMESERVER_ID_INVALID; 
			}
			
			/* when send to link server, set src_provider_id to proper game_id */
			this->src_provider_id = pinfo->gameid;
			dsm->Send(pinfo->linksid,this);
			// send server attribute to linkserver
			dsm->BroadcastStatus();
		}
		else //����յ���Э��
		{
			//��Ϊ��ԭ��->��� flag == DS_TO_CENTRALDS����ֱ�ӵ�¼��� flag == DIRECT_TO_CENTRALDS �������
			if(flag == DS_TO_CENTRALDS || flag == DIRECT_TO_CENTRALDS) 
			{
				//����result�Ľ����Σ���Ҫ��remoteloginquery֪ͨԭ��
				CrossInfoData* info = pinfo->GetCrossInfo(roleid);
				if(info) 
				{
					RemoteLoginQuery query(result, roleid, info->remote_roleid, userid, flag, GDeliveryServer::GetInstance()->GetZoneid());

					if(CentralDeliveryServer::GetInstance()->DispatchProtocol(info->src_zoneid, query)) 
					{
						LOG_TRACE("CrossRelated RemoteLoginQuery flag(%d) src_zoneid %d roleid %d userid %d change user->status from %d to _STATUS_REMOTELOGIN_QUERY",
								flag, info->src_zoneid, roleid, userid, pinfo->status);

						pinfo->status = _STATUS_REMOTE_LOGINQUERY;
					} 
					else 
					{
						if(result == ERR_SUCCESS) 
						{
							result = -100;
						}

						Log::log(LOG_ERR, "CrossRelated RemoteLoginQuery flag(%d) send to src_zoneid %d roleid %d userid %d failed", flag, info->src_zoneid, roleid, userid);
					}
				} 
				else 
				{
					result = -101;
				}

				if(result != ERR_SUCCESS) 
				{
					//ʧ��ʱҪ��User�ǳ�������֪link��Ӧ״̬
					UserContainer::GetInstance().UserLogout(pinfo);
					/* when send to link server, set src_provider_id to proper game_id */
					this->src_provider_id = pinfo->gameid;
					dsm->Send(pinfo->linksid,this);
					// send server attribute to linkserver
					dsm->BroadcastStatus();
				}
			}
		}
	}
};

};

#endif
