#include "framework.h"
#include "Objects/TR2/Entity/tr2_knifethrower.h"

#include "Game/collision/floordata.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Sound/sound.h"

BITE_INFO KnifeBiteLeft = { 0, 0, 0, 5 };
BITE_INFO KnifeBiteRight = { 0, 0, 0, 8 };

// TODO
enum KnifeThrowerState
{

};

// TODO
enum KnifeThrowerAnim
{

};

void KnifeControl(short fxNumber)
{
	auto* fx = &EffectList[fxNumber];

	if (fx->counter <= 0)
	{
		KillEffect(fxNumber);
		return;
	}
	else
		fx->counter--;

	int speed = fx->speed * phd_cos(fx->pos.xRot);
	fx->pos.zPos += speed * phd_cos(fx->pos.yRot);
	fx->pos.xPos += speed * phd_sin(fx->pos.yRot);
	fx->pos.yPos += fx->speed * phd_sin(-fx->pos.xRot);

	auto probe = GetCollision(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);

	if (fx->pos.yPos >= probe.Position.Floor ||
		fx->pos.yPos <= probe.Position.Ceiling)
	{
		KillEffect(fxNumber);
		return;
	}

	if (probe.RoomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, probe.RoomNumber);

	fx->pos.zRot += ANGLE(30.0f);

	if (ItemNearLara(&fx->pos, 200))
	{
		LaraItem->HitPoints -= 50;
		LaraItem->HitStatus = true;

		fx->pos.yRot = LaraItem->Position.yRot;
		fx->speed = LaraItem->Animation.Velocity;
		fx->frameNumber = fx->counter = 0;

		SoundEffect(SFX_TR2_CRUNCH2, &fx->pos, 0);
		DoBloodSplat(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, 80, fx->pos.yRot, fx->roomNumber);
		KillEffect(fxNumber);
	}
}

static short ThrowKnife(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = 0;
	// TODO: add fx parameters
	return fxNumber;
}

void KnifeThrowerControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short angle = 0;
	short torso = 0;
	short head = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 10)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 23;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 10;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
			info->MaxTurn = 0;

			if (AI.ahead)
				head = AI.angle;

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (Targetable(item, &AI))
				item->Animation.TargetState = 8;
			else if (info->Mood == MoodType::Bored)
			{
				if (!AI.ahead || AI.distance > pow(SECTOR(6), 2))
					item->Animation.TargetState = 2;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(4), 2))
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 3;
			
			break;

		case 2:
			info->MaxTurn = ANGLE(3.0f);

			if (AI.ahead)
				head = AI.angle;

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (Targetable(item, &AI))
			{
				if (AI.distance < pow(SECTOR(2.5f), 2) || AI.zoneNumber != AI.enemyZone)
					item->Animation.TargetState = 1;
				else if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 4;
				else
					item->Animation.TargetState = 6;
			}
			else if (info->Mood == MoodType::Bored)
			{
				if (AI.ahead && AI.distance < pow(SECTOR(6), 2))
					item->Animation.TargetState = 1;
			}
			else if (!AI.ahead || AI.distance > pow(SECTOR(4), 2))
				item->Animation.TargetState = 3;
			
			break;

		case 3:
			info->MaxTurn = ANGLE(6.0f);
			tilt = angle / 3;

			if (AI.ahead)
				head = AI.angle;

			if (Targetable(item, &AI))
			{
				item->Animation.TargetState = 2;
			}
			else if (info->Mood == MoodType::Bored)
			{
				if (AI.ahead && AI.distance < pow(SECTOR(6), 2))
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 2;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(4), 2))
				item->Animation.TargetState = 2;

			break;

		case 4:
			info->Flags = 0;

			if (AI.ahead)
				torso = AI.angle;

			if (Targetable(item, &AI))
				item->Animation.TargetState = 5;
			else
				item->Animation.TargetState = 2;

			break;

		case 6:
			info->Flags = 0;

			if (AI.ahead)
				torso = AI.angle;

			if (Targetable(item, &AI))
				item->Animation.TargetState = 7;
			else
				item->Animation.TargetState = 2;

			break;

		case 8:
			info->Flags = 0;

			if (AI.ahead)
				torso = AI.angle;

			if (Targetable(item, &AI))
				item->Animation.TargetState = 9;
			else
				item->Animation.TargetState = 1;

			break;

		case 5:
			if (AI.ahead)
				torso = AI.angle;

			if (!info->Flags)
			{
				CreatureEffect(item, &KnifeBiteLeft, ThrowKnife);
				info->Flags = 1;
			}

			break;

		case 7:
			if (AI.ahead)
				torso = AI.angle;

			if (!info->Flags)
			{
				CreatureEffect(item, &KnifeBiteRight, ThrowKnife);
				info->Flags = 1;
			}

			break;

		case 9:
			if (AI.ahead)
				torso = AI.angle;

			if (!info->Flags)
			{
				CreatureEffect(item, &KnifeBiteLeft, ThrowKnife);
				CreatureEffect(item, &KnifeBiteRight, ThrowKnife);
				info->Flags = 1;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureJoint(item, 1, head);
	CreatureAnimation(itemNumber, angle, tilt);
}
