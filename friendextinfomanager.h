#ifndef __GNET_FRIENDEXTINFOMANAGER_H
#define __GNET_FRIENDEXTINFOMANAGER_H

#include <vector>
#include <map>
#include "mapuser.h"
#include "gfriendextinfo"

#define AUMAIL_LEVEL_LIMIT 90
#define AUMAIL_REQUITE_ITEM 47220
namespace GNET
{
	class FriendextinfoManager
	{
		typedef struct
		{
			int login_time;
			int level;
			int uid;
			int reincarnation_times;
		}ExtInfo;
		typedef std::map<int,ExtInfo> RoleLoginTimeMap;
		
		private:
			bool permission;
			RoleLoginTimeMap rolelogintimelist;
		
			//���ڸ���pinfo�е�FriendExt��Ϣ
			void UpdateGFriendExt(GFriendExtInfoVector::iterator iter,int uid,int login_time,int level,int now_time, int reincarnation_times);
			//���ͺ��Ѷ�����Ϣ���ͻ���
			void SendFriendExt2Client(const PlayerInfo *pinfo);
			//���������������㽱���ȼ�
			int  GetBonusLevel(const int day);
			//���������ʼ�������ͻ���
			void SendMailResult2Client(const PlayerInfo * pinfo,const int result,const int friend_id);	
			//��cache�����ѯrole�ĵ�¼ʱ��
			void FindRoleLoginTime(std::vector<int> &roleidlist,PlayerInfo *pinfo);
			//ֻ��cache�������pinfo�����ڷ���ʱ
			void UpdateLoginTimeFromCache(PlayerInfo *pinfo);
		
		public:
			FriendextinfoManager():permission(false)
			{
			}
			~FriendextinfoManager() { }

			static FriendextinfoManager* GetInstance()
			{
				static FriendextinfoManager _instance;
				return &_instance;
			}

			void Initialize(bool recall);
			//������ߵ�ʱ�����cache
			void UpdateRoleLoginTime(PlayerInfo *pinfo,const int logintime);
			//DB��������֮�����cache �� pinfo
			void UpdateRoleLoginTime(GFriendExtInfoVector friend_list,PlayerInfo *pinfo); 
			void SearchAllExt(PlayerInfo *pinfo);
			void SendAUMail(PlayerInfo *pinfo,int friend_id,int mail_template_id);
			bool PreSendRequite(PlayerInfo *pinfo,int friend_id);
			void PostSendRequite(PlayerInfo *pinfo,int friend_id,const IntVector& maillist);
	};
};

#endif
