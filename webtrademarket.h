#ifndef __GNET_WEBTRADEMARKET_H__
#define	__GNET_WEBTRADEMARKET_H__

#include <map>
#include <set>
#include "itimer.h"
#include "mutex.h"
#include "gwebtradedetail"

#define WEBTRADE_UPDATE_INTERVAL	1000000
#define WEBTRADE_PAGE_SIZE			16
#define MAX_WEBTRADE_SEQ			2000000000
#define UNKNOWN_CATEGORY_ID			1			//δ֪��Ʒ����Ŀ¼id
#define MONEY_MATTER_ID				3044		//��Ǯ�ĵ�����Ʒid
#define ROLE_DUMMY_ID				0			//��ɫ�ļ���id
#define DEFAULT_MIN_POST_MONEY		1000000
#define DEFAULT_SHOW_PERIOD			3*24*60		//3 day
#define DEFAULT_POST_PERIOD			14*24*60	//14 day
#define DEFAULT_POST_DEPOSIT		300000
#define MAX_SELL_PERIOD				7*24*3600	//7 day
#define MAX_ATTEND_SELL				32

namespace GNET
{

class PlayerInfo;
class UserInfo;
class GMailSyncData;
class WebTradePrePost;
class WebTradeRolePrePost;

class WebTradeObj
{
public:
	enum
	{
		STATE_PRE_POST,					//Ԥ����,δ��ƽ̨ͬ��
		STATE_POST,						//���۳ɹ�,��ƽ̨ͬ�����
		STATE_PRE_CANCEL_POST,			//Ԥȡ������,δ��ƽ̨ͬ��
		STATE_SHOW,						//��ʾ
		STATE_SELL,						//�ϼ�
	};

public:
	WebTradeObj(const GWebTradeDetail & _detail) 
		:busy(false),timeout(-1),web_op_timestamp(_detail.post_time),detail(_detail){}
	~WebTradeObj(){}

	bool IsBusy(){ return busy; }
	void SetBusy(bool b, int64_t t = -1)
	{ 
		if(busy = b) timeout = t;
		else timeout = -1;
	}
	bool Update(int64_t t){ 
		if(timeout <= 0) return false;
		if((timeout -= t) <= 0)
		{
			busy = false;
			return true;
		}
		return false;
	}
	bool CheckTimestamp(int64_t t){ return t > web_op_timestamp; }
	void UpdateTimestamp(int64_t t){ web_op_timestamp = t; }
	GWebTradeDetail& GetDetail(){ return detail; }

private:
	bool busy;	//�Ƿ�������gamedbd����ƽ̨ͬ����ͬ���в��ɽ�����������	
	int64_t	timeout;//ͬ��������ʱ����
	int64_t web_op_timestamp;//���һ�����Խ���ƽ̨������ʱ��,���²�����ʱ�������ڴ�ֵ���򲻴���		
	GWebTradeDetail detail;
};

class WebTradeMarket : public IntervalTimer::Observer
{
public:
	typedef unsigned int	category_t;
	typedef int64_t			sn_t;	
	typedef std::multimap<category_t,WebTradeObj>			CategoryMap;	//�������н�����Ʒ
	typedef std::map<sn_t,CategoryMap::iterator>			SNMap;			//sn--������Ʒ  ��ѯ��
	typedef std::set<sn_t>									SNSet;	
	typedef std::map<int/*roleid*/,SNSet>					RoleMap;		//roleid--������Ʒ  ��ѯ��
	typedef std::map<unsigned int/*item_id*/,category_t>	ClassifyMap;	//item_id--Ŀ¼id  ��ѯ��
	struct find_param_t
	{
		category_t category;
		unsigned int handle;
		bool blForward;;
		int roleid;
		find_param_t(category_t c, unsigned int h, bool f, int r):category(c),handle(h),blForward(f),roleid(r){}
	};
	
	enum
	{
		ST_INIT,
		ST_OPEN,
	};	
	
	class SNMan
	{
	public:
		SNMan() : zoneid(0),max_seq(0){}
		bool Initialize(int zid){ zoneid = zid; return true; }
		
