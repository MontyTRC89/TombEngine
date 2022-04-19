#pragma once

#include "framework.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4 
{
    enum STATES_CROCGOD
    {
        STATE_MUTANT_EMPTY,
        STATE_MUTANT_APPEAR,
        STATE_MUTANT_IDLE,
        STATE_MUTANT_SHOOT,
        STATE_MUTANT_LOCUST1,
        STATE_MUTANT_LOCUST2,
    };

    enum class MissileRotationType
    {
        M_FRONT,
        M_LEFT,
        M_RIGHT
    };

    constexpr auto MUTANT_ANIM_APPEAR = 0;
    constexpr auto MUTANT_SHOOT_RANGE = SQUARE(SECTOR(10));
    constexpr auto MUTANT_LOCUST1_RANGE = SQUARE(SECTOR(15));
    constexpr auto MUTANT_LOCUST2_RANGE = SQUARE(SECTOR(30));

	void InitialiseCrocgod(short itemNumber);
	void CrocgodControl(short itemNumber);
	void TriggerCrocgodMissileFlame(short fxNumber, short xVel, short yVel, short zVel);
}