#ifndef __GNET_COUNTRY_BATTLE_MAN_H
#define __GNET_COUNTRY_BATTLE_MAN_H

#include <vector>
#include <map>

#include "countrybattleapplyentry"
#include "gcountrycapital"
#include "player"
#include "gcountrybattlepersonalscore"
#include "gcountrybattlelimit"
#include "localmacro.h"

namespace GNET
{
#define RANK_LIST_MAX 20
#define WAR_TYPE_MAX 3

const int rank_point_list[RANK_LIST_MAX] = {40, 35, 30, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12 ,11, 10, 9, 8};
const int country_battle_players_cnt[WAR_TYPE_MAX] = {30, 20, 40};

class CountryBattleMan: public IntervalTimer::Observer
{
public:		
	enum {
		ARRANGE_COUNTRY_RANDOM = 0,
		ARRANGE_COUNTRY_BY_ZONEID,
	};
	
	enum {
		WAR_TYPE_FLAG = 0,
		WAR_TYPE_TOWER,
		WAR_TYPE_CRYSTAL,
	};
	
	enum {
		COUNTRY_MAX_CNT = 4,
		PLAYER_WAIT_TIME = 10,
		DOMAIN_WAIT_TIME = 60,
		DOMAIN_COLD_TIME = 60,
		BATTLE_LAST_TIME = 10800,
		SINGLE_BATTLE_END_TIME = 1500,
		TEN_MINUTES = 600,
		BONUS_ITEM_ID = 36343,
		MAX_OCCUPATION_CNT = USER_CLASS_COUNT,
		PLAYER_MAX_BONUS = 10000,
		BONUS_PROCTYPE = 16403,
		DOMAIN_BATTLE_LIMIT_CNT = 7,
		WEEK_DAY_CNT = 7,
		
		COUNTRYBATTLE_START_TIME = (20 * 3600 + 20 * 60), //20:20 ������ս
		COUNTRYBATTLE_BONUS_TIME = (21 * 3600 + 21 * 60), //22:21 ��ս��������ʼ����
		COUNTRYBATTLE_CLEAR_TIME = (23 * 3600 + 30 * 60), //23:30 ��չ�ս��Ϣ
	};
	
	enum {
		MAJOR_STR_NONE = 0,
		MAJOR_STR_REFINE16,
		MAJOR_STR_FORCE9,
		MAJOR_STR_MAX,
	};
	
	enum {
		ST_CLOSE = 0,
		ST_OPEN,
		ST_BONUS,
		ST_MAX,
	};
	
	enum {
		PLAYER_STATUS_NORMAL = 0,
		PLAYER_STATUS_WAIT_FIGHT,
		PLAYER_STATUS_FIGHT,
		PLAYER_STATUS_MAX,
	};
	
	enum {
		DOMAIN_STATUS_NORMAL = 0,
		DOMAIN_STATUS_WAIT_FIGHT,
		DOMAIN_STATUS_FIGHT,
		DOMAIN_STATUS_COLD,
		DOMAIN_STATUS_MAX,
	};
	
	enum {
		REASON_SYNC_MOVE = 0,
		REASON_SYNC_PREV_STEP,
		REASON_SYNC_CAPITAL,
		REASON_SYNC_CLI_REQUEST,
		REASON_SYNC_FINISH,
	};
	
	enum {
		REASON_PLAYER_ENTER_BATTLE = 0,
		REASON_PLAYER_LEAVE_BATTLE,
		REASON_PLAYER_FINISHI_MOVE,
		REASON_PLAYER_RETURN_PREV,
		REASON_PLAYER_RETURN_CAPITAL,
		REASON_PLAYER_WAIT_FIGHT,
	};
	
	enum {
		REASON_DOMAIN_WAIT_FIGHT = 0,
		REASON_DOMAIN_BATTLE_TIMEOUT,
		REASON_DOMAIN_COLD_TIMEOUT,
		REASON_DOMAIN_BATTLE_START_SUCCESS,		
		REASON_DOMAIN_BATTLE_START_FAILED,	
		REASON_DOMAIN_BATTLE_END,
		REASON_DOMAIN_WAIT_FIGHT_TIMEOUT,
	};

