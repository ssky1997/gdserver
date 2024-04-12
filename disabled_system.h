
#ifndef __GNET_DISABLED_SYSTEM_H__
#define __GNET_DISABLED_SYSTEM_H__

#include <stddef.h>

namespace GNET
{

enum
{
	SYS_AUCTION = 0,	//����
	SYS_STOCK,			//Ԫ��ί�н���
	SYS_BATTLE,			//��ս
	SYS_SYSAUCTION,	//Ԫ������
	SYS_WEBTRADE,	//Ѱ����
	SYS_GAMETALK,	//GT
	SYS_SNS,		//SNS
	SYS_FACTIONFORTRESS,	//���ɻ���
	SYS_COUNTRYBATTLE,		//��ս
	SYS_REFERENCE,			//�����ƹ�
	SYS_REWARD,				//���ѷ���
	SYS_RECALLOLDPLAYER,	//�ٻ������
	SYS_KINGELECTION,		//����ѡ��
	SYS_PLAYERSHOP,			//����ϵͳ
	SYS_PLAYERPROFILE,		//��������
	SYS_AUTOTEAM,			//�Զ����
	SYS_UNIQUEDATAMAN,		//ȫ������
	SYS_TANKBATTLE,			//ս��ս��
	SYS_FACTIONRESOURCEBATTLE, //����PVP
    SYS_MAPPASSWORD,        // �ܱ���

	SYS_INDEX_MAX,
};	

class DisabledSystem
{
	bool _disabled_list[SYS_INDEX_MAX];
	
	DisabledSystem()
	{ 
		for(size_t i=0; i<SYS_INDEX_MAX; i++)
			_disabled_list[i] = false;
	}
	static DisabledSystem instance;
public:
	static bool GetDisabled(int index)
	{
		if(index >=0 && index < SYS_INDEX_MAX)
			return instance._disabled_list[index]; 
		else
			return true;
	}
	static void SetDisabled(int index)
	{
		if(index >=0 && index < SYS_INDEX_MAX)
			instance._disabled_list[index] = true; 
	}
};

}
#endif
