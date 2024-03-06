#include "framework.h"
#include "Objects/TR3/Entity/FishSwarm.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/TR3/Object/Corpse.h"
#include "Renderer/Renderer.h"
#include "Specific/level.h"

using namespace TEN::Entities::TR3;
using namespace TEN::Math;
using namespace TEN::Renderer;

// NOTES:
// HitPoints =  fish count (on spawn).
// ItemFlags[0] = leader item number.
// ItemFlags[1] = target item number.
// ItemFlags[2] = OCB rotation when in patrol mode.
// ItemFlags[3] = Start OCB of AI_FOLLOW. Do not change
// ItemFlags[5] = fish count.
// ItemFlags[6] = Does fish patrol?

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto FISH_HARM_DAMAGE = 3;
	constexpr auto MAX_FISH_VELOCITY = 10.0f;
	constexpr auto COHESION_FACTOR = 100.1f;
	constexpr auto SPACING_FACTOR = 600.0f;
	constexpr auto CATCH_UP_FACTOR = 0.2f;
	constexpr auto MAX_DISTANCE_FROM_TARGET = SQUARE(BLOCK(0.01f));
	constexpr float BASE_SEPARATION_DISTANCE = 210.0f;

	std::vector<FishData> FishSwarm = {};

	void InitializeFishSwarm(short itemNumber)
	{
		constexpr auto DEFAULT_FISH_COUNT = 24;

		auto& item = g_Level.Items[itemNumber];

		item.StartPose.Position = item.Pose.Position;
		item.Animation.Velocity.z = Random::GenerateFloat(32.0f, 160.0f);
		item.HitPoints = DEFAULT_FISH_COUNT;
		item.ItemFlags[0] = item.Index;
		item.ItemFlags[1] = item.Index;

		item.ItemFlags[5] = 0;

		if (item.AIBits)
			item.ItemFlags[6] = true;
	}

	static void SpawnFishSwarm(ItemInfo& item, CreatureInfo& creature)
	{
		constexpr auto VEL_MAX = 48.0f;
		constexpr auto VEL_MIN = 16.0f;

		// Create new fish.
		auto& fish = GetNewEffect(FishSwarm, FISH_COUNT_MAX);

		fish.Life = 1.0f;
		fish.Position = item.Pose.Position.ToVector3();
		fish.Orientation.x = Random::GenerateAngle(ANGLE(-3.0f), ANGLE(3.0f));
		fish.Orientation.y = (item.Pose.Orientation.y + ANGLE(180.0f)) + Random::GenerateAngle(ANGLE(-6.0f), ANGLE(6.0f));
		fish.RoomNumber = item.RoomNumber;
		fish.Velocity = Random::GenerateFloat(VEL_MIN, VEL_MAX);
		fish.MeshIndex = abs(item.TriggerFlags);
		fish.IsLethal = (item.TriggerFlags < 0) ? true : false;
		fish.LeaderItemPtr = &g_Level.Items[item.ItemFlags[0]];
		fish.Patrols = item.ItemFlags[6];
		fish.Undulation = Random::GenerateFloat(0.0f, PI_MUL_2);
	}

	void ControlFishSwarm(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		if (!CreatureActive(itemNumber))
			return;

		AI_INFO ai;
		CreatureAIInfo(&item, &ai);

		if (item.HitPoints != NOT_TARGETABLE)
		{
			int fishCount = item.HitPoints - item.ItemFlags[5];

			if (fishCount < 0)
			{
				int fishToTurnOff = -fishCount;
				for (auto& fish : FishSwarm)
				{
					if (fish.LeaderItemPtr == &item && fish.Life > 0.0f)
					{
						fish.Life = 0.0f;
						fishToTurnOff--;
						if (fishToTurnOff == 0)
							break;
					}
				}
			}
			else if (fishCount > 0)
			{
				for (int i = 0; i < fishCount; i++)
					SpawnFishSwarm(item, creature);
			}

			item.ItemFlags[5] = item.HitPoints;
			item.HitPoints = NOT_TARGETABLE;
		}

		int dx = creature.Target.x - item.Pose.Position.x;
		int dz = creature.Target.z - item.Pose.Position.z;
		ai.distance = SQUARE(dx) + SQUARE(dz);

		item.Animation.Velocity.z = MAX_FISH_VELOCITY;

		auto& playerRoom = g_Level.Rooms[LaraItem->RoomNumber];

		auto corpsePos = std::optional<Vector3>();;

		// Check if corpse is near.
		if (!corpsePos.has_value())
		{
			float closestDist = INFINITY;
			for (auto& targetItem : g_Level.Items)
			{
				if (!Objects.CheckID(targetItem.ObjectNumber) || targetItem.Index == itemNumber || targetItem.RoomNumber == NO_ROOM)
					continue;

				if (SameZone(&creature, &targetItem) && item.TriggerFlags < 0)
				{
					float dist = Vector3i::Distance(item.Pose.Position, targetItem.Pose.Position);
					if (dist < closestDist &&
						targetItem.ObjectNumber == ID_CORPSE &&
						targetItem.Active && TriggerActive(&targetItem) &&
						targetItem.ItemFlags[1] == (int)CorpseFlags::Lying &&
						TestEnvironment(ENV_FLAG_WATER, targetItem.RoomNumber))
					{
						corpsePos = targetItem.Pose.Position.ToVector3();
						closestDist = dist;
						item.ItemFlags[1] = targetItem.Index; // Attack corpse.
					}
				}
			}
		}

		if (ai.distance < SQUARE(BLOCK(3)) && TestEnvironment(ENV_FLAG_WATER, &playerRoom) &&
			item.TriggerFlags < 0 && !corpsePos.has_value())
		{
			item.ItemFlags[1] = LaraItem->Index;
			corpsePos = std::nullopt;
			item.ItemFlags[2] = 0;
		}

		// Circle around leader item
		else if (!corpsePos.has_value())
		{
				item.ItemFlags[1] = item.ItemFlags[0];
				corpsePos = std::nullopt;			
		}

		//Or follow a path.
		if (item.AIBits && !corpsePos.has_value())
		{
			FindAITargetObject(&creature, ID_AI_FOLLOW, item.ItemFlags[3] + item.ItemFlags[2], false);

			if (creature.AITarget->TriggerFlags == item.ItemFlags[3] + item.ItemFlags[2] && creature.AITarget->ObjectNumber == ID_AI_FOLLOW)
			{
				item.ItemFlags[1] = creature.AITarget->Index;
			}
			else
			{
				item.ItemFlags[2] = 0;
			}
			corpsePos = std::nullopt;
		}

		for (auto& fish : FishSwarm)
		{
			if (fish.LeaderItemPtr == &item)
			{
				if (fish.Life <= 0.0f)
					continue;

				fish.RoomNumber = item.RoomNumber;
				fish.TargetItemPtr = &g_Level.Items[item.ItemFlags[1]];	
			}
		}
	}

	static Vector3 GetFishStartPosition(const ItemInfo& item)
	{
		constexpr auto SPHERE_RADIUS		= BLOCK(4);
		constexpr auto DISTANCE_BUFFER = CLICK(0.5f);

		auto sphere = BoundingSphere(item.StartPose.Position.ToVector3(), SPHERE_RADIUS);
		auto pos = Random::GeneratePointInSphere(sphere);

		// Get point collision.
		auto pointColl = GetCollision(pos, item.RoomNumber);
		int waterHeight = GetWaterHeight(pointColl.Coordinates.x, pointColl.Coordinates.y, pointColl.Coordinates.z, pointColl.RoomNumber);

		if (pointColl.RoomNumber == NO_ROOM || 
			!(TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber)) ||
			pos.y >= pointColl.Position.Floor - DISTANCE_BUFFER ||
			pos.y <= waterHeight + DISTANCE_BUFFER ||
			pointColl.Block->IsWall(item.Pose.Position.x + DISTANCE_BUFFER, item.Pose.Position.z + DISTANCE_BUFFER) ||
			pointColl.Block->IsWall(item.Pose.Position.x - DISTANCE_BUFFER, item.Pose.Position.z - DISTANCE_BUFFER))

			return Vector3::Zero;
		else
			return pos;
	}

	void UpdateFishSwarm()
	{
		constexpr auto WATER_SURFACE_OFFSET = CLICK(0.5f);
		constexpr auto FLEE_VEL				= 20.0f;

		static const auto SPHERE = BoundingSphere(Vector3::Zero, BLOCK(1 / 8.0f));

		if (FishSwarm.empty())
			return;

		const FishData* closestFishPtr = nullptr;
		float minDistToTarget = INFINITY;
		int minDist = INT_MAX;

		int fishNumber = 0;
		for (auto& fish : FishSwarm)
		{
			if (fish.Life <= 0.0f)
				continue;

			// Increase separation distance for each fish.
			float separationDist = BASE_SEPARATION_DISTANCE + (fishNumber * 3);
			fishNumber += 1;

			auto& leaderItem = *fish.LeaderItemPtr;

			if (!leaderItem.ItemFlags[2] && fish.TargetItemPtr == fish.LeaderItemPtr)
			{
				if (!fish.Patrols)
				{
					fish.TargetItemPtr->Pose.Position = GetFishStartPosition(leaderItem);

					if (fish.TargetItemPtr->Pose.Position != Vector3::Zero)
						leaderItem.ItemFlags[2] = 1;
				}
			}

			int enemyVel = (fish.TargetItemPtr != fish.LeaderItemPtr) ? 16 : 26;

			fish.PositionTarget = Random::GeneratePointInSphere(SPHERE);

			// Calculate desired position based on target object and random offsets.
			auto desiredPos = fish.TargetItemPtr->Pose.Position + fish.PositionTarget;
			auto dir = desiredPos - fish.Position;

			auto dirs = dir.ToVector3();
			dirs.Normalize();
			auto dirNorm = dirs;

			// Define cohesion factor to keep fish close together.
			float distToTarget = dirs.Length();

			float targetVel = (distToTarget * COHESION_FACTOR) + Random::GenerateFloat(3.0f, 5.0f);
			fish.Velocity = std::min(targetVel, fish.TargetItemPtr->Animation.Velocity.z - 21.0f); 

			// If fish is too far from target, increase velocity to catch up.
			if (distToTarget > MAX_DISTANCE_FROM_TARGET)
				fish.Velocity += CATCH_UP_FACTOR; 

			// Translate.
			auto moveDir = fish.Orientation.ToDirection();
			moveDir.Normalize(); 
			fish.Position += (moveDir * fish.Velocity / enemyVel);
			fish.Position += (moveDir * SPACING_FACTOR) / enemyVel;

			auto orientTo = Geometry::GetOrientToPoint(fish.Position, desiredPos.ToVector3());
			fish.Orientation.Lerp(orientTo, 0.1f);

			for (const auto& otherFish : FishSwarm)
			{
				if (&fish == &otherFish)
					continue;

				float distToOtherFish = Vector3i::Distance(fish.Position, otherFish.Position);
				float distToPlayer = Vector3i::Distance(fish.Position, LaraItem->Pose.Position);
				float distToTarget = Vector3i::Distance(fish.Position, otherFish.PositionTarget);

				// Update the index of the nearest fish to the target
				if (distToTarget < minDistToTarget && (fish.TargetItemPtr == fish.LeaderItemPtr || fish.TargetItemPtr->ObjectNumber == ID_AI_FOLLOW))
				{
					minDistToTarget = distToTarget;
					closestFishPtr = &otherFish;
				}

				if (fish.TargetItemPtr != fish.LeaderItemPtr && fish.TargetItemPtr->ObjectNumber != ID_AI_FOLLOW)
					separationDist = 80.0f;

				if (distToOtherFish < separationDist )
				{
					auto separationDir = fish.Position - otherFish.Position;
					separationDir.Normalize();

					fish.Position += separationDir * (separationDist - distToOtherFish);
				}
				else
				{
				    fish.Velocity += CATCH_UP_FACTOR;
				}

				// Orient to fish nearest to target. To prevent other fish from swimming forward but orient elsewhere.
				if (fish.Orientation.x != closestFishPtr->Orientation.x && separationDist > 30.0f && 
					(fish.TargetItemPtr == fish.LeaderItemPtr || fish.TargetItemPtr->ObjectNumber == ID_AI_FOLLOW))
				{
					separationDist--;
					auto orientTo = Geometry::GetOrientToPoint(fish.Position, closestFishPtr->Position);
					fish.Velocity += CATCH_UP_FACTOR;
				}

				// If player is too close and fish are not lethal, steer away.
				if ((distToPlayer < separationDist * 3) && fish.IsLethal == false && LaraItem->Animation.ActiveState == LS_UNDERWATER_SWIM_FORWARD)
				{
					auto separationDir = fish.Position - LaraItem->Pose.Position.ToVector3();
					separationDir.Normalize();

					fish.Position += separationDir * FLEE_VEL;

					auto orientTo = Geometry::GetOrientToPoint(fish.Position, separationDir);
					fish.Orientation.Lerp(orientTo, 0.05f);

					fish.Velocity -= std::min(FLEE_VEL, fish.TargetItemPtr->Animation.Velocity.z - 1.0f);
				}
			}

			auto pointColl = GetCollision(fish.Position, fish.RoomNumber);
			const auto& room = g_Level.Rooms[fish.RoomNumber];

			// Update fish room number.
			if (pointColl.RoomNumber != fish.RoomNumber)
				fish.RoomNumber = pointColl.RoomNumber;

			// Clamp position to slightly below water surface.
			int waterHeight = GetWaterHeight(fish.Position.x, fish.Position.y, fish.Position.z, fish.RoomNumber);
			if (fish.Position.y < (waterHeight + WATER_SURFACE_OFFSET))
				fish.Position.y = waterHeight + WATER_SURFACE_OFFSET;
			
			if (ItemNearTarget(fish.Position, fish.TargetItemPtr, CLICK(0.5f)) &&
				fish.LeaderItemPtr != fish.TargetItemPtr)
			{
				if (fish.TargetItemPtr->ObjectNumber != ID_AI_FOLLOW)
				{
					TriggerBlood(fish.Position.x, fish.Position.y, fish.Position.z, 4 * GetRandomControl(), 4);
					DoDamage(fish.TargetItemPtr, FISH_HARM_DAMAGE);
				}
				else 
					leaderItem.ItemFlags[2]++;
			}
			else if (ItemNearTarget(fish.Position, fish.TargetItemPtr, BLOCK(2)) && fish.LeaderItemPtr == fish.TargetItemPtr)
			{
					leaderItem.ItemFlags[2] = 0;
			}			
			
			// Calculate undulation angle based on sine wave and fish velocity.
			float movementValue = abs(moveDir.z);
			float undulationAngle = sin(fish.Undulation) * ANGLE(std::clamp(movementValue * 7.0f, 4.0f, 7.0f));

			// Upply undulation.
			fish.Orientation.y += undulationAngle;

			// Update undulation.
			fish.Undulation += std::clamp(movementValue / 2, 0.3f, 1.0f);
			if (fish.Undulation > PI_MUL_2)
				fish.Undulation -= PI_MUL_2;
		}
	}

	void ClearFishSwarm()
	{
		FishSwarm.clear();
	}
}
