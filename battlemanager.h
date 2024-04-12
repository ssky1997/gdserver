#ifndef __GNET_BATTLEMANAGER_H
#define __GNET_BATTLEMANAGER_H

#include <vector>
#include <map>

#include "thread.h"
#include "gterritorydetail"
#include "battlemapnotice.hpp"
#include "gchallengerinfo"
#include "gchallengerinfolist"
#include "groleinventory"
#include "citywar"
namespace GNET
{   

	class BattleManager : public IntervalTimer::Observer
	{
	public:
		enum
		{
			ST_OPEN      = 0x0001,  // ��ս�����Ƿ���
			ST_BIDDING   = 0x0002,  // �������ۿ�ʼ
			ST_FIGHTING  = 0x0004,  // ��ս������
			ST_DATAREADY = 0x0008,  // ��ս�����Ƿ��Ѿ���ʼ��
			ST_BONUSOK   = 0x0010,  // �������������Ѿ�����
			ST_SETTIME   = 0x0020,  // ��սʱ����δ�趨
			//ST_SPECIALOK = 0x0040,
		};
		enum
		{
			CS_FIGHTING      = 0x0001,  // ս��������
			CS_BIDDING       = 0x0002,  // ���������Ѿ�����,���ڽ������ݿ����
			CS_SENDSTART     = 0x0004,  // ��ʼ��սЭ���Ѿ�����
			CS_BONUSOK       = 0x0008,  // ����������
			CS_BATTLECANCEL  = 0x0010,  // ��սȡ��
		};
		enum
		{
			TERRITORY_NUMBER   = 52,        // ������Ŀ
			//BEGIN_BID_TIME     = 327600,    // ��ս��ʼ,������,19:00
			//BEGIN_REWARD_TIME  = 475200,    // ��������,������,12:00
			//DEFAULT_BID_TIME   = 86400,     // ��սĬ�ϳ���ʱ��
			//MAX_REWARD_TIME    = 21600,     // ս�����淢��ʱ��
			MAX_BATTLE_TIME    = 10800,     // ��ս����ʱ��
			MAX_BONUS          = 2000000000,// ����������� 
			//MAX_DELAY          = 2400,      // Լս����ӳ�ʱ��
		};

		typedef std::vector<GTerritoryDetail>  TVector;
		typedef std::map<int,int> ServerMap;
		
		//joslian
		bool debug_open_challenge;
		bool debug_close_challenge;
		bool debug_send_reward;
		//joslian END
	private:
		/*
		time_t BidBeginTime() { return t_base + BEGIN_BID_TIME;}
		time_t BidDefDuration() { return t_base + BEGIN_BID_TIME + DEFAULT_BID_TIME;}
		time_t BidMaxDuration() { return t_base + BEGIN_BID_TIME + DEFAULT_BID_TIME + MAX_DELAY;}
		time_t RewardBeginTime() { return t_base + BEGIN_REWARD_TIME;}
		*/
		
		enum
		{
			DAY_OF_WEEK_SUNDAY		= 0, // �� [ ����������� ]
		    DAY_OF_WEEK_MONDAY		= 1, // �� [ ����������� ]
		    DAY_OF_WEEK_TUESDAY		= 2, // �� [ �������	 ]
		    DAY_OF_WEEK_WEDNESDAY	= 3, // �� [ �����		 ]
		    DAY_OF_WEEK_THURSDAY	= 4, // �� [ �������	 ]
		    DAY_OF_WEEK_FRIDAY		= 5, // �� [ �������	 ]
		    DAY_OF_WEEK_SATURDAY	= 6, // �� [ �������	 ]
		};
		
		//joslian
		bool IsTimeOpenChallenge(time_t now)
		{
			if (debug_open_challenge)
			{
				debug_open_challenge = false;
				return true;
			}
			
			int open_challenge_1 = t_base + (DAY_OF_WEEK_MONDAY * 86400) + (15 * 3600) + (00 * 60);
			int open_challenge_2 = t_base + (DAY_OF_WEEK_FRIDAY * 86400) + (15 * 3600) + (00 * 60);
			
			if ( now >= open_challenge_1 && now <= open_challenge_1 + 120 )
				return true;
			
			if ( now >= open_challenge_2 && now <= open_challenge_2 + 120 )
				return true;
			
			return false;
		}
		
