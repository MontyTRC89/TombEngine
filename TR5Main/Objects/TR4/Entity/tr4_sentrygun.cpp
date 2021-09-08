#include "framework.h"
#include "tr4_sentrygun.h"
#include "box.h"
#include "effects\effects.h"
#include "items.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "level.h"
#include "lot.h"
#include "effects\tomb4fx.h"
#include "sphere.h"
#include "people.h"
#include "Sound\sound.h"
#include "trmath.h"
#include "objectslist.h"

#ifndef NEW_INV
extern Inventory g_Inventory;
#endif
BITE_INFO sentryGunBite = { 0, 0, 0, 8 };

static void SentryGunThrowFire(ITEM_INFO* item)
{
	for (int i = 0; i < 3; i++)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = 1;
		spark->sR = (GetRandomControl() & 0x1F) + 48;
		spark->sG = 48;
		spark->sB = 255;
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dG = (GetRandomControl() & 0x3F) + -128;
		spark->dB = 32;
		spark->colFadeSpeed = 12;
		spark->fadeToBlack = 8;
		spark->transType = COLADD;

		PHD_VECTOR pos1;
		pos1.x = -140;
		pos1.y = -30;
		pos1.z = -4;

		GetJointAbsPosition(item, &pos1, 7);

		spark->x = (GetRandomControl() & 0x1F) + pos1.x - 16;
		spark->y = (GetRandomControl() & 0x1F) + pos1.y - 16;
		spark->z = (GetRandomControl() & 0x1F) + pos1.z - 16;

		PHD_VECTOR pos2;
		pos2.x = -280;
		pos2.y = -30;
		pos2.z = -4;

		GetJointAbsPosition(item, &pos2, 7);

		int v = (GetRandomControl() & 0x3F) + 192;

		spark->life = spark->sLife = v / 6;

		spark->xVel = v * (pos2.x - pos1.x) / 10;
		spark->yVel = v * (pos2.y - pos1.y) / 10;
		spark->zVel = v * (pos2.z - pos1.z) / 10;

		spark->friction = 85;
		spark->gravity = -16 - (GetRandomControl() & 0x1F);
		spark->maxYvel = 0;
		spark->flags = SP_FIRE | SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

		spark->scalar = 3;
		spark->dSize = (v * ((GetRandomControl() & 7) + 60)) / 256;
		spark->sSize = spark->dSize / 4;
		spark->size = spark->dSize / 2;
	}
}

void InitialiseSentryGun(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	ClearItem(itemNum);

	item->itemFlags[0] = 0;
	item->itemFlags[1] = 768;
	item->itemFlags[2] = 0;
}

void SentryGunControl(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (!CreatureActive(itemNum))
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	AI_INFO info;
	int c = 0;

	if (creature)
	{
		// Flags set by the ID_MINE object?
		if (item->meshBits & 0x40)
		{
			if (item->itemFlags[0])
			{
				PHD_VECTOR pos;

				pos.x = sentryGunBite.x;
				pos.y = sentryGunBite.y;
				pos.z = sentryGunBite.z;

				GetJointAbsPosition(item, &pos, sentryGunBite.meshNum);

				TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->itemFlags[0] + 12, 24, 16, 4);

				item->itemFlags[0]--;
			}

			if (item->itemFlags[0] & 1)
				item->meshBits |= 0x100;
			else
				item->meshBits &= ~0x100;

			if (item->triggerFlags == 0)
			{
				item->pos.yPos -= 512;
				CreatureAIInfo(item, &info);
				item->pos.yPos += 512;

				int deltaAngle = info.angle - creature->jointRotation[0];
				
				info.ahead = true;
				if (deltaAngle <= -ANGLE(90) || deltaAngle >= ANGLE(90))
					info.ahead = false;

				if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(SECTOR(9)))
					{
#ifdef NEW_INV
						if (!have_i_got_object(ID_PUZZLE_ITEM5) && !item->itemFlags[0])
#else
						if (!g_Inventory.IsObjectPresentInInventory(ID_PUZZLE_ITEM5) && !item->itemFlags[0])
#endif
						{
							if (info.distance <= SQUARE(SECTOR(2)))
							{
								// Throw fire
								SentryGunThrowFire(item);
								c = phd_sin((GlobalCounter & 0x1F) * 2048) * 4096;
							}
							else
							{
								// Shot to Lara with bullets
								c = 0;
								item->itemFlags[0] = 2;

								ShotLara(item, &info, &sentryGunBite, creature->jointRotation[0], 5);
								SoundEffect(SFX_TR4_AUTOGUNS, &item->pos, 0);

								item->itemFlags[2] += 256;
								if (item->itemFlags[2] > 6144)
								{
									item->itemFlags[2] = 6144;
								}
							}
						}

						deltaAngle = c + info.angle - creature->jointRotation[0];
						if (deltaAngle <= ANGLE(10))
						{
							if (deltaAngle < -ANGLE(10))
							{
								deltaAngle = -ANGLE(10);
							}
						}
						else
						{
							deltaAngle = ANGLE(10);
						}

						creature->jointRotation[0] += deltaAngle;

						CreatureJoint(item, 1, -info.xAngle);
					}
				}

				item->itemFlags[2] -= 32;

				if (item->itemFlags[2] < 0)
				{
					item->itemFlags[2] = 0;
				}

				creature->jointRotation[3] += item->itemFlags[2];
				creature->jointRotation[2] += item->itemFlags[1];

				if (creature->jointRotation[2] > ANGLE(90) ||
					creature->jointRotation[2] < -ANGLE(90))
				{
					item->itemFlags[1] = -item->itemFlags[1];
				}
			}
			else
			{
				// Stuck sentry gun 
				CreatureJoint(item, 0, (GetRandomControl() & 0x7FF) - 1024);
				CreatureJoint(item, 1, ANGLE(45));
				CreatureJoint(item, 2, (GetRandomControl() & 0x3FFF) - ANGLE(45));
			}
		}
		else
		{
			ExplodingDeath(itemNum, -1, 257);
			DisableBaddieAI(itemNum);
			KillItem(itemNum);

			item->flags |= 1u;
			item->status = ITEM_DEACTIVATED;

			RemoveAllItemsInRoom(item->roomNumber, ID_SMOKE_EMITTER_BLACK);

			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos - 768, item->pos.zPos, 3, -2, 0, item->roomNumber);
			for (int i = 0; i < 2; i++)
				TriggerExplosionSparks(item->pos.xPos, item->pos.yPos - 768, item->pos.zPos, 3, -1, 0, item->roomNumber);

			SoundEffect(SFX_TR4_EXPLOSION1, &item->pos, 25165828);
			SoundEffect(SFX_TR4_EXPLOSION2, &item->pos, 0);
		}
	}
}