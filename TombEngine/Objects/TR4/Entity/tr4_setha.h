#pragma once

struct PoseData;

namespace TEN::Entities::TR4
{
	void InitialiseSetha(short itemNumber);
	void SethaControl(short itemNumber);
	void TriggerSethaSparks1(int x, int y, int z, short xv, short yv, short zv);
	void TriggerSethaSparks2(short itemNumber, char node, int size);
	void SethaThrowAttack(PoseData* pose, short roomNumber, short mesh);
	void SethaAttack(int itemNumber);
}
