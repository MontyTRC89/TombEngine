#include "framework.h"
#include "Objects/TR3/Trap/ElectricCleaner.h"

#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Spark;

// ItemFlags[0]:		Rotation speed and heading angle.
// ItemFlags[1]:		Has reached a new sector and can unblock the one behind it. Unused but reserved just in case.
// ItemFlags[2]:		Movement velocity.
// ItemFlags[3, 4, 5]:	Counters for dynamic lights and sparks.
// ItemFlags[6]:		Goal direction angle.

// OCB:
// 0:			  Stop after killing the player.
// Anything else: Don't stop after killing the player.

namespace TEN::Entities::Traps
{
	constexpr auto ELECTRIC_CLEANER_VELOCITY  = BLOCK(1 / 16.0f);
	constexpr auto ELECTRIC_CLEANER_TURN_RATE = 1024;

	const auto ElectricCleanerHarmJoints = std::vector<unsigned int>{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

	void InitialiseElectricCleaner(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Align to the middle of the block.
		item.Pose.Position.x = (item.Pose.Position.x & ~WALL_MASK) | (int)BLOCK(0.5f);
		item.Pose.Position.z = (item.Pose.Position.z & ~WALL_MASK) | (int)BLOCK(0.5f);

		// Init flags.
		item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE;
		item.ItemFlags[1] = 0;
		item.ItemFlags[2] = ELECTRIC_CLEANER_VELOCITY;
		item.ItemFlags[6] = item.Pose.Orientation.y;
		item.Collidable = true;

		if (item.TriggerFlags)
			item.ItemFlags[1] &= ~(1 << 4);	// Turn off 1st bit for flagStopAfterKill.
		else
			item.ItemFlags[1] |= (1 << 4);	// Turn on 1st bit for flagStopAfterKill.
	}

	void ElectricCleanerControl(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		auto& object = Objects[item.ObjectNumber];
		
		auto& rotationVel = item.ItemFlags[0];
		auto& moveVel = item.ItemFlags[2];
		auto& goalAngle = item.ItemFlags[6];

		if (!TriggerActive(&item))
		{
			if (moveVel > 0)
			{
				moveVel = 0;
				SoundEffect(SFX_TR3_CLEANER_FUSEBOX, &item.Pose);
			}

			return;
		}

		if (moveVel <= 0)
			return;

		float angleDifference = abs(TO_RAD(goalAngle) - TO_RAD(item.Pose.Orientation.y));

		bool flagDoDetection		= ((item.ItemFlags[1] & (1 << 0)) != 0);
		bool flagTurnRight			= ((item.ItemFlags[1] & (1 << 1)) != 0);
		bool flagPriorityForward	= ((item.ItemFlags[1] & (1 << 2)) != 0);
		bool flagAntiClockWiseOrder	= ((item.ItemFlags[1] & (1 << 3)) != 0);

		auto col = GetCollision(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, item.RoomNumber);

		float yaw = TO_RAD(item.Pose.Orientation.y);

		Vector3 ForwardDirection = Vector3(sin(yaw), 0, cos(yaw));
		ForwardDirection.Normalize();

		Vector3 RightDirection = Vector3(cos(yaw), 0, -sin(yaw));
		RightDirection.Normalize();
		
		if (angleDifference > TO_RAD(rotationVel))
		{
			if (flagTurnRight)
				item.Pose.Orientation.y -= rotationVel;
			else
				item.Pose.Orientation.y += rotationVel;

			// Recalculate new difference to check if we should force align with axis for safety check.
			angleDifference = abs (TO_RAD(goalAngle) - TO_RAD(item.Pose.Orientation.y));
			if (angleDifference <= TO_RAD(rotationVel))
				item.Pose.Orientation.y = goalAngle;
		}
		else
		{
			if (flagDoDetection && 
				(item.Pose.Position.x & WALL_MASK) == BLOCK(0.5f) &&
				(item.Pose.Position.z & WALL_MASK) == BLOCK(0.5f))
			{
				//Do triggers
				TestTriggers(&item, true);

				//Search for next direction
				Vector3 NewDirection;

				if (flagPriorityForward)			
				{
					if (flagAntiClockWiseOrder)			//Forward Right Left
						NewDirection = ElectricCleanerSearchDirections(item, ForwardDirection, RightDirection, -RightDirection);
					else								//Forward Left Right
						NewDirection = ElectricCleanerSearchDirections(item, ForwardDirection, -RightDirection, RightDirection);
				}
				else
				{
					if (flagAntiClockWiseOrder)			//Right Forward Left
						NewDirection = ElectricCleanerSearchDirections(item, RightDirection, ForwardDirection, -RightDirection);
					else								//Left Forward Right
						NewDirection = ElectricCleanerSearchDirections(item, -RightDirection, ForwardDirection, RightDirection);
				}

				if (NewDirection == Vector3::Zero) //Return back. (We already know is a valid one because it came from there).
					NewDirection = -ForwardDirection;
								
				//Will turn left or right?
				auto crossProductResult = NewDirection.Cross(ForwardDirection);
				if (crossProductResult.y > 0)
					item.ItemFlags[1] |= (1 << 1); // Turn on 1st bit for flagTurnRight.
				else if (crossProductResult.y < 0)
					item.ItemFlags[1] &= ~(1 << 1); // Turn off 1st bit for flagTurnRight. (So it'll turn to the left)
				
				//Store goal angle to control the rotation.
				item.ItemFlags[6] = FROM_RAD(atan2(NewDirection.x, NewDirection.z));

				if (item.Pose.Orientation.y - item.ItemFlags[6] == 0)
					//If it doesn't have to rotate, do forward movement to keep smooth movement.
					item.Pose.Position = item.Pose.Position + ForwardDirection * moveVel;
				else
					//If it has to rotate, stop detection so it doesn't calculate collisions again while rotating in the same sector.
					item.ItemFlags[1] &= ~(1 << 0); // Turn off 1st bit for flagDoDetection.
			}
			else
			{
				item.Pose.Position.y = col.Position.Floor;

				//Is not in the center of a tile, keep moving forward. 
				item.Pose.Position = item.Pose.Position + ForwardDirection * moveVel;

				auto slope = col.Block->FloorSlope(0);

				if (slope.LengthSquared() > 0) //If it's a slope, don't do turns.
					item.ItemFlags[1] &= ~(1 << 0);	// Turn off 1st bit for flagDoDetection.
				else
					item.ItemFlags[1] |= (1 << 0);	// Turn on 1st bit for flagDoDetection.
			}
		}

		AnimateItem(&item);

		int probedRoomNumber = GetCollision(&item).RoomNumber;
		if (item.RoomNumber != probedRoomNumber)
			ItemNewRoom(itemNumber, probedRoomNumber);

		auto radius = Vector2(object.radius * 0.5f, object.radius * 0.5f);
		AlignEntityToSurface(&item, radius);

		SpawnElectricCleanerSparks(item);
		ElectricCleanerToItemCollision(item);
	}

	bool IsNextSectorValid(ItemInfo& item, const Vector3& Dir)
	{
		GameVector detectionPoint = item.Pose.Position + Dir * BLOCK(1);
		detectionPoint.RoomNumber = item.RoomNumber;

		auto col = GetCollision(detectionPoint);

		//Is a wall
		if (col.Block->IsWall(detectionPoint.x, detectionPoint.z))
			return false;

		//Is it a sliding slope?
		if (col.Position.FloorSlope)
			return false;

		if (abs(col.FloorTilt.x) == 0 && abs(col.FloorTilt.y) == 0) //Is a flat tile
		{
			//Is a 1 click step (higher or lower).
			int distanceToFloor = abs(col.Position.Floor - item.Pose.Position.y);
			if (distanceToFloor >= CLICK(1))
				return false;
		}
		else //Is a slope tile
		{
			//Is a 2 click step (higher or lower).
			int distanceToFloor = abs(col.Position.Floor - item.Pose.Position.y);
			if (distanceToFloor > CLICK(2))
				return false;

			short slopeAngle = ANGLE(0.0f);

			if (col.FloorTilt.x > 0)
				slopeAngle = -ANGLE(90.0f);
			else if (col.FloorTilt.x < 0)
				slopeAngle = ANGLE(90.0f);

			if (col.FloorTilt.y > 0 && col.FloorTilt.y > abs(col.FloorTilt.x))
				slopeAngle = ANGLE(180.0f);
			else if (col.FloorTilt.y < 0 && -col.FloorTilt.y > abs(col.FloorTilt.x))
				slopeAngle = ANGLE(0.0f);

			auto angleDir = FROM_RAD(atan2(Dir.x, Dir.z));
			auto alignment = slopeAngle - angleDir;

			//Is slope not aligned with the direction?
			if ((alignment != 32768) && (alignment != 0) && (alignment != -32768))
				return false;
		}

		//Is diagonal floor?
		if (col.Position.DiagonalStep)
			return false;

		//Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(col.Position.Ceiling - col.Position.Floor);	
		int cleanerHeight = 1024; //TODO change it for the collision bounding box height.
		if (distanceToCeiling < cleanerHeight)
			return false;

		//Is a non walkable tile? (So there is not any box)
		if (col.Block->Box == NO_BOX)
			return false;

		//Is a blocked grey box (So it's an Isolated box)
		if (g_Level.Boxes[col.Block->Box].flags & BLOCKABLE)
			return false;
		
		//Is a stopper tile? (There is still a shatter object).
		if (col.Block->Stopper)
			return false;

		//If nothing of that happened, then it must be a valid sector.
		return true;
	}

	Vector3 ElectricCleanerSearchDirections(ItemInfo& item, const Vector3& Dir1, const Vector3& Dir2, const Vector3& Dir3)
	{
		if (IsNextSectorValid(item, Dir1))
			return Dir1;
		if (IsNextSectorValid(item, Dir2))
			return Dir2;
		if (IsNextSectorValid(item, Dir3))
			return Dir3;

		return Vector3::Zero;
	}

	void ElectricCleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		ObjectCollision(itemNumber, laraItem, coll);

		if (item.TouchBits.Test(ElectricCleanerHarmJoints) && item.ItemFlags[2])
		{
			ItemElectricBurn(laraItem, -1);
			laraItem->HitPoints = 0;

			bool flagStopAfterKill = ((item.ItemFlags[1] & (1 << 4)) != 0);
			if (flagStopAfterKill)
				item.ItemFlags[2] = 0;

			SoundEffect(SFX_TR3_CLEANER_FUSEBOX, &item.Pose);
		}
	}
		
