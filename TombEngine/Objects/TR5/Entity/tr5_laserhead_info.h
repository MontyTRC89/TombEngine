#pragma once
#include "Game/effects/lightning.h"

using namespace TEN::Effects::Lightning;

namespace TEN::Entities::Creatures::TR5
{
	constexpr auto GUARDIAN_TENTACLE_COUNT	 = 8;
	constexpr auto GUARDIAN_FIRE_ARC_COUNT	 = 2;
	constexpr auto GUARDIAN_CHARGE_ARC_COUNT = 4;

	struct GuardianInfo
	{
		Vector3i target;
		std::array<LIGHTNING_INFO*, GUARDIAN_FIRE_ARC_COUNT> fireArcs = {}; // elptr
		std::array<LIGHTNING_INFO*, GUARDIAN_CHARGE_ARC_COUNT> chargeArcs = {}; // blptr
		bool LOS[2];
		byte trackSpeed;
		bool trackLara;
		short xRot;
		short yRot;
		int BaseItem;
		std::array<int, GUARDIAN_TENTACLE_COUNT> Tentacles = {};
		int PuzzleItem;
	};
}
