#include "framework.h"
#include "Objects/TR1/Trap/ElectricBall.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Game/control/los.h"
#include "Game/effects/item_fx.h"
#include "Game/misc.h"
#include "Objects/Effects/Boss.h"

using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Items;
using namespace TEN::Effects::Boss;

namespace TEN::Entities::Traps
{
	// NOTES:
	// ItemFlags[0] = random turn rate when active.
	// ItemFlags[1] = calculated forward velocity.


	constexpr auto ELECTRIC_LIGHTNING_DAMAGE = 350;
	constexpr auto DAMOCLES_SWORD_DAMAGE = 100;

	constexpr auto DAMOCLES_SWORD_VELOCITY_MIN = BLOCK(1 / 20.0f);
	constexpr auto DAMOCLES_SWORD_VELOCITY_MAX = BLOCK(1 / 8.0f);

	constexpr auto DAMOCLES_SWORD_IMPALE_DEPTH			  = -BLOCK(1 / 8.0f);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_2D		  = BLOCK(1.5f);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL = BLOCK(3);

	const auto ElectricBallBite = CreatureBiteInfo(Vector3::Zero, 0);

	constexpr auto DAMOCLES_SWORD_TURN_RATE_MAX = ANGLE(5.0f);
	constexpr auto DAMOCLES_SWORD_TURN_RATE_MIN = ANGLE(1.0f);

	void InitializeElectricBall(short itemNumber)
	{

		InitializeCreature(itemNumber);
		auto& item = g_Level.Items[itemNumber];

		int sign = Random::TestProbability(0.5f) ? 1 : -1;
		item.ItemFlags[0] = Random::GenerateAngle(DAMOCLES_SWORD_TURN_RATE_MIN, DAMOCLES_SWORD_TURN_RATE_MAX) * sign;
		item.SetFlagField((int)BossItemFlags::ItemNumber, NO_VALUE);
		item.ClearFlags((int)BossItemFlags::Object, (short)BossFlagValue::NonPlayerTarget);
	}

	static std::vector<int> GetTargetEntityList(const ItemInfo& item)
	{
		auto entityList = std::vector<int>{};

		for (auto& currentEntity : g_Level.Items)
		{
			if (currentEntity.ObjectNumber == ID_ELECTRIC_BALL_IMPACT_POINT &&
				currentEntity.RoomNumber == item.RoomNumber &&
				currentEntity.TriggerFlags == item.TriggerFlags)
			{
				entityList.push_back(currentEntity.Index);
			}
		}

		return entityList;
	}

	static Vector3 GetTargetPosition(ItemInfo& item)
	{
		if (!item.TestFlagField((int)BossItemFlags::ItemNumber, NO_VALUE))
		{
			const auto& targetEntity = g_Level.Items[item.GetFlagField((int)BossItemFlags::ItemNumber)];
			return targetEntity.Pose.Position.ToVector3();
		}

		// Failsafe.
		const auto& creature = *GetCreatureInfo(&item);
		return creature.Target.ToVector3();
	}

	static int GetTargetItemNumber(const ItemInfo& item)
	{
		if (!item.TestFlags((int)BossItemFlags::Object, (short)BossFlagValue::NonPlayerTarget))
			return NO_VALUE;

		auto List = GetTargetEntityList(item);
		if (List.empty())
			return NO_VALUE;

		if (List.size() == 1)
			return List[0];
		else
			return List[Random::GenerateInt(0, (int)List.size())];
	}

