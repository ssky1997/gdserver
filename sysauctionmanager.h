#ifndef __GNET_SYSAUCTIONMANAGER_H__
#define __GNET_SYSAUCTIONMANAGER_H__

#include <vector>
#include <map>
#include <set>
#include "itimer.h"
#include "mutex.h"
#include "gsysauctiondetail"

#define SYSAUCTION_UPDATE_INTERVAL 	60000000		//60�� updateһ��,��ҪС�����rpcЭ���timeout
#define SYSAUCTION_MAX_COUNT		100				//���������Ʒ�������Ӵ�ֵ�������Э���maxsize
#define SYSAUCTION_MAX_CASH			10000000		//�˺�����10��Ԫ��
#define SYSAUCTION_BID_FREEZETIME	900				//��ǰ���۱���900������˳�������������
#define SYSAUCTION_PREPAREITEM_TIME	17*3600			//17:00��ʼ���ɾ�����Ʒ
#define SYSAUCTION_START_TIME		20*3600			//20:00���Ŀ�ʼ
#define SYSAUCTION_END_TIME			23*3600			//23:00ǿ�����о��Ľ���
#define SYSAUCTION_CLEAR_TIME		23*3600+50*60	//23:50ɾ������


namespace GNET
{
	
class UserSysAuctionInfo
{
public:
	struct BidItem
	{
		unsigned int sa_id;
		unsigned int bid_price;
		BidItem(unsigned int id, unsigned int price):sa_id(id),bid_price(price){}
	};
	
public:
	UserSysAuctionInfo(int _userid): busy(false), userid(_userid), cash(0), cash_used(0){}
	void UpdateInfo(unsigned int _cash, unsigned int _cash_used)
	{
		cash = _cash;
		cash_used = _cash_used;
	}
	void UpdateInfo(unsigned int _cash)
	{
		cash = _cash;
	}

	bool IsBusy() { return busy; }
	void SetBusy(bool b) { busy = b; }
	
	void AddBid(unsigned int sa_id, unsigned int bid_price)
	{
		bid_list.push_back(BidItem(sa_id, bid_price));
	}
	void DelBid(unsigned int sa_id)
	{
		std::vector<BidItem>::iterator it = bid_list.begin(), ie = bid_list.end();
		for(; it!=ie; ++it)
		{
			if(it->sa_id == sa_id)
			{
				bid_list.erase(it);
				break;
			}
		}
	}
	void GetBid(std::vector<unsigned int>& bid_ids)
	{
		std::vector<BidItem>::iterator it = bid_list.begin(), ie = bid_list.end();
		for(; it!=ie; ++it)
		{
			bid_ids.push_back(it->sa_id);
		}	
	}
	size_t BidSize()
	{
		return bid_list.size();
	}
	bool VerifyBid(unsigned int sa_id, unsigned int bid_price)
	{
		std::vector<BidItem>::iterator it = bid_list.begin(), ie = bid_list.end();
		for(; it!=ie; ++it)
		{
		 	if(it->sa_id == sa_id && it->bid_price == bid_price)
				return true;
		}
		return false;
	}
	unsigned int GetFreeCash()
	{
		unsigned int freeze_cash = 0;
		std::vector<BidItem>::iterator it = bid_list.begin(), ie = bid_list.end();
		for(; it!=ie; ++it)
			freeze_cash += it->bid_price;
		if(cash < freeze_cash)
		{
			Log::log(LOG_ERR, "SysAuctionManager GetFreeCash userid %d cash(%u)<freeze_cash(%u)", cash, freeze_cash);
			return 0;
		}
		return cash - freeze_cash;
	}
	unsigned int GetMaxCash()
	{
		return cash;
	}

private:
	bool busy;	
	int userid;
	unsigned int cash;			//ͬUser���е�cash_2
	unsigned int cash_used;		//ͬUser���е�cash_use_2
	std::vector<BidItem> bid_list;
};

class SysAuctionObj
{
public:
	enum
	{
		STATE_PREPARE_START,		//�ȴ�����
		STATE_START,				//������
		STATE_PREPARE_END,			//�ȴ�������Ʒ
		STATE_END,					//���Ľ���
	};
public:
	SysAuctionObj(const GSysAuctionDetail& _detail) : busy(false),detail(_detail){}
	~SysAuctionObj(){}

	GSysAuctionDetail& GetDetail(){ return detail; }
	bool IsBusy() { return busy; }
	void SetBusy(bool b) { busy = b; }
	
private:
	bool busy;
	GSysAuctionDetail detail;
};

class PlayerInfo;
class SysAuctionManager : public IntervalTimer::Observer
{
public:
	enum
	{
		ST_PREPARED = 0x01,
		ST_START	= 0x02,
		ST_END		= 0x04,
	};
	
	struct SellItem
	{
		int item_id;
		int base_price;
		int auction_time;
		SellItem(int id, int p, int t):item_id(id), base_price(p), auction_time(t){}
	};
	typedef std::map<int/*userid*/, UserSysAuctionInfo> 				UserMap;
	typedef std::map<unsigned int/*sys auction id*/, SysAuctionObj> 	SysAuctionMap;
	typedef std::vector<SellItem>										SellItemList;

public:
	//��ʼ��	 
	bool Initialize();	//delivery����ʱ����
	//
	bool Update();

	void OnLogin(int userid, unsigned int cash, unsigned int cash_used);
	void OnLogout(int userid);
	
public:		
	~SysAuctionManager(){}
	static SysAuctionManager& GetInstance() { static SysAuctionManager instance; return instance; }
	
	void GetSysAuctionList(std::vector<GSysAuctionItem>& items);
	bool GetSysAuction(unsigned int sa_id, GSysAuctionDetail & detail);
	int GetSysAuctionAccount(int userid, unsigned int & cash, std::vector<unsigned int>& bid_ids);
	void OnGSPrepareItem(std::vector<int>& indexes, GRoleInventoryVector & items);
	int TryBid(int userid, int roleid, unsigned int sa_id, unsigned int bid_price, unsigned int& cash, GSysAuctionItem & info);
	int TryCashTransfer(char withdraw, unsigned int cash, PlayerInfo& ui, GMailSyncData& sync);
	bool OnDBCashTranfer(int userid, unsigned int& cash);
	bool OnDBCashSpend(unsigned int sa_id, int userid, unsigned int cash, unsigned int cash_used);
	void ClearUserBusy(int userid);
	void ClearSysAuctionBusy(unsigned int sa_id);
	int GetTime();
	void SetAdjustTime(int t);
private:
	SysAuctionManager():lock("SysAuctionManager::lock"),status(0),adjust_time(0),tick_counter(0),auction_max(0)
	{
		max_id = GetTime();
	}
	unsigned int ApplyID(){ return ++max_id; }
	void PrepareSellItem();
	void NotifyStartTime();
	void StartSysAuction();
	void UpdateSysAuction();
	void EndSysAuction();
	void ClearSellItem();

private:
	Thread::Mutex	lock;
	unsigned int 	status;
	
	unsigned int	max_id;
	int				adjust_time;		//�����ã���ǰʱ��ĵ���ֵ
	int				tick_counter;
	
	UserMap 		user_map;
	SysAuctionMap 	sysauction_map;
	SellItemList	sell_item_list;
	int				auction_max;
};

}
#endif