	enum {
		REASON_SYS_HANDLE_LEAVE_BATTLE = 0,
		REASON_SYS_HANDLE_INVALID_DOMAIN,
		REASON_SYS_HANDLE_WAIT_TIMEOUT,
		REASON_SYS_HANDLE_INVALID_BATTLE,
		REASON_SYS_HANDLE_COLD_DOMAIN,
		REASON_SYS_HANDLE_BATTLE_START_TIMEOUT,
		REASON_SYS_HANDLE_BATTLE_START_FAILED,
		REASON_SYS_HANDLE_FIGHT_TIMEOUT,
		REASON_SYS_HANDLE_COLD_DOMAIN_TIMEOUT,
		REASON_SYS_HANDLE_BATTLE_FULL,
		REASON_SYS_HANDLE_UNDER_BATTLE_LIMIT,
	};
	
	enum {
		BATTLE_CONFIG_NULL = 0x00,
		BATTLE_CONFIG_ASSAULT1 = 0x01,
		BATTLE_CONFIG_ASSAULT2 = 0x02,
		BATTLE_CONFIG_LIMIT = 0x80,
	};
	
	enum {
		BATTLE_CONFIG_KING_POINT = 3000,
		BATTLE_CONFIG_ASSAULT1_POINT = 50,
		BATTLE_CONFIG_ASSAULT2_POINT = 100,
		BATTLE_CONFIG_LIMIT_CLEAR = 0,
		BATTLE_CONFIG_LIMIT_SET = 1,
	};

	struct DomainBattleLimit
	{
		int domain_id;
		GCountryBattleLimit limit[MAX_OCCUPATION_CNT];
	};
	
	struct CountryKing
	{
		int roleid;
		int command_point;
		DomainBattleLimit battle_limit_config[DOMAIN_BATTLE_LIMIT_CNT];

		void Init(int roleid_);
	};
	
	struct PlayerEntry
	{
		int roleid;
		char country_id;
		char status;
		int in_domain_id;
		int prev_domain_id;
		int worldtag;
		int occupation;
		int minor_str; 
		unsigned int linksid;
		unsigned int localsid;
		
		void Init(int roleid_, char country_id_, char status_, int in_domain_id_, int prev_domain_id_, int worldtag_, int occupation_, int minor_str_, 
			unsigned int linksid_, unsigned int localsid_);
	};
	
	struct DomainInfo
	{
		int id;
		char owner;
		char challenger;
		char init_country_id;
		char wartype;
		char status;	
		char battle_config_mask[COUNTRY_MAX_CNT];
		char owner_occupation_cnt[MAX_OCCUPATION_CNT];
		char challenger_occupation_cnt[MAX_OCCUPATION_CNT];
		int time; //����״̬��ֵ���ã��ȴ�ս��״̬�ǳ�ʱʱ�䣬ս��״̬ʱս������ʱ�䣬��ȴ״̬����ȴʱ��
		std::vector<int> player_list;
		
		void Init(int id_, char owner_, char challenger_, char init_country_id_, char wartype_, char status_);
	};
	
	struct CapitalInfo
	{
		int id;
		struct CapitalPos{
			float x;
			float y;
			float z;
		};
		
		CapitalPos pos[4];
	};
	
	typedef std::map<int/*linkid*/, PlayerVector> COUNTRY_COMMUNICATION_MAP;
	struct CountryInfo
	{
		float major_strength;
		int minor_strength;
		int online_player_cnt;
		int domains_cnt; //��¼��Ӫռ�ݶ������
		int country_scores;
		float player_total_scores; //��¼��Ӫ����ܵ÷�

		CountryKing king_info;
		COUNTRY_COMMUNICATION_MAP communication_map;
	};
	
	struct MoveInfo
	{
		int roleid;
		int from;
		int to;
		int time;
		
		void Init(int roleid_, int from_, int to_, int time_);
	};
	
	struct ColdInfo
	{
		int id;
		int time;
		bool is_notify;

		void Init(int id_, int time_);
	};
	
	struct RegisterInfo
	{
		int war_type;
		int world_tag;
		int server_id;

		void Init(int war_type_, int world_tag_, int server_id_);
	};
	
	struct PlayerScoreInfo
	{
		int roleid;
		char country_id;
		short win_cnt;
		short dead_cnt;
		int total_combat_time;
		int total_contribute_val;
		float score;
		