	static void SpawnElectricBallLightning(ItemInfo& item, const Vector3& pos, const CreatureBiteInfo& bite)
	{
		const auto& creature = *GetCreatureInfo(&item);

		auto origin = GameVector(GetJointPosition(&item, bite), item.RoomNumber);

		auto target = GameVector(pos.x, pos.y, pos.z, creature.Enemy->RoomNumber);    //GameVector(Geometry::TranslatePoint(origin.ToVector3(), pos - origin.ToVector3(), PUNA_ATTACK_RANGE), creature.Enemy->RoomNumber);

		/*	auto origin1 = GameVector(Geometry::TranslatePoint(origin.ToVector3(), pos - origin.ToVector3(), PUNA_ATTACK_RANGE / 4), creature.Enemy->RoomNumber);
			auto origin2 = GameVector(Geometry::TranslatePoint(origin1.ToVector3(), pos - origin1.ToVector3(), PUNA_ATTACK_RANGE / 4), creature.Enemy->RoomNumber);

			auto target2 = GameVector(Geometry::TranslatePoint(origin.ToVector3(), pos - origin.ToVector3(), PUNA_ATTACK_RANGE / 6), creature.Enemy->RoomNumber);
			auto target3 = GameVector(Geometry::TranslatePoint(origin1.ToVector3(), pos - origin1.ToVector3(), PUNA_ATTACK_RANGE / 10), creature.Enemy->RoomNumber);*/

		SpawnElectricity(origin.ToVector3(), target.ToVector3(), 1, 20, 160, 160, 30, (int)(int)(int)ElectricityFlags::ThinIn | (int)ElectricityFlags::Spline | (int)ElectricityFlags::MoveEnd, 12, 12);
		SpawnElectricity(origin.ToVector3(), target.ToVector3(), 1, 80, 160, 160, 30, (int)(int)(int)ElectricityFlags::ThinIn | (int)ElectricityFlags::Spline | (int)ElectricityFlags::MoveEnd, 5, 12);

		/*

		SpawnElectricity(origin.ToVector3(), target2.ToVector3(), Random::GenerateInt(15, 40), 20, 160, 160, 20, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 4, 6);
		SpawnElectricity(origin.ToVector3(), target2.ToVector3(), Random::GenerateInt(25, 35), 20, 160, 160, 20, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 2, 7);

		SpawnElectricity(target2.ToVector3(), origin1.ToVector3(), Random::GenerateInt(15, 40), 20, 160, 160, 20, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 4, 6);
		SpawnElectricity(target2.ToVector3(), origin1.ToVector3(), Random::GenerateInt(25, 35), 20, 160, 160, 20, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 2, 7);

		SpawnElectricity(origin1.ToVector3(), target3.ToVector3(), Random::GenerateInt(15, 40), 20, 160, 160, 20, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 4, 9);
		SpawnElectricity(origin1.ToVector3(), target3.ToVector3(), Random::GenerateInt(25, 35), 20, 160, 160, 20, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 2, 10);

		SpawnElectricity(origin2.ToVector3(), target3.ToVector3(), Random::GenerateInt(15, 40), 20, 160, 160, 16, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 4, 7);
		SpawnElectricity(origin2.ToVector3(), target3.ToVector3(), Random::GenerateInt(25, 35), 20, 160, 160, 16, (int)(int)ElectricityFlags::ThinOut | (int)(int)(int)ElectricityFlags::ThinIn, 2, 8);


		TriggerDynamicLight(origin.x, origin.y, origin.z, 20, 0, 255, 255);*/

		auto hitPos = Vector3i::Zero;
		if (ObjectOnLOS2(&origin, &target, &hitPos, nullptr, ID_LARA) == creature.Enemy->Index)
		{
			if (creature.Enemy->HitPoints <= ELECTRIC_LIGHTNING_DAMAGE)
			{
				ItemElectricBurn(creature.Enemy);
				DoDamage(creature.Enemy, ELECTRIC_LIGHTNING_DAMAGE);
			}
			else
			{
				DoDamage(creature.Enemy, ELECTRIC_LIGHTNING_DAMAGE);
			}
		}

	}

	void ControlElectricBall(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& object = Objects[item.ObjectNumber];
		auto& creature = *GetCreatureInfo(&item);
		const auto& laraItem = *LaraItem;
		static auto targetPos = Vector3i::Zero;
		
		// Scan for player.
		if (item.Pose.Position.y < GetPointCollision(item).GetFloorHeight())
		{
			/*item.Pose.Orientation.y += item.ItemFlags[0];

			// Check vertical position to player.
			if (item.Pose.Position.y >= laraItem.Pose.Position.y)
				return;

			// Check vertical distance.
			float distanceV = laraItem.Pose.Position.y - item.Pose.Position.y;
			if (distanceV > DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL)
				return;

			// Check 2D distance.
			float distance2D = Vector2i::Distance(
				Vector2i(item.Pose.Position.x, item.Pose.Position.z),
				Vector2i(laraItem.Pose.Position.x, laraItem.Pose.Position.z));
			if (distance2D > DAMOCLES_SWORD_ACTIVATE_RANGE_2D)
				return;*/

			// Drop sword.
			if (item.TestFlags((int)BossItemFlags::Object, (short)BossFlagValue::NonPlayerTarget) &&
				item.TestFlagField((int)BossItemFlags::ItemNumber, NO_VALUE))
			{
				// Get random lizard item number.
				item.SetFlagField((int)BossItemFlags::ItemNumber, (short)GetTargetItemNumber(item));
				creature.Target = GetTargetPosition(item);
			}


			if (!item.TestFlagField((int)BossItemFlags::ItemNumber, NO_VALUE) && item.ItemFlags[0] == 0)
			{
				SpawnElectricBallLightning(item, targetPos.ToVector3(), ElectricBallBite);

			}

			//return;
		}
	}


	void InitializeElectricBallImpactPoint(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

	}

	


	
}
