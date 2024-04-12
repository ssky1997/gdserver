
#ifndef __GNET_POST_RE_HPP
#define __GNET_POST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "timeinfo"

namespace GNET
{

class Post_Re : public GNET::Protocol
{
	#include "post_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("post_re: receive. retcode=%d roleid=%lld,sn=%lld showperiod=%d postperiod=%d commodity_id=%d\n",retcode,roleid,sn,time.showperiod,time.postperiod,commodity_id);
/*
post_re
0.�ɹ���
1.ͬһsn�ظ�����(gs��Ϊ�Ǽ��۳ɹ�)�� 
2.�ܱ��ȼ����������ܼ��ۣ�gs��Ϊ�Ǽ���ʧ�ܣ������ط����� 
3.���������ط����� 
4.��ƽ̨��������ʧ�ܣ��ط����� 
5.��Ʒ�����ڣ�ƽ̨����xml�鲻������Ʒ��gs�յ��ô�������Ϊ����ʧ�ܣ����ط�����
6.��Ϊ�ص���ɵ�sn��ͻ��gs��Ϊ����ʧ�ܣ����ط����� 
7.�ʺű��⣬��ֹ������Ʒ��gs��Ϊ����ʧ�ܣ����ط�����
8.�÷�������ʱ��ֹ���ۣ�gs��Ϊ����ʧ�ܣ����ط�����
9.�������Ƿ�����ֹ���ۣ�gs��Ϊ����ʧ�ܣ����ط�����
-1.ƽ̨���ݿ�����ط�����
*/
		WebTradeMarket& market = WebTradeMarket::GetInstance();		
		switch(retcode)
		{
			case 0:
			case 1:
				market.RecvPostRe(true,userid, sn, time.postperiod, time.showperiod, commodity_id);
				break;
			case 6:
				market.AdvanceSN();
			case 2:
			case 5:
			case 7://Ҳ����Ҫ������ʾ���ͻ���
			case 8:
			case 9:
				market.RecvPostRe(false,userid, sn,0,0,0);
				break;
			case 3:	
			case 4:
			case -1:
			default:
				break;
		}
	}
};

};

#endif
