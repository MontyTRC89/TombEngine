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

	auto probe = GetCollisionResult(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);

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
		fx->speed = LaraItem->Velocity;
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
		if (item->ActiveState != 10)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 23;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 10;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->maximumTurn);

		switch (item->ActiveState)
		{
		case 1:
			info->maximumTurn = 0;

			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (info->mood == MoodType::Escape)
				item->TargetState = 3;
			else if (Targetable(item, &aiInfo))
				item->TargetState = 8;
			else if (info->mood == MoodType::Bored)
			{
				if (!aiInfo.ahead || aiInfo.distance > pow(SECTOR(6), 2))
					item->TargetState = 2;
			}
			else if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(4), 2))
				item->TargetState = 2;
			else
				item->TargetState = 3;
			
			break;

		case 2:
			info->maximumTurn = ANGLE(3.0f);

			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (info->mood == MoodType::Escape)
				item->TargetState = 3;
			else if (Targetable(item, &aiInfo))
			{
				if (aiInfo.distance < pow(SECTOR(2.5f), 2) || aiInfo.zoneNumber != aiInfo.enemyZone)
					item->TargetState = 1;
				else if (GetRandomControl() < 0x4000)
					item->TargetState = 4;
				else
					item->TargetState = 6;
			}
			else if (info->mood == MoodType::Bored)
			{
				if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(6), 2))
					item->TargetState = 1;
			}
			else if (!aiInfo.ahead || aiInfo.distance > pow(SECTOR(4), 2))
				item->TargetState = 3;
			
			break;

		case 3:
			info->maximumTurn = ANGLE(6.0f);
			tilt = angle / 3;

			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (Targetable(item, &aiInfo))
			{
				item->TargetState = 2;
			}
			else if (info->mood == MoodType::Bored)
			{
				if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(6), 2))
					item->TargetState = 1;
				else
					item->TargetState = 2;
			}
			else if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(4), 2))
				item->TargetState = 2;

			break;

		case 4:
			info->flags = 0;

			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (Targetable(item, &aiInfo))
				item->TargetState = 5;
			else
				item->TargetState = 2;

			break;

		case 6:
			info->flags = 0;

			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (Targetable(item, &aiInfo))
				item->TargetState = 7;
			else
				item->TargetState = 2;

			break;

		case 8:
			info->flags = 0;

			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (Targetable(item, &aiInfo))
				item->TargetState = 9;
			else
				item->TargetState = 1;

			break;

		case 5:
			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (!info->flags)
			{
				CreatureEffect(item, &KnifeBiteLeft, ThrowKnife);
				info->flags = 1;
			}

			break;

		case 7:
			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (!info->flags)
			{
				CreatureEffect(item, &KnifeBiteRight, ThrowKnife);
				info->flags = 1;
			}

			break;

		case 9:
			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (!info->flags)
			{
				CreatureEffect(item, &KnifeBiteLeft, ThrowKnife);
				CreatureEffect(item, &KnifeBiteRight, ThrowKnife);
				info->flags = 1;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureJoint(item, 1, head);
	CreatureAnimation(itemNumber, angle, tilt);
}
