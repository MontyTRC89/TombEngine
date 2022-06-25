#include "framework.h"
#include "Objects/TR2/Entity/tr2_spear_guardian.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO SpearBiteLeft = { 0, 0, 920, 11 };
BITE_INFO SpearBiteRight = { 0, 0, 920, 18 };

// TODO
enum SpearGuardianState
{

};

// TODO
enum SpearGuardianAnim
{

};

static void XianDamage(ItemInfo* item, int damage)
{
	auto* creature = GetCreatureInfo(item);

	if (!(creature->Flags & 1) && item->TouchBits & 0x40000)
	{
		DoDamage(creature->Enemy, damage);
		CreatureEffect(item, &SpearBiteRight, DoBloodSplat);
		creature->Flags |= 1;
		SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
	}

	if (!(creature->Flags & 2) && item->TouchBits & 0x800)
	{
		DoDamage(creature->Enemy, damage);
		CreatureEffect(item, &SpearBiteLeft, DoBloodSplat);
		creature->Flags |= 2;
		SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
	}
}

void InitialiseSpearGuardian(short itemNumber)
{
	ClearItem(itemNumber);

	auto* item = &g_Level.Items[itemNumber];
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 48;

	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];

	item->Animation.FrameNumber = anim->frameBase;
	item->Animation.ActiveState = anim->ActiveState;
}

void SpearGuardianControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;

	bool laraAlive = LaraItem->HitPoints > 0;

	if (item->HitPoints <= 0)
	{
		item->Animation.ActiveState = 17;
		item->MeshBits /= 2;

		if (!item->MeshBits)
		{
			SoundEffect(SFX_TR4_EXPLOSION1, NULL);
			// TODO: exploding death
		}

		return;
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if (item->Animation.ActiveState != 18)
			item->MeshBits = 0xFFFFFFFF;

		switch (item->Animation.ActiveState)
		{
		case 18:
			if (!creature->Flags)
			{
				item->MeshBits = (item->MeshBits << 1) + 1;
				creature->Flags = 3;
			}
			else
				creature->Flags--;

			break;

		case 1:
			creature->MaxTurn = 0;

			if (AI.ahead)
				neck = AI.angle;

			if (creature->Mood == MoodType::Bored)
			{
				int random = GetRandomControl();
				if (random < 0x200)
					item->Animation.TargetState = 2;
				else if (random < 0x400)
					item->Animation.TargetState = 3;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->Animation.TargetState = 5;
			else
				item->Animation.TargetState = 3;

			break;

		case 2:
			creature->MaxTurn = 0;

			if (AI.ahead)
				neck = AI.angle;

			if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (creature->Mood == MoodType::Bored)
			{
				int random = GetRandomControl();
				if (random < 0x200)
					item->Animation.TargetState = 1;
				else if (random < 0x400)
					item->Animation.TargetState = 3;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->Animation.TargetState = 13;
			else
				item->Animation.TargetState = 3;

			break;

		case 3:
			creature->MaxTurn = ANGLE(3.0f);

			if (AI.ahead)
				neck = AI.angle;

			if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 4;
			else if (creature->Mood == MoodType::Bored)
			{
				int random = GetRandomControl();
				if (random < 0x200)
					item->Animation.TargetState = 1;
				else if (random < 0x400)
					item->Animation.TargetState = 2;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
			{
				if (AI.distance < pow(SECTOR(1.5f), 2))
					item->Animation.TargetState = 7;
				else if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 9;
				else
					item->Animation.TargetState = 11;
			}
			else if (!AI.ahead || AI.distance > pow(SECTOR(3), 2))
				item->Animation.TargetState = 4;

			break;

		case 4:
			creature->MaxTurn = ANGLE(5.0f);

			if (AI.ahead)
				neck = AI.angle;

			if (creature->Mood == MoodType::Escape)
				break;
			else if (creature->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 2;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 15;

			break;

		case 5:
			if (AI.ahead)
				head = AI.angle;

			creature->Flags = 0;
			if (!AI.ahead || AI.distance > pow(SECTOR(1), 2))
				item->Animation.TargetState = 1;
			else
				item->Animation.TargetState = 6;

			break;

		case 7:
			creature->Flags = 0;

			if (AI.ahead)
				head = AI.angle;

			if (!AI.ahead || AI.distance > pow(SECTOR(1.5f), 2))
				item->Animation.TargetState = 3;
			else
				item->Animation.TargetState = 8;

			break;

		case 9:
			creature->Flags = 0;

			if (AI.ahead)
				head = AI.angle;

			if (!AI.ahead || AI.distance > pow(SECTOR(2), 2))
				item->Animation.TargetState = 3;
			else
				item->Animation.TargetState = 8;

			break;

		case 11:
			if (AI.ahead)
				head = AI.angle;

			creature->Flags = 0;
			if (!AI.ahead || AI.distance > pow(SECTOR(2), 2))
				item->Animation.TargetState = 3;
			else
				item->Animation.TargetState = 8;

			break;

		case 13:
			creature->Flags = 0;

			if (AI.ahead)
				head = AI.angle;

			if (!AI.ahead || AI.distance > pow(SECTOR(1), 2))
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 14;

			break;

		case 15:
			creature->Flags = 0;

			if (AI.ahead)
				head = AI.angle;

			if (!AI.ahead || AI.distance > pow(SECTOR(2), 2))
				item->Animation.TargetState = 4;
			else
				item->Animation.TargetState = 16;

			break;

		case 6:
			XianDamage(item, 75);
			break;

		case 8:
		case 10:
		case 12:
			XianDamage(item, 75);

			if (AI.ahead)
				head = AI.angle;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 2;
			}
			else
				item->Animation.TargetState = 3;

			break;

		case 14:
			XianDamage(item, 75);

			if (AI.ahead)
				head = AI.angle;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->Animation.TargetState = 1;
			else
				item->Animation.TargetState = 2;

			break;

		case 16:
			XianDamage(item, 120);

			if (AI.ahead)
				head = AI.angle;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 2;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 3;
			else
				item->Animation.TargetState = 4;

			break;
		}
	}

	if (laraAlive && LaraItem->HitPoints <= 0)
	{
		CreatureKill(item, 49, 19, 2);
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(itemNumber, angle, tilt);
}
