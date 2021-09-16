#include "framework.h"
#include "tr5_autoguns.h"
#include "sphere.h"
#include "lara.h"
#include "animation.h"
#include "control\los.h"
#include "effects\effects.h"
#include "effects\tomb4fx.h"
#include "level.h"
#include "Sound\sound.h"

void InitialiseAutoGuns(short itemNumber)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNumber];
    item->meshBits = 1024;
    //5702 bytes!?
	//item->data = game_malloc<uint8_t>(5702);
	item->data = std::array<short,4>();
	
}

static void TriggerAutoGunSmoke(PHD_VECTOR* pos, char shade)
{
	SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];

	spark->on = 1;
	spark->sShade = 0;
	spark->dShade = shade;
	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 32;
	spark->transType = COLADD;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 40;
	spark->x = pos->x - 16 + (GetRandomControl() & 0x1F);
	spark->y = (GetRandomControl() & 0x1F) + pos->y - 16;
	spark->xVel = 0;
	spark->yVel = 0;
	spark->zVel = 0;
	spark->z = (GetRandomControl() & 0x1F) + pos->z - 16;
	spark->friction = 4;
	spark->flags = SP_ROTATE;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 31;
	spark->maxYvel = 0;
	spark->gravity = -4 - (GetRandomControl() & 3);
	spark->mirror = 0;
	spark->dSize = (GetRandomControl() & 0xF) + 24;
	spark->sSize = spark->dSize / 4;
	spark->size = spark->dSize / 4;
}

void AutoGunsControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->frameNumber >= g_Level.Anims[item->animNumber].frameEnd)
		{
			std::array<short, 4>& data = item->data;

			item->meshBits = 1664;

			GAME_VECTOR pos1;
			pos1.x = 0;
			pos1.y = 0;
			pos1.z = -64;
			GetJointAbsPosition(item, (PHD_VECTOR*)&pos1, 8);

			GAME_VECTOR pos2;
			pos2.x = 0;
			pos2.y = 0;
			pos2.z = 0;
			GetLaraJointPosition((PHD_VECTOR*)&pos2, 0);

			pos1.roomNumber = item->roomNumber;

			short angles[2];

			int los = LOS(&pos1, &pos2);

			// FIXME:
			if (los)
			{
				phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);
				angles[0] -= item->pos.yRot;
			}
			else
			{
				angles[0] = item->itemFlags[0];
				angles[1] = item->itemFlags[1];
			}

			short angle1, angle2;

			InterpolateAngle(angles[0], item->itemFlags, &angle1, 4);
			InterpolateAngle(angles[1], &item->itemFlags[1], &angle2, 4);

			data[0] = item->itemFlags[0];
			data[1] = item->itemFlags[1];
			data[2] += item->itemFlags[2];

			if (abs(angle1) < 1024 && abs(angle2) < 1024 && los)
			{
				SoundEffect(SFX_TR5_HK_FIRE, &item->pos, 0xC00004);

				if (GlobalCounter & 1)
				{
					item->meshBits |= 0x100;

					TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 10, (GetRandomControl() & 0x1F) + 192, (GetRandomControl() & 0x1F) + 128, 0);

					if (GetRandomControl() & 3)
					{
						pos2.x = 0;
						pos2.y = 0;
						pos2.z = 0;
						GetLaraJointPosition((PHD_VECTOR*)& pos2, GetRandomControl() % 15);

						DoBloodSplat(pos2.x, pos2.y, pos2.z, (GetRandomControl() & 3) + 3, 2 * GetRandomControl(), LaraItem->roomNumber);
						LaraItem->hitPoints -= 20;
					}
					else
					{
						GAME_VECTOR pos;
						pos.x = pos2.x;
						pos.y = pos2.y;
						pos.z = pos2.z;

						int dx = pos2.x - pos1.x;
						int dy = pos2.y - pos1.y;
						int dz = pos2.z - pos1.z;

						while (true)
						{
							if (abs(dx) >= 12288)
								break;
							if (abs(dy) >= 12288)
								break;
							if (abs(dz) >= 12288)
								break;
							dx *= 2;
							dy *= 2;
							dz *= 2;
						}

						pos.x += dx + GetRandomControl() - 128;
						pos.y += dy + GetRandomControl() - 128;
						pos.z += dz + GetRandomControl() - 128;

						if (!LOS(&pos1, &pos))
							TriggerRicochetSpark(&pos, 2 * GetRandomControl(), 3, 0);
					}
				}
				else
				{
					item->meshBits &= ~0x100;
				}

				if (item->itemFlags[2] < 1024)
					item->itemFlags[2] += 64;
			}
			else
			{
				if (item->itemFlags[2])
					item->itemFlags[2] -= 64;
				item->meshBits &= ~0x100;
			}

			if (item->itemFlags[2])
				TriggerAutoGunSmoke((PHD_VECTOR*)&pos1, item->itemFlags[2] / 16);
		}
		else
		{
			item->meshBits = -1281;
			AnimateItem(item);
		}
	}
}