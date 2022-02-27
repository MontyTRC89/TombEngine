#include "framework.h"
#include "Objects/TR2/Entity/tr2_dragon.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO DragonMouthBite = { 35, 171, 1168, 12 };

#define DRAGON_SWIPE_DAMAGE 250
#define DRAGON_TOUCH_DAMAGE 10
#define DRAGON_DIE_ANIM 21
#define DRAGON_DEAD_ANIM 22
#define DRAGON_LIVE_TIME (30*11)
#define DRAGON_ALMOST_LIVE 100
#define DRAGON_STATE_WALK_TURN ANGLE(2.0f)
#define DRAGON_NEED_TURN ANGLE(1.0f)
#define DRAGON_TURN_TURN ANGLE(1.0f)
#define DRAGON_CLOSE_RANGE pow(SECTOR(3), 2)
#define DRAGON_STATE_IDLE_RANGE pow(SECTOR(6), 2)
#define DRAGON_FLAME_SPEED 200
#define DRAGON_TOUCH_R 0x0fe
#define DRAGON_TOUCH_L 0x7f000000
#define BOOM_TIME 130
#define BOOM_TIME_MIDDLE 140
#define BOOM_TIME_END 150
#define BARTOLI_RANGE SECTOR(9)
#define DRAGON_CLOSE 900
#define DRAGON_FAR 2300
#define DRAGON_MID ((DRAGON_CLOSE + DRAGON_FAR) / 2)
#define DRAGON_LCOL -CLICK(2)
#define DRAGON_RCOL CLICK(2)

enum DragonState
{
	DRAGON_STATE_NONE = 0,
	DRAGON_STATE_WALK = 1,
	DRAGON_STATE_LEFT = 2,
	DRAGON_STATE_RIGHT = 3,
	DRAGON_STATE_AIM_1 = 4,
	DRAGON_STATE_FIRE_1 = 5,
	DRAGON_STATE_IDLE = 6,
	DRAGON_STATE_TURN_LEFT = 7,
	DRAGON_STATE_TURN_RIGHT = 8,
	DRAGON_STATE_SWIPE_LEFT = 9,
	DRAGON_STATE_SWIPE_RIGHT = 10,
	DRAGON_STATE_DEATH = 11
};

// TODO
enum DragonAnim
{

};

static void CreateBartoliLight(short ItemIndex, int type)
{
	auto* item = &g_Level.Items[ItemIndex];

	if (type == 0)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos - CLICK(1), item->Position.zPos, (GetRandomControl() & 150) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 200, (GetRandomControl() & 20) + 200);
	else if (type == 1)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos - CLICK(1), item->Position.zPos, (GetRandomControl() & 75) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50);
	else if (type == 2)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos - CLICK(1), item->Position.zPos, (GetRandomControl() & 20) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 50, (GetRandomControl() & 20) + 0);
}

static short dragonFire(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	short fxNumber = NO_ITEM;
	// TODO:: set correct fx parameters
	return fxNumber;
}

static void createExplosion(ITEM_INFO* item)
{
	short ExplIndex = CreateItem();
	if (ExplIndex != NO_ITEM)
	{
		auto* explosionItem = &g_Level.Items[ExplIndex];

		if (item->Timer == BOOM_TIME)
			explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM;
		else if (item->Timer == BOOM_TIME + 10)
			explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM2;
		else if (item->Timer == BOOM_TIME + 20)
			explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM3;

		explosionItem->Position.xPos = item->Position.xPos;
		explosionItem->Position.yPos = item->Position.yPos + STEP_SIZE;
		explosionItem->Position.zPos = item->Position.zPos;
		explosionItem->RoomNumber = item->RoomNumber;
		explosionItem->Position.yRot = 0;
		explosionItem->Position.xRot = 0;
		explosionItem->Position.zRot = 0;
		explosionItem->Velocity = 0;
		explosionItem->VerticalVelocity = 0;

		InitialiseItem(ExplIndex);
		AddActiveItem(ExplIndex);

		explosionItem->Status = ITEM_ACTIVE;
	}
}

