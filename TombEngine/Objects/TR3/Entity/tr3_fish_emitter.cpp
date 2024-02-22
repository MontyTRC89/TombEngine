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
	constexpr auto LEADER_VELOCITY = 64.0f;
	constexpr auto LEADER_REACH_TARGET_RANGE = SQUARE(BLOCK(0.4f));

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

		// 1 = Pyranja or  0 = non lethal fishes:
		item->TriggerFlags = 0;

		//Save leaderitem for little fishes into itemFlags0.
		item->ItemFlags[0] = item->Index;
		//Save targetitem for little fishes into itemFlags1.
		item->ItemFlags[1] = item->Index;


		//default Coordinates for leader-Target
		
		item->ItemFlags[2] = 0;
		item->ItemFlags[3] = 0;
		item->ItemFlags[4] = 0;

		//save start cordinates
		item->StartPose.Position.x = item->Pose.Position.x;
		item->StartPose.Position.y = item->Pose.Position.y;
		item->StartPose.Position.z = item->Pose.Position.z;


		item->Animation.Velocity.z = (Random::GenerateInt() & 127) + 32;

		
	}

	// Define the bounding area for the leader fish based on the spawn point and set a random target within the bounds
	Vector3 GetRandomFishTarget(ItemInfo* item)
	{
		int XleaderBoundLeft = item->StartPose.Position.x - BLOCK(1);
		int	XleaderBoundRight = item->StartPose.Position.x + BLOCK(1);

		int	YleaderBoundUp = item->StartPose.Position.y - BLOCK(1);
		int	YleaderBoundDown = item->StartPose.Position.y + BLOCK(1);

		int	ZleaderBoundFront = item->StartPose.Position.z - BLOCK(3);
		int	ZleaderBoundBack = item->StartPose.Position.z + BLOCK(3);


		


		//Set Random Target within bounds

		auto target = Vector3(Random::GenerateInt(XleaderBoundLeft, XleaderBoundRight), Random::GenerateInt(YleaderBoundUp, YleaderBoundDown), Random::GenerateInt(ZleaderBoundFront, ZleaderBoundBack));
		return target;
	}


	void FishSwarmControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		constexpr auto INVALID_CADAVER_POSITION = Vector3(FLT_MAX);
		float MAX_VELOCITY = 5; // Maximum velocity for the fish
		constexpr float ACCELERATION = 0.1f;

		

		if (!item->ItemFlags[5])
		{
			item->ItemFlags[5] = Random::GenerateInt(50, 150);
			MAX_VELOCITY = Random::GenerateFloat(15.0f, 35.0f);
		}
		else
			item->ItemFlags[5]--;



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

		auto angle = CreatureTurn(item, creature->MaxTurn);
		creature->MaxTurn = ANGLE(365.f); //?
	
		item->TriggerFlags = 0;


		if (item->ItemFlags[2] == 0 && item->ItemFlags[3] == 0 && item->ItemFlags[4] == 0)
		{
			creature->Target = GetRandomFishTarget(item);
			item->ItemFlags[2] = creature->Target.x;
			item->ItemFlags[3] = creature->Target.y;
			item->ItemFlags[4] = creature->Target.z;


		}
		else
		{
			/*
			// Update fish's orientation
			EulerAngles orientation = Geometry::GetOrientToPoint(item->Pose.Position.ToVector3(), leaderTarget);
			item->Pose.Orientation.x = orientation.x;
			item->Pose.Orientation.y = orientation.y;

			Vector3 direction = leaderTarget - item->Pose.Position.ToVector3();

			float distanceToTarget = Vector3i::Distance(item->Pose.Position, leaderTarget);

			// Adjust fish's velocity based on distance to the target
			

			// Move fish towards the target position
			// Scale the direction vector by the calculated velocity

			Vector3 zdirection = direction;
			zdirection.Normalize();

			item->Pose.Position += (zdirection * velocity);*/

			g_Renderer.AddDebugSphere(creature->Target.ToVector3(), 46, Vector4(1, 1, 0, 1), RendererDebugPage::None);


			int dx = creature->Target.x - item->Pose.Position.x;
			int dz = creature->Target.z - item->Pose.Position.z;
			AI.distance = SQUARE(dx) + SQUARE(dz);
			AI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
			AI.ahead = (AI.angle > -FRONT_ARC && AI.angle < FRONT_ARC);

			float distanceToTarget = Vector3i::Distance(item->Pose.Position, creature->Target);

			if (AI.distance < LEADER_REACH_TARGET_RANGE)
			{
				item->ItemFlags[2] = 0;
				item->ItemFlags[3] = 0;
				item->ItemFlags[4] = 0;
				
			}

			EulerAngles orientation = Geometry::GetOrientToPoint(item->Pose.Position.ToVector3(), creature->Target.ToVector3());
			item->Pose.Orientation.x = orientation.x;
			item->Pose.Orientation.y = orientation.y;


			float velocity = std::min(ACCELERATION, MAX_VELOCITY);
			item->Pose.Position = Geometry::TranslatePoint(item->Pose.Position, orientation, MAX_VELOCITY);

			//item->Pose.Position -= Vector3(MAX_VELOCITY);

		}
		

		/*
		//phd_atan(LaraItem->Pose.Position.z - item->Pose.Position.z, LaraItem->Pose.Position.x - item->Pose.Position.x);

		GetCreatureMood(item, &AI, true);
		CreatureMood(item, &AI, true);

		short angle = CreatureTurn(item, creature->MaxTurn);

		//item->Pose.Position = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, forwardVel);
		int random = ((itemNumber & 0x7) * 0x200) - 0x700;
		creature->Target.x = item->Pose.Position.x + (BLOCK(1) * phd_sin(AI.angle + ANGLE(180.0f) + random));
		creature->Target.z = item->Pose.Position.z + (BLOCK(1) * phd_cos(AI.angle + ANGLE(180.0f) + random));*/
		
		auto* laraRoom = &g_Level.Rooms[LaraItem->RoomNumber];

		if (cadaverPos == INVALID_CADAVER_POSITION) //Check if cadaver is near
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
						item->ItemFlags[1] = targetItem.Index; //Attack cadaver
					}
				}
			}
		}
		
		if (AI.distance < pow(BLOCK(3), 2) && TestEnvironment(ENV_FLAG_WATER, laraRoom) && item->TriggerFlags && cadaverPos == INVALID_CADAVER_POSITION) //else attack Lara: item->TriggerFlags 1 Fish = pyranja.
		{
			item->ItemFlags[1] = LaraItem->Index;
			cadaverPos == INVALID_CADAVER_POSITION;
		}
		else if (cadaverPos == INVALID_CADAVER_POSITION) //else orbit around leader item
		{
			item->ItemFlags[1] = item->ItemFlags[0];
			cadaverPos == INVALID_CADAVER_POSITION;
		}

		for (int i = 0; i < NUM_FISHES; i++)
		{
			auto* fish = &FishSwarm[i];

			if (!fish->on)
				continue;

			fish->RoomNumber = item->RoomNumber;
			fish->target = &g_Level.Items[item->ItemFlags[1]];
		}

		g_Renderer.AddDebugSphere(Vector3(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z), 46, Vector4(1, 1, 1, 1), RendererDebugPage::None);

		//CreatureAnimation(itemNumber, angle, 0);
		//CreatureTilt(item, 0);
		//CreatureUnderwater(item, 341);
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

		for (int i = 0; i < NUM_FISHES; i++)
		{
			auto* fish = &FishSwarm[i];

			if (!fish->on)
				continue;


			if (!(GetRandomControl() & 7))
			{
				fish->YTarget = (GetRandomControl() & 0x1F) + 1;
				fish->XTarget = (GetRandomControl() & 0x7F) - 64;
				fish->ZTarget = (GetRandomControl() & 0x7F) - 64;
			}

			auto angles = Geometry::GetOrientToPoint(
				fish->Pose.Position.ToVector3(),
				Vector3(
					fish->target->Pose.Position.x + fish->XTarget ,
					fish->target->Pose.Position.y + fish->YTarget ,
					fish->target->Pose.Position.z + fish->ZTarget 
				));




			int x = fish->target->Pose.Position.x - fish->Pose.Position.x;
			int z = fish->target->Pose.Position.z - fish->Pose.Position.z;
			int distance = pow(x, 2) + pow(z, 2);
			if (distance < minDistance)
			{
				minDistance = distance;
				minIndex = i;
			}

			distance = sqrt(distance) / 18;
			if (distance < 48)
				distance = 48;
			else if (distance > 168)
				distance = 168;

			 if (fish->Velocity < distance)
				fish->Velocity++;
			else if (fish->Velocity > distance)
				fish->Velocity--;

			//if (fish->Counter > 90)
			//{
			auto* room = &g_Level.Rooms[fish->RoomNumber];

				short Velocity = fish->Velocity * 128; //wie eng am  target je höher desto enger

				short xAngle = abs(angles.x - fish->Pose.Orientation.x) / 4;
				short yAngle = abs(angles.y - fish->Pose.Orientation.y) / 4;

				if (xAngle < -Velocity)
					xAngle = -Velocity /2;
				else if (xAngle > Velocity)
					xAngle = Velocity / 2;

				if (yAngle < -Velocity)
					yAngle = -Velocity / 4;
				else if (yAngle > Velocity)
					yAngle = Velocity / 4;

				fish->Pose.Orientation.y += yAngle/2;
				fish->Pose.Orientation.x += xAngle;
			//}

			int sp = fish->Velocity * phd_cos(fish->Pose.Orientation.x);
			auto pointColl = GetCollision(fish->Pose.Position, fish->RoomNumber);

			fish->Pose.Position.x += sp * phd_sin(fish->Pose.Orientation.y);
			fish->Pose.Position.z += sp * phd_cos(fish->Pose.Orientation.y);

			//Prevent fishes to "jump" out of the water surface
			if (pointColl.RoomNumber != fish->RoomNumber)
			{
				if (TestEnvironment(ENV_FLAG_WATER, pointColl.RoomNumber))
				{
					fish->Pose.Position.y += (fish->Velocity ) * phd_sin(-fish->Pose.Orientation.x);
				}
				else
					fish->Pose.Position.y = room->maxceiling + 180;
			}
			else
				fish->Pose.Position.y += (fish->Velocity / 2) * phd_sin(-fish->Pose.Orientation.x);

				if (ItemNearTarget(fish->Pose.Position, fish->target, CLICK(1) / 2) && (fish->leader != fish->target))
				{
					TriggerBlood(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z, 4 * GetRandomControl(), 4);
					DoDamage(fish->target, FISH_LARA_DAMAGE);
				}

				g_Renderer.AddDebugSphere(Vector3(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z), 26, Vector4(1, 0, 0, 1), RendererDebugPage::None);

				Matrix translation = Matrix::CreateTranslation(fish->Pose.Position.x, fish->Pose.Position.y, fish->Pose.Position.z);
				Matrix rotation = fish->Pose.Orientation.ToRotationMatrix();
				fish->Transform = rotation * translation;	
		}
	}
}
