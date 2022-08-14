#include "framework.h"
#include "Game/people.h"

#include "Game/animation.h"
#include "Game/control/los.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"

bool ShotLara(ItemInfo* item, AI_INFO* AI, BiteInfo gun, short extraRotation, int damage)
{
	return ShotLara(item, AI, &gun, extraRotation, damage);
}

// TODO: Replace with above version.
bool ShotLara(ItemInfo* item, AI_INFO* AI, BiteInfo* gun, short extraRotation, int damage)
{
	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;

	bool hit = false;
	bool targetable = false;

	if (AI->distance <= pow(MAX_VISIBILITY_DISTANCE, 2) && Targetable(item, AI))
	{
		int distance = phd_sin(AI->enemyFacing) * enemy->Animation.Velocity * pow(MAX_VISIBILITY_DISTANCE, 2) / 300;
		distance = pow(distance, 2) + AI->distance;
		if (distance <= pow(MAX_VISIBILITY_DISTANCE, 2))
		{
			int random = (pow(MAX_VISIBILITY_DISTANCE, 2) - AI->distance) / (pow(MAX_VISIBILITY_DISTANCE, 2) / 0x5000) + 8192;
			hit = GetRandomControl() < random;
		}
		else
			hit = false;
		
		targetable = true;
	}
	else
	{
		hit = false;
		targetable = false;
	}

	if (damage)
	{
		if (enemy->IsLara())
		{
			if (hit)
			{
				DoDamage(enemy, damage);
				CreatureEffect(item, gun, &GunHit);
			}
			else if (targetable)
				CreatureEffect(item, gun, &GunMiss);
		}
		else
		{
			CreatureEffect(item, gun, &GunShot);
			if (hit)
			{
				enemy->HitStatus = true;
				enemy->HitPoints += damage / -10;

				// TODO: Use TestProbability().
				int random = GetRandomControl() & 0xF;
				if (random > 14)
					random = 0;

				auto pos = Vector3Int::Zero;
				GetJointAbsPosition(enemy, &pos, random);

				DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 4, enemy->Pose.Orientation.y, enemy->RoomNumber);
			}
		}
	}

	// TODO: smash objects

	return targetable;
}

short GunMiss(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	auto pos = GameVector(
		LaraItem->Pose.Position.x + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF,
		LaraItem->Floor,
		LaraItem->Pose.Position.z + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF,
		LaraItem->RoomNumber
	);

	Richochet((PHD_3DPOS*)&pos);
	return GunShot(x, y, z, velocity, yRot, roomNumber);
}

short GunHit(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	auto pos = Vector3Int::Zero;
	GetLaraJointPosition(&pos, (25 * GetRandomControl()) >> 15);

	DoBloodSplat(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 3, LaraItem->Pose.Orientation.y, LaraItem->RoomNumber);
	return GunShot(x, y, z, velocity, yRot, roomNumber);
}

short GunShot(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	return -1;
}

bool Targetable(ItemInfo* item, AI_INFO* AI)
{
	// Discard it entity is not a creature (only creatures can use Targetable())
	// or if the target is not visible.
	if (!item->IsCreature() || !AI->ahead || AI->distance >= SQUARE(MAX_VISIBILITY_DISTANCE))
		return false;

	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;

	if (creature->Enemy == nullptr)
		return false;

	// Only Lara or a creature may be targeted.
	if ((!enemy->IsCreature() && !enemy->IsLara()) || enemy->HitPoints <= 0)
		return false;

	auto& bounds = GetBestFrame(item)->boundingBox;
	auto& boundsTarget = GetBestFrame(enemy)->boundingBox;

	auto origin = GameVector(
		item->Pose.Position.x,
		(item->ObjectNumber == ID_SNIPER) ? (item->Pose.Position.y - CLICK(3)) : (item->Pose.Position.y + ((bounds.Y2 + 3 * bounds.Y1) / 4)),
		item->Pose.Position.z,
		item->RoomNumber
	);
	auto target = GameVector(
		enemy->Pose.Position.x,
		enemy->Pose.Position.y + ((boundsTarget.Y2 + 3 * boundsTarget.Y1) / 4),
		enemy->Pose.Position.z,
		enemy->RoomNumber // TODO: Check why this line didn't exist in the first place. -- TokyoSU 2022.08.05
	);
	return LOS(&origin, &target);
}

bool TargetVisible(ItemInfo* item, AI_INFO* AI, float maxAngle)
{
	return TargetVisible(item, nullptr, AI, maxAngle);
}

bool TargetVisible(ItemInfo* item, CreatureInfo* creature, AI_INFO* AI, float maxAngle)
{
	if (!item->IsCreature() || AI->distance >= SQUARE(MAX_VISIBILITY_DISTANCE))
		return false;

	// Check just in case.
	auto* creatureInfo = (creature != nullptr) ? creature : GetCreatureInfo(item);
	if (creatureInfo == nullptr)
		return false;

	auto* enemy = creatureInfo->Enemy;
	if (enemy == nullptr || enemy->HitPoints == 0)
		return false;

	short angle = AI->angle - creatureInfo->JointRotation[2];
	if (angle > -ANGLE(maxAngle) && angle < ANGLE(maxAngle))
	{
		GameVector start;
		GameVector target;
		auto& bounds = GetBestFrame(enemy)->boundingBox;

		start.x = item->Pose.Position.x;
		start.y = item->Pose.Position.y - CLICK(3);
		start.z = item->Pose.Position.z;
		start.roomNumber = item->RoomNumber;

		target.x = enemy->Pose.Position.x;
		target.y = enemy->Pose.Position.y + ((((bounds.Y1 * 2) + bounds.Y1) + bounds.Y2) / 4);
		target.z = enemy->Pose.Position.z;
		target.roomNumber = enemy->RoomNumber; // TODO: Check why this line didn't exist before. -- TokyoSU, 10/8/2022

		return LOS(&start, &target);
	}

	return false;
}
