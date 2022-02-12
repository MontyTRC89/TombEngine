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
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define DRAGON_SWIPE_DAMAGE 250
#define DRAGON_TOUCH_DAMAGE 10
#define DRAGON_DIE_ANIM 21
#define DRAGON_DEAD_ANIM 22
#define DRAGON_LIVE_TIME (30*11)
#define DRAGON_ALMOST_LIVE 100
#define DRAGON_WALK_TURN ANGLE(2)
#define DRAGON_NEED_TURN ANGLE(1)
#define DRAGON_TURN_TURN ANGLE(1)
#define DRAGON_CLOSE_RANGE SQUARE(WALL_SIZE*3)
#define DRAGON_STOP_RANGE SQUARE(WALL_SIZE*6)
#define DRAGON_FLAME_SPEED 200
#define DRAGON_TOUCH_R 0x0fe
#define DRAGON_TOUCH_L 0x7f000000
#define BOOM_TIME (130)
#define BOOM_TIME_MIDDLE (140)
#define BOOM_TIME_END (150)
#define BARTOLI_RANGE (WALL_SIZE*9)
#define DRAGON_CLOSE 900
#define DRAGON_FAR 2300
#define DRAGON_MID ((DRAGON_CLOSE+DRAGON_FAR)/2)
#define DRAGON_LCOL -512
#define DRAGON_RCOL +512

BITE_INFO dragonMouthBite = { 35, 171, 1168, 12 };

enum DRAGON_STATE 
{
	DRAGON_EMPTY,
	DRAGON_WALK,
	DRAGON_LEFT,
	DRAGON_RIGHT,
	DRAGON_AIM1,
	DRAGON_FIRE1,
	DRAGON_STOP,
	DRAGON_TURNLEFT,
	DRAGON_TURNRIGHT,
	DRAGON_SWIPELEFT,
	DRAGON_SWIPERIGHT,
	DRAGON_DEATH
};

static void createBartoliLight(short ItemIndex, int type)
{
	ITEM_INFO* item;
	item = &g_Level.Items[ItemIndex];

	if (type == 0)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos - STEP_SIZE, item->Position.zPos, (GetRandomControl() & 150) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 200, (GetRandomControl() & 20) + 200);
	else if (type == 1)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos - STEP_SIZE, item->Position.zPos, (GetRandomControl() & 75) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50);
	else if (type == 2)
		TriggerDynamicLight(item->Position.xPos, item->Position.yPos - STEP_SIZE, item->Position.zPos, (GetRandomControl() & 20) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 50, (GetRandomControl() & 20) + 0);
}

static short dragonFire(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	short fx_number = NO_ITEM;
	// TODO:: set correct fx parameters
	return fx_number;
}

static void createExplosion(ITEM_INFO* item)
{
	ITEM_INFO* itemExplo;
	short ExplIndex;

	ExplIndex = CreateItem();
	if (ExplIndex != NO_ITEM)
	{
		itemExplo = &g_Level.Items[ExplIndex];

		if (item->Timer == BOOM_TIME)
			itemExplo->ObjectNumber = ID_SPHERE_OF_DOOM;
		else if (item->Timer == BOOM_TIME + 10)
			itemExplo->ObjectNumber = ID_SPHERE_OF_DOOM2;
		else if (item->Timer == BOOM_TIME + 20)
			itemExplo->ObjectNumber = ID_SPHERE_OF_DOOM3;

		itemExplo->Position.xPos = item->Position.xPos;
		itemExplo->Position.yPos = item->Position.yPos + STEP_SIZE;
		itemExplo->Position.zPos = item->Position.zPos;
		itemExplo->RoomNumber = item->RoomNumber;
		itemExplo->Position.yRot = 0;
		itemExplo->Position.xRot = 0;
		itemExplo->Position.zRot = 0;
		itemExplo->Velocity = 0;
		itemExplo->VerticalVelocity = 0;

		InitialiseItem(ExplIndex);
		AddActiveItem(ExplIndex);

		itemExplo->Status = ITEM_ACTIVE;
	}
}

static void createDragonBone(short front_number)
{
	short bone_back, bone_front;
	ITEM_INFO* back_dragon, *front_dragon, *item;

	bone_front = CreateItem();
	bone_back = CreateItem();

	if (bone_back != NO_ITEM && bone_front != NO_ITEM)
	{
		item = &g_Level.Items[front_number];

		back_dragon = &g_Level.Items[bone_back];
		back_dragon->ObjectNumber = ID_DRAGON_BONE_BACK;
		back_dragon->Position.xPos = item->Position.xPos;
		back_dragon->Position.yPos = item->Position.yPos;
		back_dragon->Position.zPos = item->Position.zPos;
		back_dragon->Position.xRot = back_dragon->Position.zRot = 0;
		back_dragon->Position.yRot = item->Position.yRot;
		back_dragon->RoomNumber = item->RoomNumber;

		InitialiseItem(bone_back);

		front_dragon = &g_Level.Items[bone_front];
		front_dragon->ObjectNumber = ID_DRAGON_BONE_FRONT;
		front_dragon->Position.xPos = item->Position.xPos;
		front_dragon->Position.yPos = item->Position.yPos;
		front_dragon->Position.zPos = item->Position.zPos;
		front_dragon->Position.xRot = front_dragon->Position.zRot = 0;
		front_dragon->Position.yRot = item->Position.yRot;
		front_dragon->RoomNumber = item->RoomNumber;

		InitialiseItem(bone_front);

		front_dragon->MeshBits = 0xFF3FFFFF;
	}
}

void DragonCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	ITEM_INFO* item;
	int rx, rz, shift, side_shift, angle;
	int anim, frame;
	float c, s;

	item = &g_Level.Items[itemNum];

	if (!TestBoundsCollide(item, laraitem, coll->Setup.Radius))
		return;
	if (!TestCollision(item, laraitem))
		return;

	if (item->ActiveState == DRAGON_DEATH)
	{
		rx = laraitem->Position.xPos - item->Position.xPos;
		rz = laraitem->Position.zPos - item->Position.zPos;
		c = phd_cos(item->Position.yRot);
		s = phd_sin(item->Position.yRot);

		side_shift = rx * s + rz * c;
		if (side_shift > DRAGON_LCOL&& side_shift < DRAGON_RCOL)
		{
			shift = rx * c - rz * s;
			if (shift <= DRAGON_CLOSE && shift >= DRAGON_FAR)
				return;

			angle = laraitem->Position.yRot - item->Position.yRot;

			anim = item->AnimNumber - Objects[ID_DRAGON_BACK].animIndex;
			frame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
			if ((anim == DRAGON_DEAD_ANIM || (anim == DRAGON_DEAD_ANIM + 1 && frame <= DRAGON_ALMOST_LIVE)) &&
				(TrInput & IN_ACTION) && item->ObjectNumber == ID_DRAGON_BACK && !laraitem->Airborne &&
				shift <= DRAGON_MID && shift > DRAGON_CLOSE - 350 && side_shift > -350 && side_shift < 350 &&
				angle > 0x4000 - ANGLE(30) && angle < 0x4000 + ANGLE(30))
			{
				laraitem->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
				laraitem->FrameNumber = g_Level.Anims[laraitem->AnimNumber].frameBase;
				laraitem->ActiveState = 0;
				laraitem->TargetState = 7;

				laraitem->Position.xPos = item->Position.xPos;
				laraitem->Position.yPos = item->Position.yPos;
				laraitem->Position.zPos = item->Position.zPos;
				laraitem->Position.yRot = item->Position.yRot;
				laraitem->Position.xRot = item->Position.xRot;
				laraitem->Position.zRot = item->Position.zRot;
				laraitem->VerticalVelocity = 0;
				laraitem->Airborne = false;
				laraitem->Velocity = 0;

				if (item->RoomNumber != laraitem->RoomNumber)
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

			laraitem->Position.xPos += shift * c;
			laraitem->Position.zPos -= shift * s;

			return;
		}
	}

	ItemPushItem(item, laraitem, coll, 1, 0);
}