		bool IsTimeCloseChallenge(time_t now)
		{
			if (debug_close_challenge)
			{
				debug_close_challenge = false;
				return true;
			}
			
			int close_challenge_1 = t_base + (DAY_OF_WEEK_MONDAY * 86400) + (21 * 3600) + (00 * 60);
			int close_challenge_2 = t_base + (DAY_OF_WEEK_FRIDAY * 86400) + (21 * 3600) + (00 * 60);
			
			if ( now >= close_challenge_1 && now <= close_challenge_1 + 120 )
				return true;
			
			if ( now >= close_challenge_2 && now <= close_challenge_2 + 120 )
				return true;
			
			return false;
		}
		
		bool IsTimeSendReward(time_t now)
		{
			if (debug_send_reward)
			{
				debug_send_reward = false;
				return true;
			}
			
			int send_reward_time_1 = t_base + (DAY_OF_WEEK_WEDNESDAY	* 86400) + (15 * 3600) + (30 * 60);
			int send_reward_time_2 = t_base + (DAY_OF_WEEK_SUNDAY		* 86400) + (15 * 3600) + (30 * 60);
			
			if ( now >= send_reward_time_1 && now <= send_reward_time_1 + 120 )
				return true;
			
			if ( now >= send_reward_time_2 && now <= send_reward_time_2 + 120 )
				return true;
			
			return false;
		}
		//joslian
		
		
	protected:
		ServerMap servers;
		Thread::RWLock locker;
		int status;   
		
		unsigned int bonusid;
		int countoflevel1;
		int countoflevel2;
		int countoflevel3;
		int max_bonus_count;
		int proctype;
		unsigned int specialid;
		int countofspecial;
		int specialproctype;
		int max_count_special;

		time_t t_base;      // ����������,00:00
		time_t t_endbid;    // ��ս����ʱ��
		time_t t_forged;   

		int rand_num;
		
		BattleManager() : locker("BattleManager::locker"), status(0),rand_num(0)
		{ 
			t_base = 0;
			t_forged = 0;
			t_endbid = 0;
			
			debug_open_challenge = false;
			debug_close_challenge = false;
			debug_send_reward = false;
		}  
	public:
		TVector cities;

		~BattleManager() { }

		static BattleManager* GetInstance() { static BattleManager instance; return &instance;}
		bool SendMap(int roleid, unsigned int sid, unsigned int localsid);
		bool SendStatus(int roleid,unsigned int sid, unsigned int linkid);
		bool Initialize();
		int Challenge(const Protocol&, Protocol&);
		bool RegisterServer(int server, int world);
		int  FindServer(int map) { return servers[map]; }
		int  GetMapType(int id);
		bool LoadMap(std::vector<GTerritoryDetail>& v);
		bool SyncChallenge(std::vector<GTerritoryDetail>& v);

		bool OnChallenge(short result,int challenge_res,short cid,unsigned int deposit,unsigned int maxbonus,int fid,int ctime,Protocol& reply);
		bool OnBattleEnd(int id, int result, int defender, int attacker);
		bool OnBattleStart(int battleid, int retcode);
		bool OnDBConnect(Protocol::Manager *manager, int sid);
		bool OnDelFaction(unsigned int factionid);
		bool Update();
		void UpdateBid(time_t now);
		void UpdateBattle(time_t now);

		bool ChallengeMap(int roleid,int factionid);
		bool SendPlayer(int roleid, const Protocol& proto, unsigned int& localsid, unsigned int& linkid);
		bool FindSid(int roleid, unsigned int& localsid, unsigned int& linkid, unsigned int& gsid);
		char SelectColor(unsigned int factionid);
		bool GetMapNotice(BattleMapNotice&);
		bool SyncMapNotice();
		bool SyncMapNotice(short);
		bool SyncBattleFaction();
		void BeginChallenge();
		void BeginSendBonus();
		bool SendBonus();
		bool SendSpecialBonus();
		bool OnSendBonus(short ret, unsigned int fid,GRoleInventory &item, unsigned int money);
		void EndChallenge();
		bool ArrangeBattle();
		void TestBattle(int id, int challenger);
		void SetOwner(int id, int factionid);
		time_t UpdateTime();
		time_t GetTime();
		void SetForgedTime(time_t forge);
		bool IsAdjacent(short cityid, unsigned int fid);
		bool GetCityInfo(CityWar & cw);	
	//	ItVector::iterator GetCombination(const ItVector& list,int num);
	};
};
#endif

