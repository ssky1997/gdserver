#ifndef __GNET_DISCOUNTMAN_H
#define __GNET_DISCOUNTMAN_H

#include "qpdiscountinfo"
#include "merchantdiscount"
#include <map>

namespace GNET
{

// ���֧�����ۿ۹���
class DiscountMan
{
public:

	// ���¿��֧�����ۿ���Ϣ
	void UpdateDiscount(const MerchantDiscountVector& discount);

	// ֪ͨ��������ۿ���Ϣ
	void NotifyDiscount(int linksid, int localsid);

	// ����ۿ��Ƿ���ȷ����ȷ�򷵻�true
	bool CheckDiscount(int cash, int cash_after_discount, int merchant_id);
	
	static DiscountMan* GetInstance() { return &_instance; }
private:
	static DiscountMan _instance; 
	QPDiscountInfoVector discount_;
};

} // end namespace GNET

#endif // __GNET_DISCOUNTMAN_H
