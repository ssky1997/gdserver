
#ifndef __GNET_GMSHUTUP_HPP
#define __GNET_GMSHUTUP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gauthclient.hpp"
#include "gdeliveryserver.hpp"
#include "maplinkserver.h"
#include "centraldeliveryserver.hpp"

namespace GNET
{

class GMShutup : public GNET::Protocol
{
	#include "gmshutup"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		//����glinkd, GM������ң�dstuseridʵ���Ǳ������ҵ�roleid
		int tmp_roleid = dstuserid;
		
		dstuserid = UidConverter::Instance().Roleid2Uid(dstuserid);
		if(dstuserid == 0) return;
		LinkServer::GetInstance().BroadcastProtocol( this );
		
		GRoleForbid forbid(Forbid::FBD_FORBID_TALK, forbid_time, 0, reason);
		ForbidUserTalk::GetInstance().SetForbidUserTalk(dstuserid, forbid);

		bool is_central = GDeliveryServer::GetInstance()->IsCentralDS();
		if(!is_central) {
			// send to Auth
			GAuthClient::GetInstance()->SendProtocol(this);
		} else {
			UserInfo* pinfo = UserContainer::GetInstance().FindUser(dstuserid);
			if(pinfo && pinfo->src_zoneid != 0) {
				CrossInfoData* pCrsRole = pinfo->GetCrossInfo(tmp_roleid);
				//����ԭ��������roleid��remote_roleid�����෴����ô��ԭ����ǰ��������ȷ����
				this->dstuserid = pCrsRole->remote_roleid; //�����remote_roleid����ԭ����roleid
				if(CentralDeliveryServer::GetInstance()->DispatchProtocol(pinfo->src_zoneid, this)) {
					LOG_TRACE("GMShutUp dispatch to src_zoneid %d success", pinfo->src_zoneid);
				} else {
					Log::log(LOG_ERR, "GMShutUp gmroleid %d dstuserid %d dispatch to src_zoneid %d error", gmroleid, dstuserid, pinfo->src_zoneid);
				}
			} else {
				Log::log(LOG_ERR, "GMShutUp gmroleid %d dstuserid %d src_zoneid %d invalid", gmroleid, dstuserid, pinfo == NULL ? -1 : 0);
			}
		}
	}
};

};

#endif
