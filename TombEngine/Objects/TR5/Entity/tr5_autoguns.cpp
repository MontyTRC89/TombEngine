#include "framework.h"
#include "Objects/TR5/Entity/tr5_autoguns.h"

#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Sound/sound.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR5
{
	const auto AUTO_GUNS_AWARE_CONSTRAINT_ANGLE = ANGLE(5.6f);

	void InitialiseAutoGuns(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.MeshBits = 0x680;
		item.Data = std::array<short, 4>();
	}

	void AutoGunsControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		if (TestLastFrame(&item))
		{
			auto& autoGun = *GetCreatureInfo(&item);

			item.MeshBits = 0x680;

			auto pos1 = GameVector(GetJointPosition(&item, 8, Vector3i(0, 0, -64)), item.RoomNumber);
			auto pos2 = GameVector(GetJointPosition(LaraItem, LM_HIPS, Vector3i::Zero));

			auto orient = EulerAngles::Zero;

			bool los = LOS(&pos1, &pos2);
			if (los)
			{
				orient = Geometry::GetOrientToPoint(pos1.ToVector3(), pos2.ToVector3());
				orient.y -= item.Pose.Orientation.y;
			}
			else
			{
				orient.x = item.ItemFlags[1];
				orient.y = item.ItemFlags[0];
			}

			short angle1, angle2;
			InterpolateAngle(orient.x, item.ItemFlags[1], angle2, 4);
			InterpolateAngle(orient.y, *item.ItemFlags, angle1, 4);

			autoGun.JointRotation[0] = item.ItemFlags[0]; // autoGun->JointRotation
			autoGun.JointRotation[1] = item.ItemFlags[1];
			autoGun.JointRotation[2] += item.ItemFlags[2];

			if (los &&
				abs(angle1) <= AUTO_GUNS_AWARE_CONSTRAINT_ANGLE &&
				abs(angle2) <= AUTO_GUNS_AWARE_CONSTRAINT_ANGLE)
			{
				SoundEffect(SFX_TR4_HK_FIRE, &item.Pose, SoundEnvironment::Land, 0.8f);

				if (GlobalCounter & 1)
				{
					item.MeshBits |= 0x100;

					auto lightColor = Vector3((GetRandomControl() & 0x1F) + 192, (GetRandomControl() & 0x1F) + 128, 0);
					TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 10, lightColor.x, lightColor.y, lightColor.z);

					if (Random::TestProbability(3 / 4.0f))
					{
						auto pos2 = GetJointPosition(LaraItem, GetRandomControl() % 15);
						DoBloodSplat(pos2.x, pos2.y, pos2.z, (GetRandomControl() & 3) + 3, 2 * GetRandomControl(), LaraItem->RoomNumber);
						DoDamage(LaraItem, 20);
					}
					else
					{
						GameVector pos;
						pos.x = pos2.x;
						pos.y = pos2.y;
						pos.z = pos2.z;

						int dx = pos2.x - pos1.x;
						int dy = pos2.y - pos1.y;
						int dz = pos2.z - pos1.z;

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

						if (!LOS(&pos1, &pos))
							TriggerRicochetSpark(pos, 2 * GetRandomControl(), 3, 0);
					}
				}
				else
				{
					item.MeshBits &= ~0x100;
				}

				if (item.ItemFlags[2] < 1024)
					item.ItemFlags[2] += 64;
			}
			else
			{
				if (item.ItemFlags[2])
					item.ItemFlags[2] -= 64;

				item.MeshBits &= ~0x100;
			}

			if (item.ItemFlags[2])
				TriggerAutoGunSmoke(pos1.ToVector3i(), item.ItemFlags[2] / 16);
		}
		else
		{
			item.MeshBits = 0xFFFFFAFF;
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