	void ElectricCleanerToItemCollision(ItemInfo& item)
	{
		auto backupPos = item.Pose.Position;

		switch (item.Pose.Orientation.y)
		{
		case ANGLE(0.0f):
			item.Pose.Position.z += BLOCK(0.5f);
			break;

		case ANGLE(90.0f):
			item.Pose.Position.x += BLOCK(0.5f);
			break;

		case ANGLE(-180.0f):
			item.Pose.Position.z -= BLOCK(0.5f);
			break;

		case ANGLE(-90.0f):
			item.Pose.Position.x -= BLOCK(0.5f);
			break;
		}

		if (GetCollidedObjects(&item, CLICK(1), true, CollidedItems, CollidedMeshes, true))
		{
			int lp = 0;
			while (CollidedItems[lp] != nullptr)
			{
				if (Objects[CollidedItems[lp]->ObjectNumber].intelligent)
				{
					CollidedItems[lp]->HitPoints = 0;
					ItemElectricBurn(CollidedItems[lp], 120);
				}

				lp++;
			}
		}

		item.Pose.Position = backupPos;
	}

	void SpawnElectricCleanerSparks(ItemInfo& item)
	{
		static auto wireEndJoints = std::array<int, 3>{ 5, 9, 13 };

		SoundEffect(SFX_TR3_CLEANER_LOOP, &item.Pose);

		auto vel = Vector3i(
			(Random::GenerateInt(0, 255) * 4) - 512,
			Random::GenerateInt(0, 7) - 4,
			(Random::GenerateInt(0, 255) * 4) - 512);

		for (int i = 0; i < 3; i++)
		{
			if ((!(GetRandomControl() & 7) && !item.ItemFlags[3 + i]) || item.ItemFlags[3 + i])
			{
				if (!item.ItemFlags[3 + i])
					item.ItemFlags[3 + i] = Random::GenerateInt(0, 12) + 8;
				else
					item.ItemFlags[3 + i]--;

				int joint = wireEndJoints[i];
				auto pos = GetJointPosition(&item, joint, Vector3i(-160, -8, 16));

				byte c = Random::GenerateInt(0, 64) + 128;
				TriggerDynamicLight(pos.x, pos.y, pos.z, 10, c >> 2, c >> 1, c);

				auto& spark = GetFreeSparkParticle();

				spark = {};
				spark.active = true;
				spark.age = 0;
				float color = (192.0F + Random::GenerateFloat(0, 63.0F)) / 255.0F;
				spark.sourceColor = Vector4(color / 4, color / 2, color, 1.0F);
				color = (192.0F + Random::GenerateFloat(0, 63.0F)) / 255.0F;
				spark.destinationColor = Vector4(color / 4, color / 2, color, 1.0F);
				spark.life = Random::GenerateFloat(20, 27);
				spark.friction = 1.2f;
				spark.gravity = 1.5f;
				spark.width = 8.0f;
				spark.height = 96.0f;
				auto v = vel.ToVector3();
				v.Normalize(v);
				spark.velocity = v;
				spark.pos = pos.ToVector3();
				spark.room = item.RoomNumber;
			}
		}
	}
}