		void Init(int roleid_, char country_id_, int win_cnt_, int dead_cnt_, int total_combat_time_, int total_contribute_val_, float score_);
	};

	struct OccupationFactor
	{
		float win_fac;
		float fail_fac;
		float attend_time_fac;
		float kill_cnt_fac;
		float death_cnt_fac;
		float combat_time_fac;
	};
	
	struct BonusLimit
	{
		float score_limit;
		int win_cnt_limit;
		int death_cnt_limit;
		int combat_time_limit;
		int contribute_val_limit;
		int total_bonus;
	};
	
	struct PlayerBonus
	{
		int roleid;
		int bonus;
		int zoneid;
	};
	
	struct PlayerRankInfo
	{
		int roleid;
		int cls;
		int dead_cnt;
		int combat_time;
		int contribute_val;
		int rank_val;
		int rank_point;
	};
	
	typedef std::map<int/*domainid*/, DomainInfo> DOMAIN_MAP;
	typedef std::map<int/*roleid*/, PlayerEntry> PLAYER_ENTRY_MAP;
	typedef std::map<int/*roleid*/, PlayerScoreInfo> PLAYER_SCORE_MAP;
	typedef std::vector<MoveInfo> MOVE_LIST;
	typedef std::vector<ColdInfo> PLAYER_WAIT_FIGHT_LIST;
	typedef std::vector<RegisterInfo> SERVER_LIST;
	typedef std::vector<OccupationFactor> OCCUPATION_FAC_LIST;
	typedef std::vector<PlayerBonus> PLAYER_BONUS_LIST;
	typedef std::map<int/*zoneid*/, int/*countryid*/> ZONE_COUNTRY_MAP;
	
private:
	int _arrange_country_type;
	int _status; //��ս״̬
	int _capital_worldtag; //��սս�Գ�����worldtag
	int _calc_domains_timer; //�������Ӫռ�ݶ��������ļ�����
	int _country_id_ctrl;
	int	_adjust_time; //�����ã���ǰʱ��ĵ���ֵ
	unsigned int _db_send_bonus_per_sec;
	BonusLimit _bonus_limit; //��ý��������Ľṹ
	SERVER_LIST _servers; //��¼�˸���ս������ͨ�ŵĽṹ
	DOMAIN_MAP _domain_map; //����������map
	PLAYER_ENTRY_MAP _player_map; //�����ս�������Ϣ��map
	PLAYER_SCORE_MAP _player_score_map; //��¼��ҹ�ս�÷ֵ�map
	MOVE_LIST _move_info; //�����ƶ��������Ϣ�б�
	PLAYER_WAIT_FIGHT_LIST _players_wait_fight; //���ڵȴ�ս��״̬������б�
	OCCUPATION_FAC_LIST _occupation_fac_list; //��ս����÷ֵ�ְҵϵ���б�
	PLAYER_BONUS_LIST _player_bonus_list;
	CapitalInfo _capital_info[COUNTRY_MAX_CNT]; //�׶���Ϣ�������ڲ���Ϣ����
	CountryInfo _country_info[COUNTRY_MAX_CNT]; //��Ӫս�����б�
	unsigned char _open_days[WEEK_DAY_CNT];
	ZONE_COUNTRY_MAP  _zone_country_map;

	static CountryBattleMan _instance; 
	
private:
	CountryBattleMan(): _arrange_country_type(ARRANGE_COUNTRY_RANDOM), _status(ST_CLOSE), _capital_worldtag(0), _calc_domains_timer(0), _country_id_ctrl(0), _adjust_time(0), _db_send_bonus_per_sec(0)
	{
		memset(_capital_info, 0, sizeof(_capital_info));
		memset(_open_days, 0, sizeof(_open_days));
		ClearCountryInfo();
	}
	
	void Clear()
	{
		_calc_domains_timer = 0;
		_db_send_bonus_per_sec = 0;
		_player_score_map.clear();
		_player_bonus_list.clear();
		
		ClearCountryInfo();
		ResetDomainInfo();
	}
	
	/**
	 * ����domain����
	 * @param domain2_type ����type�����������������ͨ�����ǿ���������
	 * @return true ����ɹ� false ����ʧ��
	 */
	bool InitDomainInfo(char domain2_type);

