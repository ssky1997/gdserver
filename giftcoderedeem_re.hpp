
#ifndef __GNET_GIFTCODEREDEEM_RE_HPP
#define __GNET_GIFTCODEREDEEM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GiftCodeRedeem_Re : public GNET::Protocol
{
	#include "giftcoderedeem_re"
	enum
	{
		GCR_SUCCESS,			// �ɹ�
		GCR_UNACTIVE_CARD,		// δ����
		GCR_PRE_LIMIT,			// �������ظ�
		GCR_COMPLETE,			// �����
		GCR_NOT_OWNER,			// ��ӵ��
		GCR_INVALID_CARD,		// ��Ч����
		GCR_OUT_DATE,			// ����
		GCR_OTHER_USED,			// ������ɫ��ʹ��
		GCR_NET_FAIL,			// ���³���
		GCR_NOT_BIND,			// δ���˺�
		GCR_UNMARSHAL_FAIL,		// ���ݴ���
		GCR_INVALID_USER,		// ��Ч�˺�
		GCR_TYPE_LIMIT,			// �����ظ�
		GCR_INVALID_ROLE,		// ��Ч��ɫ
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
