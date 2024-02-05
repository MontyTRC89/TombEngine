#include "framework.h"
#include "Objects/TR4/Trap/RollingBall/RollingBallObject.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Objects/TR4/Trap/RollingBall/RollingBallInfo.h"
#include "Specific/level.h"

constexpr auto ROLLING_BALL_MAX_VELOCITY = BLOCK(3);
constexpr auto FLOOR_HEIGHT_TOLERANCE = 32;

RollingBallInfo& GetRollingBallInfo(const ItemInfo& item)
{
	return (RollingBallInfo&)item.Data;
}

static void SpawnRollingBallWaterSplash(ItemInfo& item, int roomNumber)
{
	int waterHeight = GetWaterHeight(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, roomNumber);
	SplashSetup.y = waterHeight - 1;
	SplashSetup.x = item.Pose.Position.x;
	SplashSetup.z = item.Pose.Position.z;
	SplashSetup.splashPower = item.Animation.Velocity.y * 4;
	SplashSetup.innerRadius = 160;
	SetupSplash(&SplashSetup, roomNumber);
}

static void RollingBallUpdateRoom(ItemInfo& item)
{
	auto roomNumber = GetCollision(item).RoomNumber;
	if (item.RoomNumber != roomNumber)
	{
		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, roomNumber) &&
			!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item.RoomNumber))
		{
			SpawnRollingBallWaterSplash(item, roomNumber);
		}

		ItemNewRoom(item.Index, roomNumber);
	}
}

static void CheckOnGround(ItemInfo& rollingBallItem, RollingBallInfo& rollingBall)
{
	int floorDist = abs(rollingBallItem.Pose.Position.y - GetCollision(rollingBallItem).Position.Floor);
	rollingBall.isOnGround = (floorDist < rollingBall.radius + FLOOR_HEIGHT_TOLERANCE) ? true : false;
}

static void RollingBallGroundMovement(ItemInfo& rollingBallItem, RollingBallInfo& rollingBall)
{
	//Calculate new movement vector
	Vector3 direction = Vector3::Zero;

	//It must do it checking the ground normal, and the movementVector stored in the rollingBall struct.
	//(movementVector is the current velocity that the ball is having)



	
}

static void RollingBallAirMovement(ItemInfo& item)
{
	
}

static void RollingBallRotation(ItemInfo& item)
{
	//Rotate Pan, Yaw and Roll depending of the movement vector and speed.
}

void RollingBallControl(short itemNumber)
{
	auto& rollingBallItem = g_Level.Items[itemNumber];
	auto& rollingBall = GetRollingBallInfo(rollingBallItem);

	if (!TriggerActive(&rollingBallItem))
		return;
		
	if (rollingBall.energy_movement > 0)
	{
		CheckOnGround(rollingBallItem, rollingBall);

		if (rollingBall.isOnGround)
		{
			RollingBallGroundMovement(rollingBallItem, rollingBall);
		}
		else
		{
			RollingBallAirMovement(rollingBallItem);
		}

		RollingBallRotation(rollingBallItem);
		RollingBallUpdateRoom(rollingBallItem);
	}
	else
	{
		//rest
	}
}

void RollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto& item = g_Level.Items[itemNumber];

	if (!TestBoundsCollide(&item, laraItem, coll->Setup.Radius) ||
		!TestCollision(&item, laraItem))
	{
		return;
	}

	if (TriggerActive(&item) &&
		(item.ItemFlags[0] || item.ItemFlags[1] || item.Animation.Velocity.y))
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
