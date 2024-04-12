
#ifndef __GNET_POSTEXPIRE_HPP
#define __GNET_POSTEXPIRE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PostExpire : public GNET::Protocol
{
	#include "postexpire"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("postexpire: receive. roleid=%lld,sn=%lld\n",roleid,sn);
/*
PostExpire_Re
0.�ɹ��� 
1.sn�����ڣ�ƽ̨��Ϊ�õ��Ӵ���ɾ���õ��ӣ������ط����� 
2.�ظ�֪ͨ���۹��� ��ƽ̨��Ϊ�ɹ��������ط�����
3.���������ط����� 
4. ����Ϸ��������������ʧ�ܣ��ط����� 
5. ��Ϸ�˸���sn�鵽��roleid�����ڣ������ǽ�ɫ�ѱ�GMɾ����ص���ԭ��gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
6.��Ʒ����Ϸ�ڴ��ڡ���ƽ̨ͬ���С���״̬ ������Ϸ����Ʒ���ڡ���ƽ̨ͬ���С���״̬ʱ�����ٴ���ƽ̨�������߼���ֱ�ӷ��ر������룬ֱ���յ�ƽ̨���ص�Re��ƽ̨�յ��ô���������κδ����ط�����
7����Ϸ����sn�鵽��userid��ƽָ̨����userid��һ�£���ʱ���Ըô���gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
8����Ϸ����sn�鵽��roleid��ƽָ̨����roleid��һ�£���ʱ���Ըô���gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
-1. ��Ϸ����������ʧ�ܣ��ط�����
*/
		int retcode = WebTradeMarket::GetInstance().DoPostExpire(userid,roleid,sn,messageid,timestamp);
		if(retcode != ERR_SUCCESS)
		{
			PostExpire_Re re;
			re.userid = userid;
			re.roleid = roleid;
			re.sn = sn;
			re.messageid = messageid;
			switch(retcode)
			{
				default:
				case ERR_WT_UNOPEN:
					re.retcode = -1;
					break;
				case ERR_WT_ENTRY_NOT_FOUND:
					re.retcode = 1; 
					break;
				case ERR_WT_ENTRY_IS_BUSY:
					re.retcode = 6;
					break;
				case ERR_WT_TIMESTAMP_MISMATCH:		//�����ظ����۹��ڵ����
					re.retcode = 0;
					break;
			}
			GWebTradeClient::GetInstance()->SendProtocol(re);
		}
	}
};

};

#endif
