#include <time.h>

#include "timer.h"
#include "gproviderserver.hpp"
#include "conv_charset.h"

#include "friendextinfomanager.h"
#include "mapuser.h"
#include "rpcdefs.h"
#include "dbfriendextlist.hpp"
#include "friendextlist.hpp"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "sendaumail_re.hpp"
#include "game2au.hpp"
#include "aumailsended.hpp"
#include "gauthclient.hpp"
#include <algorithm>
#include "dbplayerrequitefriend.hrp"
#include "getmaillist_re.hpp"
//��cache�����ѯrole�ĵ�¼ʱ��
namespace GNET
{
		
	void FriendextinfoManager::SearchAllExt(PlayerInfo *pinfo)
	{
		int now = Timer::GetTime();
		std::vector<int> roleidlist;//��Ҫ��������ʱ��ĺ���id����ȥcache�������ݿ���ȡ
		if(!pinfo->friends.size())
			return;
		
		GFriendInfoVector::iterator gfriend_iter = pinfo->friends.begin();
		for(;gfriend_iter!=pinfo->friends.end();gfriend_iter++)
		{
			GFriendExtInfoVector::iterator gfriendext_iter = pinfo->friendextinfo.begin();
			for(;gfriendext_iter != pinfo->friendextinfo.end();gfriendext_iter++)
			{
				if(gfriend_iter->rid == gfriendext_iter->rid)
				{
					break;
				}
			}
			//��Һ��Ѷ�����Ϣ�б���û�иú�������
			if(gfriendext_iter == pinfo->friendextinfo.end())  
			{
				roleidlist.push_back(gfriend_iter->rid);
			}
			//�иú������ݵ��ǹ���
			else if(now - gfriendext_iter->update_time > _SECONDS_ONE_DAY)
			{
				roleidlist.push_back(gfriend_iter->rid);
			}
		}
		
		if(!roleidlist.size()) //���û����Ҫ��cache�������ݿ��в��ҵ��������
		{
			SendFriendExt2Client(pinfo);
		}
		else
		{
			//ȥcache��ȡ
			FindRoleLoginTime(roleidlist,pinfo);
		}
	}
	
	void FriendextinfoManager::FindRoleLoginTime(std::vector<int> &roleidlist,PlayerInfo *pinfo)
	{
		//roleidlist����Ҫ���µĺ�������ʱ���id�б�
		if(!roleidlist.size()) return;
		std::vector<int> miss_vec;
		std::map<int,ExtInfo> logintimelist;
		//��manager�Ļ����в�������ʱ��
		for(size_t i=0;i<roleidlist.size();i++)
		{
			RoleLoginTimeMap::iterator irolelogintime;
			irolelogintime = rolelogintimelist.find(roleidlist[i]);
			if(irolelogintime == rolelogintimelist.end())
			{
				//���Ҳ����ķ���miss��
				miss_vec.push_back(roleidlist[i]);
			}
			else
			{
				//���ҵ��ķ�����ұ�
				logintimelist[roleidlist[i]] = irolelogintime->second;
			}
		}
		//�����ҵ��ĺ��Ѹ��µ�pinfo��
		if(logintimelist.size())
		{
			std::map<int,ExtInfo>::iterator logintime_iter = logintimelist.begin();
			int now = Timer::GetTime();
			//�������ұ�
			for(;logintime_iter!=logintimelist.end();logintime_iter++)
			{
				GFriendExtInfoVector::iterator gfriendext_iter = pinfo->friendextinfo.begin();
				for(;gfriendext_iter!=pinfo->friendextinfo.end();gfriendext_iter++)
				{
					if(logintime_iter->first == gfriendext_iter->rid)
					{
						//�ҵ��ú��������
						UpdateGFriendExt(gfriendext_iter,logintime_iter->second.uid,logintime_iter->second.login_time,logintime_iter->second.level,now,logintime_iter->second.reincarnation_times);
						break;
					}
				}
				//û�ҵ������
				if(gfriendext_iter == pinfo->friendextinfo.end())
				{
					GFriendExtInfo tmp;
					tmp.rid = logintime_iter->first;
					tmp.last_logintime = logintime_iter->second.login_time;
					tmp.update_time = now;
					tmp.level = logintime_iter->second.level;
					tmp.uid = logintime_iter->second.uid;
					tmp.reincarnation_times = logintime_iter->second.reincarnation_times;
					pinfo->friendextinfo.push_back(tmp);
				}
			}
			pinfo->friend_ver ++;
		}

		if(!miss_vec.size())
		{
			//���͸��ͻ���
			SendFriendExt2Client(pinfo);
		}
		else
		{
			//�������û���ҵ�����ȥ���ݿ������
			DBFriendExtList dbpro;
			dbpro.rid = pinfo->roleid;
			dbpro.roleidlist.swap(miss_vec);
			GameDBClient::GetInstance()->SendProtocol(dbpro);
		}
		return ;
	}