static void createDragonBone(short frontNumber)
{
	short boneBack, boneFront;
	ITEM_INFO* dragonBack, *dragonFront, *item;

	boneFront = CreateItem();
	boneBack = CreateItem();

	if (boneBack != NO_ITEM && boneFront != NO_ITEM)
	{
		item = &g_Level.Items[frontNumber];

		dragonBack = &g_Level.Items[boneBack];
		dragonBack->ObjectNumber = ID_DRAGON_BONE_BACK;
		dragonBack->Position.xPos = item->Position.xPos;
		dragonBack->Position.yPos = item->Position.yPos;
		dragonBack->Position.zPos = item->Position.zPos;
		dragonBack->Position.xRot = dragonBack->Position.zRot = 0;
		dragonBack->Position.yRot = item->Position.yRot;
		dragonBack->RoomNumber = item->RoomNumber;

		InitialiseItem(boneBack);

		dragonFront = &g_Level.Items[boneFront];
		dragonFront->ObjectNumber = ID_DRAGON_BONE_FRONT;
		dragonFront->Position.xPos = item->Position.xPos;
		dragonFront->Position.yPos = item->Position.yPos;
		dragonFront->Position.zPos = item->Position.zPos;
		dragonFront->Position.xRot = dragonFront->Position.zRot = 0;
		dragonFront->Position.yRot = item->Position.yRot;
		dragonFront->RoomNumber = item->RoomNumber;

		InitialiseItem(boneFront);

		dragonFront->MeshBits = 0xFF3FFFFF;
	}
}

void DragonCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	int rx, rz, shift, side_shift, angle;
	int anim, frame;
	float c, s;

	auto* item = &g_Level.Items[itemNumber];

	if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		return;
	if (!TestCollision(item, laraItem))
		return;

	if (item->ActiveState == DRAGON_STATE_DEATH)
	{
		rx = laraItem->Position.xPos - item->Position.xPos;
		rz = laraItem->Position.zPos - item->Position.zPos;
		c = phd_cos(item->Position.yRot);
		s = phd_sin(item->Position.yRot);

		side_shift = rx * s + rz * c;
		if (side_shift > DRAGON_LCOL&& side_shift < DRAGON_RCOL)
		{
			shift = rx * c - rz * s;
			if (shift <= DRAGON_CLOSE && shift >= DRAGON_FAR)
				return;

			angle = laraItem->Position.yRot - item->Position.yRot;

			anim = item->AnimNumber - Objects[ID_DRAGON_BACK].animIndex;
			frame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			if ((anim == DRAGON_DEAD_ANIM || (anim == DRAGON_DEAD_ANIM + 1 && frame <= DRAGON_ALMOST_LIVE)) &&
				(TrInput & IN_ACTION) && item->ObjectNumber == ID_DRAGON_BACK && !laraItem->Airborne &&
				shift <= DRAGON_MID && shift > DRAGON_CLOSE - 350 && side_shift > -350 && side_shift < 350 &&
				angle > 0x4000 - ANGLE(30) && angle < 0x4000 + ANGLE(30))
			{
				laraItem->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
				laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
				laraItem->ActiveState = 0;
				laraItem->TargetState = 7;

				laraItem->Position.xPos = item->Position.xPos;
				laraItem->Position.yPos = item->Position.yPos;
				laraItem->Position.zPos = item->Position.zPos;
				laraItem->Position.yRot = item->Position.yRot;
				laraItem->Position.xRot = item->Position.xRot;
				laraItem->Position.zRot = item->Position.zRot;
				laraItem->VerticalVelocity = 0;
				laraItem->Airborne = false;
				laraItem->Velocity = 0;

				if (item->RoomNumber != laraItem->RoomNumber)
					ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

				AnimateItem(LaraItem);

				Lara.ExtraAnim = 1;
				Lara.Control.HandStatus = HandStatus::Busy;
				Lara.hitDirection = -1;

				Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_EXTRA_ANIMS].meshIndex + LM_RHAND;
				
				((CREATURE_INFO*)g_Level.Items[(short)item->Data].Data)->flags = -1;

				return;
			}

			if (shift < DRAGON_MID)
				shift = DRAGON_CLOSE - shift;
			else
				shift = DRAGON_FAR - shift;

			laraItem->Position.xPos += shift * c;
			laraItem->Position.zPos -= shift * s;

			return;
		}
	}

	ItemPushItem(item, laraItem, coll, 1, 0);
}