void DragonControl(short backNum)
{
	ITEM_INFO* item, *back;
	CREATURE_INFO* dragon;
	AI_INFO info;
	bool ahead;
	short head, angle;
	short itemNum;

	back = &g_Level.Items[backNum];
	if (back->Data != NULL && back->ObjectNumber == ID_DRAGON_FRONT)
		return;

	itemNum = (short)back->Data;
	if (!CreatureActive(itemNum))
		return;

	item = &g_Level.Items[itemNum];
	dragon = (CREATURE_INFO*)item->Data;
	head = angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != DRAGON_DEATH)
		{
			item->AnimNumber = Objects[ID_DRAGON_FRONT].animIndex + 21;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = item->TargetState = DRAGON_DEATH;
			dragon->flags = 0;
		}
		else if (dragon->flags >= 0)
		{
			createBartoliLight(itemNum, 1);
			dragon->flags++;
			if (dragon->flags == DRAGON_LIVE_TIME)
				item->TargetState = DRAGON_STOP;
			if (dragon->flags == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
				item->HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
		}
		else
		{
			if (dragon->flags > -20)
				createBartoliLight(itemNum, 2);

			if (dragon->flags == -100)
			{
				createDragonBone(itemNum);
			}
			else if (dragon->flags == -200)
			{
				DisableBaddieAI(itemNum);
				KillItem(backNum);
				back->Status = ITEM_DEACTIVATED;
				KillItem(itemNum);
				item->Status = ITEM_DEACTIVATED;
				return;
			}
			else if (dragon->flags < -100)
			{
				item->Position.yPos += 10;
				back->Position.yPos += 10;
			}

			dragon->flags--;
			return;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, DRAGON_WALK_TURN);

		ahead = (info.ahead && info.distance > DRAGON_CLOSE_RANGE && info.distance < DRAGON_STOP_RANGE);

		if (item->TouchBits)
		{
			LaraItem->HitStatus = true;
			LaraItem->HitPoints -= DRAGON_TOUCH_DAMAGE;
		}

		switch (item->ActiveState)
		{
		case DRAGON_STOP:
			item->Position.yRot -= angle;

			if (!ahead)
			{
				if (info.distance > DRAGON_STOP_RANGE || !info.ahead)
					item->TargetState = DRAGON_WALK;
				else if (info.ahead && info.distance < DRAGON_CLOSE_RANGE && !dragon->flags)
				{
					dragon->flags = 1;
					if (info.angle < 0)
						item->TargetState = DRAGON_SWIPELEFT;
					else
						item->TargetState = DRAGON_SWIPERIGHT;
				}
				else if (info.angle < 0)
					item->TargetState = DRAGON_TURNLEFT;
				else
					item->TargetState = DRAGON_TURNRIGHT;
			}
			else
				item->TargetState = DRAGON_AIM1;
			break;

		case DRAGON_SWIPELEFT:
			if (item->TouchBits & DRAGON_TOUCH_L)
			{
				LaraItem->HitStatus = true;
				LaraItem->HitPoints -= DRAGON_SWIPE_DAMAGE;
				dragon->flags = 0;
			}
			break;

		case DRAGON_SWIPERIGHT:
			if (item->TouchBits & DRAGON_TOUCH_R)
			{
				LaraItem->HitStatus = true;
				LaraItem->HitPoints -= DRAGON_SWIPE_DAMAGE;
				dragon->flags = 0;
			}
			break;

		case DRAGON_WALK:
			dragon->flags = 0;

			if (ahead)
				item->TargetState = DRAGON_STOP;
			else if (angle < -DRAGON_NEED_TURN)
			{
				if (info.distance < DRAGON_STOP_RANGE && info.ahead)
					item->TargetState = DRAGON_STOP;
				else
					item->TargetState = DRAGON_LEFT;
			}
			else if (angle > DRAGON_NEED_TURN)
			{
				if (info.distance < DRAGON_STOP_RANGE && info.ahead)
					item->TargetState = DRAGON_STOP;
				else
					item->TargetState = DRAGON_RIGHT;
			}
			break;

		case DRAGON_LEFT:
			if (angle > -DRAGON_NEED_TURN || ahead)
				item->TargetState = DRAGON_WALK;
			break;

		case DRAGON_RIGHT:
			if (angle < DRAGON_NEED_TURN || ahead)
				item->TargetState = DRAGON_WALK;
			break;

		case DRAGON_TURNLEFT:
			dragon->flags = 0;
			item->Position.yRot += -(ANGLE(1) - angle);
			break;

		case DRAGON_TURNRIGHT:
			dragon->flags = 0;
			item->Position.yRot += (ANGLE(1) - angle);
			break;

		case DRAGON_AIM1:
			item->Position.yRot -= angle;
			if (info.ahead)
				head = -info.angle;

			if (ahead)
			{
				dragon->flags = 30;
				item->TargetState = DRAGON_FIRE1;
			}
			else
			{
				dragon->flags = 0;
				item->TargetState = DRAGON_AIM1;
			}
			break;

		case DRAGON_FIRE1:
			item->Position.yRot -= angle;
			if (info.ahead)
				head = -info.angle;

			SoundEffect(305, &item->Position, 0);

			if (dragon->flags)
			{
				if (info.ahead)
					CreatureEffect(item, &dragonMouthBite, dragonFire);
				dragon->flags--;
			}
			else
				item->TargetState = DRAGON_STOP;
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);

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
		ItemNewRoom(backNum, item->RoomNumber);
}

void InitialiseBartoli(short itemNum)
{
	ITEM_INFO* item, *back, *front;
	short back_item, front_item;

	item = &g_Level.Items[itemNum];
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
	ITEM_INFO* item, *back, *front;
	short front_item, back_item;

	item = &g_Level.Items[itemNum];

	if (item->Timer)
	{
		item->Timer++;

		if (!(item->Timer & 7))
			Camera.bounce = item->Timer;

		createBartoliLight(itemNum, 1);
		AnimateItem(item);

		if (item->Timer == BOOM_TIME || item->Timer == BOOM_TIME + 10 || item->Timer == BOOM_TIME + 20)
		{
			front_item = CreateItem();
			if (front_item != NO_ITEM)
			{
				front = &g_Level.Items[front_item];
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
				InitialiseItem(front_item);
				AddActiveItem(front_item);
				front->Status = ITEM_ACTIVE;
			}
		}
		else if (item->Timer >= 30 * 5)
		{
			back_item = (short)item->Data;
			back = &g_Level.Items[back_item];

			front_item = (short)back->Data;
			front = &g_Level.Items[front_item];

			front->TouchBits = back->TouchBits = 0;
			EnableBaddieAI(front_item, 1);
			AddActiveItem(front_item);
			AddActiveItem(back_item);
			back->Status = ITEM_ACTIVE;

			KillItem(itemNum);
		}
	}
	else if (abs(LaraItem->Position.xPos - item->Position.xPos) < BARTOLI_RANGE && abs(LaraItem->Position.zPos - item->Position.zPos) < BARTOLI_RANGE)
	{
		item->Timer = 1;
	}
}