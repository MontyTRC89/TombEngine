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
		if (enemy == LaraItem)
		{
			if (hit)
			{
				CreatureEffect(item, gun, &GunHit);
				DoDamage(LaraItem, damage);
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

				int random = GetRandomControl() & 0xF;
				if (random > 14)
					random = 0;

				Vector3Int pos = { 0, 0, 0 };
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
	GameVector pos;
	pos.x = LaraItem->Pose.Position.x + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.y = LaraItem->Floor;
	pos.z = LaraItem->Pose.Position.z + ((GetRandomControl() - 0x4000) << 9) / 0x7FFF;
	pos.roomNumber = LaraItem->RoomNumber;

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

bool Targetable(ItemInfo* item, CreatureInfo* creature, AI_INFO* AI)
{
	// Check if entity is a creature (only creatures can use Targetable()).
	// and whether target is ahead or at a visible distance.
	if (!item->IsCreature() || !AI->ahead || AI->distance >= SQUARE(MAX_VISIBILITY_DISTANCE))
		return false;

	auto* enemy = (creature != nullptr) ? creature->Enemy : GetCreatureInfo(item)->Enemy;
	if (enemy == nullptr)
		return false;

	// NOTE: we need at last creature or lara so || is required, it can't be both !
	if ((!enemy->IsCreature() || !enemy->IsLara()) || enemy->HitPoints <= 0)
		return false;

	GameVector start;
	GameVector target;
	auto& bounds = GetBestFrame(item)->boundingBox;
	auto& boundsTarget = GetBestFrame(enemy)->boundingBox;

	start.x = item->Pose.Position.x;
	start.y = (item->ObjectNumber == ID_SNIPER) ? (item->Pose.Position.y - CLICK(3)) : (item->Pose.Position.y + ((bounds.Y2 + 3 * bounds.Y1) / 4));
	start.z = item->Pose.Position.z;
	start.roomNumber = item->RoomNumber;

	target.x = enemy->Pose.Position.x;
	target.y = enemy->Pose.Position.y + ((boundsTarget.Y2 + 3 * boundsTarget.Y1) / 4);
	target.z = enemy->Pose.Position.z;
	target.roomNumber = enemy->RoomNumber; // NOTE: why do this line not existed ? TokyoSU, 5/8/2022

	return LOS(&start, &target);
}

bool Targetable(ItemInfo* item, AI_INFO* AI)
{
	return Targetable(item, nullptr, AI);
}

bool TargetVisible(ItemInfo* item, AI_INFO* AI, float maxAngle)
{
	return TargetVisible(item, nullptr, AI, maxAngle);
}

bool TargetVisible(ItemInfo* item, CreatureInfo* creature, AI_INFO* AI, float maxAngle)
{
	if (!item->IsCreature() || AI->distance >= SQUARE(MAX_VISIBILITY_DISTANCE))
		return false;

	auto* creatureInfo = (creature != nullptr) ? creature : GetCreatureInfo(item);
	// NOTE: normally, it will always pass here (since we call GetCreatureInfo())
	// But just in case we check it, since GetCreatureInfo() not check for nullptr !
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
		target.roomNumber = enemy->RoomNumber; // NOTE: why do this line not existed ? TokyoSU, 10/8/2022

		return LOS(&start, &target);
	}

	return false;
}