	/**
	 * ��GS������Ҫս�����ݻ����delivery��Ҫ��ֵ
	 * @param major_str GS�������ϣ���ҵ�װ����ֵ��0����ɶ��ľ�У�1������16Ʒ������2������9������
	 * @return ��Ҫս����delivery�ϵ���ֵ��ʾ
	 */
	float ConvertMajorStrength(int major_str);
	
	/**
	 * ��Ұ����������ҵķ�ʽ�����ս
	 * @param apply_list ��������ս������б�
	 * @return ��ӪID
	 */
	int ApplyBattleRandom( std::vector<CountryBattleApplyEntry>& apply_list );
	
	/**
	 * ������Ӫ
	 * @param has_major_str ����Ƿ�ӵ����Ҫս����
	 * @return ��ӪID
	 */
	int ArrangeCountry(bool has_major_str);
	void ArrangeCountryByZoneID();

	/**
	 * ��Ҽ����ս
	 * @param roleid ���ID
	 * @param country_id ��ӪID
	 * @param world_tag ��ͼtag
	 * @param minor_str ��ҵĻ���ֵ
	 * @return true ����ɹ� false ����ʧ��
	 */
	bool PlayerJoinBattle(int roleid, int country_id, int world_tag, int minor_str);
	
	/**
	 * ����뿪��ս
	 * @param roleid ���ID
	 * @param country_id ��ӪID
	 * @return true �뿪�ɹ� false �뿪ʧ��
	 */
	bool PlayerLeaveBattle(int roleid, int country_id);

	/**
	 * �����ս�Ե�ͼ���ƶ�
	 * @param roleid ���ID
	 * @param from ��ʼdomain ID
	 * @param to Ŀ��domain ID
	 * @param time �ƶ���Ҫ��ʱ��
	 */
	void PlayerMove(int roleid, int from, int to, int time);
	
	/**
	 * �����ս�Ե�ͼ���ƶ�
	 * @param domain ������Ϣ
	 * @param challenger ��ս��ӪID
	 */
	void StartBattle(DomainInfo& domain, int challenger);

	void SysHandlePlayer(int player_id, int reason);
	
	/**
	 * ȡ�����ڵ�ͼ���ƶ�����Ҫ���ѵ�ʱ��
	 * @param domain_src �ƶ���Դ��ͼ
	 * @param domain_dest �ƶ���Ŀ���ͼ
	 * @return �ƶ���Ҫ���ѵ�ʱ�䣬������ֵС�ڵ���0��˵��������
	 */
	int GetMoveUseTime(int domain_src, int domain_dest);
	int GetServerIdByDomainId(int domain_id);
	int GetServerIdByWorldTag(int world_tag);
	int GetWorldTagByDomainId(int domain_id);
	void MoveBetweenDomain(int roleid, int domain_src, int domain_dest);	
	int CalcPlayerBonus(const PlayerScoreInfo& info, int country_bonus[COUNTRY_MAX_CNT]);
	int CalcKingBonus(int country_bonus);
	void CalcBonus();
	void SendBonus();
	void UpdateMoveInfo();
	void UpdateWaitFightPlayers();
	void UpdateWaitFightDomains(DomainInfo& info);
	void UpdateFightDomains(DomainInfo& info);
	void UpdateColdDomains(DomainInfo& info);
	void UpdateDomains();
	void ClearMoveInfo(int roleid);
	void ClearPlayerWaitInfo(int roleid);	
	void ClearPlayerInfo(int roleid);
	void ClearCountryInfo();
	void ResetDomainInfo();
	bool IsPlayerMoving(int roleid);
	bool SyncPlayerLocation(int roleid, int domain_id, int reason, unsigned int linksid, unsigned int localsid);
	bool SyncPlayerPosToGs(int roleid, int worldtag, float posx, float posy, float posz, char is_capital);
	void SendBattleResult(int player_bonus, int country_bonus[COUNTRY_MAX_CNT], unsigned int linksid, unsigned int localsid);
	void DBSendBattleBonus(int roleid, int player_bonus, int zoneid);
	void NotifyBattleConfig(int server_id);
	//float CalcPlayerScore(int domain_point, int total_combat_time, int battle_last_time, 
	//	const GCountryBattlePersonalScore& score, bool is_winner, float winner_average_score);
	void CalcBattleScore(int battleid, int domain_point, int battle_last_time, 
		const std::vector<GCountryBattlePersonalScore>& winner_score, const std::vector<GCountryBattlePersonalScore>& loser_score, char winner_battle_mask, char loser_battle_mask);
	void PlayerChangeState(PlayerEntry& player, char new_status, int reason);
	void DomainChangeState(DomainInfo& domain, char new_status, int reason);
	void MakeCapitalsData(GCountryCapitalVector& capital_list);
	void AddPlayerCountryCommuicationInfo(int roleid, int country_id, unsigned int linksid, unsigned int localsid);
	void RemovePlayerCountryCommuicationInfo(int roleid, int country_id, unsigned int linksid);
	void BroadcastCountryBattleMsg(int src, int self_country_id, int target_country_id, int domain_id);
	void StopMovingPlayers();
	void HandlePlayerUnusualSwitchMap(PlayerEntry& player);	
	int CalcSingleBattleTotalScore(int domain_point, int battle_last_time, const std::vector<GCountryBattlePersonalScore>& enemy_score);
	void CalcPlayerScore(int battleid, int total_score, const std::vector<GCountryBattlePersonalScore>& scores, bool is_winner, char battle_config_mask);
	
