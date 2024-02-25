#include "framework.h"
#include "Objects/TR3/Entity/tr3_fish_emitter.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/flipeffect.h"
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

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto FISH_HARM_DAMAGE = 3;
	constexpr auto FISH_ENTITY_DAMAGE = 1;
	constexpr auto MAX_FISH_VELOCITY = 10.0f;
	constexpr auto LEADER_REACH_TARGET_RANGE = SQUARE(BLOCK(0.4f));
	constexpr auto FISH_ORIENT_LERP_ALPHA = 0.1f;

	FishData FishSwarm[NUM_FISHES];
	int NextFish;

	void InitializeFishSwarm(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (item.Pose.Orientation.y == 0)
		{
			item.Pose.Orientation.z += CLICK(2);
		}
		else if (item.Pose.Orientation.y == ANGLE(90.0f))
		{
			item.Pose.Orientation.x += CLICK(2);
		}
		else if (item.Pose.Orientation.y == ANGLE(-180.0f))
		{
			item.Pose.Orientation.z -= CLICK(2);
		}
		else if (item.Pose.Orientation.y == ANGLE(-90.0f))
		{
			item.Pose.Orientation.x -= CLICK(2);
		}

		item.HitPoints = 24; // NOTE: HitPoints stores fish count.
		item.ItemFlags[0] = item.Index; // Save leaderitem for little fishes into itemFlags0.
		item.ItemFlags[1] = item.Index; // Save targetitem for little fishes into itemFlags1.
		item.StartPose.Position = item.Pose.Position;
		item.Animation.Velocity.z = (Random::GenerateInt() & 127) + 32;
	}

	// Define bounding area for fish based on spawn point and set random target within bounds.
	Vector3 GetRandomFishTarget(ItemInfo* item)
	{
		int XleaderBoundLeft = item->StartPose.Position.x - BLOCK(1);
		int	XleaderBoundRight = item->StartPose.Position.x + BLOCK(1);

		int	YleaderBoundUp = item->StartPose.Position.y - BLOCK(1);
		int	YleaderBoundDown = item->StartPose.Position.y + BLOCK(1);

		int	ZleaderBoundFront = item->StartPose.Position.z - BLOCK(3);
		int	ZleaderBoundBack = item->StartPose.Position.z + BLOCK(3);

		// Calculate and return random target within bounds.
		return Vector3(
			Random::GenerateInt(XleaderBoundLeft, XleaderBoundRight),
			Random::GenerateInt(YleaderBoundUp, YleaderBoundDown),
			Random::GenerateInt(ZleaderBoundFront, ZleaderBoundBack));
	}

	void FishSwarmControl(short itemNumber)
	{
		constexpr auto INVALID_CADAVER_POSITION = Vector3(FLT_MAX);

		auto* item = &g_Level.Items[itemNumber];

		if (!CreatureActive(itemNumber))
			return;

		auto cadaverPos = INVALID_CADAVER_POSITION;

		if (item->HitPoints)
		{
			SpawnFishSwarm(item);
			item->HitPoints--;
		}

		auto* creature = GetCreatureInfo(item);
		GetAITarget(creature);
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		int dx = creature->Target.x - item->Pose.Position.x;
		int dz = creature->Target.z - item->Pose.Position.z;
		AI.distance = SQUARE(dx) + SQUARE(dz);

		item->Animation.Velocity.z = MAX_FISH_VELOCITY;

		auto* playerRoom = &g_Level.Rooms[LaraItem->RoomNumber];

		if (cadaverPos == INVALID_CADAVER_POSITION) //Check if cadaver is near.
		{
			float shortestDistance = INFINITY;
			for (auto& targetItem : g_Level.Items)
			{
				if (!Objects.CheckID(targetItem.ObjectNumber) || targetItem.Index == itemNumber || targetItem.RoomNumber == NO_ROOM)
					continue;

				if (SameZone(creature, &targetItem) && item->TriggerFlags)
				{
					float distance = Vector3i::Distance(item->Pose.Position, targetItem.Pose.Position);
					if (distance < shortestDistance &&
						targetItem.ObjectNumber == ID_CORPSE &&
						targetItem.Active && TriggerActive(&targetItem) &&
						targetItem.ItemFlags[1] == (int)CorpseFlags::Lying &&
						TestEnvironment(ENV_FLAG_WATER, targetItem.RoomNumber))
					{
						cadaverPos = targetItem.Pose.Position.ToVector3();
						shortestDistance = distance;
						item->ItemFlags[1] = targetItem.Index; // Attack cadaver.
					}
				}
			}
		}

		if (AI.distance < pow(BLOCK(3), 2) && TestEnvironment(ENV_FLAG_WATER, playerRoom) && item->TriggerFlags && cadaverPos == INVALID_CADAVER_POSITION)
		{
			item->ItemFlags[1] = LaraItem->Index;
			cadaverPos == INVALID_CADAVER_POSITION;
		}
		// Orbit around leader item.
		else if (cadaverPos == INVALID_CADAVER_POSITION)
		{
			item->ItemFlags[1] = item->ItemFlags[0];
			cadaverPos == INVALID_CADAVER_POSITION;
		}

		for (auto& fish : FishSwarm)
		{
			if (fish.leader == item)
			{
				if (!fish.on)
					continue;

				fish.RoomNumber = item->RoomNumber;
				fish.target = &g_Level.Items[item->ItemFlags[1]];
			}
		}
	}

	void SpawnFishSwarm(ItemInfo* item)
	{
		int fishNumber = GetFreeFish();
		if (fishNumber == NO_ITEM)
			return;

		auto& fish = FishSwarm[fishNumber];

		fish.on = true;
		fish.Pose.Position = item->Pose.Position;
		fish.Pose.Orientation.x = (GetRandomControl() & 0x3FF) - 512;
		fish.Pose.Orientation.y = (GetRandomControl() & 0x7FF) + item->Pose.Orientation.y + -ANGLE(180.0f) - 1024;
		fish.RoomNumber = item->RoomNumber;
		fish.Velocity = (GetRandomControl() & 0x1F) + 16;
		fish.counter = 20 * ((GetRandomControl() & 0x7) + 0xF);
		fish.leader = &g_Level.Items[item->ItemFlags[0]];
	}

	void ClearFishSwarm()
	{
		if (Objects[ID_FISH_EMITTER].loaded)
		{
			ZeroMemory(FishSwarm, NUM_FISHES * sizeof(FishData));
			NextFish = 0;
		}
	}

	short GetFreeFish()
	{
		for (int i = 0; i < NUM_FISHES; i++)
		{
			const auto& fish = FishSwarm[i];

			if (!fish.on)
				return i;
		}

		return NO_ITEM;
	}

	void UpdateFishSwarm()
	{
		constexpr auto COHESION_FACTOR			= 100.1f;			
		constexpr auto SPACING_FACTOR			= 600.0f;
		constexpr auto SPEEDUP_FACTOR			= 0.2f;
		constexpr auto MAX_DISTANCE_FROM_LEADER = SQUARE(BLOCK(0.9f));

		int minDist = MAXINT;
		int minIndex = -1;

		float separationDist = 190.0f;

		for (int i = 0; i < NUM_FISHES; i++)
		{
			auto& fish = FishSwarm[i];
		
			if (!fish.on)
				continue;

			auto* leaderItem = fish.leader;

			if (!leaderItem->ItemFlags[2] && fish.target == fish.leader)
			{
				fish.target->Pose.Position = GetRandomFishTarget(leaderItem);
				leaderItem->ItemFlags[2] = 1;
			}

			int enemyvelocity = fish.target != fish.leader ? 16 : 26;

			// Randomly adjust target position around target object.
			fish.XTarget = (GetRandomControl() & 0xFF) - 128;
			fish.YTarget = (GetRandomControl() & 0xFF) - 122;
			fish.ZTarget = (GetRandomControl() & 0xFF) - 128;

			// Calculate desired position based on target object and random offsets.
			auto desiredPos = fish.target->Pose.Position + Vector3i(fish.XTarget, fish.YTarget, fish.ZTarget);
			auto dir = desiredPos - fish.Pose.Position;

			auto dirs = dir.ToVector3();
			dirs.Normalize();
			auto normalizedDirection = dirs;

			// Define cohesion factor to keep fish close to each other.
			float distToTarget = dirs.Length();

			float targetVel = (distToTarget * COHESION_FACTOR) +  (Random::GenerateFloat(3.0f, 5.0f));
			fish.Velocity = std::min(targetVel, fish.target->Animation.Velocity.z - 21.0f); 

			// If fish is too far away from leader, make it faster to catch up.
			if (distToTarget > MAX_DISTANCE_FROM_LEADER)
			{
				fish.Velocity += SPEEDUP_FACTOR; 
			}

			// Make fish move in direction it is facing.
			auto rotMatrix = fish.Pose.Orientation.ToRotationMatrix();
			auto movement = Vector3::Transform(Vector3::UnitZ, rotMatrix);
		
			movement.Normalize(); 
			fish.Pose.Position += (movement * fish.Velocity / enemyvelocity);

			fish.Pose.Position += (movement * SPACING_FACTOR) / enemyvelocity;

			auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), desiredPos.ToVector3());
			fish.Pose.Orientation.Lerp(orientTo, 0.1f);

			for (int j = 0; j < NUM_FISHES; j++)
			{
				if (i == j)
					continue;

				if (fish.target != fish.leader)
					separationDist = 80.0f;

				auto* otherFish = &FishSwarm[j];

				float distToOtherFish = Vector3::Distance(fish.Pose.Position.ToVector3(), otherFish->Pose.Position.ToVector3());
				float distToPlayer = Vector3::Distance(fish.Pose.Position.ToVector3(), LaraItem->Pose.Position.ToVector3());

				if (distToOtherFish < separationDist)
				{
					auto separationVector = fish.Pose.Position.ToVector3() - otherFish->Pose.Position.ToVector3();
					separationVector.Normalize();

					fish.Pose.Position += separationVector * (separationDist - distToOtherFish);
				}
				else
				{
				    fish.Velocity += SPEEDUP_FACTOR;
				}

				// If Lara is too close and the fish are not lethal, steer away from it.
				if ((distToPlayer < separationDist * 3) && !leaderItem->TriggerFlags && LaraItem->Animation.ActiveState == LS_UNDERWATER_SWIM_FORWARD)
				{
					// Calculate the separation vector.
					auto separationVector = fish.Pose.Position.ToVector3() - LaraItem->Pose.Position.ToVector3();
					separationVector.Normalize();

					auto playerPos = LaraItem->Pose.Position + Vector3i(fish.XTarget, fish.YTarget, fish.ZTarget);

					auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), -playerPos.ToVector3());
					fish.Pose.Orientation.Lerp(orientTo, 0.1f);

					float fleeVel = (distToTarget * COHESION_FACTOR) + Random::GenerateFloat(3.0f, 15.0f);
					fish.Velocity -= std::min(fleeVel, fish.target->Animation.Velocity.z - 1.0f);
				}
			}

			auto pointColl = GetCollision(fish.Pose.Position, fish.RoomNumber);
			const auto& room = g_Level.Rooms[fish.RoomNumber];

			// Prevent fish from peeking out of water surface.
			if (pointColl.RoomNumber != fish.RoomNumber && !TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
				fish.Pose.Position.y = room.maxceiling + 180;

			if (ItemNearTarget(fish.Pose.Position, fish.target, CLICK(1) / 2) )
			{
				if (fish.leader != fish.target)
				{
					TriggerBlood(fish.Pose.Position.x, fish.Pose.Position.y, fish.Pose.Position.z, 4 * GetRandomControl(), 4);
					DoDamage(fish.target, FISH_HARM_DAMAGE);
				}
				else
				{
					leaderItem->ItemFlags[2] = 0;
				}
			}

			auto tMatrix = Matrix::CreateTranslation(fish.Pose.Position.x, fish.Pose.Position.y, fish.Pose.Position.z);
			auto rotMatrix2 = fish.Pose.Orientation.ToRotationMatrix();
			fish.Transform = rotMatrix2 * tMatrix;
		}
	}
}
