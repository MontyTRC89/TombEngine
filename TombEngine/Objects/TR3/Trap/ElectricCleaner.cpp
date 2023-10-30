#include "framework.h"
#include "Objects/TR3/Trap/ElectricCleaner.h"

#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Spark;

// NOTES
// ItemFlags[0] = rotation rate.
// ItemFlags[1] = behaviour flags.
//					0: flagDoDetection
//					1: flagTurnRight
//					2: flagPriorityForward
//					3: flagCounterClockwiseOrder
//					4: flagStopAfterKill
// ItemFlags[2] = movement velocity.
// ItemFlags[3, 4, 5] = counters for dynamic lights and sparks.
// ItemFlags[6] = target heading angle.

// OCB:
// 0: Stop after killing player, otherwise don't stop.

namespace TEN::Entities::Traps
{
	constexpr auto ELECTRIC_CLEANER_VELOCITY  = BLOCK(1 / 16.0f);
	constexpr auto ELECTRIC_CLEANER_TURN_RATE = ANGLE(5.6f);

	const auto ElectricCleanerHarmJoints =		std::vector<unsigned int>{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
	const auto ElectricCleanerWireEndJoints =		std::vector<unsigned int>{ 5, 9, 13 };

	void InitializeElectricCleaner(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Align to sector center.
		item.Pose.Position.x = (item.Pose.Position.x & ~WALL_MASK) | (int)BLOCK(0.5f);
		item.Pose.Position.z = (item.Pose.Position.z & ~WALL_MASK) | (int)BLOCK(0.5f);

		// Initialize flags.
		item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE;
		item.ItemFlags[1] = 0;
		item.ItemFlags[2] = ELECTRIC_CLEANER_VELOCITY;
		item.ItemFlags[6] = item.Pose.Orientation.y;
		item.Collidable = true;

		// Set flagStopAfterKill.
		if (item.TriggerFlags)
		{
			item.ItemFlags[1] &= ~(1 << 4);
		}
		else
		{
			item.ItemFlags[1] |= (1 << 4);
		}
	}

	static bool IsNextSectorValid(ItemInfo& item, const Vector3& dir)
	{
		GameVector detectionPoint = item.Pose.Position + dir * BLOCK(1);
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

			auto angleDir = FROM_RAD(atan2(dir.x, dir.z));
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
		int cleanerHeight = BLOCK(1); //TODO change it for the collision bounding box height.
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

	static Vector3 ElectricCleanerSearchDirections(ItemInfo& item, const Vector3& dir1, const Vector3& dir2, const Vector3& dir3)
	{
		if (IsNextSectorValid(item, dir1))
			return dir1;
		if (IsNextSectorValid(item, dir2))
			return dir2;
		if (IsNextSectorValid(item, dir3))
			return dir3;

		return Vector3::Zero;
	}
			
	static void ElectricCleanerToItemCollision(ItemInfo& item)
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

	static void SpawnElectricCleanerSparks(ItemInfo& item)
	{
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

				int joint = ElectricCleanerWireEndJoints[i];
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

	void ControlElectricCleaner(short itemNumber)
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

		auto angleDifference = abs(TO_RAD(goalAngle) - TO_RAD(item.Pose.Orientation.y));

		bool flagDoDetection = ((item.ItemFlags[1] & (1 << 0)) != 0);
		bool flagTurnRight = ((item.ItemFlags[1] & (1 << 1)) != 0);
		bool flagPriorityForward = ((item.ItemFlags[1] & (1 << 2)) != 0);
		bool flagAntiClockWiseOrder = ((item.ItemFlags[1] & (1 << 3)) != 0);

		auto col = GetCollision(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, item.RoomNumber);

		float yaw = TO_RAD(item.Pose.Orientation.y);

		auto forwardDirection = Vector3(sin(yaw), 0, cos(yaw));
		forwardDirection.Normalize();

		auto rightDirection = Vector3(cos(yaw), 0, -sin(yaw));
		rightDirection.Normalize();

		if (angleDifference > TO_RAD(rotationVel))
		{
			if (flagTurnRight)
				item.Pose.Orientation.y -= rotationVel;
			else
				item.Pose.Orientation.y += rotationVel;

			// Recalculate new difference to check if we should force align with axis for safety check.
			angleDifference = abs(TO_RAD(goalAngle) - TO_RAD(item.Pose.Orientation.y));
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
						NewDirection = ElectricCleanerSearchDirections(item, forwardDirection, rightDirection, -rightDirection);
					else								//Forward Left Right
						NewDirection = ElectricCleanerSearchDirections(item, forwardDirection, -rightDirection, rightDirection);
				}
				else
				{
					if (flagAntiClockWiseOrder)			//Right Forward Left
						NewDirection = ElectricCleanerSearchDirections(item, rightDirection, forwardDirection, -rightDirection);
					else								//Left Forward Right
						NewDirection = ElectricCleanerSearchDirections(item, -rightDirection, forwardDirection, rightDirection);
				}

				if (NewDirection == Vector3::Zero) //Return back. (We already know is a valid one because it came from there).
					NewDirection = -forwardDirection;

				//Will turn left or right?
				auto crossProductResult = NewDirection.Cross(forwardDirection);
				if (crossProductResult.y > 0)
					item.ItemFlags[1] |= (1 << 1); // Turn on 1st bit for flagTurnRight.
				else if (crossProductResult.y < 0)
					item.ItemFlags[1] &= ~(1 << 1); // Turn off 1st bit for flagTurnRight. (So it'll turn to the left)

				//Store goal angle to control the rotation.
				item.ItemFlags[6] = FROM_RAD(atan2(NewDirection.x, NewDirection.z));

				if (item.Pose.Orientation.y - item.ItemFlags[6] == 0)
					//If it doesn't have to rotate, do forward movement to keep smooth movement.
					item.Pose.Position = item.Pose.Position + forwardDirection * moveVel;
				else
					//If it has to rotate, stop detection so it doesn't calculate collisions again while rotating in the same sector.
					item.ItemFlags[1] &= ~(1 << 0); // Turn off 1st bit for flagDoDetection.
			}
			else
			{
				item.Pose.Position.y = col.Position.Floor;

				//Is not in the center of a tile, keep moving forward. 
				item.Pose.Position = item.Pose.Position + forwardDirection * moveVel;

				auto slope = col.Block->GetSurfaceSlope(0, true);

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

		auto radius = Vector2(object.radius, object.radius);
		AlignEntityToSurface(&item, radius);

		SpawnElectricCleanerSparks(item);
		ElectricCleanerToItemCollision(item);
	}

	void CollideElectricCleaner(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
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
}
