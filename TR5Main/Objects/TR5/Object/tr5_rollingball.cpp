#include "framework.h"
#include "tr5_rollingball.h"
#include "Game/collision/sphere.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Sound/sound.h"
#include "Game/effects/effects.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"

constexpr auto ROLLING_BALL_MAX_VELOCITY = SECTOR(3);

void RollingBallCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* ballItem = &g_Level.Items[itemNumber];

	if (TestBoundsCollide(ballItem, laraItem, coll->Setup.Radius))
	{
		if (TestCollision(ballItem, laraItem))
		{
			if (TriggerActive(ballItem) && (ballItem->ItemFlags[0] || ballItem->Animation.VerticalVelocity))
			{
				laraItem->Animation.AnimNumber = LA_BOULDER_DEATH;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].FrameBase;
				laraItem->Animation.TargetState = LS_DEATH;
				laraItem->Animation.ActiveState = LS_DEATH;
				laraItem->Animation.Airborne = false;
			}
			else
				ObjectCollision(itemNumber, laraItem, coll);
		}
	}
}

void RollingBallControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	item->Animation.VerticalVelocity += GRAVITY;
	item->Position.xPos += item->ItemFlags[0] / 32;
	item->Position.yPos += item->Animation.VerticalVelocity;
	item->Position.zPos += item->ItemFlags[1] / 32;

	int dh = GetCollision(item).Position.Floor - CLICK(2);

	if (item->Position.yPos > dh)
	{
		if (abs(item->Animation.VerticalVelocity) > 16)
		{
			int distance = sqrt(
				pow(Camera.pos.x - item->Position.xPos, 2) +
				pow(Camera.pos.y - item->Position.yPos, 2) +
				pow(Camera.pos.z - item->Position.zPos, 2));

			if (distance < 16384)
				Camera.bounce = -(((16384 - distance) * abs(item->Animation.VerticalVelocity)) / 16384);
		}

		if ((item->Position.yPos - dh) < CLICK(2))
			item->Position.yPos = dh;

		if (item->Animation.VerticalVelocity <= 64)
		{
			if (abs(item->Animation.Velocity) <= CLICK(2) || (GetRandomControl() & 0x1F))
				item->Animation.VerticalVelocity = 0;
			else
				item->Animation.VerticalVelocity = -(short)(GetRandomControl() % ((int)item->Animation.Velocity / 8));
		}
		else
			item->Animation.VerticalVelocity = -(short)(item->Animation.VerticalVelocity / 4);
	}

	int frontX = item->Position.xPos;
	int frontZ = item->Position.zPos + CLICK(0.5f);
	int backX  = item->Position.xPos;
	int backZ  = item->Position.zPos - CLICK(0.5f);
	int rightX = item->Position.xPos + CLICK(0.5f);
	int rightZ = item->Position.zPos;
	int leftX  = item->Position.xPos - CLICK(0.5f);
	int leftZ  = item->Position.zPos;

	auto frontFloor = GetCollision(frontX, item->Position.yPos, frontZ, item->RoomNumber);
	auto backFloor  = GetCollision(backX,  item->Position.yPos, backZ,  item->RoomNumber);
	auto rightFloor = GetCollision(rightX, item->Position.yPos, rightZ, item->RoomNumber);
	auto leftFloor  = GetCollision(leftX,  item->Position.yPos, leftZ,  item->RoomNumber);

	int frontHeight = frontFloor.Position.Floor - CLICK(2);
	int backHeight  = backFloor.Position.Floor  - CLICK(2);
	int rightHeight = rightFloor.Position.Floor - CLICK(2);
	int leftHeight  = leftFloor.Position.Floor  - CLICK(2);

	int frontCeiling = frontFloor.Position.Ceiling + CLICK(2);
	int backCeiling  = backFloor.Position.Ceiling  + CLICK(2);
	int rightCeiling = rightFloor.Position.Ceiling + CLICK(2);
	int leftCeiling  = leftFloor.Position.Ceiling  + CLICK(2);

	frontX = item->Position.xPos;
	frontZ = item->Position.zPos + CLICK(2);
	backX  = item->Position.xPos;
	backZ  = item->Position.zPos - CLICK(2);
	rightX = item->Position.xPos + CLICK(2);
	rightZ = item->Position.zPos;
	leftX  = item->Position.xPos - CLICK(2);
	leftZ  = item->Position.zPos;

	auto fronFarFloor  = GetCollision(frontX, item->Position.yPos, frontZ, item->RoomNumber);
	auto backFarFloor  = GetCollision(backX,  item->Position.yPos, backZ,  item->RoomNumber);
	auto rightFarFloor = GetCollision(rightX, item->Position.yPos, rightZ, item->RoomNumber);
	auto leftFarFloor  = GetCollision(leftX,  item->Position.yPos, leftZ,  item->RoomNumber);

	int frontFarHeight = fronFarFloor.Position.Floor  - CLICK(2);
	int backFarHeight  = backFarFloor.Position.Floor  - CLICK(2);
	int rightFarHeight = rightFarFloor.Position.Floor - CLICK(2);
	int leftFarHeight  = leftFarFloor.Position.Floor  - CLICK(2);

	int frontFarCeiling = fronFarFloor.Position.Ceiling  + CLICK(2);
	int backFarCeiling  = backFarFloor.Position.Ceiling  + CLICK(2);
	int rightFarCeiling = rightFarFloor.Position.Ceiling + CLICK(2);
	int leftFarCeiling  = leftFarFloor.Position.Ceiling  + CLICK(2);

	if (item->Position.yPos - dh > -CLICK(1) ||
		item->Position.yPos - frontFarHeight >= CLICK(2) ||
		item->Position.yPos - rightFarHeight >= CLICK(2) ||
		item->Position.yPos - backFarHeight  >= CLICK(2) ||
		item->Position.yPos - leftFarHeight  >= CLICK(2))
	{
		int counterZ = 0;

		if ((frontFarHeight - dh) <= CLICK(1))
		{
			if (frontFarHeight - dh < -CLICK(4) || frontHeight - dh < -CLICK(1))
			{
				if (item->ItemFlags[1] <= 0)
				{
					if (!item->ItemFlags[1] && item->ItemFlags[0])
						item->Position.zPos = (item->Position.zPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[1] = -item->ItemFlags[1] / 2;
					item->Position.zPos = (item->Position.zPos & ~(CLICK(4) - 1)) | CLICK(2);
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
						item->Position.zPos = (item->Position.zPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[1] = -item->ItemFlags[1] / 2;
					item->Position.zPos = (item->Position.zPos & ~(CLICK(4) - 1)) | CLICK(2);
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
						item->Position.xPos = (item->Position.xPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[0] = -item->ItemFlags[0] / 2;
					item->Position.xPos = (item->Position.xPos & ~(CLICK(4) - 1)) | CLICK(2);
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
						item->Position.xPos = (item->Position.xPos & ~(CLICK(4) - 1)) | CLICK(2);
				}
				else
				{
					item->ItemFlags[0] = -item->ItemFlags[0] / 2;
					item->Position.xPos = (item->Position.xPos & ~(CLICK(4) - 1)) | CLICK(2);
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
		ItemNewRoom(itemNumber, roomNumber);

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
		angle = item->Position.yRot;

	if (item->Position.yRot != angle)
	{
		if (((angle - item->Position.yRot) & 0x7fff) >= 512)
		{
			if (angle <= item->Position.yRot || angle - item->Position.yRot >= 0x8000)
				item->Position.yRot -= CLICK(2);
			else
				item->Position.yRot += CLICK(2);
		}
		else
			item->Position.yRot = angle;
	}

	item->Position.xRot -= (abs(item->ItemFlags[0]) + abs(item->ItemFlags[1])) / 2;

	TestTriggers(item, true);
}

void ClassicRollingBallCollision(short itemNum, ITEM_INFO* lara, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNum];

	if (item->Status == ITEM_ACTIVE)
	{
		if (!TestBoundsCollide(item, lara, coll->Setup.Radius))
			return;
		if (!TestCollision(item, lara))
			return;
		if (lara->Animation.Airborne)
		{
			if (coll->Setup.EnableObjectPush)
				ItemPushItem(item, lara, coll, coll->Setup.EnableSpasm, 1);

			lara->HitPoints -= 100;
			int x = lara->Position.xPos - item->Position.xPos;
			int y = (lara->Position.yPos - 350) - (item->Position.yPos - 512);
			int z = lara->Position.zPos - item->Position.zPos;
			short d = (short)sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

			if (d < 512)
				d = 512;

			x = item->Position.xPos + ((x * 512) / d);
			y = item->Position.yPos - 512 + ((y * 512) / d);
			z = item->Position.zPos + ((z * 512) / d);
			DoBloodSplat(x, y, z, item->Animation.Velocity, item->Position.yRot, item->RoomNumber);
		}
		else
		{
			lara->HitStatus = 1;
			if (lara->HitPoints > 0)
			{
				lara->HitPoints = -1;//?
				lara->Position.yRot = item->Position.yRot;
				lara->Position.zPos = 0;
				lara->Position.zRot = 0;	

				lara->Animation.AnimNumber = LA_BOULDER_DEATH;
				lara->Animation.FrameNumber = g_Level.Anims[lara->Animation.AnimNumber].FrameBase;
				lara->Animation.ActiveState = LS_BOULDER_DEATH;
				lara->Animation.TargetState = LS_BOULDER_DEATH;
						
				Camera.flags = CF_FOLLOW_CENTER;
				Camera.targetAngle = ANGLE(170);
				Camera.targetElevation = -ANGLE(25);
				for (int i = 0; i < 15; i++)
				{
					int x = lara->Position.xPos + (GetRandomControl() - ANGLE(180.0f) / 256);
					int y = lara->Position.yPos - (GetRandomControl() / 64);
					int z = lara->Position.zPos + (GetRandomControl() - ANGLE(180.0f) / 256);
					short d = ((GetRandomControl() - ANGLE(180) / 8) + item->Position.yRot);
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
	FLOOR_INFO* floor;
	GAME_VECTOR* old;
	ROOM_INFO* r;

	auto* item = &g_Level.Items[itemNum];

	if (item->Status == ITEM_ACTIVE)
	{
		if (item->Animation.TargetState == 2)
		{
			AnimateItem(item);
			return;
		}

		if (item->Position.yPos < item->Floor)
		{
			if (!item->Animation.Airborne)
			{
				item->Animation.Airborne = 1;
				item->Animation.VerticalVelocity = -10;
			}
		}
		else if (item->Animation.ActiveState == 0)
			item->Animation.TargetState = 1;

		oldx = item->Position.xPos;
		oldz = item->Position.zPos;
		AnimateItem(item);
		roomNum = item->RoomNumber;
		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNum);
		if (item->RoomNumber != roomNum)
			ItemNewRoom(itemNum, item->RoomNumber);

		item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

		TestTriggers(item->Position.xPos, item->Position.yPos, item->Position.zPos, roomNum, true);

		if (item->Position.yPos >= (int)floor - 256)
		{
			item->Animation.Airborne = false;
			item->Animation.VerticalVelocity = 0;
			item->Position.yPos = item->Floor;
			SoundEffect(SFX_TR4_ROLLING_BALL, &item->Position, 0);
			dist = sqrt((SQUARE(Camera.mikePos.x - item->Position.xPos)) + (SQUARE(Camera.mikePos.z - item->Position.zPos)));
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

		x = item->Position.xPos + dist * phd_sin(item->Position.yRot);
		z = item->Position.zPos + dist * phd_cos(item->Position.yRot);

		floor = GetFloor(x, item->Position.yPos, z, &roomNum);
		y1 = GetFloorHeight(floor, x, item->Position.yPos, z);

		roomNum = item->RoomNumber;
		floor = GetFloor(x, item->Position.yPos - ydist, z, &roomNum);
		y2 = GetCeiling(floor, x, item->Position.yPos - ydist, z);

		if (y1 < item->Position.yPos || y2 > (item->Position.yPos-ydist)) //there's something wrong here, this if statement returns true, executing this block, deactivating the boulders.
		{
			/*stupid sound crap hardcoded to object # idk*/
			item->Status = ITEM_DEACTIVATED;
			item->Position.yPos = item->Floor;
			item->Position.xPos = oldx;
			item->Position.zPos = oldz;
			item->Animation.Velocity = 0;
			item->Animation.VerticalVelocity = 0;
			item->TouchBits = 0;
		}
	}
	else if (item->Status == ITEM_DEACTIVATED)
	{
		if (!TriggerActive(item))
		{
			item->Status = ITEM_NOT_ACTIVE;
			old = (GAME_VECTOR*)item->Data;
			item->Position.xPos = old->x;
			item->Position.yPos = old->y;
			item->Position.zPos = old->z;
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
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			item->Animation.ActiveState = g_Level.Anims[item->Animation.AnimNumber].ActiveState; 
			item->Animation.TargetState = g_Level.Anims[item->Animation.AnimNumber].ActiveState;
			item->Animation.RequiredState = 0;
			RemoveActiveItem(itemNum);
		}
	}
}

void InitialiseClassicRollingBall(short itemNum)
{
	ITEM_INFO *item;
	GAME_VECTOR* old;

	item = &g_Level.Items[itemNum];
	item->Data = GAME_VECTOR{ };
	old = item->Data;
	old->x = item->Position.xPos;
	old->y = item->Position.yPos;
	old->z = item->Position.zPos;
	old->roomNumber = item->RoomNumber;

}
