
#ifndef __GNET_SWITCHSERVERTIMEOUT_HPP
#define __GNET_SWITCHSERVERTIMEOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
namespace GNET
{

class SwitchServerTimeout : public GNET::Protocol
{
	#include "switchservertimeout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//ֻ�����û����л������з����������˵���������������ȴ�link�������Ķ�����Ϣ
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
		if(!UserContainer::GetInstance().CheckSwitch(pinfo,roleid,link_id,localsid,manager,sid))
			return;
		//�û����л�������û�з������˻��ߵ��ߣ����û���switch_gs_id����Ϊ��
		pinfo->user->switch_gsid = _GAMESERVER_ID_INVALID;
		DEBUG_PRINT("gdelivery::SwitchServerTimeout. user(r:%d, gs_id:%d).pinfo(%d),pinfo->user(%d)\n",pinfo->roleid,pinfo->gameid,(int)pinfo,(int)pinfo->user);

	}
};

};

#endif
