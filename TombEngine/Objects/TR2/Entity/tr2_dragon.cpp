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

using namespace TEN::Input;

BITE_INFO DragonMouthBite = { 35, 171, 1168, 12 };

#define DRAGON_SWIPE_DAMAGE 250
#define DRAGON_TOUCH_DAMAGE 10
#define DRAGON_LIVE_TIME (30 * 11)
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
	DRAGON_ANIM_DEATH = 21,
	DRAGON_ANIM_DEAD = 22
};

static void CreateBartoliLight(short ItemIndex, int type)
{
	auto* item = &g_Level.Items[ItemIndex];

	if (type == 0)
		TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, (GetRandomControl() & 150) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 200, (GetRandomControl() & 20) + 200);
	else if (type == 1)
		TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, (GetRandomControl() & 75) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50);
	else if (type == 2)
		TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, (GetRandomControl() & 20) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 50, (GetRandomControl() & 20) + 0);
}

static short DragonFire(int x, int y, int z, short speed, short yRot, short roomNumber)
{
	short fxNumber = NO_ITEM;
	// TODO:: set correct fx parameters
	return fxNumber;
}

static void createExplosion(ItemInfo* item)
{
	short ExplosionIndex = CreateItem();

	if (ExplosionIndex != NO_ITEM)
	{
		auto* explosionItem = &g_Level.Items[ExplosionIndex];

		if (item->Timer == BOOM_TIME)
			explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM;
		else if (item->Timer == BOOM_TIME + 10)
			explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM2;
		else if (item->Timer == BOOM_TIME + 20)
			explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM3;

		explosionItem->Pose.Position.x = item->Pose.Position.x;
		explosionItem->Pose.Position.y = item->Pose.Position.y + CLICK(1);
		explosionItem->Pose.Position.z = item->Pose.Position.z;
		explosionItem->RoomNumber = item->RoomNumber;
		explosionItem->Pose.Orientation.y = 0;
		explosionItem->Pose.Orientation.x = 0;
		explosionItem->Pose.Orientation.z = 0;
		explosionItem->Animation.Velocity = 0;
		explosionItem->Animation.VerticalVelocity = 0;

		InitialiseItem(ExplosionIndex);
		AddActiveItem(ExplosionIndex);

		explosionItem->Status = ITEM_ACTIVE;
	}
}

static void createDragonBone(short frontNumber)
{
	short boneFront = CreateItem();
	short boneBack = CreateItem();

	if (boneBack != NO_ITEM && boneFront != NO_ITEM)
	{
		auto* item = &g_Level.Items[frontNumber];
		auto* dragonBack = &g_Level.Items[boneBack];

		dragonBack->ObjectNumber = ID_DRAGON_BONE_BACK;
		dragonBack->Pose = item->Pose;
		dragonBack->Pose.Orientation.x = 0;
		dragonBack->Pose.Orientation.z = 0;
		dragonBack->RoomNumber = item->RoomNumber;

		InitialiseItem(boneBack);

		auto* dragonFront = &g_Level.Items[boneFront];

		dragonFront->ObjectNumber = ID_DRAGON_BONE_FRONT;
		dragonFront->Pose = item->Pose;
		dragonFront->Pose.Orientation.x = 0;
		dragonFront->Pose.Orientation.z = 0;
		dragonFront->RoomNumber = item->RoomNumber;

		InitialiseItem(boneFront);

		dragonFront->MeshBits = 0xFF3FFFFF;
	}
}

void DragonCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		return;
	if (!TestCollision(item, laraItem))
		return;

	if (item->Animation.ActiveState == DRAGON_STATE_DEATH)
	{
		int rx = laraItem->Pose.Position.x - item->Pose.Position.x;
		int rz = laraItem->Pose.Position.z - item->Pose.Position.z;
		float s = phd_sin(item->Pose.Orientation.y);
		float c = phd_cos(item->Pose.Orientation.y);

		int sideShift = rx * s + rz * c;
		if (sideShift > DRAGON_LCOL&& sideShift < DRAGON_RCOL)
		{
			int shift = rx * c - rz * s;
			if (shift <= DRAGON_CLOSE && shift >= DRAGON_FAR)
				return;

			int angle = laraItem->Pose.Orientation.y - item->Pose.Orientation.y;

			int anim = item->Animation.AnimNumber - Objects[ID_DRAGON_BACK].animIndex;
			int frame = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

			if ((anim == DRAGON_ANIM_DEAD || (anim == DRAGON_ANIM_DEAD + 1 && frame <= DRAGON_ALMOST_LIVE)) &&
				TrInput & IN_ACTION &&
				item->ObjectNumber == ID_DRAGON_BACK &&
				!laraItem->Animation.IsAirborne &&
				shift <= DRAGON_MID && 
				shift > (DRAGON_CLOSE - 350) &&
				sideShift > -350 &&
				sideShift < 350 &&
				angle > (ANGLE(45.0f) - ANGLE(30.0f)) &&
				angle < (ANGLE(45.0f) + ANGLE(30.0f)))
			{
				laraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = 0;
				laraItem->Animation.TargetState = 7;

				laraItem->Pose = item->Pose;
				laraItem->Animation.IsAirborne = false;
				laraItem->Animation.Velocity = 0;
				laraItem->Animation.VerticalVelocity = 0;

				if (item->RoomNumber != laraItem->RoomNumber)
					ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

				AnimateItem(LaraItem);

				Lara.ExtraAnim = 1;
				Lara.Control.HandStatus = HandStatus::Busy;
				Lara.HitDirection = -1;

				Lara.MeshPtrs[LM_RHAND] = Objects[ID_LARA_EXTRA_ANIMS].meshIndex + LM_RHAND;
				
				((CreatureInfo*)g_Level.Items[(short)item->Data].Data)->Flags = -1;

				return;
			}

			if (shift < DRAGON_MID)
				shift = DRAGON_CLOSE - shift;
			else
				shift = DRAGON_FAR - shift;

			laraItem->Pose.Position.x += shift * c;
			laraItem->Pose.Position.z -= shift * s;

			return;
		}
	}

	ItemPushItem(item, laraItem, coll, 1, 0);
}

