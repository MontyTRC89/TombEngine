#pragma once
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	struct BeetleData
	{
		PHD_3DPOS Pose;
		short RoomNumber;
		int Velocity;
		int VerticalVelocity;
		bool On;

		byte Flags;
	};

	constexpr auto NUM_BEETLES = 256;

	extern BeetleData BeetleSwarm[NUM_BEETLES];
	extern int NextBeetle;

	void InitialiseBeetleSwarm(short itemNumber);
	void BeetleSwarmControl(short itemNumber);
	short GetFreeBeetle();
	void ClearBeetleSwarm();
	void UpdateBeetleSwarm();
}
