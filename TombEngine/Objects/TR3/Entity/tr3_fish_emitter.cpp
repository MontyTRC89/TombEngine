#include "framework.h"
#include "Objects/TR3/Entity/tr3_fish_emitter.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/tomb4fx.h"
#include "Game/misc.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Specific/level.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Objects/TR3/Object/Corpse.h"

#include "Math/Geometry.h"
#include "Math/Legacy.h"
#include "Math/Objects/EulerAngles.h"

using namespace TEN::Math;
using namespace TEN::Renderer;
using namespace TEN::Entities::TR3;

namespace TEN::Entities::Creatures::TR3
{
	FishData FishSwarm[NUM_FISHES];

	constexpr auto FISH_LARA_DAMAGE = 3;
	constexpr auto FISH_ENTITY_DAMAGE = 1;
	constexpr auto MAX_FISH_VELOCITY = 10.0f;
	constexpr auto LEADER_REACH_TARGET_RANGE = SQUARE(BLOCK(0.4f));
	constexpr auto FISH_ORIENT_LERP_ALPHA = 0.1f;

	int NextFish;

	void InitializeFishSwarm(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!item->Pose.Orientation.y)
			item->Pose.Orientation.z += CLICK(2);
		else if (item->Pose.Orientation.y == ANGLE(90.0f))
			item->Pose.Orientation.x += CLICK(2);
		else if (item->Pose.Orientation.y == -ANGLE(180.0f))
			item->Pose.Orientation.z -= CLICK(2);
		else if (item->Pose.Orientation.y == -ANGLE(90.0f))
			item->Pose.Orientation.x -= CLICK(2);

		//Number of Fishes 24.
		item->HitPoints = 24;

		//Save leaderitem for little fishes into itemFlags0.
		item->ItemFlags[0] = item->Index;
		//Save targetitem for little fishes into itemFlags1.
		item->ItemFlags[1] = item->Index;

		//save start cordinates.
		item->StartPose.Position.x = item->Pose.Position.x;
		item->StartPose.Position.y = item->Pose.Position.y;
		item->StartPose.Position.z = item->Pose.Position.z;

