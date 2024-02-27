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

#define DEG2RAD (3.14159265358979323846f / 180.0f)

using namespace TEN::Entities::TR3;
using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto FISH_HARM_DAMAGE = 3;
	constexpr auto FISH_ENTITY_DAMAGE = 1;
	constexpr auto MAX_FISH_VELOCITY = 10.0f;
	constexpr auto MIN_FISH_VELOCITY = 8.0f;
	constexpr auto LEADER_REACH_TARGET_RANGE = SQUARE(BLOCK(0.4f));
	constexpr auto FISH_ORIENT_LERP_ALPHA = 0.1f;
	constexpr float MAX_ANGLE = ANGLE(10.0f);
	constexpr float MIN_ANGLE = ANGLE(-4.0f);
	constexpr float ANGLE_STEP = ANGLE(1.0f);

	FishData FishSwarm[FISH_COUNT_MAX];
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
		item.ItemFlags[5] = 0; //Save Hitpoints (Fish number) into item.ItemFlags[5].
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

		Vector3 validTarget = Vector3(
			Random::GenerateInt(XleaderBoundLeft, XleaderBoundRight),
			Random::GenerateInt(YleaderBoundUp, YleaderBoundDown),
			Random::GenerateInt(ZleaderBoundFront, ZleaderBoundBack));


		auto pointColl = GetCollision(validTarget, item->RoomNumber);
		const auto& room = g_Level.Rooms[item->RoomNumber];

		// Prevent fish from peeking out of water surface.
		if (pointColl.RoomNumber != NO_ROOM && !TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber) || pointColl.Position.Floor < validTarget.y ||
			pointColl.Position.Ceiling > validTarget.y)
			return Vector3::Zero;
		else
			return validTarget;
	}

	void FishSwarmControl(short itemNumber)
	{
		constexpr auto INVALID_CADAVER_POSITION = Vector3(FLT_MAX);

		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		if (!CreatureActive(itemNumber))
			return;

		auto cadaverPos = INVALID_CADAVER_POSITION;

		if (item.ItemFlags[5] != item.HitPoints)
		{
			// Calculate the difference between the current and the previous hit points
			int hitPointsDifference = item.HitPoints - item.ItemFlags[5];

			if (hitPointsDifference < 0)
			{
				int fishToTurnOff = -hitPointsDifference;
				for (auto& fish : FishSwarm)
				{
					if (fish.leader == &item && fish.on)
					{
						fish.on = false;
						fishToTurnOff--;
						if (fishToTurnOff == 0)
							break;
					}
				}
			}
			else if (hitPointsDifference > 0)
			{
				for (int i = 0; i < hitPointsDifference; i++)
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

		// Check if cadaver is near.
		if (cadaverPos == INVALID_CADAVER_POSITION)
		{
			float shortestDist = INFINITY;
			for (auto& targetItem : g_Level.Items)
			{
				if (!Objects.CheckID(targetItem.ObjectNumber) || targetItem.Index == itemNumber || targetItem.RoomNumber == NO_ROOM)
					continue;

				if (SameZone(&creature, &targetItem) && item.TriggerFlags < 0)
				{
					float dist = Vector3i::Distance(item.Pose.Position, targetItem.Pose.Position);
					if (dist < shortestDist &&
						targetItem.ObjectNumber == ID_CORPSE &&
						targetItem.Active && TriggerActive(&targetItem) &&
						targetItem.ItemFlags[1] == (int)CorpseFlags::Lying &&
						TestEnvironment(ENV_FLAG_WATER, targetItem.RoomNumber))
					{
						cadaverPos = targetItem.Pose.Position.ToVector3();
						shortestDist = dist;
						item.ItemFlags[1] = targetItem.Index; // Attack cadaver.
					}
				}
			}
		}

		if (ai.distance < SQUARE(BLOCK(3)) && TestEnvironment(ENV_FLAG_WATER, &playerRoom) &&
			item.TriggerFlags < 0 && cadaverPos == INVALID_CADAVER_POSITION)
		{
			item.ItemFlags[1] = LaraItem->Index;
			cadaverPos == INVALID_CADAVER_POSITION;
		}
		// Orbit around leader item.
		else if (cadaverPos == INVALID_CADAVER_POSITION)
		{
			item.ItemFlags[1] = item.ItemFlags[0];
			cadaverPos == INVALID_CADAVER_POSITION;
		}

		for (auto& fish : FishSwarm)
		{
			if (fish.leader == &item)
			{
				if (!fish.on)
					continue;

				fish.RoomNumber = item.RoomNumber;
				fish.target = &g_Level.Items[item.ItemFlags[1]];	
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
		fish.Species = item->TriggerFlags < 0 ? abs(item->TriggerFlags) : item->TriggerFlags;
		fish.Lethal = item->TriggerFlags < 0 ? true : false;
		fish.leader = &g_Level.Items[item->ItemFlags[0]];
	}

	void ClearFishSwarm()
	{
		if (Objects[ID_FISH_EMITTER].loaded)
		{
			ZeroMemory(FishSwarm, FISH_COUNT_MAX * sizeof(FishData));
			NextFish = 0;
			FlipEffect = -1;
		}
	}

	short GetFreeFish()
	{
		for (int i = 0; i < FISH_COUNT_MAX; i++)
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
		constexpr auto MAX_DISTANCE_FROM_TARGET = SQUARE(BLOCK(0.01f));
		constexpr float BASE_SEPARATION_DISTANCE = 210.0f;


		int nearestFishIndex = -1; // Index of the nearest fish
		float minDistanceToTarget = std::numeric_limits<float>::max(); // Initialize to maximum possible value

		int minDist = MAXINT;
		int minIndex = -1;

		for (int i = 0; i < FISH_COUNT_MAX; i++)
		{
			auto& fish = FishSwarm[i];
		
			if (!fish.on)
				continue;

			float separationDist = BASE_SEPARATION_DISTANCE + (i * 3.0f); // Increase separation distance for each fish

			auto& leaderItem = *fish.leader;

			if (!leaderItem.ItemFlags[2] && fish.target == fish.leader)
			{
				fish.target->Pose.Position = GetRandomFishTarget(&leaderItem);

				if (fish.target->Pose.Position != Vector3::Zero)
				leaderItem.ItemFlags[2] = 1;
			}

			//g_Renderer.AddDebugSphere(Vector3(fish.target->Pose.Position.x, fish.target->Pose.Position.y, fish.target->Pose.Position.z), 46, Vector4(1, 1, 1, 1), RendererDebugPage::None);

			int enemyvelocity = fish.target != fish.leader ? 16 : 26;

			// Randomly adjust target position around target object.
			fish.PositionTarget = Vector3i(
				(GetRandomControl() & 0xFF) - 128,
				(GetRandomControl() & 0xFF) - 122,
				(GetRandomControl() & 0xFF) - 128);

			// Calculate desired position based on target object and random offsets.
			auto desiredPos = fish.target->Pose.Position + fish.PositionTarget;
			auto dir = desiredPos - fish.Pose.Position;

			auto dirs = dir.ToVector3();
			dirs.Normalize();
			auto dirNorm = dirs;

			// Define cohesion factor to keep fish close to each other.
			float distToTarget = dirs.Length();

			float targetVel = (distToTarget * COHESION_FACTOR) +  (Random::GenerateFloat(3.0f, 5.0f));
			fish.Velocity = std::min(targetVel, fish.target->Animation.Velocity.z - 21.0f); 

			// If fish is too far away from target, make it faster to catch up.
			if (distToTarget > MAX_DISTANCE_FROM_TARGET)
				fish.Velocity += SPEEDUP_FACTOR; 

			// Make fish move in direction it is facing.
			auto rotMatrix = fish.Pose.Orientation.ToRotationMatrix();
			auto movement = Vector3::Transform(Vector3::UnitZ, rotMatrix);
		
			movement.Normalize(); 
			fish.Pose.Position += (movement * fish.Velocity / enemyvelocity);

			fish.Pose.Position += (movement * SPACING_FACTOR) / enemyvelocity;

			auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), desiredPos.ToVector3());
			fish.Pose.Orientation.Lerp(orientTo, 0.1f);

			for (int j = 0; j < FISH_COUNT_MAX; j++)
			{
				if (i == j)
					continue;

				const auto& otherFish = FishSwarm[j];

				float distToOtherFish = Vector3i::Distance(fish.Pose.Position, otherFish.Pose.Position);
				float distToPlayer = Vector3i::Distance(fish.Pose.Position, LaraItem->Pose.Position);
				float distanceToTarget = Vector3i::Distance(fish.Pose.Position, otherFish.PositionTarget);

				// Update the index of the nearest fish to the target
				if (distanceToTarget < minDistanceToTarget && fish.target == fish.leader)
				{
					minDistanceToTarget = distanceToTarget;
					nearestFishIndex = j;
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
				    fish.Velocity += SPEEDUP_FACTOR;
				}

				//Orient to the fish that is nearest to the target. To prevent other fishes from swimming forward but orientate elsewhere.
				if (fish.Pose.Orientation.x != FishSwarm[nearestFishIndex].Pose.Orientation.x && separationDist > 30.0f && fish.target == fish.leader)
				{
					separationDist--;
					auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), FishSwarm[nearestFishIndex].Pose.Position.ToVector3());
					fish.Velocity += SPEEDUP_FACTOR;
				}

				// If player is too close and fish are not lethal, steer away.
				if ((distToPlayer < separationDist * 3) && fish.Lethal == false && LaraItem->Animation.ActiveState == LS_UNDERWATER_SWIM_FORWARD)
				{
					const float fleeVel = 20;

					auto separationDir = (fish.Pose.Position - LaraItem->Pose.Position).ToVector3();
					separationDir.Normalize();

					fish.Pose.Position += separationDir * fleeVel;

					auto orientTo = Geometry::GetOrientToPoint(fish.Pose.Position.ToVector3(), fish.Pose.Position.ToVector3() + separationDir);
					fish.Pose.Orientation.Lerp(orientTo, 0.05f);

					fish.Velocity -= std::min(fleeVel, fish.target->Animation.Velocity.z - 1.0f);
				}
			}

				auto pointColl = GetCollision(fish.Pose.Position, fish.RoomNumber);
				const auto& room = g_Level.Rooms[fish.RoomNumber];

				//Update fish's roomnumber
				if (pointColl.RoomNumber != fish.RoomNumber)
					fish.RoomNumber = pointColl.RoomNumber;

				// Prevent fish from peeking out of water surface.
				if (pointColl.RoomNumber != fish.RoomNumber && !TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
					fish.Pose.Position.y = room.maxceiling + 180;
			
			if (ItemNearTarget(fish.Pose.Position, fish.target, CLICK(2) / 2) )
			{
				if (fish.leader != fish.target)
				{
					TriggerBlood(fish.Pose.Position.x, fish.Pose.Position.y, fish.Pose.Position.z, 4 * GetRandomControl(), 4);
					DoDamage(fish.target, FISH_HARM_DAMAGE);
				}
				else
				{
					leaderItem.ItemFlags[2] = 0;
				}
			}
			
			// Calculate wiggle angle based on sine wave
			//TODO: Include speed of the fish
			float wiggleAngle = sin(fish.Timer) * MAX_ANGLE;

			fish.Pose.Orientation.y += wiggleAngle;
			auto rotMatrix2 = fish.Pose.Orientation.ToRotationMatrix();

			fish.Timer += 1.0f; 

			if (fish.Timer > PI_MUL_2)
				fish.Timer -= PI_MUL_2;

			// Update fish's transformation matrix
			auto tMatrix = Matrix::CreateTranslation(fish.Pose.Position.x, fish.Pose.Position.y, fish.Pose.Position.z);
			fish.Transform = rotMatrix2 * tMatrix;
		}
	}
}
