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

	const auto ElectricCleanerHarmJoints	= std::vector<unsigned int>{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
	const auto ElectricCleanerWireEndJoints = std::vector<unsigned int>{ 5, 9, 13 };

	auto PushableItemPtrs = std::vector<ItemInfo*>{};

	// TODO: Way of detecting pushables after pushables are refactored.
	static std::vector<ItemInfo*> GetPushableItemPtrs()
	{
		auto pushableItemPtrs = std::vector<ItemInfo*>{};

		for (int itemNumber = 0; itemNumber < g_Level.Items.size(); itemNumber++)
		{
			auto& item = g_Level.Items[itemNumber];

			if (item.ObjectNumber >= (ID_PUSHABLE_OBJECT1) &&
				item.ObjectNumber <= (ID_PUSHABLE_OBJECT10))
			{
				pushableItemPtrs.push_back(&item);
			}
		}

		return pushableItemPtrs;
	}

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

		PushableItemPtrs = GetPushableItemPtrs();
	}

	static bool TestPushableCollision(const std::vector<ItemInfo*>& pushableItemPtrs, const Vector3& refPoint)
	{
		float pushableDist = INFINITY;
		for (int itemNumber = 0; itemNumber < pushableItemPtrs.size(); itemNumber++)
		{
			const auto& currentItem = *pushableItemPtrs[itemNumber];
			if (&currentItem == nullptr)
				continue;

			auto pos = currentItem.Pose.Position.ToVector3();
			auto dist = Vector3::Distance(pos, refPoint);

			if (dist < BLOCK(1))
				return true;
		}

		return false;
	}

	static bool IsNextSectorValid(const ItemInfo& item, const Vector3& dir)
	{
		auto projectedPos = Geometry::TranslatePoint(item.Pose.Position, dir, BLOCK(1));

		auto pointColl = GetCollision(projectedPos);

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
			if (alignAngle != ANGLE(180.0f) && alignAngle != 0 && alignAngle != ANGLE(-180.0f))
				return false;
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
			return false;

		// Check for stopper flag.
		if (pointColl.Block->Stopper)
			return false;

		// Test for pushable object.
		if (TestPushableCollision(PushableItemPtrs, projectedPos.ToVector3()))
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
		const auto& object = Objects[item.ObjectNumber];
		
		short& moveVel = item.ItemFlags[2];
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

		// Get flags.
		bool flagDoDetection		   = ((item.ItemFlags[1] & (1 << 0)) != 0);
		bool flagTurnRight			   = ((item.ItemFlags[1] & (1 << 1)) != 0);
		bool flagPriorityForward	   = ((item.ItemFlags[1] & (1 << 2)) != 0);
		bool flagCounterClockwiseOrder = ((item.ItemFlags[1] & (1 << 3)) != 0);

		short& rotRate = item.ItemFlags[0];
		short& targetHeadingAngle = item.ItemFlags[6];

		short headingAngleDelta = abs(targetHeadingAngle - item.Pose.Orientation.y);
		if (headingAngleDelta > rotRate)
		{
			item.Pose.Orientation.y += flagTurnRight ? -rotRate : rotRate;

			// Recalculate new dela to check if alignment with axis should be forced for safety check.
			headingAngleDelta = abs(targetHeadingAngle - item.Pose.Orientation.y);
			if (headingAngleDelta <= rotRate)
				item.Pose.Orientation.y = targetHeadingAngle;
		}
		else
		{
			auto forwardDir = EulerAngles(0, item.Pose.Orientation.y, 0).ToDirection();
			auto rightDir = EulerAngles(0, item.Pose.Orientation.y + ANGLE(90.0f), 0).ToDirection();

			if (flagDoDetection &&
				(item.Pose.Position.x & WALL_MASK) == BLOCK(0.5f) &&
				(item.Pose.Position.z & WALL_MASK) == BLOCK(0.5f))
			{
				TestTriggers(&item, true);

				// Find new direction.
				auto newDir = Vector3::Zero;
				if (flagPriorityForward)			
				{
					// Forward, right, left.
					if (flagCounterClockwiseOrder)
					{
						newDir = GetElectricCleanerMovementDirection(item, forwardDir, rightDir, -rightDir);
					}
					// Forward, left, right.
					else
					{
						newDir = GetElectricCleanerMovementDirection(item, forwardDir, -rightDir, rightDir);
					}
				}
				else
				{
					// Right, forward, left.
					if (flagCounterClockwiseOrder)
					{
						newDir = GetElectricCleanerMovementDirection(item, rightDir, forwardDir, -rightDir);
					}
					// Left, forward, right.
					else
					{
						newDir = GetElectricCleanerMovementDirection(item, -rightDir, forwardDir, rightDir);
					}
				}

				// Turn back.
				if (newDir == Vector3::Zero)
					newDir = -forwardDir;

				// Set flagTurnRight.		
				auto cross = newDir.Cross(forwardDir);
				if (cross.y > 0)
				{
					item.ItemFlags[1] |= (1 << 1);
				}
				else if (cross.y < 0)
				{
					item.ItemFlags[1] &= ~(1 << 1);
				}
				
				// Store target heading angle.
				item.ItemFlags[6] = phd_atan(newDir.z, newDir.x);

				// Not rotating; keep moving forward.
				if (item.Pose.Orientation.y - item.ItemFlags[6] == 0)
				{
					item.Pose.Position = Geometry::TranslatePoint(item.Pose.Position, forwardDir, moveVel);
				}
				// Skip collision detection while rotating on current sector.
				else
				{
					// Unset flagDoDetection.
					item.ItemFlags[1] &= ~(1 << 0);
				}
			}
			else
			{
				auto pointColl = GetCollision(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, item.RoomNumber);

				item.Pose.Position.y = pointColl.Position.Floor;

				// Not in sector center; continue moving forward. 
				item.Pose.Position = Geometry::TranslatePoint(item.Pose.Position, forwardDir, moveVel);

				// TODO: Cleaner check.
				// Slippery slope; don't turn.
				auto slope = pointColl.Block->GetSurfaceSlope(0, true);
				if (slope.LengthSquared() > 0)
				{
					// Unset flagDoDetection.
					item.ItemFlags[1] &= ~(1 << 0);
				}
				else
				{
					// Unset flagDoDetection.
					item.ItemFlags[1] |= (1 << 0);
				}
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