	void FriendextinfoManager::UpdateRoleLoginTime(PlayerInfo * pinfo,const int logintime)
	{
		rolelogintimelist[pinfo->roleid].login_time = logintime;
		rolelogintimelist[pinfo->roleid].level = pinfo->level;
		rolelogintimelist[pinfo->roleid].uid = pinfo->userid;
		rolelogintimelist[pinfo->roleid].reincarnation_times = pinfo->reincarnation_times;
	}

	void FriendextinfoManager::UpdateRoleLoginTime(GFriendExtInfoVector friend_list,PlayerInfo *pinfo)
	{
		int now = Timer::GetTime();
		GFriendExtInfoVector::iterator gfeiv_iter = friend_list.begin();
		//����cache
		for(;gfeiv_iter != friend_list.end(); gfeiv_iter++)
		{
			rolelogintimelist[gfeiv_iter->rid].login_time = gfeiv_iter->last_logintime;
			rolelogintimelist[gfeiv_iter->rid].level = gfeiv_iter->level;
			rolelogintimelist[gfeiv_iter->rid].uid = gfeiv_iter->uid;
			rolelogintimelist[gfeiv_iter->rid].reincarnation_times = gfeiv_iter->reincarnation_times;
		}
		gfeiv_iter = friend_list.begin();
		//����pinfo
		for(;gfeiv_iter != friend_list.end(); gfeiv_iter++)
		{
			GFriendExtInfoVector::iterator gfriendext_iter = pinfo->friendextinfo.begin();
			for(;gfriendext_iter != pinfo->friendextinfo.end(); gfriendext_iter++)
			{
				if(gfriendext_iter->rid == gfeiv_iter->rid)
				{
					UpdateGFriendExt(gfriendext_iter,gfeiv_iter->uid,gfeiv_iter->last_logintime,gfeiv_iter->level,now,gfeiv_iter->reincarnation_times);
					break;
				}
			}
			if(gfriendext_iter == pinfo->friendextinfo.end())
			{
				GFriendExtInfo info = *gfeiv_iter;
				pinfo->friendextinfo.push_back(info);
			}
		}
		pinfo->friend_ver++;
		SendFriendExt2Client(pinfo);
	}
	
	class EventIsInSendAUMailInfo
	{
	public:
		EventIsInSendAUMailInfo(const int rid):s_rid(rid){}
		bool operator() (const std::vector<GFriendInfo>::value_type &gfriend)
		{
			return gfriend.rid == s_rid;
		}
	private:
		int s_rid;
	};

