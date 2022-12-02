#pragma once

class Pose;
struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseSetha(short itemNumber);
	void SethaControl(short itemNumber);
	void TriggerSethaSparks1(int x, int y, int z, short xv, short yv, short zv);
	void TriggerSethaSparks2(short itemNumber, char node, int size);
	void SethaThrowAttack(Pose* pose, short roomNumber, short mesh);
	void SethaKill(ItemInfo* sethItem, ItemInfo* laraItem);
	void SethaAttack(int itemNumber);
}
