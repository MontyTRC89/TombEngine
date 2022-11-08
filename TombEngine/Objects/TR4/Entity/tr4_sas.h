#pragma once

struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseSas(short itemNumber);
	void SasControl(short itemNumber);
	void InitialiseInjuredSas(short itemNumber);
	void InjuredSasControl(short itemNumber);
	void SasFireGrenade(ItemInfo* item, short angle1, short angle2);
}
