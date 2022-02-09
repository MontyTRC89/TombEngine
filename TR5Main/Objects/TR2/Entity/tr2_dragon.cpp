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
		TriggerDynamicLight(item->pos.xPos, item->pos.yPos - STEP_SIZE, item->pos.zPos, (GetRandomControl() & 150) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 200, (GetRandomControl() & 20) + 200);
	else if (type == 1)
		TriggerDynamicLight(item->pos.xPos, item->pos.yPos - STEP_SIZE, item->pos.zPos, (GetRandomControl() & 75) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50);
	else if (type == 2)
		TriggerDynamicLight(item->pos.xPos, item->pos.yPos - STEP_SIZE, item->pos.zPos, (GetRandomControl() & 20) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 50, (GetRandomControl() & 20) + 0);
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

		if (item->timer == BOOM_TIME)
			itemExplo->objectNumber = ID_SPHERE_OF_DOOM;
		else if (item->timer == BOOM_TIME + 10)
			itemExplo->objectNumber = ID_SPHERE_OF_DOOM2;
		else if (item->timer == BOOM_TIME + 20)
			itemExplo->objectNumber = ID_SPHERE_OF_DOOM3;

		itemExplo->pos.xPos = item->pos.xPos;
		itemExplo->pos.yPos = item->pos.yPos + STEP_SIZE;
		itemExplo->pos.zPos = item->pos.zPos;
		itemExplo->roomNumber = item->roomNumber;
		itemExplo->pos.yRot = 0;
		itemExplo->pos.xRot = 0;
		itemExplo->pos.zRot = 0;
		itemExplo->Velocity = 0;
		itemExplo->VerticalVelocity = 0;

		InitialiseItem(ExplIndex);
		AddActiveItem(ExplIndex);

		itemExplo->status = ITEM_ACTIVE;
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
		back_dragon->objectNumber = ID_DRAGON_BONE_BACK;
		back_dragon->pos.xPos = item->pos.xPos;
		back_dragon->pos.yPos = item->pos.yPos;
		back_dragon->pos.zPos = item->pos.zPos;
		back_dragon->pos.xRot = back_dragon->pos.zRot = 0;
		back_dragon->pos.yRot = item->pos.yRot;
		back_dragon->roomNumber = item->roomNumber;

		InitialiseItem(bone_back);

		front_dragon = &g_Level.Items[bone_front];
		front_dragon->objectNumber = ID_DRAGON_BONE_FRONT;
		front_dragon->pos.xPos = item->pos.xPos;
		front_dragon->pos.yPos = item->pos.yPos;
		front_dragon->pos.zPos = item->pos.zPos;
		front_dragon->pos.xRot = front_dragon->pos.zRot = 0;
		front_dragon->pos.yRot = item->pos.yRot;
		front_dragon->roomNumber = item->roomNumber;

		InitialiseItem(bone_front);

		front_dragon->meshBits = 0xFF3FFFFF;
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

	if (item->activeState == DRAGON_DEATH)
	{
		rx = laraitem->pos.xPos - item->pos.xPos;
		rz = laraitem->pos.zPos - item->pos.zPos;
		c = phd_cos(item->pos.yRot);
		s = phd_sin(item->pos.yRot);

		side_shift = rx * s + rz * c;
		if (side_shift > DRAGON_LCOL&& side_shift < DRAGON_RCOL)
		{
			shift = rx * c - rz * s;
			if (shift <= DRAGON_CLOSE && shift >= DRAGON_FAR)
				return;

			angle = laraitem->pos.yRot - item->pos.yRot;

			anim = item->animNumber - Objects[ID_DRAGON_BACK].animIndex;
			frame = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
			if ((anim == DRAGON_DEAD_ANIM || (anim == DRAGON_DEAD_ANIM + 1 && frame <= DRAGON_ALMOST_LIVE)) &&
				(TrInput & IN_ACTION) && item->objectNumber == ID_DRAGON_BACK && !laraitem->Airborne &&
				shift <= DRAGON_MID && shift > DRAGON_CLOSE - 350 && side_shift > -350 && side_shift < 350 &&
				angle > 0x4000 - ANGLE(30) && angle < 0x4000 + ANGLE(30))
			{
				laraitem->animNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
				laraitem->frameNumber = g_Level.Anims[laraitem->animNumber].frameBase;
				laraitem->activeState = 0;
				laraitem->targetState = 7;

				laraitem->pos.xPos = item->pos.xPos;
				laraitem->pos.yPos = item->pos.yPos;
				laraitem->pos.zPos = item->pos.zPos;
				laraitem->pos.yRot = item->pos.yRot;
				laraitem->pos.xRot = item->pos.xRot;
				laraitem->pos.zRot = item->pos.zRot;
				laraitem->VerticalVelocity = 0;
				laraitem->Airborne = false;
				laraitem->Velocity = 0;

				if (item->roomNumber != laraitem->roomNumber)
					ItemNewRoom(Lara.itemNumber, item->roomNumber);

				AnimateItem(LaraItem);

				Lara.ExtraAnim = 1;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.hitDirection = -1;

				Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_EXTRA_ANIMS].meshIndex + LM_RHAND;
				
				((CREATURE_INFO*)g_Level.Items[(short)item->data].data)->flags = -1;

				return;
			}

			if (shift < DRAGON_MID)
				shift = DRAGON_CLOSE - shift;
			else
				shift = DRAGON_FAR - shift;

			laraitem->pos.xPos += shift * c;
			laraitem->pos.zPos -= shift * s;

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
	if (back->data != NULL && back->objectNumber == ID_DRAGON_FRONT)
		return;

	itemNum = (short)back->data;
	if (!CreatureActive(itemNum))
		return;

	item = &g_Level.Items[itemNum];
	dragon = (CREATURE_INFO*)item->data;
	head = angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState != DRAGON_DEATH)
		{
			item->animNumber = Objects[ID_DRAGON_FRONT].animIndex + 21;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = item->targetState = DRAGON_DEATH;
			dragon->flags = 0;
		}
		else if (dragon->flags >= 0)
		{
			createBartoliLight(itemNum, 1);
			dragon->flags++;
			if (dragon->flags == DRAGON_LIVE_TIME)
				item->targetState = DRAGON_STOP;
			if (dragon->flags == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
				item->hitPoints = Objects[ID_DRAGON_FRONT].hitPoints / 2;
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
				back->status = ITEM_DEACTIVATED;
				KillItem(itemNum);
				item->status = ITEM_DEACTIVATED;
				return;
			}
			else if (dragon->flags < -100)
			{
				item->pos.yPos += 10;
				back->pos.yPos += 10;
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

		if (item->touchBits)
		{
			LaraItem->hitStatus = true;
			LaraItem->hitPoints -= DRAGON_TOUCH_DAMAGE;
		}

		switch (item->activeState)
		{
		case DRAGON_STOP:
			item->pos.yRot -= angle;

			if (!ahead)
			{
				if (info.distance > DRAGON_STOP_RANGE || !info.ahead)
					item->targetState = DRAGON_WALK;
				else if (info.ahead && info.distance < DRAGON_CLOSE_RANGE && !dragon->flags)
				{
					dragon->flags = 1;
					if (info.angle < 0)
						item->targetState = DRAGON_SWIPELEFT;
					else
						item->targetState = DRAGON_SWIPERIGHT;
				}
				else if (info.angle < 0)
					item->targetState = DRAGON_TURNLEFT;
				else
					item->targetState = DRAGON_TURNRIGHT;
			}
			else
				item->targetState = DRAGON_AIM1;
			break;

		case DRAGON_SWIPELEFT:
			if (item->touchBits & DRAGON_TOUCH_L)
			{
				LaraItem->hitStatus = true;
				LaraItem->hitPoints -= DRAGON_SWIPE_DAMAGE;
				dragon->flags = 0;
			}
			break;

		case DRAGON_SWIPERIGHT:
			if (item->touchBits & DRAGON_TOUCH_R)
			{
				LaraItem->hitStatus = true;
				LaraItem->hitPoints -= DRAGON_SWIPE_DAMAGE;
				dragon->flags = 0;
			}
			break;

		case DRAGON_WALK:
			dragon->flags = 0;

			if (ahead)
				item->targetState = DRAGON_STOP;
			else if (angle < -DRAGON_NEED_TURN)
			{
				if (info.distance < DRAGON_STOP_RANGE && info.ahead)
					item->targetState = DRAGON_STOP;
				else
					item->targetState = DRAGON_LEFT;
			}
			else if (angle > DRAGON_NEED_TURN)
			{
				if (info.distance < DRAGON_STOP_RANGE && info.ahead)
					item->targetState = DRAGON_STOP;
				else
					item->targetState = DRAGON_RIGHT;
			}
			break;

		case DRAGON_LEFT:
			if (angle > -DRAGON_NEED_TURN || ahead)
				item->targetState = DRAGON_WALK;
			break;

		case DRAGON_RIGHT:
			if (angle < DRAGON_NEED_TURN || ahead)
				item->targetState = DRAGON_WALK;
			break;

		case DRAGON_TURNLEFT:
			dragon->flags = 0;
			item->pos.yRot += -(ANGLE(1) - angle);
			break;

		case DRAGON_TURNRIGHT:
			dragon->flags = 0;
			item->pos.yRot += (ANGLE(1) - angle);
			break;

		case DRAGON_AIM1:
			item->pos.yRot -= angle;
			if (info.ahead)
				head = -info.angle;

			if (ahead)
			{
				dragon->flags = 30;
				item->targetState = DRAGON_FIRE1;
			}
			else
			{
				dragon->flags = 0;
				item->targetState = DRAGON_AIM1;
			}
			break;

		case DRAGON_FIRE1:
			item->pos.yRot -= angle;
			if (info.ahead)
				head = -info.angle;

			SoundEffect(305, &item->pos, 0);

			if (dragon->flags)
			{
				if (info.ahead)
					CreatureEffect(item, &dragonMouthBite, dragonFire);
				dragon->flags--;
			}
			else
				item->targetState = DRAGON_STOP;
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);

	back->activeState = item->activeState;
	back->animNumber = Objects[ID_DRAGON_BACK].animIndex + (item->animNumber - Objects[ID_DRAGON_FRONT].animIndex);
	back->frameNumber = g_Level.Anims[back->animNumber].frameBase + (item->frameNumber - g_Level.Anims[item->animNumber].frameBase);
	back->pos.xPos = item->pos.xPos;
	back->pos.yPos = item->pos.yPos;
	back->pos.zPos = item->pos.zPos;
	back->pos.xRot = item->pos.xRot;
	back->pos.yRot = item->pos.yRot;
	back->pos.zRot = item->pos.zRot;
	if (back->roomNumber != item->roomNumber)
		ItemNewRoom(backNum, item->roomNumber);
}

void InitialiseBartoli(short itemNum)
{
	ITEM_INFO* item, *back, *front;
	short back_item, front_item;

	item = &g_Level.Items[itemNum];
	item->pos.xPos -= STEP_SIZE * 2;
	item->pos.zPos -= STEP_SIZE * 2;

	back_item = CreateItem();
	front_item = CreateItem();
	if (back_item != NO_ITEM && front_item != NO_ITEM)
	{
		back = &g_Level.Items[back_item];
		back->objectNumber = ID_DRAGON_BACK;
		back->pos.xPos = item->pos.xPos;
		back->pos.yPos = item->pos.yPos;
		back->pos.zPos = item->pos.zPos;
		back->pos.yRot = item->pos.yRot;
		back->roomNumber = item->roomNumber;
		back->status = ITEM_INVISIBLE;
		back->shade = -1;

		InitialiseItem(back_item);
		back->meshBits = 0x1FFFFF;

		item->data = back_item;

		front = &g_Level.Items[front_item];
		front->objectNumber = ID_DRAGON_FRONT;
		front->pos.xPos = item->pos.xPos;
		front->pos.yPos = item->pos.yPos;
		front->pos.zPos = item->pos.zPos;
		front->pos.yRot = item->pos.yRot;
		front->roomNumber = item->roomNumber;
		front->status = ITEM_INVISIBLE;
		front->shade = -1;

		InitialiseItem(front_item);

		back->data = front_item;

		g_Level.NumItems += 2;
	}
}

void BartoliControl(short itemNum)
{
	ITEM_INFO* item, *back, *front;
	short front_item, back_item;

	item = &g_Level.Items[itemNum];

	if (item->timer)
	{
		item->timer++;

		if (!(item->timer & 7))
			Camera.bounce = item->timer;

		createBartoliLight(itemNum, 1);
		AnimateItem(item);

		if (item->timer == BOOM_TIME || item->timer == BOOM_TIME + 10 || item->timer == BOOM_TIME + 20)
		{
			front_item = CreateItem();
			if (front_item != NO_ITEM)
			{
				front = &g_Level.Items[front_item];
				if (item->timer == BOOM_TIME)
					front->objectNumber = ID_SPHERE_OF_DOOM;
				else if (item->timer == BOOM_TIME + 10)
					front->objectNumber = ID_SPHERE_OF_DOOM2;
				else
					front->objectNumber = ID_SPHERE_OF_DOOM3;
				front->pos.xPos = item->pos.xPos;
				front->pos.yPos = item->pos.yPos + STEP_SIZE;
				front->pos.zPos = item->pos.zPos;
				front->roomNumber = item->roomNumber;
				front->shade = -1;
				InitialiseItem(front_item);
				AddActiveItem(front_item);
				front->status = ITEM_ACTIVE;
			}
		}
		else if (item->timer >= 30 * 5)
		{
			back_item = (short)item->data;
			back = &g_Level.Items[back_item];

			front_item = (short)back->data;
			front = &g_Level.Items[front_item];

			front->touchBits = back->touchBits = 0;
			EnableBaddieAI(front_item, 1);
			AddActiveItem(front_item);
			AddActiveItem(back_item);
			back->status = ITEM_ACTIVE;

			KillItem(itemNum);
		}
	}
	else if (abs(LaraItem->pos.xPos - item->pos.xPos) < BARTOLI_RANGE && abs(LaraItem->pos.zPos - item->pos.zPos) < BARTOLI_RANGE)
	{
		item->timer = 1;
	}
}