#include "framework.h"
#include "Objects/TR3/Trap/RollingBallClassic.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/level.h"

void ClassicRollingBallCollision(short itemNum, ItemInfo* lara, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNum];

	if (item->Status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, lara, coll->Setup.Radius))
			return;

		if (!TestCollision(item, lara))
			return;

		if (lara->Animation.IsAirborne)
		{
			if (coll->Setup.EnableObjectPush)
				ItemPushItem(item, lara, coll, coll->Setup.EnableSpasm, 1);

			DoDamage(lara, 100);
			int x = lara->Pose.Position.x - item->Pose.Position.x;
			int y = (lara->Pose.Position.y - 350) - (item->Pose.Position.y - 512);
			int z = lara->Pose.Position.z - item->Pose.Position.z;
			short d = (short)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

			if (d < 512)
				d = 512;

			x = item->Pose.Position.x + ((x * 512) / d);
			y = item->Pose.Position.y - 512 + ((y * 512) / d);
			z = item->Pose.Position.z + ((z * 512) / d);
			DoBloodSplat(x, y, z, item->Animation.Velocity.z, item->Pose.Orientation.y, item->RoomNumber);
		}
		else
		{
			lara->HitStatus = true;

			if (lara->HitPoints > 0)
			{
				lara->HitPoints = 0;
				lara->Pose.Orientation.y = item->Pose.Orientation.y;
				lara->Pose.Orientation.x = lara->Pose.Orientation.z = 0;

				SetAnimation(lara, LA_BOULDER_DEATH);

				Camera.flags = CF_FOLLOW_CENTER;
				Camera.targetAngle = ANGLE(170);
				Camera.targetElevation = -ANGLE(25);

				for (int i = 0; i < 15; i++)
				{
					int x = lara->Pose.Position.x + (GetRandomControl() - ANGLE(180.0f) / 256);
					int y = lara->Pose.Position.y - (GetRandomControl() / 64);
					int z = lara->Pose.Position.z + (GetRandomControl() - ANGLE(180.0f) / 256);
					short d = ((GetRandomControl() - ANGLE(180) / 8) + item->Pose.Orientation.y);
					DoBloodSplat(x, y, z, (short)(item->Animation.Velocity.z * 2), d, item->RoomNumber);
				}
			}
		}
	}
	else if (item->Status != ITEM_INVISIBLE)
		ObjectCollision(itemNum, lara, coll);

}

void ClassicRollingBallControl(short itemNum)
{
	int ydist, dist;
	GameVector* old;
	ROOM_INFO* r;

	auto* item = &g_Level.Items[itemNum];

	if (item->Status == ITEM_ACTIVE)
	{
		if (item->Animation.TargetState == 2)
		{
			AnimateItem(item);
			return;
		}

		if (item->Pose.Position.y < item->Floor)
		{
			if (!item->Animation.IsAirborne)
			{
				item->Animation.IsAirborne = true;
				item->Animation.Velocity.y = -10;
			}
		}
		else if (item->Animation.ActiveState == 0)
			item->Animation.TargetState = 1;

		int oldx = item->Pose.Position.x;
		int oldz = item->Pose.Position.z;

		AnimateItem(item);

		auto coll = GetCollision(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);

		item->Floor = coll.Position.Floor;

		if (item->RoomNumber != coll.RoomNumber)
			ItemNewRoom(itemNum, coll.RoomNumber);

		if (item->Pose.Position.y >= item->Floor - CLICK(1))
		{
			item->Animation.IsAirborne = false;
			item->Animation.Velocity.y = 0;
			item->Pose.Position.y = item->Floor;
			SoundEffect(SFX_TR4_ROLLING_BALL, &item->Pose);
			dist = sqrt((SQUARE(Camera.mikePos.x - item->Pose.Position.x)) + (SQUARE(Camera.mikePos.z - item->Pose.Position.z)));
			if (dist < BLOCK(10))
				Camera.bounce = -40 * (BLOCK(10) - dist) / BLOCK(10);
		}

		if (item->ObjectNumber == ID_CLASSIC_ROLLING_BALL)
		{
			dist = 320;
			ydist = 832;
		}
		else if (item->ObjectNumber == ID_BIG_ROLLING_BALL)
		{
			dist = 1088;
			ydist = 2112;
		}
		else
		{
			dist = BLOCK(1);
			ydist = BLOCK(1);
		}

		int x = item->Pose.Position.x + dist * phd_sin(item->Pose.Orientation.y);
		int z = item->Pose.Position.z + dist * phd_cos(item->Pose.Orientation.y);

		int y1 = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).Position.Floor;
		int y2 = GetCollision(x, item->Pose.Position.y - ydist, z, item->RoomNumber).Position.Ceiling;

		if (y1 < item->Pose.Position.y || y2 >(item->Pose.Position.y - ydist))
		{
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
			item->Status = ITEM_DEACTIVATED;
			item->Pose.Position.y = item->Floor;
			item->Pose.Position.x = oldx;
			item->Pose.Position.z = oldz;
			item->Animation.Velocity.z = 0;
			item->Animation.Velocity.y = 0;
			item->TouchBits = NO_JOINT_BITS;
		}
	}
	else if (item->Status == ITEM_DEACTIVATED)
	{
		if (!TriggerActive(item))
		{
			item->Status = ITEM_NOT_ACTIVE;
			old = (GameVector*)item->Data;
			item->Pose.Position.x = old->x;
			item->Pose.Position.y = old->y;
			item->Pose.Position.z = old->z;
			if (item->RoomNumber != old->RoomNumber)
			{
				RemoveDrawnItem(itemNum);
				r = &g_Level.Rooms[old->RoomNumber];
				item->NextItem = r->itemNumber;
				r->itemNumber = itemNum;
				item->RoomNumber = old->RoomNumber;
			}
			item->Animation.ActiveState = 0;
			item->Animation.TargetState = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->Animation.FrameNumber = GetAnimData(item).frameBase;
			item->Animation.ActiveState = GetAnimData(item).ActiveState;
			item->Animation.TargetState = GetAnimData(item).ActiveState;
			item->Animation.RequiredState = NO_STATE;
			RemoveActiveItem(itemNum);
		}
	}

	TestTriggers(item, true);
	DoVehicleCollision(item, CLICK(1.5f));
}

void InitializeClassicRollingBall(short itemNum)
{
	ItemInfo* item;
	GameVector* old;

	item = &g_Level.Items[itemNum];
	item->Data = GameVector{ };
	old = item->Data;
	old->x = item->Pose.Position.x;
	old->y = item->Pose.Position.y;
	old->z = item->Pose.Position.z;
	old->RoomNumber = item->RoomNumber;
}
