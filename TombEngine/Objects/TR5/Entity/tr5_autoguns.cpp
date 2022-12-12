#include "framework.h"
#include "Objects/TR5/Entity/tr5_autoguns.h"

#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR5
{
	constexpr auto AUTO_GUN_SHOT_DAMAGE			= 20;
	constexpr auto AUTO_GUN_BLOOD_EFFECT_CHANCE = 3 / 4.0f;

	constexpr auto AUTO_GUN_ORIENT_LERP_ALPHA	  = 0.1f;
	const	  auto AUTO_GUN_FIRE_CONSTRAINT_ANGLE = ANGLE(5.6f);

	const auto AutoGunFlashJoints = std::vector<unsigned int>{ 8 };
	const auto AutoGunJoints1	  = std::vector<unsigned int>{ 7, 9, 10 }; // TODO: Check what these are.

	void InitialiseAutoGuns(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.MeshBits.Set(AutoGunJoints1);
		item.Data = std::array<short, 4>();
	}

	void AutoGunsControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		auto& laraItem = *LaraItem;

		if (!TriggerActive(&item))
			return;

		if (TestLastFrame(&item))
		{
			auto& autoGun = *GetCreatureInfo(&item);

			item.MeshBits.ClearAll();
			item.MeshBits.Set(AutoGunJoints1);

			auto origin = GameVector(item.Pose.Position, item.RoomNumber);
			auto target = GameVector(GetJointPosition(&laraItem, LM_TORSO), laraItem.RoomNumber);
			bool los = LOS(&origin, &target);

			auto targetOrient = item.Pose.Orientation;
			if (los)
				targetOrient = Geometry::GetOrientToPoint(origin.ToVector3(), target.ToVector3());

			item.Pose.Orientation.Lerp(targetOrient, AUTO_GUN_ORIENT_LERP_ALPHA);
			auto deltaAngle = item.Pose.Orientation - targetOrient;

			autoGun.JointRotation[0] = item.ItemFlags[0];
			autoGun.JointRotation[1] = item.ItemFlags[1];
			autoGun.JointRotation[2] += item.ItemFlags[2];

			if (los &&
				abs(deltaAngle.x) <= AUTO_GUN_FIRE_CONSTRAINT_ANGLE &&
				abs(deltaAngle.y) <= AUTO_GUN_FIRE_CONSTRAINT_ANGLE)
			{
				SoundEffect(SFX_TR4_HK_FIRE, &item.Pose, SoundEnvironment::Land, 0.8f);

				if (GlobalCounter & 1)
				{
					item.MeshBits.Set(AutoGunFlashJoints);

					auto lightColor = Vector3(Random::GenerateFloat(0.75f, 0.85f), Random::GenerateFloat(0.5f, 0.6f), 0.0f) * 255;
					TriggerDynamicLight(origin.x, origin.y, origin.z, 10, lightColor.x, lightColor.y, lightColor.z);

					if (Random::TestProbability(AUTO_GUN_BLOOD_EFFECT_CHANCE))
					{
						auto bloodPos = GetJointPosition(&laraItem, Random::GenerateInt(0, NUM_LARA_MESHES - 1));
						DoBloodSplat(bloodPos.x, bloodPos.y, bloodPos.z, (GetRandomControl() & 3) + 3, 2 * GetRandomControl(), laraItem.RoomNumber);
						DoDamage(&laraItem, AUTO_GUN_SHOT_DAMAGE);
					}
					else
					{
						auto pos = target;

						int dx = target.x - origin.x;
						int dy = target.y - origin.y;
						int dz = target.z - origin.z;

						while (true)
						{
							if (abs(dx) >= BLOCK(12))
								break;

							if (abs(dy) >= BLOCK(12))
								break;

							if (abs(dz) >= BLOCK(12))
								break;

							dx *= 2;
							dy *= 2;
							dz *= 2;
						}

						pos.x += dx + GetRandomControl() - 128;
						pos.y += dy + GetRandomControl() - 128;
						pos.z += dz + GetRandomControl() - 128;

						if (!LOS(&origin, &pos))
							TriggerRicochetSpark(pos, 2 * GetRandomControl(), 3, 0);
					}
				}
				else
				{
					item.MeshBits.Clear(AutoGunFlashJoints);
				}

				if (item.ItemFlags[2] < 1024)
					item.ItemFlags[2] += 64;
			}
			else
			{
				if (item.ItemFlags[2])
					item.ItemFlags[2] -= 64;

				item.MeshBits.Clear(AutoGunFlashJoints);
			}

			if (item.ItemFlags[2])
				TriggerAutoGunSmoke(origin.ToVector3i(), item.ItemFlags[2] / 16);
		}
		else
		{
			item.MeshBits = 0xFFFFFAFF; // TODO: Demagic.
			AnimateItem(&item);
		}		
	}

	void TriggerAutoGunSmoke(const Vector3i& pos, char shade)
	{
		auto& spark = SmokeSparks[GetFreeSmokeSpark()];

		spark.on = true;
		spark.sShade = 0;
		spark.dShade = shade;
		spark.colFadeSpeed = 4;
		spark.fadeToBlack = 32;
		spark.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark.life = spark.sLife = (GetRandomControl() & 3) + 40;
		spark.x = pos.x - 16 + (GetRandomControl() & 0x1F);
		spark.y = (GetRandomControl() & 0x1F) + pos.y - 16;
		spark.xVel = 0;
		spark.yVel = 0;
		spark.zVel = 0;
		spark.z = (GetRandomControl() & 0x1F) + pos.z - 16;
		spark.friction = 4;
		spark.flags = SP_ROTATE;
		spark.rotAng = GetRandomControl() & 0xFFF;
		spark.rotAdd = (GetRandomControl() & 0x3F) - 31;
		spark.maxYvel = 0;
		spark.gravity = -4 - (GetRandomControl() & 3);
		spark.mirror = 0;
		spark.dSize = (GetRandomControl() & 0xF) + 24;
		spark.sSize = spark.dSize / 4;
		spark.size = spark.dSize / 4;
	}
}
