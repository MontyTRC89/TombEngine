#include "framework.h"
#include "tr5_robotclaw.h"
#include "Game/animation.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include <Game/collision/collide_room.h>



void InitialiseRobotClaw(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
}


void RobotClawControl(short itemNumber)
{
	short roomNumber = item->RoomNumber;
	GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
}
