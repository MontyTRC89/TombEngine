#include "framework.h"
#include "Objects/TR3/Entity/tr3_fish_emitter.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
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
// ItemFlags[5] = fish count.

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

		switch (item.Pose.Orientation.y)
		{
		case ANGLE(0.0f):
			item.Pose.Orientation.z += ANGLE(3.0f);
			break;

		case ANGLE(90.0f):
			item.Pose.Orientation.x += ANGLE(3.0f);
			break;

		case ANGLE(-180.0f):
			item.Pose.Orientation.z -= ANGLE(3.0f);
			break;

		case ANGLE(-90.0f):
			item.Pose.Orientation.x -= ANGLE(3.0f);
			break;
		}

		item.StartPose.Position = item.Pose.Position;
		item.Animation.Velocity.z = Random::GenerateFloat(32.0f, 160.0f);
		item.HitPoints = DEFAULT_FISH_COUNT;
		item.ItemFlags[0] = item.Index;
		item.ItemFlags[1] = item.Index;
		item.ItemFlags[5] = 0;
	}

	void ControlFishSwarm(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		if (!CreatureActive(itemNumber))
			return;

		if (item.ItemFlags[5] != item.HitPoints)
		{
			// Calculate the difference between current and previous hit points.
			int deltaHitPoints = item.HitPoints - item.ItemFlags[5];

			if (deltaHitPoints < 0)
			{
				int fishToTurnOff = -deltaHitPoints;
				for (auto& fish : FishSwarm)
				{
					if (fish.leader == &item && fish.Life > 0.0f)
					{
						fish.Life = 0.0f;
						fishToTurnOff--;
						if (fishToTurnOff == 0)
							break;
					}
				}
			}
			else if (deltaHitPoints > 0)
			{
				for (int i = 0; i < deltaHitPoints; i++)
					SpawnFishSwarm(&item);
			}

			item.ItemFlags[5] = item.HitPoints;
		}

		GetAITarget(&creature);
		AI_INFO ai;
		CreatureAIInfo(&item, &ai);

		int dx = creature.Target.x - item.Pose.Position.x;
		int dz = creature.Target.z - item.Pose.Position.z;
		ai.distance = SQUARE(dx) + SQUARE(dz);

		item.Animation.Velocity.z = MAX_FISH_VELOCITY;

		auto& playerRoom = g_Level.Rooms[LaraItem->RoomNumber];

		auto corpsePos = std::optional<Vector3>();;

		// Check if cadaver is near.
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
		}
		// Orbit around leader item.
		else if (!corpsePos.has_value())
		{
			item.ItemFlags[1] = item.ItemFlags[0];
			corpsePos = std::nullopt;
		}

		for (auto& fish : FishSwarm)
		{
			if (fish.leader == &item)
			{
				if (fish.Life <= 0.0f)
					continue;

				fish.RoomNumber = item.RoomNumber;
				fish.target = &g_Level.Items[item.ItemFlags[1]];	
			}
		}
	}

	void SpawnFishSwarm(ItemInfo* item)
	{
		// Create new fish.
		auto& fish = GetNewEffect(FishSwarm, FISH_COUNT_MAX);

		fish.Life = 1.0f;
		fish.Pose.Position = item->Pose.Position;
		fish.Pose.Orientation.x = (GetRandomControl() & 0x3FF) - 512;
		fish.Pose.Orientation.y = (GetRandomControl() & 0x7FF) + item->Pose.Orientation.y + -ANGLE(180.0f) - 1024;
		fish.RoomNumber = item->RoomNumber;
		fish.Velocity = (GetRandomControl() & 0x1F) + 16;
		fish.Species = item->TriggerFlags < 0 ? abs(item->TriggerFlags) : item->TriggerFlags;
		fish.IsLethal = item->TriggerFlags < 0 ? true : false;
		fish.leader = &g_Level.Items[item->ItemFlags[0]];

		fish.Undulation = Random::GenerateFloat(0.0f, PI_MUL_2);
	}

	// Define bounding area for fish based on spawn point and set random target within bounds.
	static Vector3 GetRandomFishTarget(ItemInfo* item)
	{
		int XleaderBoundLeft = item->StartPose.Position.x - BLOCK(1);
		int	XleaderBoundRight = item->StartPose.Position.x + BLOCK(1);

		int	YleaderBoundUp = item->StartPose.Position.y - BLOCK(1);
		int	YleaderBoundDown = item->StartPose.Position.y + BLOCK(1);

		int	ZleaderBoundFront = item->StartPose.Position.z - BLOCK(3);
		int	ZleaderBoundBack = item->StartPose.Position.z + BLOCK(3);

		// Calculate and return random target within bounds.
		auto validTarget = Vector3(
			Random::GenerateInt(XleaderBoundLeft, XleaderBoundRight),
			Random::GenerateInt(YleaderBoundUp, YleaderBoundDown),
			Random::GenerateInt(ZleaderBoundFront, ZleaderBoundBack));

		auto pointColl = GetCollision(validTarget, item->RoomNumber);
		const auto& room = g_Level.Rooms[item->RoomNumber];

		// Prevent fish from peeking out of water surface.
		if (pointColl.RoomNumber != NO_ROOM && !TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber) || pointColl.Position.Floor < validTarget.y ||
			pointColl.Position.Ceiling > validTarget.y)
		{
			return Vector3::Zero;
		}
		else
		{
			return validTarget;
		}
	}

	void UpdateFishSwarm()
	{
		constexpr auto FLEE_VEL = 20.0f;

		static const auto SPHERE = BoundingSphere(Vector3::Zero, BLOCK(1 / 8.0f));

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

			auto& leaderItem = *fish.leader;

			if (!leaderItem.ItemFlags[2] && fish.target == fish.leader)
			{
				fish.target->Pose.Position = GetRandomFishTarget(&leaderItem);

				if (fish.target->Pose.Position != Vector3::Zero)
					leaderItem.ItemFlags[2] = 1;
			}

			//g_Renderer.AddDebugSphere(Vector3(fish.target->Pose.Position.x, fish.target->Pose.Position.y, fish.target->Pose.Position.z), 46, Vector4(1, 1, 1, 1), RendererDebugPage::None);

			int enemyVel = (fish.target != fish.leader) ? 16 : 26;

			fish.PositionTarget = Random::GeneratePointInSphere(SPHERE);

			// Calculate desired position based on target object and random offsets.
			auto desiredPos = fish.target->Pose.Position + fish.PositionTarget;
			auto dir = desiredPos - fish.Pose.Position;

			auto dirs = dir.ToVector3();
			dirs.Normalize();
			auto dirNorm = dirs;

			// Define cohesion factor to keep fish close together.
			float distToTarget = dirs.Length();

			float targetVel = (distToTarget * COHESION_FACTOR) + Random::GenerateFloat(3.0f, 5.0f);
			fish.Velocity = std::min(targetVel, fish.target->Animation.Velocity.z - 21.0f); 

			// If fish is too far from target, increase velocity to catch up.
			if (distToTarget > MAX_DISTANCE_FROM_TARGET)
				fish.Velocity += CATCH_UP_FACTOR; 

			// Move fish in direction it is pointing.
			auto rotMatrix = fish.Pose.Orientation.ToRotationMatrix();
			auto moveDir = Vector3::Transform(Vector3::UnitZ, rotMatrix);
		
			moveDir.Normalize(); 
			fish.Pose.Position += (moveDir * fish.Velocity / enemyVel);
			fish.Pose.Position += (moveDir * SPACING_FACTOR) / enemyVel;

			auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), desiredPos.ToVector3());
			fish.Pose.Orientation.Lerp(orientTo, 0.1f);

			for (const auto& otherFish : FishSwarm)
			{
				if (&fish == &otherFish)
					continue;

				float distToOtherFish = Vector3i::Distance(fish.Pose.Position, otherFish.Pose.Position);
				float distToPlayer = Vector3i::Distance(fish.Pose.Position, LaraItem->Pose.Position);
				float distToTarget = Vector3i::Distance(fish.Pose.Position, otherFish.PositionTarget);

				// Update the index of the nearest fish to the target
				if (distToTarget < minDistToTarget && fish.target == fish.leader)
				{
					minDistToTarget = distToTarget;
					closestFishPtr = &otherFish;
				}

				if (fish.target != fish.leader)
					separationDist = 80.0f;

				if (distToOtherFish < separationDist )
				{
					auto separationDir = (fish.Pose.Position - otherFish.Pose.Position).ToVector3();
					separationDir.Normalize();

					fish.Pose.Position += separationDir * (separationDist - distToOtherFish);
				}
				else
				{
				    fish.Velocity += CATCH_UP_FACTOR;
				}

				// Orient to fish nearest to target. To prevent other fish from swimming forward but orient elsewhere.
				if (fish.Pose.Orientation.x != closestFishPtr->Pose.Orientation.x && separationDist > 30.0f && fish.target == fish.leader)
				{
					separationDist--;
					auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), closestFishPtr->Pose.Position.ToVector3());
					fish.Velocity += CATCH_UP_FACTOR;
				}

				// If player is too close and fish are not lethal, steer away.
				if ((distToPlayer < separationDist * 3) && fish.IsLethal == false && LaraItem->Animation.ActiveState == LS_UNDERWATER_SWIM_FORWARD)
				{
					auto separationDir = (fish.Pose.Position - LaraItem->Pose.Position).ToVector3();
					separationDir.Normalize();

					fish.Pose.Position += separationDir * FLEE_VEL;

					auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), separationDir);
					fish.Pose.Orientation.Lerp(orientTo, 0.05f);

					fish.Velocity -= std::min(FLEE_VEL, fish.target->Animation.Velocity.z - 1.0f);
				}
			}

			auto pointColl = GetCollision(fish.Pose.Position, fish.RoomNumber);
			const auto& room = g_Level.Rooms[fish.RoomNumber];

			// Update fish room number.
			if (pointColl.RoomNumber != fish.RoomNumber)
				fish.RoomNumber = pointColl.RoomNumber;

			// Prevent fish from peeking out of water surface.
			if (pointColl.RoomNumber != fish.RoomNumber && !TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
				fish.Pose.Position.y = room.maxceiling + 180;
			
			if (ItemNearTarget(fish.Pose.Position, fish.target, CLICK(0.5f)) &&
				fish.leader != fish.target)
			{
				TriggerBlood(fish.Pose.Position.x, fish.Pose.Position.y, fish.Pose.Position.z, 4 * GetRandomControl(), 4);
				DoDamage(fish.target, FISH_HARM_DAMAGE);
			}
			else if (ItemNearTarget(fish.Pose.Position, fish.target, BLOCK(2)) && fish.leader == fish.target)
			{
				leaderItem.ItemFlags[2] = 0;
			}			
			
			// Calculate undulation angle based on sine wave and fish velocity.
			float movementValue = abs(moveDir.z);
			float undulationAngle = sin(fish.Undulation) * ANGLE(std::clamp(movementValue * 7.0f, 4.0f, 7.0f));

			fish.Pose.Orientation.y += undulationAngle;
			auto rotMatrix2 = fish.Pose.Orientation.ToRotationMatrix();

			// Update undulation.
			fish.Undulation += std::clamp(movementValue / 2, 0.3f, 1.0f);
			if (fish.Undulation > PI_MUL_2)
				fish.Undulation -= PI_MUL_2;

			auto tMatrix = Matrix::CreateTranslation(fish.Pose.Position.x, fish.Pose.Position.y, fish.Pose.Position.z);
			fish.Transform = rotMatrix2 * tMatrix;
		}
	}

	void ClearFishSwarm()
	{
		FishSwarm.clear();
	}
}
