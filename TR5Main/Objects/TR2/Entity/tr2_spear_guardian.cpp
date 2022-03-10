#include "framework.h"
#include "Objects/TR2/Entity/tr2_spear_guardian.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO spearLeftBite = { 0, 0, 920, 11 };
BITE_INFO spearRightBite = { 0, 0, 920, 18 };

static void XianDamage(ITEM_INFO* item, CreatureInfo* xian, int damage)
{
	if (!(xian->Flags & 1) && (item->TouchBits & 0x40000))
	{
		LaraItem->HitPoints -= damage;
		LaraItem->HitStatus = true;
		CreatureEffect(item, &spearRightBite, DoBloodSplat);
		xian->Flags |= 1;
		SoundEffect(SFX_TR2_CRUNCH2, &item->Position, 0);
	}

	if (!(xian->Flags & 2) && (item->TouchBits & 0x800))
	{
		LaraItem->HitPoints -= damage;
		LaraItem->HitStatus = true;
		CreatureEffect(item, &spearLeftBite, DoBloodSplat);
		xian->Flags |= 2;
		SoundEffect(SFX_TR2_CRUNCH2, &item->Position, 0);
	}
}

void InitialiseSpearGuardian(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	ClearItem(itemNum);

	item = &g_Level.Items[itemNum];
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 48;

	anim = &g_Level.Anims[item->AnimNumber];

	item->FrameNumber = anim->frameBase;
	item->ActiveState = anim->ActiveState;
}

void SpearGuardianControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CreatureInfo* xian;
	short angle, head, neck, tilt;
	int random, lara_alive;
	AI_INFO info;

	item = &g_Level.Items[itemNum];
	xian = (CreatureInfo*)item->Data;
	head = neck = angle = tilt = 0;
	lara_alive = (LaraItem->HitPoints > 0);

	if (item->HitPoints <= 0)
	{
		item->ActiveState = 17;
		item->MeshBits /= 2;

		if (!item->MeshBits)
		{
			SoundEffect(105, NULL, 0);
			// TODO: exploding death
		}
		return;
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, xian->MaxTurn);

		if (item->ActiveState != 18)
			item->MeshBits = 0xFFFFFFFF;

		switch (item->ActiveState)
		{
		case 18:
			if (!xian->Flags)
			{
				item->MeshBits = (item->MeshBits << 1) + 1;
				xian->Flags = 3;
			}
			else
				xian->Flags--;
			break;

		case 1:
			if (info.ahead)
				neck = info.angle;

			xian->MaxTurn = 0;

			if (xian->Mood == MoodType::Bored)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->TargetState = 2;
				else if (random < 0x400)
					item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->TargetState = 5;
			else
				item->TargetState = 3;
			break;

		case 2:
			if (info.ahead)
				neck = info.angle;

			xian->MaxTurn = 0;

			if (xian->Mood == MoodType::Escape)
				item->TargetState = 3;
			else if (xian->Mood == MoodType::Bored)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->TargetState = 1;
				else if (random < 0x400)
					item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->TargetState = 13;
			else
				item->TargetState = 3;
			break;

		case 3:
			if (info.ahead)
				neck = info.angle;

			xian->MaxTurn = ANGLE(3);

			if (xian->Mood == MoodType::Escape)
				item->TargetState = 4;
			else if (xian->Mood == MoodType::Bored)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->TargetState = 1;
				else if (random < 0x400)
					item->TargetState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				if (info.distance < SQUARE(WALL_SIZE * 3 / 2))
					item->TargetState = 7;
				else if (GetRandomControl() < 0x4000)
					item->TargetState = 9;
				else
					item->TargetState = 11;
			}
			else if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 3))
				item->TargetState = 4;
			break;

		case 4:
			if (info.ahead)
				neck = info.angle;

			xian->MaxTurn = ANGLE(5);

			if (xian->Mood == MoodType::Escape)
				break;
			else if (xian->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
				item->TargetState = 15;
			break;

		case 5:
			if (info.ahead)
				head = info.angle;

			xian->Flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE))
				item->TargetState = 1;
			else
				item->TargetState = 6;
			break;

		case 7:
			if (info.ahead)
				head = info.angle;

			xian->Flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 3 / 2))
				item->TargetState = 3;
			else
				item->TargetState = 8;
			break;

		case 9:
			if (info.ahead)
				head = info.angle;

			xian->Flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
				item->TargetState = 3;
			else
				item->TargetState = 8;
			break;

		case 11:
			if (info.ahead)
				head = info.angle;

			xian->Flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
				item->TargetState = 3;
			else
				item->TargetState = 8;
			break;

		case 13:
			if (info.ahead)
				head = info.angle;

			xian->Flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE))
				item->TargetState = 2;
			else
				item->TargetState = 14;
			break;

		case 15:
			if (info.ahead)
				head = info.angle;

			xian->Flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
				item->TargetState = 4;
			else
				item->TargetState = 16;
			break;

		case 6:
			XianDamage(item, xian, 75);
			break;

		case 8:
		case 10:
		case 12:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 75);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 2;
			}
			else
				item->TargetState = 3;
			break;

		case 14:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 75);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->TargetState = 1;
			else
				item->TargetState = 2;
			break;

		case 16:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 120);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
				item->TargetState = 3;
			else
				item->TargetState = 4;
			break;
		}
	}

	if (lara_alive && LaraItem->HitPoints <= 0)
	{
		CreatureKill(item, 49, 19, 2);
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(itemNum, angle, tilt);
}