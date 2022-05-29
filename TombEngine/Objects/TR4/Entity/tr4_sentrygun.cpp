#include "framework.h"
#include "tr4_sentrygun.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/gui.h"
#include "Specific/level.h"
#include "Game/control/lot.h"
#include "Game/effects/tomb4fx.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Objects/objectslist.h"
#include "Game/itemdata/creature_info.h"
#include "Game/animation.h"
#include "Game/misc.h"

namespace TEN::Entities::TR4
{
	BITE_INFO SentryGunBite = { 0, 0, 0, 8 };

	static void SentryGunThrowFire(ItemInfo* item)
	{
		for (int i = 0; i < 3; i++)
		{
			auto* spark = &Sparks[GetFreeSpark()];

			spark->on = 1;
			spark->sR = (GetRandomControl() & 0x1F) + 48;
			spark->sG = 48;
			spark->sB = 255;
			spark->dR = (GetRandomControl() & 0x3F) - 64;
			spark->dG = (GetRandomControl() & 0x3F) + -128;
			spark->dB = 32;
			spark->colFadeSpeed = 12;
			spark->fadeToBlack = 8;
			spark->transType = TransTypeEnum::COLADD;

			auto pos1 = Vector3Int(-140, -30, -4);
			GetJointAbsPosition(item, &pos1, 7);

			spark->x = (GetRandomControl() & 0x1F) + pos1.x - 16;
			spark->y = (GetRandomControl() & 0x1F) + pos1.y - 16;
			spark->z = (GetRandomControl() & 0x1F) + pos1.z - 16;

			auto pos2 = Vector3Int(-280, -30, -4);
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
		auto* item = &g_Level.Items[itemNum];

		ClearItem(itemNum);

		item->ItemFlags[0] = 0;
		item->ItemFlags[1] = 768;
		item->ItemFlags[2] = 0;
	}

	void SentryGunControl(short itemNum)
	{
		auto* item = &g_Level.Items[itemNum];

		if (!CreatureActive(itemNum))
			return;

		auto* creature = GetCreatureInfo(item);

		int c = 0;

		if (!creature)
			return;

		// Flags set by the ID_MINE object?
		if (item->MeshBits & 0x40)
		{
			if (item->ItemFlags[0])
			{
				auto pos = Vector3Int(SentryGunBite.x, SentryGunBite.y, SentryGunBite.z);
				GetJointAbsPosition(item, &pos, SentryGunBite.meshNum);

				TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->ItemFlags[0] + 12, 24, 16, 4);

				item->ItemFlags[0]--;
			}

			if (item->ItemFlags[0] & 1)
				item->MeshBits |= 0x100;
			else
				item->MeshBits &= ~0x100;

			if (item->TriggerFlags == 0)
			{
				item->Pose.Position.y -= CLICK(2);

				AI_INFO AI;;
				CreatureAIInfo(item, &AI);

				item->Pose.Position.y += CLICK(2);

				int deltaAngle = AI.angle - creature->JointRotation[0];

				AI.ahead = true;
				if (deltaAngle <= -ANGLE(90.0f) || deltaAngle >= ANGLE(90.0f))
					AI.ahead = false;

				if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(9), 2))
					{
						if (!g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM5) && !item->ItemFlags[0])
						{
							if (AI.distance <= pow(SECTOR(2), 2))
							{
								// Throw fire
								SentryGunThrowFire(item);
								c = phd_sin((GlobalCounter & 0x1F) * 2048) * 4096;
							}
							else
							{
								// Shot to Lara with bullets
								c = 0;
								item->ItemFlags[0] = 2;

								ShotLara(item, &AI, &SentryGunBite, creature->JointRotation[0], 5);
								SoundEffect(SFX_TR4_AUTOGUNS, &item->Pose);

								item->ItemFlags[2] += 256;
								if (item->ItemFlags[2] > 6144)
									item->ItemFlags[2] = 6144;
							}
						}

						deltaAngle = c + AI.angle - creature->JointRotation[0];
						if (deltaAngle <= ANGLE(10.0f))
						{
							if (deltaAngle < -ANGLE(10.0f))
								deltaAngle = -ANGLE(10.0f);
						}
						else
							deltaAngle = ANGLE(10.0f);

						creature->JointRotation[0] += deltaAngle;

						CreatureJoint(item, 1, -AI.xAngle);
					}
				}

				item->ItemFlags[2] -= 32;

				if (item->ItemFlags[2] < 0)
					item->ItemFlags[2] = 0;

				creature->JointRotation[3] += item->ItemFlags[2];
				creature->JointRotation[2] += item->ItemFlags[1];

				if (creature->JointRotation[2] > ANGLE(90.0f) ||
					creature->JointRotation[2] < -ANGLE(90.0f))
				{
					item->ItemFlags[1] = -item->ItemFlags[1];
				}
			}
			else
			{
				// Stuck sentry gun 
				CreatureJoint(item, 0, (GetRandomControl() & 0x7FF) - 1024);
				CreatureJoint(item, 1, ANGLE(45.0f));
				CreatureJoint(item, 2, (GetRandomControl() & 0x3FFF) - ANGLE(45.0f));
			}
		}
		else
		{
			ExplodingDeath(itemNum, -1, 257);
			DisableEntityAI(itemNum);
			KillItem(itemNum);

			item->Flags |= 1u;
			item->Status = ITEM_DEACTIVATED;

			RemoveAllItemsInRoom(item->RoomNumber, ID_SMOKE_EMITTER_BLACK);

			TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y - CLICK(3), item->Pose.Position.z, 3, -2, 0, item->RoomNumber);
			for (int i = 0; i < 2; i++)
				TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y - CLICK(3), item->Pose.Position.z, 3, -1, 0, item->RoomNumber);

			SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose, SoundEnvironment::Land, 1.5f);
			SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose);
		}
	}
}