	void FriendextinfoManager::SendAUMail(PlayerInfo *pinfo,int friend_id,int mail_template_id)
	{
		if(!permission)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_NOTENABLE,friend_id);
			return;
		}
		time_t now = Timer::GetTime();
		struct tm* dt = localtime(&now);
		dt->tm_sec = 0;
		dt->tm_min = 0;
		dt->tm_hour = 0;
		int day_time = mktime(dt);
		GFriendExtInfoVector::iterator gfeiv_iter = pinfo->friendextinfo.begin();
		GSendAUMailRecordVector::iterator send_iter = pinfo->sendaumailinfo.begin();
		int count = 0;
		//ͨ��cache����һ����ҵ�friendextinfo����ֹ���ݹ��ϳ���bug
		UpdateLoginTimeFromCache(pinfo);
		if(pinfo->level < 90 && !pinfo->reincarnation_times)
		{
			return;
		}
		for(;gfeiv_iter != pinfo->friendextinfo.end();gfeiv_iter++)
		{
			//�жϸú����Ƿ����
			if(gfeiv_iter->rid == friend_id)
			{
				break;
			}
		}
		//û�иú���
		if(gfeiv_iter == pinfo->friendextinfo.end())
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_NOTFOUND,friend_id);
			return;
		}
		//���ѵȼ�����
		if(gfeiv_iter->level < AUMAIL_LEVEL_LIMIT && !gfeiv_iter->reincarnation_times)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_LEVELLOW,friend_id);	
			return;
		}
		//�뿪ʱ������
		if(((now-gfeiv_iter->last_logintime)/_SECONDS_ONE_DAY) < 20)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_NOTSOLONG,friend_id);
			return;
		}
		
		//�ж�CD
		for(;send_iter != pinfo->sendaumailinfo.end();send_iter++)
		{
			if(send_iter->rid == friend_id)
			{
				//10��CD
				if(now - send_iter->sendmail_time < _SECONDS_ONE_DAY *10)
				{
					SendMailResult2Client(pinfo,ERR_AUMAIL_TENCD,friend_id);
					return;
				}
			}
			//ͳ��һ���ڷ��͹�������,ÿ��0��Ϊ����ʱ��
			if(day_time < send_iter->sendmail_time)
			{
				count ++;
			}
		}
		//5��CD
		if(count >= 5)
		{
			SendMailResult2Client(pinfo,ERR_AUMAIL_FIVECD,friend_id);
			return;
		}

		//����sendaumailinfo
		if(pinfo->sendaumailinfo.size() > 50)
		{
			GSendAUMailRecordVector::iterator sendaumail_iter = pinfo->sendaumailinfo.begin();
			while(sendaumail_iter != pinfo->sendaumailinfo.end())
			{
				//ɾ����¼����������10��
				if(now - sendaumail_iter->sendmail_time > _SECONDS_ONE_DAY*10)
				{
					sendaumail_iter = pinfo->sendaumailinfo.erase(sendaumail_iter);
				}
				else
				{
					//ɾ����¼����������1�Ҳ��Ǻ��ѵ�
					GFriendInfoVector::iterator temp_it = std::find_if(pinfo->friends.begin(),pinfo->friends.end(),EventIsInSendAUMailInfo(sendaumail_iter->rid));
					if(temp_it == pinfo->friends.end())
					{
						if(day_time > sendaumail_iter->sendmail_time)
						{
							sendaumail_iter=pinfo->sendaumailinfo.erase(sendaumail_iter);
						}
						else
						{
							sendaumail_iter++;
						}
					}
					else
					{
						sendaumail_iter++;
					}
				}
			}
		}
		
		//֪ͨ�ͻ���
		SendMailResult2Client(pinfo,ERR_SUCCESS,friend_id);

		//��ӵ��ѷ����ʼ��б�
		GSendAUMailRecordVector::iterator gsaurv_iter = pinfo->sendaumailinfo.begin();
		for(;gsaurv_iter != pinfo->sendaumailinfo.end();gsaurv_iter++)
		{
			if(gsaurv_iter->rid == gfeiv_iter->rid)
			{
				gsaurv_iter->sendmail_time = now;
				break;
			}
		}
		if(gsaurv_iter == pinfo->sendaumailinfo.end())
		{
			GSendAUMailRecord gsaur;
			gsaur.rid = gfeiv_iter->rid;
			gsaur.sendmail_time = now;
			pinfo->sendaumailinfo.push_back(gsaur);
		}
		pinfo->friend_ver++;
		
		//ȡ�øú�����Ϣ
		GFriendInfoVector::iterator gfriend_iter = 	pinfo->friends.begin();
		for(;gfriend_iter!=pinfo->friends.end();gfriend_iter++)
		{
			if(gfriend_iter->rid == gfeiv_iter->rid)
			{
				break;
			}
		}
	
		//���͵�AU
		Game2AU g2a;
		g2a.userid = pinfo->userid;
		g2a.qtype = 1;
		Marshal::OctetsStream data;
		data << gfeiv_iter->uid << pinfo->name << gfriend_iter->name << mail_template_id;
		g2a.info = data;
		GAuthClient::GetInstance()->SendProtocol( g2a );
		
		int send_lvl = GetBonusLevel((now - gfeiv_iter->last_logintime)/_SECONDS_ONE_DAY);

		char ex_reward = (count == 4)?1:0;
		
		Log::formatlog("SendAUMail","send_userid=%d:send_role=%d:received_userid=%d:received_roleid=%d:level=%d:offline_seconds=%d:bonus_lvl=%d:ex_reward=%d",g2a.userid,pinfo->roleid,gfeiv_iter->uid,gfeiv_iter->rid,gfeiv_iter->level,gfeiv_iter->last_logintime,send_lvl,ex_reward);	

		//����Э�鵽gamed�Դ�������


		//���͵�gamed
		AUMailSended ams;
		ams.roleid = pinfo->roleid;
		ams.level = send_lvl;
		ams.ext_reward = ex_reward;
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->user->gameid,ams);
		//���������ʼ�����ɫ����
		DBPlayerRequiteFriend * rpc = (DBPlayerRequiteFriend*)Rpc::Call( RPC_DBPLAYERREQUITEFRIEND,
				DBPlayerRequiteFriendArg(pinfo->roleid,gfeiv_iter->rid,pinfo->name,PLAYERREQUITE_CALL));
		GameDBClient::GetInstance()->SendProtocol(rpc);

		return;
	}

	bool FriendextinfoManager::PreSendRequite(PlayerInfo *pinfo,int friend_id)
	{
		IntVector mail_list;
		PostOffice::GetInstance().FindMail(pinfo->roleid,mail_list,_MST_FRIENDCALLBACK,PLAYERREQUITE_CALL,-1);
		if(0 == mail_list.size()) return false;
		bool rst = PostOffice::GetInstance().CheckSpecialTitle(pinfo->roleid,mail_list,friend_id);
		if(!rst) return false;

		//���������ʼ�����ɫ����
		GRoleInventory item;
		item.id = AUMAIL_REQUITE_ITEM;
		item.count = 1;
		item.max_count = 9999;
		item.proctype = 16435;
		
		DBPlayerRequiteFriend * rpc = (DBPlayerRequiteFriend*)Rpc::Call( RPC_DBPLAYERREQUITEFRIEND,
				DBPlayerRequiteFriendArg(pinfo->roleid,friend_id,pinfo->name,PLAYERREQUITE_ANSWER,item,mail_list));
		GameDBClient::GetInstance()->SendProtocol(rpc);

		Log::formatlog("PreSendRequite","send_role=%d:received_roleid=%d",pinfo->roleid,friend_id);	
		return true;
	}
	void FriendextinfoManager::PostSendRequite(PlayerInfo *pinfo,int friend_id,const IntVector& maillist)
	{
		//ɾ��ǧ�ﴫ�������ʼ����¿ͻ���
		PostOffice::GetInstance().DeleteMail(pinfo->roleid,maillist);
		GetMailList_Re gml_re(0,pinfo->roleid,pinfo->localsid);
		PostOffice::GetInstance().GetMailList(pinfo->roleid,gml_re.maillist);
		GDeliveryServer::GetInstance()->Send( pinfo->linksid,gml_re );
	
		//���͵�gamed ���������ڶ�����
		AUMailSended ams;
		ams.roleid = pinfo->roleid;
		ams.level = 5;
		ams.ext_reward = 0;
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->user->gameid,ams);
		
		Log::formatlog("PostSendRequite","send_role=%d:received_roleid=%d",pinfo->roleid,friend_id);	
	}

	void FriendextinfoManager::SendMailResult2Client(const PlayerInfo *pinfo,const int result,const int friend_id)
	{
		SendAUMail_Re re;
		re.roleid = friend_id;
		re.result = result;
		re.localsid =pinfo->localsid;
		GDeliveryServer::GetInstance()->Send( pinfo->linksid,re); 
	}
	
	void FriendextinfoManager::SendFriendExt2Client(const PlayerInfo *pinfo)
	{
		FriendExtList pro;
		pro.extra_info = pinfo->friendextinfo;
		pro.roleid = pinfo->roleid;
		pro.send_info = pinfo->sendaumailinfo;
		pro.localsid = pinfo->localsid;
		GDeliveryServer::GetInstance()->Send( pinfo->linksid,pro);
	}

	int FriendextinfoManager::GetBonusLevel(const int day)
	{
		if(day <=30 && day >= 20)
		{
			return 1;
		}
		if(day <=45 && day > 30)
		{
			return 2;
		}
		if(day > 45)
		{
			return 3;	
		}
		return 0;
	}
	
	void FriendextinfoManager::UpdateGFriendExt(GFriendExtInfoVector::iterator iter,int uid,int login_time,int level,int now_time, int reincarnation_times)
	{
		iter->uid = uid;
		iter->level = level;
		iter->last_logintime = login_time;
		iter->update_time = now_time;
		iter->reincarnation_times = reincarnation_times;
		return;
	}
	
	void FriendextinfoManager::UpdateLoginTimeFromCache(PlayerInfo *pinfo)
	{
		bool sendExtList2Client = false;
		GFriendExtInfoVector::iterator gfeiv_iter = pinfo->friendextinfo.begin();
		int now = Timer::GetTime();
		for(;gfeiv_iter != pinfo->friendextinfo.end();gfeiv_iter++)
		{
			RoleLoginTimeMap::iterator irolelogintime;
			irolelogintime = rolelogintimelist.find(gfeiv_iter->rid);
			if(irolelogintime != rolelogintimelist.end())
			{
				if(gfeiv_iter->last_logintime != irolelogintime->second.login_time)
				{
					gfeiv_iter->last_logintime = irolelogintime->second.login_time;
					gfeiv_iter->update_time = now;
					sendExtList2Client = true;
				}
			}
		}
		if(sendExtList2Client)
		{
			SendFriendExt2Client(pinfo);
		}
	}

	void FriendextinfoManager::Initialize(bool recall)
	{
		permission = recall;
		return;
	}

}