		item->Animation.Velocity.z = (Random::GenerateInt() & 127) + 32;
	}

	// Define the bounding area for the fish based on the spawn point and set a random target within the bounds.
	Vector3 GetRandomFishTarget(ItemInfo* item)
	{
		int XleaderBoundLeft = item->StartPose.Position.x - BLOCK(1);
		int	XleaderBoundRight = item->StartPose.Position.x + BLOCK(1);

		int	YleaderBoundUp = item->StartPose.Position.y - BLOCK(1);
		int	YleaderBoundDown = item->StartPose.Position.y + BLOCK(1);

		int	ZleaderBoundFront = item->StartPose.Position.z - BLOCK(3);
		int	ZleaderBoundBack = item->StartPose.Position.z + BLOCK(3);

		//Set Random Target within bounds.
		auto target = Vector3(Random::GenerateInt(XleaderBoundLeft, XleaderBoundRight), Random::GenerateInt(YleaderBoundUp, YleaderBoundDown), Random::GenerateInt(ZleaderBoundFront, ZleaderBoundBack));
		return target;
	}


	void FishSwarmControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		constexpr auto INVALID_CADAVER_POSITION = Vector3(FLT_MAX);
		
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

		auto* laraRoom = &g_Level.Rooms[LaraItem->RoomNumber];

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
						item->ItemFlags[1] = targetItem.Index; //Attack cadaver.
					}
				}
			}
		}

		if (AI.distance < pow(BLOCK(3), 2) && TestEnvironment(ENV_FLAG_WATER, laraRoom) && item->TriggerFlags && cadaverPos == INVALID_CADAVER_POSITION)
		{
			item->ItemFlags[1] = LaraItem->Index;
			cadaverPos == INVALID_CADAVER_POSITION;
		}
		else if (cadaverPos == INVALID_CADAVER_POSITION) //else orbit around leader item.
		{
			item->ItemFlags[1] = item->ItemFlags[0];
			cadaverPos == INVALID_CADAVER_POSITION;
		}

		for (int i = 0; i < NUM_FISHES; i++)
		{
			auto* fish = &FishSwarm[i];

			if (fish->leader == item)

			{
				if (!fish->on)
					continue;

				fish->RoomNumber = item->RoomNumber;
				fish->target = &g_Level.Items[item->ItemFlags[1]];
			}
		}
	}

	void SpawnFishSwarm(ItemInfo* item)
	{
		Vector3i origin, target;
		short fishNumber = GetFreeFish();
		EulerAngles orient;
		if (fishNumber != NO_ITEM)
		{
			auto* fish = &FishSwarm[fishNumber];

			fish->on = true;
			fish->Pose.Position = item->Pose.Position;
			fish->Pose.Orientation.x = (GetRandomControl() & 0x3FF) - 512;
			fish->Pose.Orientation.y = (GetRandomControl() & 0x7FF) + item->Pose.Orientation.y + -ANGLE(180.0f) - 1024;
			fish->roomNumber = item->RoomNumber;
			fish->randomRotation = (GetRandomControl() & 0x1F) + 0x10;
			fish->Velocity = (GetRandomControl() & 0x1F) + 16;
			fish->counter = 20 * ((GetRandomControl() & 0x7) + 0xF);
			fish->leader = &g_Level.Items[item->ItemFlags[0]];
		}
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
			auto* fish = &FishSwarm[i];
			if (!fish->on)
				return i;
		}

		return NO_ITEM;
	}

	void UpdateFishSwarm()
	{
		int minDistance = MAXINT;
		int minIndex = -1;
		
		float SEPARATION_DISTANCE = 190.0f;

		for (int i = 0; i < NUM_FISHES; i++)
		{
			auto* fish = &FishSwarm[i];

			if (!fish->on)
				continue;

			auto leaderItem = fish->leader;

			if (!leaderItem->ItemFlags[2] && fish->target == fish->leader)
			{
				fish->target->Pose.Position = GetRandomFishTarget(leaderItem);
				leaderItem->ItemFlags[2] = 1;
			}

			int enemyvelocity = fish->target != fish->leader ? 16 : 26;

			// Randomly adjust the target position around the target object.
			fish->XTarget = (GetRandomControl() & 0xFF) - 128;
			fish->YTarget = (GetRandomControl() & 0xFF) - 122;
			fish->ZTarget = (GetRandomControl() & 0xFF) - 128;

			// Calculate the desired position based on the target object and the random offsets.
			Vector3i desiredPosition = fish->target->Pose.Position + Vector3i(fish->XTarget, fish->YTarget, fish->ZTarget);
			Vector3i direction = desiredPosition - fish->Pose.Position;

			Vector3 directions = direction.ToVector3();
			directions.Normalize();
			Vector3 normalizedDirection = directions;

			// Define a cohesion factor to keep the fish close to each other.
			constexpr float COHESION_FACTOR = 100.1f;			
			float distanceToTarget = directions.Length();

			float targetVelocity = (distanceToTarget * COHESION_FACTOR) +  (Random::GenerateFloat(3.0f, 5.0f));
			fish->Velocity = std::min(targetVelocity, fish->target->Animation.Velocity.z - 21.0f); 

			// If the fish is too far away from the leader, make it faster to catch up.
			constexpr float MAX_DISTANCE_FROM_LEADER = SQUARE(BLOCK(0.9f));
			if (distanceToTarget > MAX_DISTANCE_FROM_LEADER)
			{
				constexpr float SPEEDUP_FACTOR = 0.2f; 
				fish->Velocity += SPEEDUP_FACTOR; 
			}

			//Make the fish move in the direction they are facing
			auto rotMatrix = fish->Pose.Orientation.ToRotationMatrix();
			auto movement = Vector3::Transform(Vector3::UnitZ, rotMatrix);
		
			movement.Normalize(); 
			fish->Pose.Position += (movement * fish->Velocity / enemyvelocity);

			constexpr float SPACING_FACTOR = 600.0f;

			fish->Pose.Position += (movement * SPACING_FACTOR) / enemyvelocity;

			auto orientTo = Geometry::GetOrientToPoint(fish->Pose.Position.ToVector3(), desiredPosition.ToVector3());
			fish->Pose.Orientation.Lerp(orientTo, 0.1f);

			for (int j = 0; j < NUM_FISHES; j++)
			{
				if (i == j)
					continue;

				if (fish->target != fish->leader)
					SEPARATION_DISTANCE = 80;

				auto* otherFish = &FishSwarm[j];

				float distanceToOtherFish = Vector3::Distance(fish->Pose.Position.ToVector3(), otherFish->Pose.Position.ToVector3());
				float distanceToLara = Vector3::Distance(fish->Pose.Position.ToVector3(), LaraItem->Pose.Position.ToVector3());

				if (distanceToOtherFish < SEPARATION_DISTANCE)
				{
					Vector3 separationVector = fish->Pose.Position.ToVector3() - otherFish->Pose.Position.ToVector3();
					separationVector.Normalize();

					fish->Pose.Position += separationVector * (SEPARATION_DISTANCE - distanceToOtherFish);
				}
				else
				{
					constexpr float SPEEDUP_FACTOR = 0.2f;
				    fish->Velocity += SPEEDUP_FACTOR;
				}

				// If Lara is too close and the fish are not lethal, steer away from it.
				if ((distanceToLara < SEPARATION_DISTANCE * 3) && !leaderItem->TriggerFlags && LaraItem->Animation.ActiveState == LS_UNDERWATER_SWIM_FORWARD)
				{
					// Calculate the separation vector.
					Vector3 separationVector = fish->Pose.Position.ToVector3() - LaraItem->Pose.Position.ToVector3();
					separationVector.Normalize();

					float distance2 = separationVector.Length();

					Vector3i LaraPosition = LaraItem->Pose.Position + Vector3i(fish->XTarget, fish->YTarget, fish->ZTarget);

					auto orientTo = Geometry::GetOrientToPoint(fish->Pose.Position.ToVector3(), -LaraPosition.ToVector3());
					fish->Pose.Orientation.Lerp(orientTo, 0.1f);

					float fleeVelocity = (distanceToTarget * COHESION_FACTOR) + (Random::GenerateFloat(3.0f, 15.0f));
					fish->Velocity -= std::min(fleeVelocity, fish->target->Animation.Velocity.z - 1.0f);
				}
			}

			auto pointColl = GetCollision(fish->Pose.Position, fish->RoomNumber);
			auto* room = &g_Level.Rooms[fish->RoomNumber];

			//Prevent fishes to "jump" out of the water surface.
			if (pointColl.RoomNumber != fish->RoomNumber && !TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
			{
					fish->Pose.Position.y = room->maxceiling + 180;
			}

			if (ItemNearTarget(fish->Pose.Position, fish->target, CLICK(1) / 2) )
			{
				if (fish->leader != fish->target)
				{
					TriggerBlood(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z, 4 * GetRandomControl(), 4);
					DoDamage(fish->target, FISH_LARA_DAMAGE);
				}
				else
				{
					leaderItem->ItemFlags[2] = 0;
				}
			}

			Matrix translation = Matrix::CreateTranslation(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z);
			Matrix rotation = fish->Pose.Orientation.ToRotationMatrix();
			fish->Transform = rotation * translation;
		}
	}
}
