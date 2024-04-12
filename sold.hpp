
#ifndef __GNET_SOLD_HPP
#define __GNET_SOLD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class Sold : public GNET::Protocol
{
	#include "sold"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("sold: receive. zoneid=%d,roleid=%lld,sn=%lld,buyerroleid=%d,orderid=%d\n",zoneid,sellerroleid,sn,buyerroleid,orderid);
/*
Sold_Re
0.�ɹ��� 
1.sn�����ڣ�ƽ̨�˿��
2.��Ʒ��������ƽ̨��Ϊ�ɹ��������ط����� 
3.���������ط���
4.�Ҳ�����ң�ƽ̨�˿�� 
5. ����Ϸ��������������ʧ�ܣ��ط����� 
6. ��Ϸ�˸���sn�鵽��sellerroleid�����ڣ������ǽ�ɫ��ɾ�����ǻص��ڼ佨���Ľ�ɫ��gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
7.��Ʒ����Ϸ�ڴ��ڡ���ƽ̨ͬ���С���״̬ ������Ϸ����Ʒ���ڡ���ƽ̨ͬ���С���״̬ʱ�����ٴ���ƽ̨�������߼���ֱ�ӷ��ر������룬ֱ���յ�ƽ̨���ص�Re��ƽ̨�յ��ô���������κδ����ط�����
8����Ϸ����sn�鵽��selleruserid��ƽָ̨����selleruserid��һ�£���ʱ���Ըô���gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
9����Ϸ����sn�鵽��sellerroleid��ƽָ̨����sellerroleid��һ�£���ʱ���Ըô���gs��ƽ̨���Ըô���ƽ̨��Ϊ�ɹ������ط�����
10.Э���з��͵�buyerrolied��buyeruserid��ƥ�䣨�ܴ�����ǺϷ����������roleid�����仯��ƽ̨��Ϊ�ɹ������ط�����log����
11 ����������
12 �����ʺ����ɽ�ɫ����
13 ��ɫ����ָ������Ҳ�����ƽ̨�˿��������
14 ��ɫ�������Ľ�ɫ�Ҳ�����ƽ̨�˿��Ʒ�ϳ�
15 �����ϳ����������˿�
-1. ��Ϸ����������ʧ�ܣ��ط�����
*/
		int retcode = WebTradeMarket::GetInstance().DoSold(zoneid, selleruserid, sellerroleid, sn, buyeruserid, buyerroleid, orderid, stype, timestamp);
		if(retcode != ERR_SUCCESS)
		{
			Sold_Re re;
			re.zoneid = zoneid;
			re.selleruserid = selleruserid;
			re.sellerroleid = sellerroleid;
			re.buyeruserid = buyeruserid;
			re.buyerroleid = buyerroleid;
			re.sn = sn;
			re.orderid = orderid;
			switch(retcode)
			{
				default:
				case ERR_WT_UNOPEN:
				case ERR_WT_BUYER_STATUS_INAPPROPRIATE:
					re.retcode = -1;
					break;
				case ERR_WT_ENTRY_HAS_BEEN_SOLD:
					re.retcode = 2;
					break;
				case ERR_WT_ENTRY_NOT_FOUND:
					re.retcode = 1; 
					break;
				case ERR_WT_ENTRY_IS_BUSY:
					re.retcode = 7; 
					break;
				case ERR_WT_TIMESTAMP_MISMATCH:
					re.retcode = 0;
					break;
				case ERR_WT_BUYER_NOT_EXIST:
					re.retcode = 4;
					break;
			}
			GWebTradeClient::GetInstance()->SendProtocol(re);
		}
	}
};

};

#endif
