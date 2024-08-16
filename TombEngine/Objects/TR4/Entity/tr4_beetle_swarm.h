#pragma once

#include "Game/items.h"

namespace TEN::Entities::TR4
{
	struct BeetleData
	{
		Pose Pose;
		short RoomNumber;
		int Velocity;
		int VerticalVelocity;
		bool On;

		byte Flags;

		Matrix Transform;
	};

	constexpr auto NUM_BEETLES = 256;

	extern BeetleData BeetleSwarm[NUM_BEETLES];
	extern int NextBeetle;

	void InitializeBeetleSwarm(short itemNumber);
	void BeetleSwarmControl(short itemNumber);
	short GetFreeBeetle();
	void ClearBeetleSwarm();
	void UpdateBeetleSwarm();
}
