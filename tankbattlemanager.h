#ifndef __GNET_TANK_BATTLE_MANAGER_H__
#define __GNET_TANK_BATTLE_MANAGER_H__

#include <vector>
#include <map>
#include <functional>

#include "itimer.h"
#include "tankbattleplayerscoreinfo"

namespace GNET
{

namespace TankBattleConfig
{
const char split_tag = ';';		//���config���ַ����ı��
const bool only_one_battle_server = false;	//�Ƿ�ֻ������һ��ս��ս��������
const size_t assign_count_each_tick = 10;	//ÿ��tick��������ҷ���ս��,0�Ļ�����û������
const int player_switch_time_out = 120;		//�����ת��ͼ�ĳ�ʱʱ��
const int server_create_time_out = 30;		//֪ͨgs������ս���ĳ�ʱʱ��
const int server_error_time_out = 60;		//�������������ʱ��
const int battle_man_wait_bonus_time = 600;	//�ȴ����з���رյ�ʱ��
const int battle_man_wait_bonus_time_out = 120;	//�ȴ����з���رյĳ�ʱʱ��
const int battle_man_before_close_time = 3600;	//����ر�ǰ�Ľ���͵ȴ�����ʱ��
const int battle_close_clear_time = 60;	//ս�������೤ʱ��֮����ձ�ս���������Ϣ
const int battle_close_time_out = 60;	//ս������֮������ĳ�ʱʱ��
const int max_battle_count = 50;	//ÿ����������������ܿ���ս��������
const int rank_show_count = 10;		//�ͻ�����ʾ���а��ǰ������
}

class TankBattleManager : public IntervalTimer::Observer
{
private:

	enum SERVER_STAT
	{
		SERVER_STAT_NORMAL,		//����״̬�����Կ����µ�ս������
		SERVER_STAT_CREATE,		//���ڿ����µĸ���������
		SERVER_STAT_DISCONNECT,	//�������Ѿ����ߣ��Ȳ������µ�ս��
		SERVER_STAT_ERROR,		//��������������
		SERVER_STAT_FULL,		//�������Ѿ�������������
	};
	
	enum BATTLE_MAN_STAT
	{
		BATTLE_MAN_STAT_CLOSE,		//�û�п���
		BATTLE_MAN_STAT_OPEN,		//����ڿ���״̬
		BATTLE_MAN_STAT_WAIT_BONUS,	//��Ѿ��������ȴ�����ս���������֮�����
		BATTLE_MAN_STAT_BONUS,		//���з��䶼�Ѿ���������ʼ����ͷ���
		BATTLE_MAN_STAT_WAIT_CLOSE,	//�����Ѿ��������ȴ�����رս׶�
	};

	enum BATTLE_STAT
	{
		BATTLE_STAT_CREATE,		//���ڴ����Ĺ�����
		BATTLE_STAT_OPEN,		//���ڿ���״̬
		BATTLE_STAT_WAIT_CLOSE,	//ս�������ս���
		BATTLE_STAT_CLOSE,		//���ڹر�״̬
	};

	enum PLAYER_STAT
	{
		PLAYER_STAT_APPLY,		//����������ս�����ȴ�����
		PLAYER_STAT_SWITCH,		//���������ת������
		PLAYER_STAT_ENTER,		//����Ѿ�������ս��
		PLAYER_STAT_LEAVE,		//����뿪��ս��
	};
	
	struct ServerInfo
	{
		int world_tag;
		int server_id;
		int battle_count;   //��ǰ���˶��ٸ�ս��
		int time_stamp;		//server������һ��ʱ���
		SERVER_STAT stat;	//��ǰ�Ŀ���״̬

		ServerInfo()
		{
			world_tag = 0;
			server_id = 0;
			battle_count = 0;
			time_stamp = 0;
			stat = SERVER_STAT_NORMAL;
		}
	};
	
	struct BattleInfo
	{
		int battle_id;
		int world_tag;
		int end_time;		//ս��������ʱ���
		int time_stamp;		//״̬�仯��ʱ���
		BATTLE_STAT status;