void DragonControl(short backItemNumber)
{
	auto* back = &g_Level.Items[backItemNumber];
	if (back->Data != NULL && back->ObjectNumber == ID_DRAGON_FRONT)
		return;

	short itemNumber = (short)back->Data;
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;

	bool ahead;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != DRAGON_STATE_DEATH)
		{
			item->Animation.AnimNumber = Objects[ID_DRAGON_FRONT].animIndex + 21;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = DRAGON_STATE_DEATH;
			item->Animation.TargetState = DRAGON_STATE_DEATH;
			creature->Flags = 0;
		}
		else if (creature->Flags >= 0)
		{
			CreateBartoliLight(itemNumber, 1);
			creature->Flags++;

			if (creature->Flags == DRAGON_LIVE_TIME)
				item->Animation.TargetState = DRAGON_STATE_IDLE;
			if (creature->Flags == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
				item->HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
		}
		else
		{
			if (creature->Flags > -20)
				CreateBartoliLight(itemNumber, 2);

			if (creature->Flags == -100)
				createDragonBone(itemNumber);
			else if (creature->Flags == -200)
			{
				DisableEntityAI(itemNumber);
				KillItem(backItemNumber);
				back->Status = ITEM_DEACTIVATED;
				KillItem(itemNumber);
				item->Status = ITEM_DEACTIVATED;
				return;
			}
			else if (creature->Flags < -100)
			{
				item->Pose.Position.y += 10;
				back->Pose.Position.y += 10;
			}

			creature->Flags--;
			return;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);
		angle = CreatureTurn(item, DRAGON_STATE_WALK_TURN);

		ahead = (AI.ahead && AI.distance > DRAGON_CLOSE_RANGE && AI.distance < DRAGON_STATE_IDLE_RANGE);

		if (item->TouchBits)
		{
			DoDamage(creature->Enemy, DRAGON_TOUCH_DAMAGE);
		}

		switch (item->Animation.ActiveState)
		{
		case DRAGON_STATE_IDLE:
			item->Pose.Orientation.y -= angle;

			if (!ahead)
			{
				if (AI.distance > DRAGON_STATE_IDLE_RANGE || !AI.ahead)
					item->Animation.TargetState = DRAGON_STATE_WALK;
				else if (AI.ahead && AI.distance < DRAGON_CLOSE_RANGE && !creature->Flags)
				{
					creature->Flags = 1;
					if (AI.angle < 0)
						item->Animation.TargetState = DRAGON_STATE_SWIPE_LEFT;
					else
						item->Animation.TargetState = DRAGON_STATE_SWIPE_RIGHT;
				}
				else if (AI.angle < 0)
					item->Animation.TargetState = DRAGON_STATE_TURN_LEFT;
				else
					item->Animation.TargetState = DRAGON_STATE_TURN_RIGHT;
			}
			else
				item->Animation.TargetState = DRAGON_STATE_AIM_1;

			break;

		case DRAGON_STATE_SWIPE_LEFT:
			if (item->TouchBits & DRAGON_TOUCH_L)
			{
				creature->Flags = 0;
				DoDamage(creature->Enemy, DRAGON_SWIPE_DAMAGE);
			}

			break;

		case DRAGON_STATE_SWIPE_RIGHT:
			if (item->TouchBits & DRAGON_TOUCH_R)
			{
				creature->Flags = 0;
				DoDamage(creature->Enemy, DRAGON_SWIPE_DAMAGE);
			}

			break;

		case DRAGON_STATE_WALK:
			creature->Flags = 0;

			if (ahead)
				item->Animation.TargetState = DRAGON_STATE_IDLE;
			else if (angle < -DRAGON_NEED_TURN)
			{
				if (AI.distance < DRAGON_STATE_IDLE_RANGE && AI.ahead)
					item->Animation.TargetState = DRAGON_STATE_IDLE;
				else
					item->Animation.TargetState = DRAGON_STATE_LEFT;
			}
			else if (angle > DRAGON_NEED_TURN)
			{
				if (AI.distance < DRAGON_STATE_IDLE_RANGE && AI.ahead)
					item->Animation.TargetState = DRAGON_STATE_IDLE;
				else
					item->Animation.TargetState = DRAGON_STATE_RIGHT;
			}

			break;

		case DRAGON_STATE_LEFT:
			if (angle > -DRAGON_NEED_TURN || ahead)
				item->Animation.TargetState = DRAGON_STATE_WALK;

			break;

		case DRAGON_STATE_RIGHT:
			if (angle < DRAGON_NEED_TURN || ahead)
				item->Animation.TargetState = DRAGON_STATE_WALK;

			break;

		case DRAGON_STATE_TURN_LEFT:
			item->Pose.Orientation.y += -(ANGLE(1.0f) - angle);
			creature->Flags = 0;

			break;

		case DRAGON_STATE_TURN_RIGHT:
			item->Pose.Orientation.y += (ANGLE(1.0f) - angle);
			creature->Flags = 0;

			break;

		case DRAGON_STATE_AIM_1:
			item->Pose.Orientation.y -= angle;

			if (AI.ahead)
				head = -AI.angle;

			if (ahead)
			{
				item->Animation.TargetState = DRAGON_STATE_FIRE_1;
				creature->Flags = 30;
			}
			else
			{
				item->Animation.TargetState = DRAGON_STATE_AIM_1;
				creature->Flags = 0;
			}

			break;

		case DRAGON_STATE_FIRE_1:
			item->Pose.Orientation.y -= angle;

			if (AI.ahead)
				head = -AI.angle;

			SoundEffect(SFX_TR2_DRAGON_FIRE, &item->Pose);

			if (creature->Flags)
			{
				if (AI.ahead)
					CreatureEffect(item, &DragonMouthBite, DragonFire);
				creature->Flags--;
			}
			else
				item->Animation.TargetState = DRAGON_STATE_IDLE;

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);

	back->Animation.ActiveState = item->Animation.ActiveState;
	back->Animation.AnimNumber = Objects[ID_DRAGON_BACK].animIndex + (item->Animation.AnimNumber - Objects[ID_DRAGON_FRONT].animIndex);
	back->Animation.FrameNumber = g_Level.Anims[back->Animation.AnimNumber].frameBase + (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase);
	back->Pose.Position.x = item->Pose.Position.x;
	back->Pose.Position.y = item->Pose.Position.y;
	back->Pose.Position.z = item->Pose.Position.z;
	back->Pose.Orientation.x = item->Pose.Orientation.x;
	back->Pose.Orientation.y = item->Pose.Orientation.y;
	back->Pose.Orientation.z = item->Pose.Orientation.z;

	if (back->RoomNumber != item->RoomNumber)
		ItemNewRoom(backItemNumber, item->RoomNumber);
}

