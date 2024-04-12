#ifndef __GNET_PSHOPMANAGEFUND_HPP
#define __GNET_PSHOPMANAGEFUND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "pshopmanagefund_re.hpp"
#include "dbpshopmanagefund.hrp"

namespace GNET
{

class PShopManageFund : public GNET::Protocol
{
	#include "pshopmanagefund"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}

	void SendErr(const PlayerInfo *pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopManageFund_Re(retcode, pinfo->localsid));
	}

	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData &data) const
	{
		DBPShopManageFundArg arg(roleid, optype, money, yinpiao, data);
		DBPShopManageFund *rpc = (DBPShopManageFund *)Rpc::Call(RPC_DBPSHOPMANAGEFUND, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		rpc->save_gsid = pinfo.gameid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	bool CheckCondition(const PShopObj *obj)
	{
		/*
		 * ���������gamed�������
		 * ����ֻ���е�����ؼ��
		 * ȡǮ:
		 *      ��Ǯ������ʧ��,��������Ʊ���
		 *      ��ȡǮ����ӡ���չ����������չ�,����ʧ��
		 * ��Ǯ:
		 *      ��Ҫ��֤���ֿ̲����㹻�ռ䱣����������ۻ�õĽ�Ǯ
		 */

		if(optype == 0)//��Ǯ
		{
			if((PSHOP_MONEY_CAP - obj->GetMoney()) < money)
				return false;//��Ǯ��
			if((PSHOP_YINPIAO_CAP - obj->GetYinPiao()) < yinpiao)
				return false;//��Ʊ��

			uint64_t money_save = (uint64_t)money + (uint64_t)yinpiao * (uint64_t)WANMEI_YINPIAO_PRICE;
			uint64_t total_item_value = 0;//��������ֵ
			const PShopItemVector &slist = obj->GetListSale();
			for(size_t i=0; i<slist.size(); ++i)
				total_item_value += ((uint64_t)slist[i].item.count * (uint64_t)slist[i].price);
			if((obj->GetTotalMoney() + total_item_value + money_save) > obj->GetTotalMoneyCap())
				return false;//��Ǯ��
			return true;
		}
		else if(optype == 1)//ȡǮ
		{
			if(obj->GetMoney() < money) return false;
			if(obj->GetYinPiao() < yinpiao) return false;

			uint64_t money_draw = (uint64_t)money + (uint64_t)yinpiao * (uint64_t)WANMEI_YINPIAO_PRICE;
			uint64_t total_cost = 0;//�չ��������Ķ��ٽ�Ǯ
			const PShopItemVector &blist = obj->GetListBuy();
			for(size_t i=0; i<blist.size(); ++i)
				total_cost += ((uint64_t)blist[i].item.count * (uint64_t)blist[i].price);
			return (obj->GetTotalMoney() - total_cost) >= money_draw;

		}

		return true;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GMailSyncData data;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}
		catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::PShopManageFund: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendErr(pinfo, ERR_SHOPMARKET_NOT_INIT);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}
		
		if(optype < 0 || (money <= 0 && yinpiao <= 0))
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}

		PShopObj *obj = PShopMarket::GetInstance().GetShop(roleid);
		if(obj)
		{
			if(!CheckCondition(obj))
			{
				SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
				return;
			}
		}
		else if(optype == 0)//��Ǯ
		{
			//���̲����ڻ��ѹ���
			//����״̬��֧�ִ�Ǯ����
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			if(PShopMarket::GetInstance().GetFromTimeMap(roleid))
				SendErr(pinfo, ERR_PLAYERSHOP_EXPIRED);
			return;
		}

		if(!QueryDB(*pinfo, data))
		{
			SendErr(pinfo, ERR_FAILED);
			SyncGameServer(pinfo, data, ERR_FAILED);
		}
	}
};

};

#endif
