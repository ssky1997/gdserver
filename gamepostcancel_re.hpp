
#ifndef __GNET_GAMEPOSTCANCEL_RE_HPP
#define __GNET_GAMEPOSTCANCEL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GamePostCancel_Re : public GNET::Protocol
{
	#include "gamepostcancel_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("gamepostcancel_re: receive. retcode=%d roleid=%lld,sn=%lld\n",retcode,roleid,sn);
/*
gamepostcancel_re
0.�ɹ��� 
1.sn�����ڣ�gs��Ϊ�ɹ������ط����� 
2.�ظ�ȡ�����ۣ�gs��Ϊ�ɹ������ط�����
3.���������ط����� 
4. ��ƽ̨��������ʧ�ܣ��ط����� 
5.ƽ̨����sn�鵽��roleid����Ϸ������ָ����roleid���ȣ�gs��ƽ̨���Ըô���gs��Ϊ�ɹ������ط�����
6.��Ʒ��ǰ����ȡ�����ۣ�gs�յ�����Ʒ״̬��Ϊǰһ״̬��һ����ƽ̨���˺ü���������û֪ͨ����Ϸ����ʱ��ƽ̨����Ʒ״̬Ϊ׼����Ʒ��ƽ̨�˿������ڼ�/��ʾ/�۳��� �����ط����� 
7��ƽ̨����sn�鵽��userid����Ϸָ����userid��һ�£�gs��ƽ̨���Ըô���gs��Ϊ�ɹ������ط�����
-1. ƽ̨���ݿ�����ط�����
*/
		WebTradeMarket& market = WebTradeMarket::GetInstance();
		switch(retcode)
		{
			case 0:
			case 1:
			case 2:
			case 5:
			case 7:
				market.RecvCancelPostRe(true,userid,sn);
				break;
			case 6:
				market.RecvCancelPostRe(false,userid,sn);
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