	bool IsPlayerKing(const PlayerEntry& player);
	bool IsPlayerBeyondBattleLimit(const PlayerEntry& player, const DomainInfo& domain, int occupation_wait_fight_cnt);
	void BroadcastCountryBattleMsg2(int src, int country_id, const Marshal::OctetsStream& os);
	void ClearBattleLimit(int country_id, CountryKing& king, DomainInfo& domain);
	void SetBattleLimit(int country_id, CountryKing& king, DomainInfo& domain, const std::vector<GCountryBattleLimit>& battle_limit);
	void ClearBattleConfig(DomainInfo& domain);
	void OutputZoneCountryScoreLog();
	int GetBattleCountryCnt(std::set<int>& country_set);

public:
	static CountryBattleMan* GetInstance() { return &_instance; }
	
	/**
	 * ��սϵͳ��ʼ��
	 * @return true ��ʼ���ɹ� false ��ʼ��ʧ��
	 */
	bool Initialize(bool arrange_country_by_zoneid);

	/**
	 * ��սϵͳ�Ƿ��Ѿ�����
	 * @return true ���� false δ����
	 */
	bool IsBattleStart() const { return _status == ST_OPEN; }

	/**
	 * ȡ�ù�սս�Գ�����worldtag
	 * @return ��սս�Գ�����worldtag
	 */
	int GetCapitalWorldTag() const { return _capital_worldtag; }
	
	/**
	 * ��Ҽ����ս
	 * @param roleid ���ID
	 * @param country_id ��ӪID
	 * @param world_tag �������������world_tag
	 * @param major_strength �����Ҫս��
	 * @param minor_strength ��Ҵ�Ҫս��
	 * @param is_king �Ƿ����
	 */
	void JoinBattle(int roleid, int country_id, int world_tag, int major_strength, int minor_strength, char is_king);

	/**
	 * ����뿪��ս
	 * @param roleid ���ID
	 * @param country_id ��ӪID
	 * @param major_strength �����Ҫս��
	 * @param minor_strength ��Ҵ�Ҫս��
	 */
	void LeaveBattle(int roleid, int country_id, int major_strength, int minor_strength);

	/**
	 * ��ҵ�½ʱ����սϵͳ�Ĵ���
	 * @param roleid ���ID
	 * @param country_id ��ӪID
	 * @param world_tag �������������world_tag
	 * @param minor_str ��Ҵ�Ҫս��
	 * @param is_king �Ƿ����
	 */
	void OnPlayerLogin(int roleid, int country_id, int world_tag, int minor_str, char is_king);

	/**
	 * ��ҵǳ�ʱ����սϵͳ�Ĵ���
	 * @param roleid ���ID
	 * @param country_id ��ӪID
	 */
	void OnPlayerLogout(int roleid, int country_id);

	/**
	 * ����л���ͼ����ʱ����սϵͳ�Ĵ���
	 * @param roleid ���ID
	 * @param world_tag �������������world_tag
	 */
	void OnPlayerEnterMap(int roleid, int worldtag);

