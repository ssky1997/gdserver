#ifndef __GNET_KINGELECTION_H__
#define __GNET_KINGELECTION_H__

#include "mapuser.h"
#include "gmailsyncdata"
#include "kecandidate"
#include "keking"
#include "kingelectiondetail"

#define KINGELECTION_UPDATE_INTERVAL	60000000
#define CANDIDATE_APPLY_MAX		1000	//���Ӵ�ֵ��Ҫ����Э��maxsize
#define CANDIDATE_MAX			3		//���Ӵ�ֵ��Ҫ����Э��maxsize
namespace GNET
{

class KingElection : public IntervalTimer::Observer
{

public:
	enum
	{
		ST_CLOSE = 0,
		ST_INIT,
		ST_OPEN,
		ST_CANDIDATE_APPLY,
		ST_WAIT_VOTE,
		ST_VOTE,
	};
	enum
	{
		CANDIDATE_APPLY_BEGIN_TIME 	= 3*86400 + 20*3600 + 30*60,	//����20:30��ʼ��ѡ������
		CANDIDATE_APPLY_END_TIME 	= 3*86400 + 21*3600,			//����21:00������ѡ������
		VOTE_BEGIN_TIME 			= 3*86400 + 22*3600,			//����22:00��ʼѡ��
		VOTE_END_TIME 				= 4*86400 + 22*3600,			//����22:00����ѡ��

		KING_END_TIME				= 3*86400 + 20*3600,			//����������������20:00
	};
	enum{ DELKING_REASON_EXPIRE = 0,	DELKING_REASON_ERROR = 1, };
	enum{ DELCANDIDATE_REASON_ERROR = 1, };


public:
	static KingElection & GetInstance(){ static KingElection instance; return instance; }
	bool Initialize();
	void OnDBConnect(Protocol::Manager * manager, int sid);
	void OnDBLoad(KingElectionDetail & detail);
	bool Update();
	
	inline bool IsOpen(){ return _status != ST_CLOSE && _status != ST_INIT; }
	const char * Status2Str(int status);
	time_t UpdateTime(); 
	void BroadCastMsg(int id, const Octets & msg = Octets());

	void GetStatus(int & status, KEKing & king, KECandidateVector & candidate_list);
	int TryCandidateApply(int roleid, int item_id, int item_num, const PlayerInfo & pinfo, const GMailSyncData & sync);
	int TryVote(int roleid, int item_id, int item_pos, int item_num, int candidate_roleid, const PlayerInfo & pinfo, const GMailSyncData & sync);

	bool AddCandidate(int roleid, int serial_num, int deposit);
	void DecCandidateApplyHalf(); 
	bool AddVote(int candidate_roleid);
	void OnLogin(const PlayerInfo & pinfo);

private:
	void __DeleteKing(int reason);
	void __DeleteCandidate(int reason);
	void __GenerateCandidate();
	void __GenerateKing();

private:
	KingElection() : _status(ST_CLOSE),_week_begin(0),_max_serial_num(0),_candidate_apply_half(0){}

	int 				_status;
	int					_week_begin;
	int					_max_serial_num;
	
	int					_candidate_apply_half;
	KEKing 				_king;
	KECandidateVector 	_candidate_list;

	
};

}

#endif