void InitialiseBartoli(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Pose.Position.x -= CLICK(2);
	item->Pose.Position.z -= CLICK(2);

	short backItem = CreateItem();
	short frontItem = CreateItem();

	if (backItem != NO_ITEM && frontItem != NO_ITEM)
	{
		auto* back = &g_Level.Items[backItem];
		back->ObjectNumber = ID_DRAGON_BACK;
		back->Pose.Position.x = item->Pose.Position.x;
		back->Pose.Position.y = item->Pose.Position.y;
		back->Pose.Position.z = item->Pose.Position.z;
		back->Pose.Orientation.y = item->Pose.Orientation.y;
		back->RoomNumber = item->RoomNumber;
		back->Status = ITEM_INVISIBLE;
		back->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

		InitialiseItem(backItem);
		back->MeshBits = 0x1FFFFF;

		item->Data = backItem;

		auto* front = &g_Level.Items[frontItem];

		front->ObjectNumber = ID_DRAGON_FRONT;
		front->Pose.Position.x = item->Pose.Position.x;
		front->Pose.Position.y = item->Pose.Position.y;
		front->Pose.Position.z = item->Pose.Position.z;
		front->Pose.Orientation.y = item->Pose.Orientation.y;
		front->RoomNumber = item->RoomNumber;
		front->Status = ITEM_INVISIBLE;
		front->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

		InitialiseItem(frontItem);

		back->Data = frontItem;

		g_Level.NumItems += 2;
	}
}

void BartoliControl(short itemNumber)
{
	ItemInfo* back, *front;
	short frontItem, backItem;

	auto* item = &g_Level.Items[itemNumber];

	if (item->Timer)
	{
		item->Timer++;

		if (!(item->Timer & 7))
			Camera.bounce = item->Timer;

		CreateBartoliLight(itemNumber, 1);
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

				front->Pose.Position.x = item->Pose.Position.x;
				front->Pose.Position.y = item->Pose.Position.y + CLICK(1);
				front->Pose.Position.z = item->Pose.Position.z;
				front->RoomNumber = item->RoomNumber;
				front->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

				InitialiseItem(frontItem);
				AddActiveItem(frontItem);
				front->Status = ITEM_ACTIVE;
			}
		}
		else if (item->Timer >= 30 * 5)
		{
			backItem = (short)item->Data;
			back = &g_Level.Items[backItem];

			frontItem = (short)back->Data;
			front = &g_Level.Items[frontItem];

			front->TouchBits = back->TouchBits = NO_JOINT_BITS;
			EnableEntityAI(frontItem, 1);
			AddActiveItem(frontItem);
			AddActiveItem(backItem);
			back->Status = ITEM_ACTIVE;

			KillItem(itemNumber);
		}
	}
	else if (abs(LaraItem->Pose.Position.x - item->Pose.Position.x) < BARTOLI_RANGE &&
		abs(LaraItem->Pose.Position.z - item->Pose.Position.z) < BARTOLI_RANGE)
	{
		item->Timer = 1;
	}
}