		std::vector<int> players;

		BattleInfo()
		{
			battle_id = 0;
			world_tag = 0;
			end_time = 0;
			time_stamp = 0;
			status = BATTLE_STAT_CLOSE;
		}

		inline bool DelPlayer(int roleid)
		{
			std::vector<int>::iterator it = players.begin(),eit=players.end();
			for( ; it != eit; ++it)
			{
				if (*it == roleid)
				{
					players.erase(it);
					return true;
				}
			}
			return false;
		}
	};
	
	struct PlayerApplyEntry
	{
		int gameid;		//��ұ�����ʱ�����ڵ�gameid�������ͷ����ʱ�������֤�����������ͼ�˲��ý�
		int model;		//��ұ�����ʱ��ѡ���ģ��
	};
	
	struct PlayerEntry
	{
		int roleid;
		int battle_id;
		time_t time_stamp;
		PLAYER_STAT stat;

		PlayerEntry()
		{
			roleid = 0;
			battle_id = 0;
			time_stamp = 0;
			stat = PLAYER_STAT_APPLY;
		}
	};
	
	struct PlayerBonus
	{
		int roleid;
		int rank;
		int bonus;
	};

	struct BonusEntry
	{
		int rank;
		int count;
	};
	
	typedef std::map<int/*roleid*/, PlayerApplyEntry> PLAYER_APPLY_MAP;
	typedef std::map<int/*world_tag*/, ServerInfo> SERVER_INFO_MAP;
	typedef std::map<int/*battleid*/, BattleInfo> BATTLE_MAP;
	typedef std::map<int/*roleid*/, PlayerEntry> PLAYER_ENTRY_MAP;
	typedef std::map<int/*roleid*/, TankBattlePlayerScoreInfo> PLAYER_SCORE_MAP;
	typedef std::vector<PlayerBonus> PLAYER_BONUS_VEC;
	typedef std::multimap<int/*score*/, TankBattlePlayerScoreInfo * , std::greater<int> > PLAYER_SCORE_RANK_MAP;
	typedef std::vector<const TankBattlePlayerScoreInfo *> TOP_SCORE_VEC;
	typedef std::vector<BonusEntry> BONUS_ENTRY_VEC;

	TankBattleManager():_status(BATTLE_MAN_STAT_CLOSE),_battle_index(1),_start_time(0),_end_time(0),
			_adjust_time(0),_min_time(0),_max_time(0),_max_player(0),_no_new_battle_time(0),_cant_enter_time(0),
			_second_of_day(0),_send_bonus_per_second(0),_calc_bonus(false),_bonus_item_id(0),_bonus_max_count(0),_bonus_proctype(0)
	{
		memset(_open_days,0,sizeof(_open_days));
	}

public:

	static TankBattleManager * GetInstance()
	{
		static TankBattleManager instance;
		return &instance;
	}

	bool Initialize();

	//����ս����Ϣ
	void ResetBattleMgr();
	
	//���õ�ǰ����ʱ��
	inline void SetAdjustTime(int t) {_adjust_time = t;}

	//����ս���������,����ǰ��Ҫ��֤�����������Ч��
	inline void SetMaxPlayer(int n) {_max_player = n;}

	//����ս�����ʱ����ʱ��,ֻ�������Ժ��¿�����ս����Ч,����ǰ��Ҫ��֤�����������Ч��
	inline void SetMinAndMaxTime(int min, int max) {_min_time=min;_max_time=max;}
	
	//ս������gs����ע����Ϣ
	void RegisterServerInfo(int world_tag, int server_id);

	//ս������������Ϣ��gs���븱��
	void StartBattle();

	//ս������֮��gs���ͷ��صĻص�
	void OnBattleStart(int world_tag, int battleid, int retcode);

	//gs֪ͨս������
	void OnBattleEnd(int world_tag, int battleid);

	//gs�Ͽ����ӣ��ں��������Ƿ�ս��
	void DisableServerInfo(int world_tag);