	/**
	 * ��ҽ��ܵ��ͻ����ƶ�����Ĵ���
	 * @param roleid ���ID
	 * @param dest �ƶ���Ŀ������
	 */
	int OnPlayerMove(int roleid, int dest);

	/**
	 * ��ҽ��ܵ��ͻ���ֹͣ�ƶ�����Ĵ���
	 * @param roleid ���ID
	 */
	int OnPlayerStopMove(int roleid);
	
	void OnPlayerReturnCapital(int roleid);

	void SendApplyResultToGs(int country_id, const std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);
	void PlayersApplyBattleByZoneID(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);
	void PlayersApplyBattleRandom(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);
	void OnPlayersApplyBattle(std::vector<CountryBattleApplyEntry>& apply_list, unsigned int sid);

	/**
	 * ս����������ʱ�Ĵ���
	 * @param battleid ����ս��������ID
	 * @param worldtag ����ս������������worldtag
	 * @param retcode ���������Ľ����0Ϊ�����ɹ�������Ϊʧ��
	 * @return true ս����ʼ false δ����ȷ��ʼս��
	 */
	bool OnBattleStart(int battleid, int worldtag, int retcode, int defender, int attacker);

	/**
	 * ս������ʱ�Ĵ���
	 * @param battleid ����ս��������ID
	 * @param result ս����� 1Ϊ����ʤ�� 2Ϊ�ط�ʤ��
	 * @param defender �ط���ӪID
	 * @param attacker ������ӪID
	 * @param defender_score �ط��÷�
	 * @param attacker_score �����÷�
	 * @return true ս���������� false ս��δ����ȷ����
	 */
	bool OnBattleEnd(int battleid, int result, int defender, int attacker, 
		const std::vector<GCountryBattlePersonalScore>& defender_score, const std::vector<GCountryBattlePersonalScore>& attacker_score);
	bool OnPlayerPreEnter(int battle_id, int roleid);
	bool SendMap(int roleid, unsigned int sid, unsigned int localsid);
	bool SendCountryScore(int roleid, unsigned int sid, unsigned int localsid);
	bool RegisterServer(int server_type, int war_type, int server_id, int worldtag);
	bool Update();
	int GetPlayerDomainId(int roleid);
	
	time_t GetCountryBattleStartTime() 
	{ 
		//���ڿͻ�����ʾ��ս����ʱ��
		int hour = 20;
		int minute = 20;
		return (hour * 3600 + minute * 60);
	}
	
	time_t GetCountryBattleEndTime() 
	{ 
		//���ڿͻ�����ʾ��սս������ʱ��
		int hour = 22;
		int minute = 20;
		return (hour * 3600 + minute * 60);
	}

	time_t GetCountryBattleFinishTime() 
	{ 
		time_t now = Timer::GetTime();

		struct tm dt;
		localtime_r(&now, &dt);
		//��Ϊ22��20��սս��������23:30�����������������
		//���ƹ�սID����ʱ����22:30:00��23:30:00��һСʱ֮��
		dt.tm_hour = 22;
		dt.tm_min = 30;
		dt.tm_sec = 0;
		int base = mktime(&dt);
		int offset = rand() % 3600; //���һ��һСʱ(3600��)֮�������
		return (base + offset);
	}

	int GetZoneIDByCountryID(int cid)
	{
		for(ZONE_COUNTRY_MAP::iterator iter = _zone_country_map.begin();
				iter != _zone_country_map.end(); ++ iter)
		{
			if(iter->second == cid)
				return iter->first;
		}
		return 0;		
	}

	int GetTotalBonus() { return _bonus_limit.total_bonus; }
	char GetDomain2DataType();
	void SetCountryIDCtrl(int id) { _country_id_ctrl = id; }
	int GetTime();
	void SetAdjustTime(int t);

	const std::map<int/*linksid*/, PlayerVector>* GetCountryOnlinePlayers(int roleid);
	void KingAssignAssault(int king_roleid, int domain_id, char assault_type);
	void KingResetBattleLimit(int king_roleid, int domain_id, char op, const std::vector<GCountryBattleLimit>& limit);
	void SendBattleLimit(int roleid, int domain_id);
	void SendKingCmdPoint(int roleid);
};

}

#endif
