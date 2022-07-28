#pragma once

#include "framework.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR4 
{
	void InitialiseCrocgod(short itemNumber);
	void CrocgodControl(short itemNumber);
	void TriggerCrocgodMissileFlame(short fxNumber, short xVel, short yVel, short zVel);
}
