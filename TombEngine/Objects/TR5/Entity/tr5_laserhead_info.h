#pragma once
#include "Game/effects/lightning.h"

namespace TEN::Entities::Creatures::TR5
{
	constexpr auto GUARDIAN_TENTACLE_COUNT	 = 8;
	constexpr auto GUARDIAN_FIRE_ARC_COUNT	 = 2;
	constexpr auto GUARDIAN_CHARGE_ARC_COUNT = 4;

	struct GuardianInfo
	{
		Vector3i target;
		TEN::Effects::Lightning::LIGHTNING_INFO* fireArcs[GUARDIAN_FIRE_ARC_COUNT]; // elptr
		TEN::Effects::Lightning::LIGHTNING_INFO* chargeArcs[GUARDIAN_CHARGE_ARC_COUNT]; // blptr
		bool LOS[2];
		byte trackSpeed;
		byte trackLara;
		short xRot;
		short yRot;
		short BaseItem;
		short Tentacles[GUARDIAN_TENTACLE_COUNT];
		short PuzzleItem;
	};
}
