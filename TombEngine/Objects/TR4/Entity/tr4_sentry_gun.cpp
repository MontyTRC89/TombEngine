#include "framework.h"
#include "Objects/TR4/Entity/tr4_sentry_gun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Gui.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Objects/objectslist.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Gui;

// NOTES:
// item.ItemFlags[0] = Duration of the shooting flash light
// item.ItemFlags[1] = Angle of Radar mesh
// item.ItemFlags[2] = Angle of gun Barrel mesh
// item.ItemFlags[3] = ID of the item that deactivate the sentry gun if it's in Lara's inventory.

namespace TEN::Entities::TR4
{
	constexpr auto SENTRYGUN_DAMAGE_SHOOT = 5;

	const auto SentryGunRadarJoints = std::vector<unsigned int>{ 3 };
	const auto SentryGunBarrelJoints = std::vector<unsigned int>{ 4 };
	const auto SentryGunWeaknessJoints = std::vector<unsigned int>{ 6 };
	const auto SentryGunFlashJoints = std::vector<unsigned int>{ 8 };

	const auto SentryGunBodyJoints = std::vector<unsigned int>{ 7, 9 };
	

	const auto SentryGunFlameOffset = Vector3i(-140, 0, 0);
	const auto SentryGunBite = CreatureBiteInfo(Vector3::Zero, 8);

	void InitializeSentryGun(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitializeCreature(itemNumber);

		auto& flashLightTimer			= item.ItemFlags[0];
		auto& angleRadar				= item.ItemFlags[1];
		auto& angleGunBarrel			= item.ItemFlags[2];
		auto& deactivatorItemID		= item.ItemFlags[3];

		flashLightTimer = 0;
		angleRadar = 768;
		angleGunBarrel = 0;

		if (deactivatorItemID == 0)
			deactivatorItemID = ID_PUZZLE_ITEM5;
	}

	void SentryGunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(&item);

		auto& flashLightTimer			= item.ItemFlags[0];
		auto& angleRadar				= item.ItemFlags[1];
		auto& angleGunBarrel			= item.ItemFlags[2];
		auto& deactivatorItemID		= item.ItemFlags[3];

		int anglePitchBody = 0;

		if (!creature)
			return;

		// Was fuel can exploded?
		if (item.MeshBits.Test(SentryGunWeaknessJoints))
		{
			if (flashLightTimer)
			{
				auto pos = GetJointPosition(item, SentryGunBite);
				TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * flashLightTimer + 12, 24, 16, 4);
				flashLightTimer--;
			}

			if (flashLightTimer & 1)
				item.MeshBits.Set(SentryGunFlashJoints);
			else
				item.MeshBits.Clear(SentryGunFlashJoints);

			if (item.TriggerFlags == 0)
			{
				item.Pose.Position.y -= CLICK(2);

				AI_INFO AI;
				CreatureAIInfo(&item, &AI);

				item.Pose.Position.y += CLICK(2);

				int deltaAngle = AI.angle - creature->JointRotation[0];

				AI.ahead = true;
				if (deltaAngle <= -ANGLE(90.0f) || deltaAngle >= ANGLE(90.0f))
					AI.ahead = false;

				if (Targetable(&item, &AI))
				{
					if (AI.distance < pow(BLOCK(9), 2))
					{
						if (!g_Gui.IsObjectInInventory(deactivatorItemID) && !flashLightTimer)
						{
							if (AI.distance <= pow(BLOCK(2), 2))
							{
								// Throw fire
								ThrowFire(itemNumber, 7, SentryGunFlameOffset, SentryGunFlameOffset);
								anglePitchBody = phd_sin((GlobalCounter & 0x1F) * 2048) * 4096;
							}
							else
							{
								// Shot to Lara with bullets
								anglePitchBody = 0;
								flashLightTimer = 2;

								ShotLara(&item, &AI, SentryGunBite, creature->JointRotation[0], SENTRYGUN_DAMAGE_SHOOT);
								SoundEffect(SFX_TR4_AUTOGUNS, &item.Pose);

								angleGunBarrel += 256;
								if (angleGunBarrel > 6144)
									angleGunBarrel = 6144;
							}
						}

						deltaAngle = anglePitchBody + AI.angle - creature->JointRotation[0];
						if (deltaAngle <= ANGLE(10.0f))
						{
							if (deltaAngle < -ANGLE(10.0f))
								deltaAngle = -ANGLE(10.0f);
						}
						else
							deltaAngle = ANGLE(10.0f);

						creature->JointRotation[0] += deltaAngle;

						CreatureJoint(&item, 1, -AI.xAngle);
					}
				}

				// Rotating gunbarrel
				angleGunBarrel -= 32;
				if (angleGunBarrel < 0)
					angleGunBarrel = 0;

				creature->JointRotation[3] += angleGunBarrel;

				// Rotating radar
				creature->JointRotation[2] += angleRadar;
				if (creature->JointRotation[2] > ANGLE(90.0f) || creature->JointRotation[2] < -ANGLE(90.0f))
					angleRadar = -angleRadar;
			}
			else
			{
				// Stuck sentry gun 
				CreatureJoint(&item, 0, (GetRandomControl() & 0x7FF) - 1024);
				CreatureJoint(&item, 1, ANGLE(45.0f));
				CreatureJoint(&item, 2, (GetRandomControl() & 0x3FFF) - ANGLE(45.0f));
			}
		}
		else
		{
			ExplodingDeath(itemNumber, BODY_DO_EXPLOSION | BODY_NO_BOUNCE);
			DisableEntityAI(itemNumber);
			KillItem(itemNumber);

			item.Flags |= 1u;
			item.Status = ITEM_DEACTIVATED;

			RemoveAllItemsInRoom(item.RoomNumber, ID_SMOKE_EMITTER_BLACK);

			TriggerExplosionSparks(item.Pose.Position.x, item.Pose.Position.y - CLICK(3), item.Pose.Position.z, 3, -2, 0, item.RoomNumber);
			for (int i = 0; i < 2; i++)
				TriggerExplosionSparks(item.Pose.Position.x, item.Pose.Position.y - CLICK(3), item.Pose.Position.z, 3, -1, 0, item.RoomNumber);

			SoundEffect(SFX_TR4_EXPLOSION1, &item.Pose, SoundEnvironment::Land, 1.5f);
			SoundEffect(SFX_TR4_EXPLOSION2, &item.Pose);
		}
	}
}