		int64_t ApplySN()
		{
			if(seq_pool.size() >= MAX_WEBTRADE_SEQ)
				return (int64_t)0;  // �޿���sn
			do{
				max_seq = (max_seq+1)%MAX_WEBTRADE_SEQ;
			} while(seq_pool.find(max_seq)!=seq_pool.end());
			seq_pool.insert(max_seq);
			return ((int64_t)zoneid<<32) + max_seq;
		}
		void FreeSN(int64_t sn)
		{
			//�Ϸ�֮ǰ������sn�����ô���
			if((sn>>32) != zoneid) return;
			
			seq_pool.erase(sn & 0xFFFFFFFF);
		}
		void HoldSN(int64_t sn)
		{
			//�Ϸ�֮ǰ������sn�����ô���
			if((sn>>32) != zoneid) return;
			
			seq_pool.insert(sn & 0xFFFFFFFF);
		}
		void SetMaxSN(int64_t max_sn)
		{
			max_seq = max_sn & 0xFFFFFFFF;	
		}
		void Advance()
		{
			max_seq = (max_seq+10)%MAX_WEBTRADE_SEQ;
		}
	private:
		int zoneid;
		int	max_seq;
		std::set<int> seq_pool;
	};

private:
	WebTradeMarket():status(ST_INIT),lock("WebTradeMarket::lock"),tick(0){}
	
public:
	//��ʼ��	 
	bool Initialize();	//delivery����ʱ����
	bool LoadConfig();	//��ȡ���ã���ʼ������
	void OnDBConnect(Protocol::Manager * manager, int sid);	//������DBʱ���ã������ȡ���н�����Ʒ����
	void OnDBLoad(int64_t max_sn, std::vector<GWebTradeDetail>& list, bool finish);	//�����ȡ���н�����Ʒ����
	void OnDBLoadSold(std::vector<int64_t>& list, bool finish);	//�����ȡ����ɽ��׵�sn
	//
	bool Update();

public:		
	~WebTradeMarket(){}
	static WebTradeMarket& GetInstance() { static WebTradeMarket instance; return instance; }
	void ClearBusy(int64_t sn);
	//gs ��ز���
	int TryPrePost(WebTradePrePost& proto, PlayerInfo& ui, int ip, GMailSyncData & sync);
	bool OnDBPrePost(const GWebTradeDetail & detail);
	int TryPreCancelPost(int roleid, int64_t sn, PlayerInfo& ui);
	bool OnDBPreCancelPost(int roleid, int64_t sn);				
	int TryRolePrePost(WebTradeRolePrePost& proto, const UserInfo& ui);
	int TryRolePreCancelPost(int roleid, const UserInfo& ui);
	void GetWebTradeList(const find_param_t& param,std::vector<GWebTradeItem>& result,unsigned int& end);
	bool GetWebTrade(int64_t sn, GWebTradeDetail& detail);
	bool GetRoleWebTrade(int roleid, GWebTradeDetail& detail);
	void GetAttendWebTradeList(int roleid, bool getsell, unsigned int begin, std::vector<GWebTradeItem>& result,unsigned int& end);
	//��trade serverͬ������
	void SendPost(category_t c, WebTradeObj & obj);
	void RecvPostRe(bool success, int userid, int64_t sn, int postperiod, int showperiod, int commodity_id);
	bool OnDBPost(int roleid, int64_t sn, int state, int post_endtime, int show_endtime, int sell_endtime, int commodity_id);
	void SendCancelPost(WebTradeObj & obj);
	void RecvCancelPostRe(bool success, int userid, int64_t sn);
	bool OnDBCancelPost(int roleid, int64_t sn);
	//����trade server�Ĳ�������֤sn,roleid,timestampͨ���Ϳ��Ը�����Ʒ״̬��
	int DoWebPostCancel(int userid, int roleid, int64_t sn, int ctype, int64_t messageid, int64_t timestamp);	
	int DoShelf(int userid, int roleid, int64_t sn, int price, int64_t actiontime, int showperiod, int sellperiod, int buyerroleid, int64_t messageid, int64_t timestamp);
	bool OnDBShelf(int roleid, int64_t sn, int state, int show_endtime, int price, int sell_endtime, int buyer_roleid, int buyer_userid, Octets& buyer_name, int64_t timestamp);
	int DoShelfCancel(int userid, int roleid, int64_t sn, int64_t messageid, int64_t timestamp);
	bool OnDBCancelShelf(int roleid, int64_t sn, int64_t timestamp);
	int DoSold(int zoneid, int userid, int roleid, int64_t sn, int buyeruserid, int buyerroleid, int64_t orderid, int stype, int64_t timestamp);
	bool OnDBSold(int roleid, int64_t sn);
	int DoPostExpire(int userid, int roleid, int64_t sn, int64_t messageid, int64_t timestamp);	
	bool OnDBPostExpire(int roleid, int64_t sn);
	void AdvanceSN(){ sn_man.Advance(); }
	
private:
	bool IsMarketOpen()	{ return status == ST_OPEN; }
	category_t GetCategory(int posttype, int item_id);
	int64_t ApplySN(){ return sn_man.ApplySN(); }
	bool RemoveItem(int roleid, int64_t sn, bool freesn);
	int GetAttendListNum(int roleid, bool getsell=true);
		
private:
	int 			status;
	Thread::Mutex 	lock;
	int 			tick;
	
	int				aid;		//��Ϸid
	int 			zoneid;		//������id
	unsigned int	min_post_money;	//��Ϸ�Ҽ�������
	int				show_period;	//��ʾ��ʱ��
	int				post_period;	//����ʱ��
	int				post_deposit;	//���۱�֤��
	
	SNMan			sn_man;
	CategoryMap		category_map;
	SNMap			sn_map;
	RoleMap			role_sell_map;
	RoleMap			role_buy_map;
	ClassifyMap     classify_map;
	SNSet			sold_set;
};

}
#endif



