
#ifndef __GNET_WEBPOSTCANCEL_HPP
#define __GNET_WEBPOSTCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webpostcancel_re.hpp"

#include "webtrademarket.h"
#include "gwebtradeclient.hpp"

namespace GNET
{

class WebPostCancel : public GNET::Protocol
{
	#include "webpostcancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webpostcancel: receive. roleid=%lld,sn=%lld,ctype=%d\n",roleid,sn,ctype);
/*
WebPostCancel_Re
0.�ɹ���
1.sn�����ڣ�ƽ̨��Ϊ�õ��Ӵ���ɾ���õ��ӣ������ط����� 
2.�ظ�ȡ�����ۣ�ƽ̨��Ϊ�ɹ��������ط����� 
3.���������ط�����
4.����Ϸ��������������ʧ�ܣ��ط����� 
5.��Ϸ�˸���sn�鵽��roleid�����ڣ������ǽ�ɫ�ѱ�GMɾ����ص���ԭ��gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
6.��Ʒ����Ϸ�ڴ��ڡ���ƽ̨ͬ���С���״̬������Ϸ����Ʒ���ڡ���ƽ̨ͬ���С���״̬ʱ�����ٴ���ƽ̨�������߼���ֱ�ӷ��ر������룬ֱ���յ�ƽ̨���ص�Re��ƽ̨�յ��ô���������κδ����ط�����
7����Ϸ����sn�鵽��userid��ƽָ̨����userid��һ�£�gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
8����Ϸ����sn�鵽��roleid��ƽָ̨����roleid��һ�£�gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
-1.��Ϸ����������ʧ�ܣ��ط�����
*/
		int retcode = WebTradeMarket::GetInstance().DoWebPostCancel(userid,roleid,sn,ctype,messageid,timestamp);
		if(retcode != ERR_SUCCESS)
		{
			WebPostCancel_Re re;
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
				case ERR_WT_TIMESTAMP_MISMATCH:	//�����ظ�ȡ�����۵����
					re.retcode = 0;
					break;
			}
			GWebTradeClient::GetInstance()->SendProtocol(re);
		}
	}
};

};

#endif
