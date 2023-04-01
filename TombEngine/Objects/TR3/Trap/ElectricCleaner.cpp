#include "framework.h"
#include "Objects/TR3/Trap/ElectricCleaner.h"

#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Spark;

// ItemFlags[0]:		Angular velocity of rotation.
// ItemFlags[1]:		Flags, each bit is used to check the status of a flag
//						b0: flagDoDetection
//						b1: flagTurnRight
//						b2: flagPriorityForward
//						b3: flagAntiClockWiseOrder
//						b4: flagStopAfterKill - If true the cleaner will stop when kills Lara.
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
		
		auto& angularVel = item.ItemFlags[0];
		auto& moveVel = item.ItemFlags[2];
		auto& targetAngle = item.ItemFlags[6];

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

		float angleDelta = abs(TO_RAD(targetAngle) - TO_RAD(item.Pose.Orientation.y));

		bool doDetection			  = ((item.ItemFlags[1] & (1 << 0)) != 0);
		bool doRightTurn			  = ((item.ItemFlags[1] & (1 << 1)) != 0);
		bool isForwardPriority		  = ((item.ItemFlags[1] & (1 << 2)) != 0);
		bool useCounterClockwiseOrder = ((item.ItemFlags[1] & (1 << 3)) != 0);

		auto pointColl = GetCollision(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, item.RoomNumber);

		short headingAngle = item.Pose.Orientation.y;
		float sinHeadingAngle = phd_sin(headingAngle);
		float cosHeadingAngle = phd_cos(headingAngle);

		auto forwardDirection = Vector3(sinHeadingAngle, 0, cosHeadingAngle);
		forwardDirection.Normalize();

		auto rightDirection = Vector3(cosHeadingAngle, 0, -sinHeadingAngle);
		rightDirection.Normalize();
		
		if (angleDelta > TO_RAD(angularVel))
		{
			if (doRightTurn)
				item.Pose.Orientation.y -= angularVel;
			else
				item.Pose.Orientation.y += angularVel;

			// Recalculate new delta to check if alignment with axis for should be forced for safety check.
			angleDelta = abs(TO_RAD(targetAngle) - TO_RAD(item.Pose.Orientation.y));
			if (angleDelta <= TO_RAD(angularVel))
				item.Pose.Orientation.y = targetAngle;
		}
		else
		{
			if (doDetection && 
				(item.Pose.Position.x & WALL_MASK) == BLOCK(0.5f) &&
				(item.Pose.Position.z & WALL_MASK) == BLOCK(0.5f))
			{
				TestTriggers(&item, true);

				// Search for next direction.
				auto newDirection = Vector3::Zero;
				if (isForwardPriority)			
				{
					// Forward right left.
					if (useCounterClockwiseOrder)
						newDirection = ElectricCleanerSearchDirections(item, forwardDirection, rightDirection, -rightDirection);
					// Forward left right.
					else
						newDirection = ElectricCleanerSearchDirections(item, forwardDirection, -rightDirection, rightDirection);
				}
				else
				{
					// Right forward left.
					if (useCounterClockwiseOrder)
						newDirection = ElectricCleanerSearchDirections(item, rightDirection, forwardDirection, -rightDirection);
					// Left forward right.
					else
						newDirection = ElectricCleanerSearchDirections(item, -rightDirection, forwardDirection, rightDirection);
				}

				// Return back (already know is a valid one because it came from there).
				if (newDirection == Vector3::Zero)
					newDirection = -forwardDirection;
								
				// Turn left or right?
				auto cross = newDirection.Cross(forwardDirection);
				if (cross.y > 0)
				{
					// Set 1st bit of flagTurnRight.
					item.ItemFlags[1] |= (1 << 1);
				}
				else if (cross.y < 0)
				{
					// Clear 1st bit of flagTurnRight (turn left).
					item.ItemFlags[1] &= ~(1 << 1);
				}
				
				// Store goal angle to control the rotation.
				item.ItemFlags[6] = FROM_RAD(atan2(newDirection.x, newDirection.z));

				// If doesn't need to rotate, go forward to maintain smooth movement.
				if (item.Pose.Orientation.y - item.ItemFlags[6] == 0)
				{
					item.Pose.Position = item.Pose.Position + forwardDirection * moveVel;
				}
				// If must rotate, stop detection to avoid calculating collisions again while rotating in the same block.
				else
				{
					// Clear 1st bit of flagDoDetection.
					item.ItemFlags[1] &= ~(1 << 0);
				}
			}
			else
			{
				item.Pose.Position.y = pointColl.Position.Floor;

				// Is not in the center of a tile; keep moving forward. 
				item.Pose.Position = item.Pose.Position + forwardDirection * moveVel;

				auto slope = pointColl.Block->GetSurfaceSlope(0, true);

				// If slope, don't turn.
				if (slope.LengthSquared() > 0)
				{
					// Clear 1st bit of flagDoDetection.
					item.ItemFlags[1] &= ~(1 << 0);
				}
				else
				{
					// Set 1st bit of flagDoDetection.
					item.ItemFlags[1] |= (1 << 0);
				}
			}
		}

		AnimateItem(&item);

		int probedRoomNumber = GetCollision(&item).RoomNumber;
		if (item.RoomNumber != probedRoomNumber)
			ItemNewRoom(itemNumber, probedRoomNumber);

		auto ellipse = Vector2(object.radius, object.radius);
		AlignEntityToSurface(&item, ellipse);

		SpawnElectricCleanerSparks(item);
		ElectricCleanerToItemCollision(item);
	}

	bool IsNextSectorValid(ItemInfo& item, const Vector3& direction)
	{
		auto point = GameVector(item.Pose.Position + BLOCK(direction), item.RoomNumber);
		auto pointColl = GetCollision(point);

		// Check for wall.
		if (pointColl.Block->IsWall(point.x, point.z))
			return false;

		// Check for floor slope.
		if (pointColl.Position.FloorSlope)
			return false;

		// Is flat floor.
		if (abs(pointColl.FloorTilt.x) == 0 && abs(pointColl.FloorTilt.y) == 0)
		{
			// Is 1 step (higher or lower).
			int distanceToFloor = abs(pointColl.Position.Floor - item.Pose.Position.y);
			if (distanceToFloor >= CLICK(1))
				return false;
		}
		// Is sloped floor.
		else
		{
			// Is 2 steps (higher or lower).
			int distanceToFloor = abs(pointColl.Position.Floor - item.Pose.Position.y);
			if (distanceToFloor > CLICK(2))
				return false;

			short slopeAngle = 0;
			if (pointColl.FloorTilt.x > 0)
			{
				slopeAngle = ANGLE(-90.0f);
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
				slopeAngle = 0;
			}

			short headingAngle = FROM_RAD(atan2(direction.x, direction.z));
			int alignment = slopeAngle - headingAngle;

			// Check if slope is not aligned with direction.
			if (alignment != 32768 && alignment != 0 && alignment != -32768)
				return false;
		}

		// Check if floor is diagonal.
		if (pointColl.Position.DiagonalStep)
			return false;

		// Assess ceiling height.
		int distanceToCeiling = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);	
		if (distanceToCeiling < BLOCK(1))
			return false;

		// Check walkability.
		if (pointColl.Block->Box == NO_BOX)
			return false;

		// Check for blocked grey box (i.e. box is isolated).
		if (g_Level.Boxes[pointColl.Block->Box].flags & BLOCKABLE && g_Level.Boxes[pointColl.Block->Box].flags & BLOCKED)
			return false;
		
		// Check for stopper flag (there is still a shatter object).
		if (pointColl.Block->Stopper)
			return false;

		return true;
	}

	Vector3 ElectricCleanerSearchDirections(ItemInfo& item, const Vector3& dir1, const Vector3& dir2, const Vector3& dir3)
	{
		if (IsNextSectorValid(item, dir1))
			return dir1;

		if (IsNextSectorValid(item, dir2))
			return dir2;

		if (IsNextSectorValid(item, dir3))
			return dir3;

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
