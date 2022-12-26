#pragma once
#include "Game/effects/lightning.h"

namespace TEN::Entities::Creatures::TR5
{
	constexpr auto LASERHEAD_FIRE_ARCS_COUNT = 2;
	constexpr auto LASERHEAD_CHARGE_ARCS_COUNT = 4;

	struct LaserHeadInfo
	{
		Vector3i target;
		TEN::Effects::Lightning::LIGHTNING_INFO* fireArcs[LASERHEAD_FIRE_ARCS_COUNT]; // elptr
		TEN::Effects::Lightning::LIGHTNING_INFO* chargeArcs[LASERHEAD_CHARGE_ARCS_COUNT]; // blptr
		bool LOS[2];
		byte trackSpeed;
		byte trackLara;
		short xRot;
		short yRot;
		short BaseItem;
		short Tentacles[8];
		short PuzzleItem;
	};
}
