#include "framework.h"
#include "Objects/TR5/Object/tr5_rollingball.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;

constexpr auto ROLLING_BALL_CONTACT_DAMAGE = 100;
constexpr auto ROLLING_BALL_VELOCITY_MAX = SECTOR(3);

void InitialiseClassicRollingBall(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	item->Data = GameVector();
	auto* oldPos = (GameVector*)item->Data;

	*oldPos = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y,
		item->Pose.Position.z,
		item->RoomNumber
	);
}

void RollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* rBallItem = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(rBallItem, laraItem, coll->Setup.Radius) && 
		TestCollision(rBallItem, laraItem))
	{
		if (TriggerActive(rBallItem) && 
			(rBallItem->ItemFlags[0] || (rBallItem->Animation.Velocity.y != 0.0f)))
		{
			if (laraItem->Animation.IsAirborne || TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, laraItem))
				laraItem->HitPoints = 0;
			else
			{
				SetAnimation(laraItem, LA_BOULDER_DEATH);
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

	auto oldPose = item->Pose;
	bool isWater = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item);
	int hDivider = isWater ? 64 : 32;
	int vDivider = isWater ? 3 : 1;

	item->Animation.Velocity.y += GRAVITY;
	item->Pose.Position.x += item->ItemFlags[0] / hDivider;
	item->Pose.Position.y += item->Animation.Velocity.y / vDivider;
	item->Pose.Position.z += item->ItemFlags[1] / hDivider;
	item->Animation.Velocity.z = phd_Distance(&item->Pose, &oldPose);

	int dh = GetCollision(item).Position.Floor - CLICK(2);

	if (item->Pose.Position.y > dh)
	{
		if (abs(item->Animation.Velocity.y) > 16.0f)
		{
			int distance = sqrt(
				pow(Camera.pos.x - item->Pose.Position.x, 2) +
				pow(Camera.pos.y - item->Pose.Position.y, 2) +
				pow(Camera.pos.z - item->Pose.Position.z, 2));

			if (distance < SQUARE(CLICK(0.5f)))
			{
				Camera.bounce = -(((SQUARE(CLICK(0.5f)) - distance) * abs(item->Animation.Velocity.y)) / SQUARE(CLICK(0.5f)));
				SoundEffect(SFX_TR4_BOULDER_FALL, &item->Pose);
			}
		}

		if ((item->Pose.Position.y - dh) < CLICK(2))
			item->Pose.Position.y = dh;

		if (item->Animation.Velocity.y <= 64.0f)
		{
			if (abs(item->Animation.Velocity.z) <= CLICK(2) || TestProbability(0.97f))
				item->Animation.Velocity.y = 0.0f;
			else
				item->Animation.Velocity.y = -(GetRandomControl() % int(round(item->Animation.Velocity.z) / 8.0f));
		}
		else
			item->Animation.Velocity.y = -item->Animation.Velocity.y / 4.0f;
	}

	// Probe setup.
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
						item->Pose.Position.z = (item->Pose.Position.z & ~WALL_MASK) | CLICK(2);
				}
				else
				{
					item->ItemFlags[1] = -item->ItemFlags[1] / 2;
					item->Pose.Position.z = (item->Pose.Position.z & ~WALL_MASK) | CLICK(2);
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
						item->Pose.Position.z = (item->Pose.Position.z & ~WALL_MASK) | CLICK(2);
				}
				else
				{
					item->ItemFlags[1] = -item->ItemFlags[1] / 2;
					item->Pose.Position.z = (item->Pose.Position.z & ~WALL_MASK) | CLICK(2);
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
						item->Pose.Position.x = (item->Pose.Position.x & ~WALL_MASK) | CLICK(2);
				}
				else
				{
					item->ItemFlags[0] = -item->ItemFlags[0] / 2;
					item->Pose.Position.x = (item->Pose.Position.x & ~WALL_MASK) | CLICK(2);
				}
			}
			else if (leftHeight == dh)
				counterX++;
			else
				item->ItemFlags[0] -= (leftHeight - dh) / 2;
		}

		if ((rightHeight - dh) <= CLICK(1))
		{
			if ((rightFarHeight - dh) < -CLICK(4) ||
				(rightHeight - dh) < -CLICK(1))
			{
				if (item->ItemFlags[0] <= 0)
				{
					if (!item->ItemFlags[0] && item->ItemFlags[1])
						item->Pose.Position.x = (item->Pose.Position.x & ~WALL_MASK) | CLICK(2);
				}
				else
				{
					item->ItemFlags[0] = -item->ItemFlags[0] / 2;
					item->Pose.Position.x = (item->Pose.Position.x & ~WALL_MASK) | CLICK(2);
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
			!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item))
		{
			int waterHeight = GetWaterHeight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, roomNumber);
			SplashSetup.y = waterHeight - 1;
			SplashSetup.x = item->Pose.Position.x;
			SplashSetup.z = item->Pose.Position.z;
			SplashSetup.splashPower = item->Animation.Velocity.y * 4.0f;
			SplashSetup.innerRadius = 160;
			SetupSplash(&SplashSetup, roomNumber);
		}

		ItemNewRoom(itemNumber, roomNumber);
	}

	// Clamp (Y, Z?) velocity.
	if (item->ItemFlags[0] > ROLLING_BALL_VELOCITY_MAX)
		item->ItemFlags[0] = ROLLING_BALL_VELOCITY_MAX;
	else if (item->ItemFlags[0] < -ROLLING_BALL_VELOCITY_MAX)
		item->ItemFlags[0] = -ROLLING_BALL_VELOCITY_MAX;

	// Clamp (Y, Z?) velocity.
	if (item->ItemFlags[1] > ROLLING_BALL_VELOCITY_MAX)
		item->ItemFlags[1] = ROLLING_BALL_VELOCITY_MAX;
	else if (item->ItemFlags[1] < -ROLLING_BALL_VELOCITY_MAX)
		item->ItemFlags[1] = -ROLLING_BALL_VELOCITY_MAX;

	short angle = 0;

	if (item->ItemFlags[1] || item->ItemFlags[0])
		angle = phd_atan(item->ItemFlags[1], item->ItemFlags[0]);
	else
		angle = item->Pose.Orientation.y;

	if (item->Pose.Orientation.y != angle)
	{
		if (((angle - item->Pose.Orientation.y) & 0x7fff) >= ANGLE(2.8f))
		{
			if (angle <= item->Pose.Orientation.y || (angle - item->Pose.Orientation.y) >= ANGLE(180.0f))
				item->Pose.Orientation.y -= CLICK(2);
			else
				item->Pose.Orientation.y += CLICK(2);
		}
		else
			item->Pose.Orientation.y = angle;
	}

	item->Pose.Orientation.x -= ((abs(item->ItemFlags[0]) + abs(item->ItemFlags[1])) / 2) / vDivider;

	TestTriggers(item, true);
	DoVehicleCollision(item, CLICK(0.9f));
}

void ClassicRollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* rBallItem = &g_Level.Items[itemNumber];

	if (rBallItem->Status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(rBallItem, laraItem, coll->Setup.Radius))
			return;

		if (!TestCollision(rBallItem, laraItem))
			return;

		if (laraItem->Animation.IsAirborne)
		{
			if (coll->Setup.EnableObjectPush)
				ItemPushItem(rBallItem, laraItem, coll, coll->Setup.EnableSpasm, 1);

			DoDamage(laraItem, ROLLING_BALL_CONTACT_DAMAGE);
			int x = laraItem->Pose.Position.x - rBallItem->Pose.Position.x;
			int y = (laraItem->Pose.Position.y - 350) - (rBallItem->Pose.Position.y - 512);
			int z = laraItem->Pose.Position.z - rBallItem->Pose.Position.z;
			short d = (short)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

			if (d < 512)
				d = 512;

			x = rBallItem->Pose.Position.x + ((x * 512) / d);
			y = rBallItem->Pose.Position.y - 512 + ((y * 512) / d);
			z = rBallItem->Pose.Position.z + ((z * 512) / d);
			DoBloodSplat(x, y, z, rBallItem->Animation.Velocity.z, rBallItem->Pose.Orientation.y, rBallItem->RoomNumber);
		}
		else
		{
			laraItem->HitStatus = true;

			if (laraItem->HitPoints > 0)
			{
				laraItem->HitPoints = -1;//?
				laraItem->Pose.Orientation.y = rBallItem->Pose.Orientation.y;
				laraItem->Pose.Position.z = 0;
				laraItem->Pose.Orientation.z = 0;	

				SetAnimation(laraItem, LA_BOULDER_DEATH);
						
				Camera.flags = CF_FOLLOW_CENTER;
				Camera.targetAngle = ANGLE(170.0f);
				Camera.targetElevation = -ANGLE(25.0f);
				for (int i = 0; i < 15; i++)
				{
					int x = laraItem->Pose.Position.x + (GetRandomControl() - ANGLE(180.0f) / 256);
					int y = laraItem->Pose.Position.y - (GetRandomControl() / 64);
					int z = laraItem->Pose.Position.z + (GetRandomControl() - ANGLE(180.0f) / 256);
					short d = ((GetRandomControl() - ANGLE(180.0f) / 8) + rBallItem->Pose.Orientation.y);
					DoBloodSplat(x, y, z, (short)(rBallItem->Animation.Velocity.z * 2), d, rBallItem->RoomNumber);
				}
			}
		}
	}
	else if (rBallItem->Status != ITEM_INVISIBLE)
		ObjectCollision(itemNumber, laraItem, coll);
}

void ClassicRollingBallControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

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
				item->Animation.Velocity.y = -10.0f;
			}
		}
		else if (item->Animation.ActiveState == 0)
			item->Animation.TargetState = 1;

		int oldx = item->Pose.Position.x;
		int oldz = item->Pose.Position.z;
		AnimateItem(item);
		short roomNumber = item->RoomNumber;
		FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
		if (item->RoomNumber != roomNumber)
			ItemNewRoom(itemNumber, item->RoomNumber);

		item->Floor = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

		TestTriggers(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, roomNumber, true);

		int dist = 0;
		int ydist = 0;

		if (item->Pose.Position.y >= (int)floor - CLICK(1)) // TODO: Figure out what this FloorInfo -> int cast means. -- Sezz 2022.08.18
		{
			item->Animation.IsAirborne = false;
			item->Animation.Velocity.y = 0;
			item->Pose.Position.y = item->Floor;
			SoundEffect(SFX_TR4_ROLLING_BALL, &item->Pose);
			dist = sqrt((SQUARE(Camera.mikePos.x - item->Pose.Position.x)) + (SQUARE(Camera.mikePos.z - item->Pose.Position.z)));
			if (dist < SECTOR(10))
				Camera.bounce = -40 * (SECTOR(10) - dist) / SECTOR(10);
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
			dist = SECTOR(1);
			ydist = SECTOR(1);
		}

		int x = item->Pose.Position.x + dist * phd_sin(item->Pose.Orientation.y);
		int z = item->Pose.Position.z + dist * phd_cos(item->Pose.Orientation.y);

		floor = GetFloor(x, item->Pose.Position.y, z, &roomNumber);
		int y1 = GetFloorHeight(floor, x, item->Pose.Position.y, z);

		roomNumber = item->RoomNumber;
		floor = GetFloor(x, item->Pose.Position.y - ydist, z, &roomNumber);
		int y2 = GetCeiling(floor, x, item->Pose.Position.y - ydist, z);

		if (y1 < item->Pose.Position.y ||
			y2 > (item->Pose.Position.y - ydist)) //there's something wrong here, this if statement returns true, executing this block, deactivating the boulders.
		{
			item->Status = ITEM_DEACTIVATED;
			item->Pose.Position.x = oldx;
			item->Pose.Position.y = item->Floor;
			item->Pose.Position.z = oldz;
			item->Animation.Velocity.y = 0;
			item->Animation.Velocity.z = 0;
			item->TouchBits = NO_JOINT_BITS;
		}
	}
	else if (item->Status == ITEM_DEACTIVATED)
	{
		if (!TriggerActive(item))
		{
			auto* oldPos = (GameVector*)item->Data;

			item->Status = ITEM_NOT_ACTIVE;
			item->Pose.Position.x = oldPos->x;
			item->Pose.Position.y = oldPos->y;
			item->Pose.Position.z = oldPos->z;

			if (item->RoomNumber != oldPos->roomNumber)
			{
				RemoveDrawnItem(itemNumber);

				auto* room = &g_Level.Rooms[oldPos->roomNumber];

				item->NextItem = room->itemNumber;
				room->itemNumber = itemNumber;
				item->RoomNumber = oldPos->roomNumber;
			}

			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
			item->Animation.TargetState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
			item->Animation.RequiredState = 0;

			RemoveActiveItem(itemNumber);
		}
	}
}