void DragonControl(short backItemNumber)
{
	bool ahead;

	auto* back = &g_Level.Items[backItemNumber];
	if (back->Data != NULL && back->ObjectNumber == ID_DRAGON_FRONT)
		return;

	short itemNumber = (short)back->Data;
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != DRAGON_STATE_DEATH)
		{
			item->AnimNumber = Objects[ID_DRAGON_FRONT].animIndex + 21;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = DRAGON_STATE_DEATH;
			item->TargetState = DRAGON_STATE_DEATH;
			info->flags = 0;
		}
		else if (info->flags >= 0)
		{
			CreateBartoliLight(itemNumber, 1);
			info->flags++;

			if (info->flags == DRAGON_LIVE_TIME)
				item->TargetState = DRAGON_STATE_IDLE;
			if (info->flags == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
				item->HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
		}
		else
		{
			if (info->flags > -20)
				CreateBartoliLight(itemNumber, 2);

			if (info->flags == -100)
				createDragonBone(itemNumber);
			else if (info->flags == -200)
			{
				DisableEntityAI(itemNumber);
				KillItem(backItemNumber);
				back->Status = ITEM_DEACTIVATED;
				KillItem(itemNumber);
				item->Status = ITEM_DEACTIVATED;
				return;
			}
			else if (info->flags < -100)
			{
				item->Position.yPos += 10;
				back->Position.yPos += 10;
			}

			info->flags--;
			return;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);
		angle = CreatureTurn(item, DRAGON_STATE_WALK_TURN);

		ahead = (aiInfo.ahead && aiInfo.distance > DRAGON_CLOSE_RANGE && aiInfo.distance < DRAGON_STATE_IDLE_RANGE);

		if (item->TouchBits)
		{
			LaraItem->HitStatus = true;
			LaraItem->HitPoints -= DRAGON_TOUCH_DAMAGE;
		}

		switch (item->ActiveState)
		{
		case DRAGON_STATE_IDLE:
			item->Position.yRot -= angle;

			if (!ahead)
			{
				if (aiInfo.distance > DRAGON_STATE_IDLE_RANGE || !aiInfo.ahead)
					item->TargetState = DRAGON_STATE_WALK;
				else if (aiInfo.ahead && aiInfo.distance < DRAGON_CLOSE_RANGE && !info->flags)
				{
					info->flags = 1;
					if (aiInfo.angle < 0)
						item->TargetState = DRAGON_STATE_SWIPE_LEFT;
					else
						item->TargetState = DRAGON_STATE_SWIPE_RIGHT;
				}
				else if (aiInfo.angle < 0)
					item->TargetState = DRAGON_STATE_TURN_LEFT;
				else
					item->TargetState = DRAGON_STATE_TURN_RIGHT;
			}
			else
				item->TargetState = DRAGON_STATE_AIM_1;

			break;

		case DRAGON_STATE_SWIPE_LEFT:
			if (item->TouchBits & DRAGON_TOUCH_L)
			{
				info->flags = 0;

				LaraItem->HitStatus = true;
				LaraItem->HitPoints -= DRAGON_SWIPE_DAMAGE;
			}

			break;

		case DRAGON_STATE_SWIPE_RIGHT:
			if (item->TouchBits & DRAGON_TOUCH_R)
			{
				info->flags = 0;

				LaraItem->HitStatus = true;
				LaraItem->HitPoints -= DRAGON_SWIPE_DAMAGE;
			}

			break;

		case DRAGON_STATE_WALK:
			info->flags = 0;

			if (ahead)
				item->TargetState = DRAGON_STATE_IDLE;
			else if (angle < -DRAGON_NEED_TURN)
			{
				if (aiInfo.distance < DRAGON_STATE_IDLE_RANGE && aiInfo.ahead)
					item->TargetState = DRAGON_STATE_IDLE;
				else
					item->TargetState = DRAGON_STATE_LEFT;
			}
			else if (angle > DRAGON_NEED_TURN)
			{
				if (aiInfo.distance < DRAGON_STATE_IDLE_RANGE && aiInfo.ahead)
					item->TargetState = DRAGON_STATE_IDLE;
				else
					item->TargetState = DRAGON_STATE_RIGHT;
			}

			break;

		case DRAGON_STATE_LEFT:
			if (angle > -DRAGON_NEED_TURN || ahead)
				item->TargetState = DRAGON_STATE_WALK;

			break;

		case DRAGON_STATE_RIGHT:
			if (angle < DRAGON_NEED_TURN || ahead)
				item->TargetState = DRAGON_STATE_WALK;

			break;

		case DRAGON_STATE_TURN_LEFT:
			item->Position.yRot += -(ANGLE(1.0f) - angle);
			info->flags = 0;

			break;

		case DRAGON_STATE_TURN_RIGHT:
			item->Position.yRot += (ANGLE(1.0f) - angle);
			info->flags = 0;

			break;

		case DRAGON_STATE_AIM_1:
			item->Position.yRot -= angle;

			if (aiInfo.ahead)
				head = -aiInfo.angle;

			if (ahead)
			{
				info->flags = 30;
				item->TargetState = DRAGON_STATE_FIRE_1;
			}
			else
			{
				info->flags = 0;
				item->TargetState = DRAGON_STATE_AIM_1;
			}

			break;

		case DRAGON_STATE_FIRE_1:
			item->Position.yRot -= angle;

			if (aiInfo.ahead)
				head = -aiInfo.angle;

			SoundEffect(305, &item->Position, 0);

			if (info->flags)
			{
				if (aiInfo.ahead)
					CreatureEffect(item, &DragonMouthBite, dragonFire);
				info->flags--;
			}
			else
				item->TargetState = DRAGON_STATE_IDLE;

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);

	back->ActiveState = item->ActiveState;
	back->AnimNumber = Objects[ID_DRAGON_BACK].animIndex + (item->AnimNumber - Objects[ID_DRAGON_FRONT].animIndex);
	back->FrameNumber = g_Level.Anims[back->AnimNumber].frameBase + (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase);
	back->Position.xPos = item->Position.xPos;
	back->Position.yPos = item->Position.yPos;
	back->Position.zPos = item->Position.zPos;
	back->Position.xRot = item->Position.xRot;
	back->Position.yRot = item->Position.yRot;
	back->Position.zRot = item->Position.zRot;

	if (back->RoomNumber != item->RoomNumber)
		ItemNewRoom(backItemNumber, item->RoomNumber);
}

