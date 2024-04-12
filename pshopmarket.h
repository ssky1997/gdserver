#ifndef __GNET_PSHOP_MARKET_H__
#define __GNET_PSHOP_MARKET_H__

#include <itimer.h>
#include <pshopbase>
#include <pshopdetail>
#include <pshopentry>
#include <pshopitementry>

namespace GNET
{

class PShopDetail;
class PShopObj
{
	bool _busy;//ȡ�����ۻ��չ�,�������ʱ����
	PShopDetail _detail;

public:
	friend class PShopMarket;
	PShopObj(const PShopDetail &shop):_busy(false)				{ _detail = shop; }
	operator PShopDetail &() 									{ return _detail; }
	operator const PShopDetail &() const 						{ return _detail; }
	inline char GetType() const 								{ return _detail.shoptype; }
	inline int GetRoleID() const  								{ return _detail.roleid; }
	inline int GetCreateTime() const 							{ return _detail.createtime; }
	inline int GetExpireTime() const 							{ return _detail.expiretime; }
	inline unsigned int GetMoney() const						{ return _detail.money; }
	inline const PShopItemVector & GetListBuy() const			{ return _detail.blist; } 
	inline const PShopItemVector & GetListSale() const			{ return _detail.slist; } 
	inline const GRoleInventoryVector &GetStore() const			{ return _detail.store; }
	inline void SetType(char type)								{ _detail.shoptype = type; }
	inline void SetStatus(int status)							{ _detail.status = status; }
	inline void SetExpireTime(int time)							{ _detail.expiretime = time; }
	inline void SetMoney(int money)								{ if(money < 0) return; _detail.money = money; }
	inline void SetStore(const GRoleInventoryVector & store)	{ _detail.store = store; }
	inline void AddItemBuy(const PShopItem & item)				{ _detail.blist.push_back(item); }
	inline void AddItemSale(const PShopItem & item)				{ _detail.slist.push_back(item); }
	inline void AddItemStore(const GRoleInventory & item)		{ _detail.store.push_back(item); }
	inline void ClearBuyList()									{ _detail.blist.clear(); }
	inline void ClearSaleList()									{ _detail.slist.clear(); }
	inline void ClearStore()									{ _detail.store.clear(); }
	inline bool IsBusy() const									{ return _busy; }
	inline void SetBusy(bool b)									{ _busy = b; }
	inline void SetYinPiao(const GRoleInventoryVector & yp)		{ _detail.yinpiao = yp; }
	unsigned int GetYinPiao() const;
	PShopBase GetShopBase() const;
	uint64_t GetTotalMoney() const;
	uint64_t GetTotalMoneyCap() const;
	const PShopItem * GetItemBuy(int pos) const;
	const PShopItem * GetItemSale(int pos) const;
	const GRoleInventory * GetItemStore(int pos) const;
	void RemoveItemBuy(int pos);
	void RemoveItemSale(int pos);
	void RemoveItemStore(int pos);
	void UpdateItemStore(const GRoleInventory & item);
	void Destroy();
	void Trace() const;
};

class PShopMarket : public IntervalTimer::Observer
{
	/*��Ʒ������*/
	struct IndexEntry
	{
		int ref;//���ü���(��Ǳ������ж�����λ�ڳ��ۻ��չ�����Ʒ)
		const PShopObj *p;
		IndexEntry():ref(0),p(0){}
		explicit IndexEntry(const PShopObj *ptr):ref(1),p(ptr){}
	};

public:
	typedef unsigned int									ROLEID;
	typedef std::pair<int/*ʱ��*/,ROLEID>					TIME_PAIR;
	typedef std::map<ROLEID, PShopObj*>						PSHOP_MAP;
	typedef std::vector<std::vector<PShopObj*> >			PSHOP_POOL;
	typedef std::vector<IndexEntry>							INDEX_LIST;
	typedef std::map<int/*��Ʒid*/, INDEX_LIST>				ITEM_MAP;
	typedef std::multimap<int/*ʱ��*/, ROLEID>				TIME_MAP;
	struct find_param_t
	{
		char type;//��ѯ��Ʒ����(0:����1:�չ�)
		int itemid;//��ѯ��ƷID
		int page_num;//����ڼ�ҳ(��0ҳ��ʼ)
		find_param_t(char t,int id,int pn):type(t),itemid(id),page_num(pn){}
	};

