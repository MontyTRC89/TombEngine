#pragma once

#include "framework.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4 
{
    enum MutantState
    {
        MUTANT_STATE_NONE = 0,
        MUTANT_STATE_APPEAR = 1,
        MUTANT_STATE_IDLE = 2,
        MUTANT_STATE_SHOOT = 3,
        MUTANT_STATE_LOCUST_1 = 4,
        MUTANT_STATE_LOCUST_2 = 5,
    };

    // TODO
    enum MutantAnim
    {
        MUTANT_ANIM_APPEAR = 0
    };

    enum class MissileRotationType
    {
        Front,
        Left,
        Right
    };

    #define MUTANT_SHOOT_RANGE pow(SECTOR(10), 2)
    #define MUTANT_LOCUST_1_RANGE pow(SECTOR(15), 2)
    #define MUTANT_LOCUST_2_RANGE pow(SECTOR(30), 2)

	void InitialiseCrocgod(short itemNumber);
	void CrocgodControl(short itemNumber);
	void TriggerCrocgodMissileFlame(short fxNumber, short xVel, short yVel, short zVel);
}