void InitialiseBartoli(short itemNum)
{
	ITEM_INFO* back, *front;
	short back_item, front_item;

	auto* item = &g_Level.Items[itemNum];
	item->Position.xPos -= STEP_SIZE * 2;
	item->Position.zPos -= STEP_SIZE * 2;

	back_item = CreateItem();
	front_item = CreateItem();
	if (back_item != NO_ITEM && front_item != NO_ITEM)
	{
		back = &g_Level.Items[back_item];
		back->ObjectNumber = ID_DRAGON_BACK;
		back->Position.xPos = item->Position.xPos;
		back->Position.yPos = item->Position.yPos;
		back->Position.zPos = item->Position.zPos;
		back->Position.yRot = item->Position.yRot;
		back->RoomNumber = item->RoomNumber;
		back->Status = ITEM_INVISIBLE;
		back->Shade = -1;

		InitialiseItem(back_item);
		back->MeshBits = 0x1FFFFF;

		item->Data = back_item;

		front = &g_Level.Items[front_item];
		front->ObjectNumber = ID_DRAGON_FRONT;
		front->Position.xPos = item->Position.xPos;
		front->Position.yPos = item->Position.yPos;
		front->Position.zPos = item->Position.zPos;
		front->Position.yRot = item->Position.yRot;
		front->RoomNumber = item->RoomNumber;
		front->Status = ITEM_INVISIBLE;
		front->Shade = -1;

		InitialiseItem(front_item);

		back->Data = front_item;

		g_Level.NumItems += 2;
	}
}

void BartoliControl(short itemNum)
{
	ITEM_INFO* back, *front;
	short frontItem, back_item;

	auto* item = &g_Level.Items[itemNum];

	if (item->Timer)
	{
		item->Timer++;

		if (!(item->Timer & 7))
			Camera.bounce = item->Timer;

		CreateBartoliLight(itemNum, 1);
		AnimateItem(item);

		if (item->Timer == BOOM_TIME ||
			item->Timer == BOOM_TIME + 10 ||
			item->Timer == BOOM_TIME + 20)
		{
			frontItem = CreateItem();
			if (frontItem != NO_ITEM)
			{
				front = &g_Level.Items[frontItem];

				if (item->Timer == BOOM_TIME)
					front->ObjectNumber = ID_SPHERE_OF_DOOM;
				else if (item->Timer == BOOM_TIME + 10)
					front->ObjectNumber = ID_SPHERE_OF_DOOM2;
				else
					front->ObjectNumber = ID_SPHERE_OF_DOOM3;

				front->Position.xPos = item->Position.xPos;
				front->Position.yPos = item->Position.yPos + STEP_SIZE;
				front->Position.zPos = item->Position.zPos;
				front->RoomNumber = item->RoomNumber;
				front->Shade = -1;

				InitialiseItem(frontItem);
				AddActiveItem(frontItem);
				front->Status = ITEM_ACTIVE;
			}
		}
		else if (item->Timer >= 30 * 5)
		{
			back_item = (short)item->Data;
			back = &g_Level.Items[back_item];

			frontItem = (short)back->Data;
			front = &g_Level.Items[frontItem];

			front->TouchBits = back->TouchBits = 0;
			EnableBaddieAI(frontItem, 1);
			AddActiveItem(frontItem);
			AddActiveItem(back_item);
			back->Status = ITEM_ACTIVE;

			KillItem(itemNum);
		}
	}
	else if (abs(LaraItem->Position.xPos - item->Position.xPos) < BARTOLI_RANGE &&
		abs(LaraItem->Position.zPos - item->Position.zPos) < BARTOLI_RANGE)
	{
		item->Timer = 1;
	}
}
