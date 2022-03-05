#pragma once
#include <items.h>

struct BeetleInfo
{
	PHD_3DPOS Position;
	short RoomNumber;
	int Velocity;
	int VerticalVelocity;
	bool On;

	byte Flags;
};

namespace TEN::Entities::TR4
{
	constexpr auto NUM_BEETLES = 256;

	extern BeetleInfo BeetleSwarm[NUM_BEETLES];
	extern int NextBeetle;

	void InitialiseBeetleSwarm(short itemNumber);
	void BeetleSwarmControl(short itemNumber);
	short GetFreeBeetle();
	void ClearBeetleSwarm();
	void UpdateBeetleSwarm();
}
