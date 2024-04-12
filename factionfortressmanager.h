#ifndef __GNET_FACTIONFORTRESSMANAGER_H__
#define __GNET_FACTIONFORTRESSMANAGER_H__

#include "gfactionfortressdetail"
#include "gfactionfortressbriefinfo"
#include "gfactionfortressbattleinfo"
#include "factionext"
#define FACTION_FORTRESS_UPDATE_INTERVAL	1000000	//1��updateһ��
#define FACTION_FORTRESS_CHECKSUM_ONUPDATE	10		//ÿ��update���10��
#define FACTION_FORTRESS_PAGE_SIZE			16

namespace GNET
{

void FactionFortressDetailToBrief(const GFactionFortressDetail & detail, GFactionFortressBriefInfo & brief);
	
class FactionFortressObj
{
public:
	FactionFortressObj():open(false),change_counter(0),syncdb(false),needsyncgs(false){}
	FactionFortressObj(const GFactionFortressDetail & _detail):open(false),change_counter(0),syncdb(false),needsyncgs(false),detail(_detail){}
	~FactionFortressObj(){}
	
	GFactionFortressDetail & GetDetail(){ return detail; }
	bool IsActive(){ return detail.info2.health > 0; }
	void SetOpen(bool b){ open = b; }

	/*gs�����Լ������Ȳ����ӳ�д�̷�ʽ��
	1 �յ�����������޸����ݣ�IncChangeFlag
	2 update()���ж��Ƿ�NeedSyncDB��������SyncDB ��ʼд��
	3 д�̽���OnDBSync(counter)*/
	void IncChangeFlag()
	{ 
		if(++change_counter == 0) 
			change_counter = 1; 
	}
	bool NeedSyncDB(){ return change_counter != 0 && !syncdb; }
	void SyncDB();
	void OnDBSync(size_t save_counter);
	/*��ս��ز��ü�ʱд�̷�ʽ:
	1 �յ�����������ж�InSync�������ʼд��SetSync(true)
	2 д�̽���OnDBSync(0)*/
	bool InSyncDB(){ return change_counter != 0 || syncdb; }
	void SetSyncDB(bool b){ syncdb = b; }

	void SetNeedSyncGS(bool b){ needsyncgs = b && open; }
	bool NeedSyncGS(){ return needsyncgs; }
	void SyncGS(int server_id);
private:
	bool open;		//���صĿ���״̬��gs֪ͨ�ģ�������Ϊ����ԭ���²�׼ȷ
	size_t change_counter;
	bool syncdb;
	bool needsyncgs;
	GFactionFortressDetail detail;
};

class CreateFactionFortress;
class FactionFortressChallenge;
class PlayerInfo;
class GMailSyncData;
class FactionFortressMan : public IntervalTimer::Observer
{
public:
	enum
	{
		ST_INIT,		//���ڳ�ʼ��
		ST_OPEN,		//�ѿ���
		ST_CHALLENGE,	//��ս�׶�
		ST_BATTLE_WAIT,	//�ȴ�ս���׶�
		ST_BATTLE,		//ս���׶�
	};	
		
	typedef std::map<int/*factionid*/,FactionFortressObj> FortressMap;

	struct BATTLE_PERIOD
	{
		int challenge_start_time;
		int challenge_end_time;
		int battle_start_time;
		int battle_end_time;
		BATTLE_PERIOD(int cst,int cet,int bst,int bet):challenge_start_time(cst),challenge_end_time(cet),battle_start_time(bst),battle_end_time(bet){}
	};
	typedef std::vector<BATTLE_PERIOD> BattlePeriodList;
	
public:		
	FactionFortressMan():status(ST_INIT),lock("FactionFortressMan::lock"),health_update_time(0),update_cursor(0),server_id(0),world_tag(0),t_base(0){}
	~FactionFortressMan(){}
	static FactionFortressMan & GetInstance(){ static FactionFortressMan instance; return instance; }
	bool Initialize();
	void OnDBConnect(Protocol::Manager * manager, int sid);
	void OnDBLoad(const std::vector<GFactionFortressDetail>& list, bool finish);
	bool Update();

private:
	void FinalInit();
	bool IsOpen(){ return status != ST_INIT; }
	void UpdateTime(int cur_t);
	void UpdateStatus(int cur_t);
	void GetNextBattleTime(int cur_t, int& start, int& end);
	void ChallengeStart();
	void ChallengeEnd();
	void BattleStart();
	void BattleEnd();
	void BattleClear(const char * msg);
	
public:
	void RegisterServer(int server_id, int world_tag);
	int GameGetFortress(int factionid, GFactionFortressDetail & detail);
	int GamePutFortress(int factionid, const GFactionFortressInfo & info);
	void GameNotifyFortressState(int factionid, int state);
	void SyncFactionServer();
		
public:
	int TryCreateFortress(const CreateFactionFortress & proto, const GFactionFortressInfo & info, const PlayerInfo & ui, const GMailSyncData & sync);
	bool OnDBCreateFortress(const GFactionFortressDetail & detail);
	bool OnDBPutFortress(int factionid, size_t save_counter);
	bool OnDBDelFortress(int factionid);
	void OnDBSyncFailed(int factionid);
	bool CheckEnterFortress(int factionid, int dst_factionid, int & dst_world_tag);
	void GetFortressList(unsigned int & begin, int & status, std::vector<GFactionFortressBriefInfo> & list);
	int TryChallenge(const FactionFortressChallenge & proto, const PlayerInfo & ui, const GMailSyncData & sync);
	bool OnDBChallenge(int factionid, int target_faction); 
	void OnDelFaction(int factionid);
	void GetBattleList(int& status, std::vector<GFactionFortressBattleInfo>& list);
	bool GetFortress(int factionid, GFactionFortressBriefInfo & brief);
	bool GetFactionExt(int factionid, FactionExt & fe);	
	bool DebugDecHealthUpdateTime();
	void DebugAdjustBattlePeriod(bool fastmode);
private:
	int				status;
	Thread::Mutex 	lock;
	FortressMap		fortress_map;

	int health_update_time;
	int update_cursor;

	int	server_id;
	int	world_tag;	
	
	int 			t_base;		//ÿ������ʼʱ�䣬����00:00:00
	BattlePeriodList bp_list;
};

}
#endif