	PShopMarket():_init(false),_hb_count(0) {}
	~PShopMarket() {__OnDestroy();}
	static PShopMarket &GetInstance() {static PShopMarket instance; return instance;}
	static void MoneyToYinPiao(uint64_t money, unsigned int &yp, unsigned int &remains);
	bool Initialize();
	void LoadFromDB(const PShopDetailVector &, bool);
	void AddShop(const PShopDetail & shop);
	void RemoveShop(int roleid);
	void OnAddItemBuy(int roleid, const PShopItem &item);
	void OnAddItemSale(int roleid, const PShopItem &item);
	void OnRemoveItemBuy(int roleid, int pos);
	void OnRemoveItemSale(int roleid, int pos);
	void OnRemoveListBuy(PShopObj *obj);
	void OnRemoveListSale(PShopObj *obj);
	void OnModifyType(int roleid, int newtype);
	void OnModifyExpireTime(int roleid, int time);
	virtual bool Update();
	void Trace() const;
	bool IsLoadComplete() const { return _init;}

	PShopObj* GetShop(ROLEID roleid);
	const PShopObj* GetShop(ROLEID roleid) const {return GetShop(roleid);}
	bool GetFromTimeMap(ROLEID roleid) const;
	void ListShops(char type, PShopEntryVector &list) const;
	void ListItems(const find_param_t &param, PShopItemEntryVector &list, int &pagenum) const;

private:
	void __OnDestroy();
	void __OnAddItem(const PShopObj *obj, const PShopItem &, ITEM_MAP &);
	void __OnRemoveItem(const PShopObj *obj, const PShopItem &, ITEM_MAP &);
	void __RemoveShop(ROLEID roleid);
	void __RemoveFromTimeMap(ROLEID roleid);
	void __RemoveFromTimeMap(ROLEID roleid, int time);

private:
	PSHOP_MAP	_shopmap;		//�����б�
	PSHOP_POOL	_shoppool;		//��������������
	ITEM_MAP	_buymap;		//�չ���Ʒ������
	ITEM_MAP	_salemap;		//������Ʒ������
	TIME_MAP	_timemap;		//����ʱ��������
	bool		_init;			//DB�������
	int			_hb_count;		//��������
};

enum PLAYERSHOP_INDEX
{
	PSHOP_INDEX_MIN					= 0,
	PSHOP_INDEX_VEHICLE				= 0,//��ͨ����
	PSHOP_INDEX_TREASURECHESTS		= 1,//����
	PSHOP_INDEX_MEDICAL				= 2,//ҩƷ
	PSHOP_INDEX_DRESS				= 3,//ʱװ
	PSHOP_INDEX_EQUIPMENT			= 4,//װ��
	PSHOP_INDEX_STRENGTHENITEM		= 5,//ǿ������
	PSHOP_INDEX_MATERIAL			= 6,//����
	PSHOP_INDEX_GROCERIES			= 7,//�ӻ�
	PSHOP_INDEX_MAX,
};

enum PLAYERSHOP_MASK
{
	PSHOP_MASK_VEHICLE				= 0x01,
	PSHOP_MASK_TREASURECHESTS		= 0x02,
	PSHOP_MASK_MEDICAL				= 0x04,
	PSHOP_MASK_DRESS				= 0x08,
	PSHOP_MASK_EQUIPMENT			= 0x10,
	PSHOP_MASK_STRENGTHENITEM		= 0x20,
	PSHOP_MASK_MATERIAL			= 0x40,
	PSHOP_MASK_GROCERIES			= 0x80,
	PSHOP_MASK_ALL=0xFF,
};

};
#endif