	//��ұ����μ�ս��
	void PlayerApply(int roleid, int model);

	//��ҳɹ�����ս��
	void PlayerEnterBattle(int roleid, int battle_id, int world_tag);

	//����뿪ս������������
	void PlayerLeaveBattle(int roleid, int battle_id, int world_tag);
	
	//������һ�����Ϣ
	void PlayerScoreUpdate(int battle_id, int world_tag, const TankBattlePlayerScoreInfoVector & player_scores);

	//��Ҳ�ѯ���а�
	int PlayerGetRank(int roleid, TankBattlePlayerScoreInfo & self_score, 
			TankBattlePlayerScoreInfoVector & player_scores) const;

private:

	bool Update();

	//���ͻ��˷�������������ս���Ľ�����ں�����߸���roleid��������ڲ�����
	void SendPlayerApplyRe(int roleid, int retcode, unsigned int linksid = 0, unsigned int localsid = 0);

	//����ս��������״̬��Ϣ
	void UpdateServerInfo(time_t nowtime);

	//����ս��������Ϣ
	void UpdateBattleInfo(time_t nowtime);

	//���������Ϣ
	void UpdatePlayerInfo(time_t nowtime);

	//�����������(�������ȫ�����¼��㣬�����ٵ���)
	void CalcPlayerRank();

	//����������ҽ���
	void CalcAllBonus();

	//���㵥����ҽ���
	int CalcPlayerBonus(int rank);

	//���������б����ҷ���
	int SendAllBonus();

	//��������ҷ��ͽ���
	void SendPlayerBonus(int roleid, int player_bonus, int rank);

	//ѡȡһ�����������п���λ�õ�ս��
	BattleInfo * GetMostPlayerAndHaveVacancyBattle();

	//����battle_id��ȡbattle_info
	BattleInfo * GetBattleByBattleID(int battle_id);

	//����world_tag��ȡserver_info
	ServerInfo * GetServerInfoByWorldTag(int world_tag);

	//����roleid��ȡplayer_entry
	PlayerEntry * GetPlayerEntryByRoleID(int roleid);

	//��ȡ��ǰʱ��
	int GetTime() const;

	//��ȡ����battle_id
	inline int GetFreeBattleID() { return _battle_index++;}

private:

	BATTLE_MAN_STAT _status;	//��ǰ״̬
	int _battle_index;	//��ǰս������,����ֵ
	int _start_time;	//����쿪ʼʱ�䣬��0���ʱ������
	int _end_time;		//��������ʱ�䣬��0���ʱ������
	int _adjust_time;	//ʱ�������ֵ����������(��)
	int _min_time;		//һ��ս�������ʱ��(��)
	int _max_time;		//һ��ս�������ʱ��(��)
	int _max_player;	//ս�����������
	int _no_new_battle_time;	//���������೤ʱ�䲻������ս��(��)
	int _cant_enter_time;	//����ÿ��ս�����������೤ʱ�䲻��������ս��(��)
	int _second_of_day;		//��ǰʱ���Ǵ�0�㿪ʼ�Ķ�����,ÿ��Update����
	int _send_bonus_per_second;	//ÿ�뷢�Ž���������
	bool _calc_bonus;		//�Ƿ��Ѿ����������
	unsigned char _open_days[7];	//�ÿ���ܼ�����(0 ���첻���� 1���쿪��)
	int _bonus_item_id;		//�������Ʒ��id
	int _bonus_max_count;	//������Ʒ�ĵ�����������
	int _bonus_proctype;	//��Ʒ�İ�����

	BATTLE_MAP _battle_map;
	SERVER_INFO_MAP _server_info_map;
	PLAYER_APPLY_MAP _player_apply_map;
	PLAYER_ENTRY_MAP _player_entry_map;
	PLAYER_SCORE_MAP _player_score_map;
	PLAYER_BONUS_VEC _player_bonus_vec;
	BONUS_ENTRY_VEC _bonus_entry_vec;
	TOP_SCORE_VEC _top_score_vec;
};

}
#endif
