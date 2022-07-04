#include "framework.h"
#include "tr4_sentry_gun.h"
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
	auto SentryGunFlameOffset = Vector3Int(-140, 0, 0);

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

		if (item->TestBits(JointBitType::Mesh, 6)) // Was fuel can exploded?
		{
			if (item->ItemFlags[0])
			{
				auto pos = Vector3Int(SentryGunBite.x, SentryGunBite.y, SentryGunBite.z);
				GetJointAbsPosition(item, &pos, SentryGunBite.meshNum);
				TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->ItemFlags[0] + 12, 24, 16, 4);
				item->ItemFlags[0]--;
			}

			if (item->ItemFlags[0] & 1)
				item->SetBits(JointBitType::Mesh, 8);
			else
				item->ClearBits(JointBitType::Mesh, 8);

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
								ThrowFire(itemNum, 7, SentryGunFlameOffset, SentryGunFlameOffset);
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

				// Rotating gunbarrel
				item->ItemFlags[2] -= 32;
				if (item->ItemFlags[2] < 0)
					item->ItemFlags[2] = 0;

				creature->JointRotation[3] += item->ItemFlags[2];

				// Rotating radar
				creature->JointRotation[2] += item->ItemFlags[1];
				if (creature->JointRotation[2] > ANGLE(90.0f) || creature->JointRotation[2] < -ANGLE(90.0f))
					item->ItemFlags[1] = -item->ItemFlags[1];
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
			ExplodingDeath(itemNum, BODY_EXPLODE | BODY_NO_BOUNCE);
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
