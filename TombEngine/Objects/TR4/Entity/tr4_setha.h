#pragma once

class Pose;
struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseSetha(short itemNumber);
	void SethaControl(short itemNumber);
	void SethaThrowAttack(Pose* pose, short roomNumber, int flags);
	void SethaKill(ItemInfo* sethItem, ItemInfo* laraItem);
	void SethaAttack(int itemNumber);
}
