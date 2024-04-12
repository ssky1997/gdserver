#ifndef __GNET_SNSMANAGER_H__
#define __GNET_SNSMANAGER_H__

#include <map>

namespace GNET
{

enum SNS_DTYPE
{
	SNS_DTYPE_ROLEBRIEF = 0,
	SNS_DTYPE_ROLESKILLS,
	SNS_DTYPE_ROLEEQUIPMENT,
	SNS_DTYPE_ROLEPETCORRAL,
	SNS_DTYPE_FACTIONEXT = 10,
	SNS_DTYPE_CITY	= 11,
};

enum SNS_ERR
{
	SNS_ERR_SUCCESS = 0,
	SNS_ERR_DATA_NOT_EXIST,
	SNS_ERR_DTYPE_INVALID,
};


class SNSRoleBrief;
class SNSRoleSkills;
class SNSRoleEquipment;
class SNSRolePetCorral;
class SNSManager
{
	struct SNSRoleData
	{
		int timestamp;
		Octets skills;
		Octets equipment;
		Octets petcorral;
		SNSRoleData(int _timestamp=0, const Octets& _skills=Octets(), const Octets& _equipment=Octets(), const Octets& _petcorral=Octets())
			:timestamp(_timestamp),skills(_skills),equipment(_equipment),petcorral(_petcorral){}
	};
	typedef std::map<int/*roleid*/, SNSRoleData> RoleDataMap;
	enum
	{
		CLEARUP_INTERVAL = 300,	
	};
	//һ��ƽ������5200(���5900)�˵ķ�����,һ���ڽ�ɫ���ߴ���30256,map��ÿ�뽫����0.35��SNSRoleData
	//map������ۻ�2*INTERVAL�������,ԼΪ210��
	//��������SNSRoleDataһ����5KB����,����mapռ���ڴ�1050KB
	//��һЩ�����������������������н�ɫͬʱ���ߣ���mapռ���ڴ�ɴﵽ29.5MB
	SNSManager():lock("SNSManager::lock"),last_clearup_time(0){}

public:
	~SNSManager(){}
	static SNSManager* GetInstance(){ static SNSManager instance; return &instance; }

public:
	void ForwardRoleBrief(int roleid, SNSRoleBrief& brief, SNSRoleSkills& skills, SNSRoleEquipment& equipment, SNSRolePetCorral& petcorral);
	void FetchRoleData(int roleid, int dtype);		
	void FetchFactionExt(int factionid,int dtype);
	void FetchCityInfo(int roleid,int dtype);

private:
	Thread::Mutex 	lock;
	RoleDataMap 	role_data_map;
	int 			last_clearup_time;
};

}
#endif
