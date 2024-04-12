#ifndef __GNET_PSHOPLISTITEM_HPP
#define __GNET_PSHOPLISTITEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "gdeliveryserver.hpp"
#include "pshoplistitem_re.hpp"

namespace GNET
{

class PShopListItem : public GNET::Protocol
{
	#include "pshoplistitem"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		if(itemid <= 0 || (listtype != 0 && listtype != 1)) return;
		
		/*
		 * Э����page_numΪ�ͻ���ϣ�������ҳ���
		 * ���涨���pagenumΪ������ʵ�ʷ��ص�ҳ��
		 * �����������,����ҳ���ʵ������ҳ�治һ��
		 * ��������ҳ�泬���Ѵ���ҳ�淶Χ,�򷵻����һҳ
		 */
		int pagenum = 0;
		PShopItemEntryVector list;
		PShopMarket::find_param_t param(listtype, itemid, page_num);
		PShopMarket::GetInstance().ListItems(param, list, pagenum);
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopListItem_Re(pinfo->localsid, list, listtype, pagenum));
	}
};

};

#endif
