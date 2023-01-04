#pragma once
#include "Game/collision/collide_room.h"

class Pose;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseTwogun(short itemNumber);
	void TwogunControl(short itemNumber);
	void TriggerTwogunPlasma(const Vector3i& posr, const Pose& pos, float life);
	void FireTwogunWeapon(short itemNumber, short LeftRight, short plasma);
}

