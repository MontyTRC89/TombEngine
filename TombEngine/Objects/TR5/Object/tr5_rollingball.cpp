#include "framework.h"
#include "Objects/TR5/Object/tr5_rollingball.h"

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
	int bigRadius   = CLICK(2) - 1;

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
	int backX  = item->Pose.Position.x;
	int backZ  = item->Pose.Position.z - smallRadius;
	int rightX = item->Pose.Position.x + smallRadius;
	int rightZ = item->Pose.Position.z;
	int leftX  = item->Pose.Position.x - smallRadius;
	int leftZ  = item->Pose.Position.z;

	auto frontFloor = GetCollision(frontX, item->Pose.Position.y, frontZ, item->RoomNumber);
	auto backFloor  = GetCollision(backX,  item->Pose.Position.y, backZ,  item->RoomNumber);
	auto rightFloor = GetCollision(rightX, item->Pose.Position.y, rightZ, item->RoomNumber);
	auto leftFloor  = GetCollision(leftX,  item->Pose.Position.y, leftZ,  item->RoomNumber);

	int frontHeight = frontFloor.Position.Floor - bigRadius;
	int backHeight  = backFloor.Position.Floor  - bigRadius;
	int rightHeight = rightFloor.Position.Floor - bigRadius;
	int leftHeight  = leftFloor.Position.Floor  - bigRadius;

	int frontCeiling = frontFloor.Position.Ceiling + bigRadius;
	int backCeiling  = backFloor.Position.Ceiling  + bigRadius;
	int rightCeiling = rightFloor.Position.Ceiling + bigRadius;
	int leftCeiling  = leftFloor.Position.Ceiling  + bigRadius;

	frontX = item->Pose.Position.x;
	frontZ = item->Pose.Position.z + bigRadius;
	backX  = item->Pose.Position.x;
	backZ  = item->Pose.Position.z - bigRadius;
	rightX = item->Pose.Position.x + bigRadius;
	rightZ = item->Pose.Position.z;
	leftX  = item->Pose.Position.x - bigRadius;
	leftZ  = item->Pose.Position.z;

	auto fronFarFloor  = GetCollision(frontX, item->Pose.Position.y, frontZ, item->RoomNumber);
	auto backFarFloor  = GetCollision(backX,  item->Pose.Position.y, backZ,  item->RoomNumber);
	auto rightFarFloor = GetCollision(rightX, item->Pose.Position.y, rightZ, item->RoomNumber);
	auto leftFarFloor  = GetCollision(leftX,  item->Pose.Position.y, leftZ,  item->RoomNumber);

	int frontFarHeight = fronFarFloor.Position.Floor  - bigRadius;
	int backFarHeight  = backFarFloor.Position.Floor  - bigRadius;
	int rightFarHeight = rightFarFloor.Position.Floor - bigRadius;
	int leftFarHeight  = leftFarFloor.Position.Floor  - bigRadius;

	int frontFarCeiling = fronFarFloor.Position.Ceiling  + bigRadius;
	int backFarCeiling  = backFarFloor.Position.Ceiling  + bigRadius;
	int rightFarCeiling = rightFarFloor.Position.Ceiling + bigRadius;
	int leftFarCeiling  = leftFarFloor.Position.Ceiling  + bigRadius;

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

		if (y1 < item->Pose.Position.y || y2 > (item->Pose.Position.y - ydist))
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
	ItemInfo *item;
	GameVector* old;

	item = &g_Level.Items[itemNum];
	item->Data = GameVector{ };
	old = item->Data;
	old->x = item->Pose.Position.x;
	old->y = item->Pose.Position.y;
	old->z = item->Pose.Position.z;
	old->RoomNumber = item->RoomNumber;
}
