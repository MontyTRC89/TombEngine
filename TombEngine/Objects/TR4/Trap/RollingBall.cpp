#include "framework.h"
#include "Objects/TR4/Trap/RollingBall.h"

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

constexpr auto ROLLING_BALL_MAX_VELOCITY = BLOCK(3);

void RollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* ballItem = &g_Level.Items[itemNumber];

	if (!TestBoundsCollide(ballItem, laraItem, coll->Setup.Radius) ||
		!TestCollision(ballItem, laraItem))
	{
		return;
	}

	if (TriggerActive(ballItem) &&
		(ballItem->ItemFlags[0] || ballItem->ItemFlags[1] || ballItem->Animation.Velocity.y))
	{
		laraItem->HitPoints = 0;

		if (!laraItem->Animation.IsAirborne &&
			!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, laraItem))
		{
			SetAnimation(laraItem, LA_BOULDER_DEATH);
		}
	}
	else
	{
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

	int smallRadius = CLICK(0.5f);
	int bigRadius = CLICK(2) - 1;

	item->Animation.Velocity.y += GRAVITY;
	item->Pose.Position.x += item->ItemFlags[0] / hDivider;
	item->Pose.Position.y += item->Animation.Velocity.y / vDivider;
	item->Pose.Position.z += item->ItemFlags[1] / hDivider;
	item->Animation.Velocity.z = Vector3i::Distance(item->Pose.Position, oldPos.Position);

	int dh = GetCollision(item).Position.Floor - bigRadius;

	if (item->Pose.Position.y > dh)
	{
		if (abs(item->Animation.Velocity.y) > 16.0f)
		{
			float distance = Vector3::Distance(item->Pose.Position.ToVector3(), Camera.pos.ToVector3());
			if (distance < BLOCK(16))
			{
				if ((item->TriggerFlags & 1) != 1) // Flag 1 = silent.
				{
					Camera.bounce = -((BLOCK(16) - distance) * abs(item->Animation.Velocity.y)) / BLOCK(16);
					SoundEffect(SFX_TR4_BOULDER_FALL, &item->Pose);
				}
			}
		}

		if ((item->Pose.Position.y - dh) < bigRadius)
			item->Pose.Position.y = dh;

		if (item->Animation.Velocity.y <= 64)
		{
			if (abs(item->Animation.Velocity.z) <= CLICK(2) || (GetRandomControl() & 0x1F))
				item->Animation.Velocity.y = 0;
			else
				item->Animation.Velocity.y = -(GetRandomControl() % int(round(item->Animation.Velocity.z) / 8.0f));
		}
		else
			item->Animation.Velocity.y = -item->Animation.Velocity.y / 4.0f;
	}

	int frontX = item->Pose.Position.x;
	int frontZ = item->Pose.Position.z + smallRadius;
	int backX = item->Pose.Position.x;
	int backZ = item->Pose.Position.z - smallRadius;
	int rightX = item->Pose.Position.x + smallRadius;
	int rightZ = item->Pose.Position.z;
	int leftX = item->Pose.Position.x - smallRadius;
	int leftZ = item->Pose.Position.z;

	auto frontFloor = GetCollision(frontX, item->Pose.Position.y, frontZ, item->RoomNumber);
	auto backFloor = GetCollision(backX, item->Pose.Position.y, backZ, item->RoomNumber);
	auto rightFloor = GetCollision(rightX, item->Pose.Position.y, rightZ, item->RoomNumber);
	auto leftFloor = GetCollision(leftX, item->Pose.Position.y, leftZ, item->RoomNumber);

	int frontHeight = frontFloor.Position.Floor - bigRadius;
	int backHeight = backFloor.Position.Floor - bigRadius;
	int rightHeight = rightFloor.Position.Floor - bigRadius;
	int leftHeight = leftFloor.Position.Floor - bigRadius;

	int frontCeiling = frontFloor.Position.Ceiling + bigRadius;
	int backCeiling = backFloor.Position.Ceiling + bigRadius;
	int rightCeiling = rightFloor.Position.Ceiling + bigRadius;
	int leftCeiling = leftFloor.Position.Ceiling + bigRadius;

	frontX = item->Pose.Position.x;
	frontZ = item->Pose.Position.z + bigRadius;
	backX = item->Pose.Position.x;
	backZ = item->Pose.Position.z - bigRadius;
	rightX = item->Pose.Position.x + bigRadius;
	rightZ = item->Pose.Position.z;
	leftX = item->Pose.Position.x - bigRadius;
	leftZ = item->Pose.Position.z;

	auto fronFarFloor = GetCollision(frontX, item->Pose.Position.y, frontZ, item->RoomNumber);
	auto backFarFloor = GetCollision(backX, item->Pose.Position.y, backZ, item->RoomNumber);
	auto rightFarFloor = GetCollision(rightX, item->Pose.Position.y, rightZ, item->RoomNumber);
	auto leftFarFloor = GetCollision(leftX, item->Pose.Position.y, leftZ, item->RoomNumber);

	int frontFarHeight = fronFarFloor.Position.Floor - bigRadius;
	int backFarHeight = backFarFloor.Position.Floor - bigRadius;
	int rightFarHeight = rightFarFloor.Position.Floor - bigRadius;
	int leftFarHeight = leftFarFloor.Position.Floor - bigRadius;

	int frontFarCeiling = fronFarFloor.Position.Ceiling + bigRadius;
	int backFarCeiling = backFarFloor.Position.Ceiling + bigRadius;
	int rightFarCeiling = rightFarFloor.Position.Ceiling + bigRadius;
	int leftFarCeiling = leftFarFloor.Position.Ceiling + bigRadius;

	if (item->Pose.Position.y - dh > -CLICK(1) ||
		item->Pose.Position.y - frontFarHeight >= CLICK(2) ||
		item->Pose.Position.y - rightFarHeight >= CLICK(2) ||
		item->Pose.Position.y - backFarHeight >= CLICK(2) ||
		item->Pose.Position.y - leftFarHeight >= CLICK(2))
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
			SplashSetup.splashPower = item->Animation.Velocity.y * 4;
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

	short angle = 0;

	if (item->ItemFlags[1] || item->ItemFlags[0])
		angle = phd_atan(item->ItemFlags[1], item->ItemFlags[0]);
	else
		angle = item->Pose.Orientation.y;

	if (item->Pose.Orientation.y != angle)
	{
		if (((angle - item->Pose.Orientation.y) & 0x7fff) >= 512)
		{
			if (angle <= item->Pose.Orientation.y || angle - item->Pose.Orientation.y >= 0x8000)
				item->Pose.Orientation.y -= CLICK(2);
			else
				item->Pose.Orientation.y += CLICK(2);
		}
		else
			item->Pose.Orientation.y = angle;
	}

	item->Pose.Orientation.x -= ((abs(item->ItemFlags[0]) + abs(item->ItemFlags[1])) / 2) / vDivider;

	TestTriggers(item, true);
	DoVehicleCollision(item, bigRadius * 0.9f);
}