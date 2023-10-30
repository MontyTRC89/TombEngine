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

	static bool IsNextSectorValid(const ItemInfo& item, const Vector3& dir)
	{
		auto projectedPos = Geometry::TranslatePoint(item.Pose.Position, dir, BLOCK(1));
		auto pointColl = GetCollision(item.Pose.Position, item.RoomNumber, dir, BLOCK(1));

		// Test for wall.
		if (pointColl.Block->IsWall(projectedPos.x, projectedPos.z))
			return false;

		// Test for slippery slope.
		if (pointColl.Position.FloorSlope)
			return false;

		// Flat floor.
		if (abs(pointColl.FloorTilt.x) == 0 && abs(pointColl.FloorTilt.y) == 0)
		{
			// Test for step.
			int relFloorHeight = abs(pointColl.Position.Floor - item.Pose.Position.y);
			if (relFloorHeight >= CLICK(1))
				return false;
		}
		// Sloped floor.
		else
		{
			// Half block.
			int relFloorHeight = abs(pointColl.Position.Floor - item.Pose.Position.y);
			if (relFloorHeight > CLICK(2))
				return false;

			short slopeAngle = ANGLE(0.0f);
			if (pointColl.FloorTilt.x > 0)
			{
				slopeAngle = -ANGLE(90.0f);
			}
			else if (pointColl.FloorTilt.x < 0)
			{
				slopeAngle = ANGLE(90.0f);
			}

			if (pointColl.FloorTilt.y > 0 && pointColl.FloorTilt.y > abs(pointColl.FloorTilt.x))
			{
				slopeAngle = ANGLE(180.0f);
			}
			else if (pointColl.FloorTilt.y < 0 && -pointColl.FloorTilt.y > abs(pointColl.FloorTilt.x))
			{
				slopeAngle = ANGLE(0.0f);
			}

			int angleDir = phd_atan(dir.z, dir.x);
			int alignAngle = slopeAngle - angleDir;

			// Test if slope aspect is not aligned with the direction.
			if ((alignAngle != 32768) && (alignAngle != 0) && (alignAngle != -32768))
				return false;
			
			// TODO: ANGLE(180.0f) and ANGLE(-180) both returns -32768 due to the short type range (-32,768 to 32,767)
			//if (alignAngle != ANGLE(180.0f) && alignAngle != 0 && alignAngle != ANGLE(-180.0f))
				//return false;
		}

		// Check for diagonal split.
		if (pointColl.Position.DiagonalStep)
			return false;

		// Test ceiling height.
		int relCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);
		int cleanerHeight = BLOCK(1);
		if (relCeilHeight < cleanerHeight)
			return false;

		// Check for inaccessible sector.
		if (pointColl.Block->Box == NO_BOX)
			return false;

		// Check for blocked grey box.
		if (g_Level.Boxes[pointColl.Block->Box].flags & BLOCKABLE)
		{
			if (g_Level.Boxes[pointColl.Block->Box].flags& BLOCKED)
				return false;
		}

		// Check for stopper flag.
		if (pointColl.Block->Stopper)
			return false;

		return true;
	}

	static Vector3 GetElectricCleanerMovementDirection(const ItemInfo& item, const Vector3& dir0, const Vector3& dir1, const Vector3& dir2)
	{
		if (IsNextSectorValid(item, dir0))
			return dir0;
		if (IsNextSectorValid(item, dir1))
			return dir1;
		if (IsNextSectorValid(item, dir2))
			return dir2;

		return Vector3::Zero;
	}
			
	static void HandleElectricCleanerItemCollision(ItemInfo& item)
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
						NewDirection = GetElectricCleanerMovementDirection(item, forwardDirection, rightDirection, -rightDirection);
					else								//Forward Left Right
						NewDirection = GetElectricCleanerMovementDirection(item, forwardDirection, -rightDirection, rightDirection);
				}
				else
				{
					if (flagAntiClockWiseOrder)			//Right Forward Left
						NewDirection = GetElectricCleanerMovementDirection(item, rightDirection, forwardDirection, -rightDirection);
					else								//Left Forward Right
						NewDirection = GetElectricCleanerMovementDirection(item, -rightDirection, forwardDirection, rightDirection);
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
		HandleElectricCleanerItemCollision(item);
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
