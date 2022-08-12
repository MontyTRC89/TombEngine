#include "framework.h"
#include "tr5_rollingball.h"
#include "Game/collision/sphere.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/setup.h"
#include "Sound/sound.h"
#include "Game/effects/effects.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"
#include "Objects/Utils/VehicleHelpers.h"

constexpr auto ROLLING_BALL_MAX_VELOCITY = SECTOR(3);

void RollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* ballItem = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(ballItem, laraItem, coll->Setup.Radius) && 
		TestCollision(ballItem, laraItem))
	{
		if (TriggerActive(ballItem) && (ballItem->ItemFlags[0] || ballItem->Animation.VerticalVelocity))
		{
			if (laraItem->Animation.IsAirborne || TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, laraItem))
				laraItem->HitPoints = 0;
			else
			{
				laraItem->Animation.AnimNumber = LA_BOULDER_DEATH;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.TargetState = LS_DEATH;
				laraItem->Animation.ActiveState = LS_DEATH;
				laraItem->Animation.IsAirborne = false;
			}
		}
		else
			ObjectCollision(itemNumber, laraItem, coll);
	}
}

void RollingBallControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	auto oldPos = item->Pose;

	bool isWater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item->RoomNumber);
	int hDivider = isWater ? 64 : 32;
	int vDivider = isWater ? 3 : 1;

	item->Animation.VerticalVelocity += GRAVITY;
	item->Pose.Position.x += item->ItemFlags[0] / hDivider;
	item->Pose.Position.y += item->Animation.VerticalVelocity / vDivider;
	item->Pose.Position.z += item->ItemFlags[1] / hDivider;
	item->Animation.Velocity = Vector3Int::Distance(item->Pose.Position, oldPos.Position);

	int dh = GetCollision(item).Position.Floor - CLICK(2);

	if (item->Pose.Position.y > dh)
	{
		if (abs(item->Animation.VerticalVelocity) > 16)
		{
			int distance = sqrt(
				pow(Camera.pos.x - item->Pose.Position.x, 2) +
				pow(Camera.pos.y - item->Pose.Position.y, 2) +
				pow(Camera.pos.z - item->Pose.Position.z, 2));

			if (distance < 16384)
			{
				Camera.bounce = -(((16384 - distance) * abs(item->Animation.VerticalVelocity)) / 16384);
				SoundEffect(SFX_TR4_BOULDER_FALL, &item->Pose);
			}
		}

		if ((item->Pose.Position.y - dh) < CLICK(2))
			item->Pose.Position.y = dh;

		if (item->Animation.VerticalVelocity <= 64)
		{
			if (abs(item->Animation.Velocity) <= CLICK(2) || (GetRandomControl() & 0x1F))
				item->Animation.VerticalVelocity = 0;
			else
				item->Animation.VerticalVelocity = -(short)(GetRandomControl() % (item->Animation.Velocity / 8));
		}
		else
			item->Animation.VerticalVelocity = -(short)(item->Animation.VerticalVelocity / 4);
	}

	int frontX = item->Pose.Position.x;
	int frontZ = item->Pose.Position.z + CLICK(0.5f);
	int backX  = item->Pose.Position.x;
	int backZ  = item->Pose.Position.z - CLICK(0.5f);
	int rightX = item->Pose.Position.x + CLICK(0.5f);
	int rightZ = item->Pose.Position.z;
	int leftX  = item->Pose.Position.x - CLICK(0.5f);
	int leftZ  = item->Pose.Position.z;

	auto frontFloor = GetCollision(frontX, item->Pose.Position.y, frontZ, item->RoomNumber);
	auto backFloor  = GetCollision(backX,  item->Pose.Position.y, backZ,  item->RoomNumber);
	auto rightFloor = GetCollision(rightX, item->Pose.Position.y, rightZ, item->RoomNumber);
	auto leftFloor  = GetCollision(leftX,  item->Pose.Position.y, leftZ,  item->RoomNumber);

	int frontHeight = frontFloor.Position.Floor - CLICK(2);
	int backHeight  = backFloor.Position.Floor  - CLICK(2);
	int rightHeight = rightFloor.Position.Floor - CLICK(2);
	int leftHeight  = leftFloor.Position.Floor  - CLICK(2);

	int frontCeiling = frontFloor.Position.Ceiling + CLICK(2);
	int backCeiling  = backFloor.Position.Ceiling  + CLICK(2);
	int rightCeiling = rightFloor.Position.Ceiling + CLICK(2);
	int leftCeiling  = leftFloor.Position.Ceiling  + CLICK(2);

	frontX = item->Pose.Position.x;
	frontZ = item->Pose.Position.z + CLICK(2);
	backX  = item->Pose.Position.x;
	backZ  = item->Pose.Position.z - CLICK(2);
	rightX = item->Pose.Position.x + CLICK(2);
	rightZ = item->Pose.Position.z;
	leftX  = item->Pose.Position.x - CLICK(2);
	leftZ  = item->Pose.Position.z;

	auto fronFarFloor  = GetCollision(frontX, item->Pose.Position.y, frontZ, item->RoomNumber);
	auto backFarFloor  = GetCollision(backX,  item->Pose.Position.y, backZ,  item->RoomNumber);
	auto rightFarFloor = GetCollision(rightX, item->Pose.Position.y, rightZ, item->RoomNumber);
	auto leftFarFloor  = GetCollision(leftX,  item->Pose.Position.y, leftZ,  item->RoomNumber);

	int frontFarHeight = fronFarFloor.Position.Floor  - CLICK(2);
	int backFarHeight  = backFarFloor.Position.Floor  - CLICK(2);
	int rightFarHeight = rightFarFloor.Position.Floor - CLICK(2);
	int leftFarHeight  = leftFarFloor.Position.Floor  - CLICK(2);

	int frontFarCeiling = fronFarFloor.Position.Ceiling  + CLICK(2);
	int backFarCeiling  = backFarFloor.Position.Ceiling  + CLICK(2);
	int rightFarCeiling = rightFarFloor.Position.Ceiling + CLICK(2);
	int leftFarCeiling  = leftFarFloor.Position.Ceiling  + CLICK(2);

	if (item->Pose.Position.y - dh > -CLICK(1) ||
		item->Pose.Position.y - frontFarHeight >= CLICK(2) ||
		item->Pose.Position.y - rightFarHeight >= CLICK(2) ||
		item->Pose.Position.y - backFarHeight  >= CLICK(2) ||
		item->Pose.Position.y - leftFarHeight  >= CLICK(2))
	{
		int counterZ = 0;

		if ((frontFarHeight - dh) <= CLICK(1))
		{
			if (frontFarHeight - dh < -CLICK(4) || frontHeight - dh < -CLICK(1))
			{
				if (item->ItemFlags[1] <= 0)
				{
					if (!item->ItemFlags[1] && item->ItemFlags[0])
						item->Pose.Position.z = (item->Pose.Position.z & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[1] = -item->ItemFlags[1] / 2;
					item->Pose.Position.z = (item->Pose.Position.z & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (frontHeight == dh)
				counterZ++;
			else
				item->ItemFlags[1] += (frontHeight - dh) / 2;
		}

		if (backHeight - dh <= CLICK(1))
		{
			if (backFarHeight - dh < -CLICK(4) || backHeight - dh < -CLICK(1))
			{
				if (item->ItemFlags[1] >= 0)
				{
					if (!item->ItemFlags[1] && item->ItemFlags[0])
						item->Pose.Position.z = (item->Pose.Position.z & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[1] = -item->ItemFlags[1] / 2;
					item->Pose.Position.z = (item->Pose.Position.z & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (backHeight == dh)
				counterZ++;
			else
				item->ItemFlags[1] -= (backHeight - dh) / 2;
		}

		if (counterZ == 2)
		{
			if (abs(item->ItemFlags[1]) <= 64)
				item->ItemFlags[1] = 0;
			else
				item->ItemFlags[1] -= (item->ItemFlags[1] / 64);
		}

		int counterX = 0;

		if ((leftHeight - dh) <= CLICK(1))
		{
			if ((leftFarHeight - dh) < -CLICK(4) || leftHeight - dh < -CLICK(1))
			{
				if (item->ItemFlags[0] >= 0)
				{
					if (!item->ItemFlags[0] && item->ItemFlags[1])
						item->Pose.Position.x = (item->Pose.Position.x & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[0] = -item->ItemFlags[0] / 2;
					item->Pose.Position.x = (item->Pose.Position.x & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (leftHeight == dh)
				counterX++;
			else
				item->ItemFlags[0] -= (leftHeight - dh) / 2;
		}

		if ((rightHeight - dh) <= CLICK(1))
		{
			if ((rightFarHeight - dh) < -CLICK(4) || rightHeight - dh < -CLICK(1))
			{
				if (item->ItemFlags[0] <= 0)
				{
					if (!item->ItemFlags[0] && item->ItemFlags[1])
						item->Pose.Position.x = (item->Pose.Position.x & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[0] = -item->ItemFlags[0] / 2;
					item->Pose.Position.x = (item->Pose.Position.x & ~(CLICK(4) - 1)) | CLICK(2);
				}
			}
			else if (rightHeight == dh)
				counterX++;
			else
				item->ItemFlags[0] += (rightHeight - dh) / 2;
		}

		if (counterX == 2)
		{
			if (abs(item->ItemFlags[0]) <= 64)
				item->ItemFlags[0] = 0;
			else
				item->ItemFlags[0] -= (item->ItemFlags[0] / 64);
		}
	}

	auto roomNumber = GetCollision(item).RoomNumber;

	if (item->RoomNumber != roomNumber)
	{
		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, roomNumber) &&
			!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item->RoomNumber))
		{
			int waterHeight = GetWaterHeight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, roomNumber);
			SplashSetup.y = waterHeight - 1;
			SplashSetup.x = item->Pose.Position.x;
			SplashSetup.z = item->Pose.Position.z;
			SplashSetup.splashPower = item->Animation.VerticalVelocity * 4;
			SplashSetup.innerRadius = 160;
			SetupSplash(&SplashSetup, roomNumber);
		}

		ItemNewRoom(itemNumber, roomNumber);
	}

	if (item->ItemFlags[0] > ROLLING_BALL_MAX_VELOCITY)
		item->ItemFlags[0] = ROLLING_BALL_MAX_VELOCITY;
	else if (item->ItemFlags[0] < -ROLLING_BALL_MAX_VELOCITY)
		item->ItemFlags[0] = -ROLLING_BALL_MAX_VELOCITY;

	if (item->ItemFlags[1] > ROLLING_BALL_MAX_VELOCITY)
		item->ItemFlags[1] = ROLLING_BALL_MAX_VELOCITY;
	else if (item->ItemFlags[1] < -ROLLING_BALL_MAX_VELOCITY)
		item->ItemFlags[1] = -ROLLING_BALL_MAX_VELOCITY;

	float angle = 0;

	if (item->ItemFlags[1] || item->ItemFlags[0])
		angle = atan2(item->ItemFlags[1], item->ItemFlags[0]);
	else
		angle = item->Pose.Orientation.y;

	if (item->Pose.Orientation.y != angle)
	{
		/*if (((angle - item->Pose.Orientation.y) & 0x7fff) >= 512)
		{
			if (angle <= item->Pose.Orientation.y || angle - item->Pose.Orientation.y >= Angle::DegToRad(180.0f))
				item->Pose.Orientation.y -= CLICK(2));
			else
				item->Pose.Orientation.y += CLICK(2));
		}
		else*/
			item->Pose.Orientation.y = angle;
	}

	item->Pose.Orientation.x -= ((abs(item->ItemFlags[0]) + abs(item->ItemFlags[1])) / 2) / vDivider;

	TestTriggers(item, true);
	DoVehicleCollision(item, CLICK(0.9f));
}

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
			DoBloodSplat(x, y, z, item->Animation.Velocity, item->Pose.Orientation.y, item->RoomNumber);
		}
		else
		{
			lara->HitStatus = 1;
			if (lara->HitPoints > 0)
			{
				lara->HitPoints = -1;//?
				lara->Pose.Orientation.y = item->Pose.Orientation.y;
				lara->Pose.Position.z = 0;
				lara->Pose.Orientation.z = 0.0f;	

				lara->Animation.AnimNumber = LA_BOULDER_DEATH;
				lara->Animation.FrameNumber = g_Level.Anims[lara->Animation.AnimNumber].frameBase;
				lara->Animation.ActiveState = LS_BOULDER_DEATH;
				lara->Animation.TargetState = LS_BOULDER_DEATH;
						
				Camera.flags = CF_FOLLOW_CENTER;
				Camera.targetAngle = Angle::DegToRad(170);
				Camera.targetElevation = Angle::DegToRad(-25);
				for (int i = 0; i < 15; i++)
				{
					int x = lara->Pose.Position.x + (GetRandomControl() - Angle::DegToRad(180.0f) / 256);
					int y = lara->Pose.Position.y - (GetRandomControl() / 64);
					int z = lara->Pose.Position.z + (GetRandomControl() - Angle::DegToRad(180.0f) / 256);
					short d = ((GetRandomControl() - Angle::DegToRad(180) / 8) + item->Pose.Orientation.y);
					DoBloodSplat(x, y, z, (short)(item->Animation.Velocity * 2), d, item->RoomNumber);
				}
			}
		}
	}
	else if (item->Status != ITEM_INVISIBLE)
		ObjectCollision(itemNum, lara, coll);

}

void ClassicRollingBallControl(short itemNum)
{
	short x, z, dist, oldx, oldz, roomNum;
	short y1, y2, ydist;
	FloorInfo* floor;
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
				item->Animation.IsAirborne = 1;
				item->Animation.VerticalVelocity = -10;
			}
		}
		else if (item->Animation.ActiveState == 0)
			item->Animation.TargetState = 1;

		oldx = item->Pose.Position.x;
		oldz = item->Pose.Position.z;
		AnimateItem(item);
		roomNum = item->RoomNumber;
		floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNum);
		if (item->RoomNumber != roomNum)
			ItemNewRoom(itemNum, item->RoomNumber);

		item->Floor = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

		TestTriggers(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, roomNum, true);

		if (item->Pose.Position.y >= (int)floor - 256)
		{
			item->Animation.IsAirborne = false;
			item->Animation.VerticalVelocity = 0;
			item->Pose.Position.y = item->Floor;
			SoundEffect(SFX_TR4_ROLLING_BALL, &item->Pose);
			dist = sqrt((SQUARE(Camera.mikePos.x - item->Pose.Position.x)) + (SQUARE(Camera.mikePos.z - item->Pose.Position.z)));
			if (dist < 10240)
				Camera.bounce = -40 * (10240 - dist) / 10240;
		}

//		dist = (item->objectNumber == ID_CLASSIC_ROLLING_BALL) ? 384 : 1024;//huh?
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
			dist = 1024;
			ydist = 1024;
		}

		x = item->Pose.Position.x + dist * sin(item->Pose.Orientation.y);
		z = item->Pose.Position.z + dist * cos(item->Pose.Orientation.y);

		floor = GetFloor(x, item->Pose.Position.y, z, &roomNum);
		y1 = GetFloorHeight(floor, x, item->Pose.Position.y, z);

		roomNum = item->RoomNumber;
		floor = GetFloor(x, item->Pose.Position.y - ydist, z, &roomNum);
		y2 = GetCeiling(floor, x, item->Pose.Position.y - ydist, z);

		if (y1 < item->Pose.Position.y || y2 > (item->Pose.Position.y-ydist)) //there's something wrong here, this if statement returns true, executing this block, deactivating the boulders.
		{
			/*stupid sound crap hardcoded to object # idk*/
			item->Status = ITEM_DEACTIVATED;
			item->Pose.Position.y = item->Floor;
			item->Pose.Position.x = oldx;
			item->Pose.Position.z = oldz;
			item->Animation.Velocity = 0;
			item->Animation.VerticalVelocity = 0;
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
			if (item->RoomNumber != old->roomNumber)
			{
				RemoveDrawnItem(itemNum);
				r = &g_Level.Rooms[old->roomNumber];
				item->NextItem = r->itemNumber;
				r->itemNumber = itemNum;
				item->RoomNumber = old->roomNumber;
			}
			item->Animation.ActiveState = 0;
			item->Animation.TargetState = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = g_Level.Anims[item->Animation.AnimNumber].ActiveState; 
			item->Animation.TargetState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
			item->Animation.RequiredState = 0;
			RemoveActiveItem(itemNum);
		}
	}
}

void InitialiseClassicRollingBall(short itemNum)
{
	ItemInfo *item;
	GameVector* old;

	item = &g_Level.Items[itemNum];
	item->Data = GameVector{ };
	old = item->Data;
	old->x = item->Pose.Position.x;
	old->y = item->Pose.Position.y;
	old->z = item->Pose.Position.z;
	old->roomNumber = item->RoomNumber;

}
