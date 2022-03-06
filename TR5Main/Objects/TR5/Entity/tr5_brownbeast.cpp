#include "framework.h"
#include "tr5_brownbeast.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO BrownBeastBite1 = { 0, 0, 0, 16 };
BITE_INFO BrownBeastBite2 = { 0, 0, 0, 22 };

// TODO
enum BrownBeastState
{

};

// TODO
enum BrownBeastAnim
{

};

void InitialiseBrownBeast(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	
	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = 1;
	item->ActiveState = 1;
}

void ControlBrowsBeast(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;
	
	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short angle  = 0;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->ActiveState != 7)
		{
			item->AnimNumber = Objects[ID_BROWN_BEAST].animIndex + 10;
			item->ActiveState = 7;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(info);
		else if (info->HurtByLara)
			info->Enemy = LaraItem;

		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		int distance;

		if (info->Enemy == LaraItem)
			distance = aiInfo.distance;
		else
		{
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;
			phd_atan(dz, dz);

			distance = pow(dx, 2) + pow(dz, 2);
		}

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);
		info->MaxTurn = ANGLE(7.0f);

		switch (item->ActiveState)
		{
		case 1:
			info->Flags = 0;
			if (info->Mood == MoodType::Attack)
			{
				if (distance <= pow(SECTOR(1), 2))
				{
					if (GetRandomControl() & 1)
						item->TargetState = 4;
					else
						item->TargetState = 6;
				}
				else if (GetRandomControl() & 1)
					item->TargetState = 2;
				else
					item->TargetState = 3;
			}
			else
				item->TargetState = 1;
			
			break;

		case 2:
		case 3:
			if (distance < pow(SECTOR(1), 2) || info->Mood != MoodType::Attack)
				item->TargetState = 1;

			SoundEffect(SFX_TR5_IMP_BARREL_ROLL, &item->Position, 0);
			break;

		case 4:
		case 6:
			info->MaxTurn = 0;

			if (abs(aiInfo.angle) >= ANGLE(2.0f))
			{
				if (aiInfo.angle > 0)
					item->Position.yRot += ANGLE(2.0f);
				else
					item->Position.yRot -= ANGLE(2.0f);
			}
			else
				item->Position.yRot += aiInfo.angle;

			if (info->Flags)
				break;

			if (item->TouchBits & 0x3C000)
			{
				if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 19 &&
						item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 25)
					{
						CreatureEffect2(item, &BrownBeastBite1, 20, item->Position.yRot, DoBloodSplat);
						info->Flags |= 1;

						LaraItem->HitPoints -= 150;
						LaraItem->HitStatus = true;
						break;
					}
				}

				if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 6 &&
						item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 16)
					{
						CreatureEffect2(item, &BrownBeastBite1, 20, item->Position.yRot, DoBloodSplat);
						info->Flags |= 1;

						LaraItem->HitPoints -= 150;
						LaraItem->HitStatus = true;
						break;
					}
				}
			}

			if (!(item->TouchBits & 0xF00000))
				break;

			if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
			{
				if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 13 &&
					item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 20)
				{
					CreatureEffect2(item, &BrownBeastBite2, 20, item->Position.yRot, DoBloodSplat);
					info->Flags |= 2;

					LaraItem->HitPoints -= 150;
					LaraItem->HitStatus = true;
					break;
				}
			}

			if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
			{
				if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 33 &&
					item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 43)
				{
					CreatureEffect2(item, &BrownBeastBite2, 20, item->Position.yRot, DoBloodSplat);
					info->Flags |= 2;

					LaraItem->HitPoints -= 150;
					LaraItem->HitStatus = true;
					break;
				}
			}

			break;

		default:
			break;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
