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

// ItemFlags[0]:	   Rotation speed and heading angle.
// ItemFlags[1]:	   Has reached a new sector and can unblock the one behind it. Unused but reserved just in case.
// ItemFlags[2]:	   Movement velocity.
// ItemFlags[3, 4, 5]: Counters for dynamic lights and sparks.
// ItemFlags[6]:	   Target heading angle.

// OCB:
// 0:			  Stop after killing the player.
// Anything else: Don't stop after killing the player.

namespace TEN::Entities::Traps
{
	constexpr auto ELECTRIC_CLEANER_VELOCITY	  = BLOCK(1 / 16.0f);
	constexpr auto ELECTRIC_CLEANER_TURN_RATE_MAX = ANGLE(8.5f);

	const auto ElectricCleanerHarmJoints = std::vector<unsigned int>{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

	void InitialiseElectricCleaner(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Align to the middle of the block.
		item.Pose.Position.x = (item.Pose.Position.x & ~WALL_MASK) | (int)BLOCK(0.5f);
		item.Pose.Position.z = (item.Pose.Position.z & ~WALL_MASK) | (int)BLOCK(0.5f);

		// Init flags.
		item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
		item.ItemFlags[1] = 0;
		item.ItemFlags[2] = ELECTRIC_CLEANER_VELOCITY;
		item.Collidable = 1;
	}

	void ElectricCleanerControl(short itemNumber)
	{
		static constexpr auto quadrantMask = 16383;

		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
		{
			if (item.ItemFlags[2])
			{
				item.ItemFlags[2] = 0;
				SoundEffect(SFX_TR3_CLEANER_FUSEBOX, &item.Pose);
			}

			return;
		}

		if (!item.ItemFlags[2])
			return;

		short rawAngle = item.Pose.Orientation.y & quadrantMask;

		// Not facing a quadrant; keep turning.
		if (rawAngle != ANGLE(0.0f))
		{
			float coeff = std::max(0.25f, float((quadrantMask / 2) - abs(rawAngle - (quadrantMask / 2))) / float(quadrantMask / 2));

			float targetAngleDeg = TO_DEGREES(item.ItemFlags[6]);
			float turnAngleDeg = TO_DEGREES(item.ItemFlags[0]) * coeff;
			float newAngle = TO_DEGREES(item.Pose.Orientation.y) + turnAngleDeg;

			if ((item.ItemFlags[0] > 0 && abs(abs(newAngle) - abs(targetAngleDeg)) < 5.0f) ||
				(item.ItemFlags[0] < 0 && abs(abs(targetAngleDeg) - abs(newAngle)) < 5.0f))
			{
				newAngle = targetAngleDeg;
			}

			item.Pose.Orientation.y = ANGLE(newAngle);
		}
		else
		{
			// Do we need a new target?
			if (NeedNewTarget(item))
			{
				long x, y, z;
				FloorInfo* floor;
				ROOM_INFO* r;
				short roomNumber;

				if (item.ItemFlags[1])
				{
					x = item.Pose.Position.x + (BLOCK(1) * phd_sin(item.Pose.Orientation.y + ANGLE(180.0F)));
					y = item.Pose.Position.y;
					z = item.Pose.Position.z + (BLOCK(1) * phd_cos(item.Pose.Orientation.y + ANGLE(180.0F)));
					roomNumber = item.RoomNumber;
					floor = GetFloor(x, y, z, &roomNumber);
					r = &g_Level.Rooms[roomNumber];
					r->floor[((z - r->z) / BLOCK(1)) + r->xSize * ((x - r->x) / BLOCK(1))].Stopper = 0;
					item.ItemFlags[1] = 0;
				}


				// Now check where we are heading and determine where to go next.
				bool left, ahead;
				y = item.Pose.Position.y;

				switch (item.Pose.Orientation.y)
				{
				// Facing Z+
				case ANGLE(0.0f):

					// Check if we can go left.
					x = item.Pose.Position.x - BLOCK(1);
					z = item.Pose.Position.z;
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, left);

					// Now check ahead.
					x = item.Pose.Position.x;
					z = item.Pose.Position.z + BLOCK(1);
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, ahead);

					if (!ahead && !left && item.ItemFlags[0] > 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90);
						item.Pose.Orientation.y++;
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else if (!ahead && !left && item.ItemFlags[0] < 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] -= ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					// Prioritize left.
					else if (left && item.ItemFlags[0] > 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] = -ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90);
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
						item.ItemFlags[1] = 1;
						item.Pose.Position.z += item.ItemFlags[2];
						x = item.Pose.Position.x;
						y = item.Pose.Position.y;
						z = item.Pose.Position.z + BLOCK(1);
						roomNumber = item.RoomNumber;
						floor = GetFloor(x, y, z, &roomNumber);
						r = &g_Level.Rooms[roomNumber];
						r->floor[((z - r->z) / BLOCK(1)) + r->xSize * ((x - r->x) / BLOCK(1))].Stopper = 1;
					}

					break;

				// Facing X+
				case ANGLE(90.0f):
					x = item.Pose.Position.x;
					z = item.Pose.Position.z + BLOCK(1);
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, left);

					x = item.Pose.Position.x + BLOCK(1);
					z = item.Pose.Position.z;
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, ahead);

					if (!ahead && !left && item.ItemFlags[0] > 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90.0f);
						item.Pose.Orientation.y++;
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else if (!ahead && !left && item.ItemFlags[0] < 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90.0f);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] -= ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else if (left && item.ItemFlags[0] > 0)	// Prioritize left first.
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90.0f);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] = -ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90.0f);
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
						item.ItemFlags[1] = 1;
						item.Pose.Position.x += item.ItemFlags[2];
						x = item.Pose.Position.x + BLOCK(1);
						y = item.Pose.Position.y;
						z = item.Pose.Position.z;
						roomNumber = item.RoomNumber;
						floor = GetFloor(x, y, z, &roomNumber);
						r = &g_Level.Rooms[roomNumber];
						r->floor[((z - r->z) / BLOCK(1)) + r->xSize * ((x - r->x) / BLOCK(1))].Stopper = 1;
					}

					break;

				// Facing X-
				case ANGLE(-90.0f):
					x = item.Pose.Position.x;
					z = item.Pose.Position.z - BLOCK(1);
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, left);

					x = item.Pose.Position.x - BLOCK(1);
					z = item.Pose.Position.z;
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, ahead);

					if (!ahead && !left && item.ItemFlags[0] > 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90.0f);
						item.Pose.Orientation.y++;
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else if (!ahead && !left && item.ItemFlags[0] < 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90.0f);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] -= ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else if (left && item.ItemFlags[0] > 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90.0f);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] = -ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90.0f);
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
						item.ItemFlags[1] = 1;
						item.Pose.Position.x -= item.ItemFlags[2];
						x = item.Pose.Position.x - BLOCK(1);
						y = item.Pose.Position.y;
						z = item.Pose.Position.z;
						roomNumber = item.RoomNumber;
						floor = GetFloor(x, y, z, &roomNumber);
						r = &g_Level.Rooms[roomNumber];
						r->floor[((z - r->z) / BLOCK(1)) + r->xSize * ((x - r->x) / BLOCK(1))].Stopper = 1;
					}

					break;

				//facing Z-
				case ANGLE(-180.0f):
					x = item.Pose.Position.x + BLOCK(1);
					z = item.Pose.Position.z;
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, left);

					x = item.Pose.Position.x;
					z = item.Pose.Position.z - BLOCK(1);
					CheckCleanerHeading(item, x, y, z, item.RoomNumber, ahead);

					if (!ahead && !left && item.ItemFlags[0] > 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90.0f);
						item.Pose.Orientation.y++;
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else if (!ahead && !left && item.ItemFlags[0] < 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90.0f);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] -= ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else if (left && item.ItemFlags[0] > 0)
					{
						item.ItemFlags[6] = item.Pose.Orientation.y - ANGLE(90.0f);
						item.Pose.Orientation.y--;
						item.ItemFlags[0] = -ELECTRIC_CLEANER_TURN_RATE_MAX;
					}
					else
					{
						item.ItemFlags[6] = item.Pose.Orientation.y + ANGLE(90.0f);
						item.ItemFlags[0] = ELECTRIC_CLEANER_TURN_RATE_MAX;
						item.ItemFlags[1] = 1;
						item.Pose.Position.z -= item.ItemFlags[2];
						x = item.Pose.Position.x;
						y = item.Pose.Position.y;
						z = item.Pose.Position.z - BLOCK(1);
						roomNumber = item.RoomNumber;
						floor = GetFloor(x, y, z, &roomNumber);
						r = &g_Level.Rooms[roomNumber];
						r->floor[((z - r->z) / BLOCK(1)) + r->xSize * ((x - r->x) / BLOCK(1))].Stopper = 1;
					}

					break;
				}

				TestTriggers(&item, 1);
			}
			else
			{
				// No new target; keep updating position.
				switch (item.Pose.Orientation.y)
				{
				case ANGLE(0.0f):
					item.Pose.Position.z += item.ItemFlags[2];
					break;

				case ANGLE(90.0f):
					item.Pose.Position.x += item.ItemFlags[2];
					break;

				case ANGLE(-180.0f):
					item.Pose.Position.z -= item.ItemFlags[2];
					break;

				case ANGLE(-90.0f):
					item.Pose.Position.x -= item.ItemFlags[2];
					break;
				}
			}
		}

		AnimateItem(&item);

		int probedRoomNumber = GetCollision(&item).RoomNumber;
		if (item.RoomNumber != probedRoomNumber)
			ItemNewRoom(itemNumber, probedRoomNumber);

		SpawnElectricCleanerSparks(item);
		CleanerToItemCollision(item);
	}

	void ElectricCleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		ObjectCollision(itemNumber, laraItem, coll);

		if (item.TouchBits.Test(ElectricCleanerHarmJoints) && item.ItemFlags[2])
		{
			ItemElectricBurn(laraItem, -1);
			laraItem->HitPoints = 0;

			if (!item.TriggerFlags)
				item.ItemFlags[2] = 0;

			SoundEffect(SFX_TR3_CLEANER_FUSEBOX, &item.Pose);
		}
	}

	// Checks if the cleaner is in the centre of a block and facing the proper direction.
	bool NeedNewTarget(const ItemInfo& item)
	{
		if ((item.Pose.Position.z & WALL_MASK) == BLOCK(0.5f) &&
			(item.Pose.Orientation.y == ANGLE(0.0f) || item.Pose.Orientation.y == ANGLE(180.0f)))
		{
			return true;
		}

		if ((item.Pose.Position.x & WALL_MASK) == BLOCK(0.5f) &&
			(item.Pose.Orientation.y == ANGLE(90.0f) || item.Pose.Orientation.y == ANGLE(-90.0f)))
		{
			return true;
		}

		return false;
	}

	bool CheckObjectAhead(ItemInfo& item)
	{
		if (!GetCollidedObjects(&item, CLICK(1), true, CollidedItems, CollidedMeshes, true))
			return false;

		int lp = 0;
		while (CollidedItems[lp])
		{
			if (CollidedItems[lp] == &item || Objects[CollidedItems[lp]->ObjectNumber].intelligent)
			{
				lp++;
				continue;
			}

			return true;
		}

		return false;
	}

	void CheckCleanerHeading(ItemInfo& item, long x, long y, long z, short roomNumber, bool& heading)
	{
		FloorInfo* floor = GetFloor(x, y, z, &roomNumber);
		long h = GetFloorHeight(floor, x, y, z);
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];
		floor = &r->floor[((z - r->z) / BLOCK(1)) + r->xSize * ((x - r->x) / BLOCK(1))];
		bool collide;

		/*
		long ox = item.Pose.Position.x;
		long oz = item.Pose.Position.z;
		item.Pose.Position.x = x;
		item.Pose.Position.z = z;
		collide = CheckObjectAhead(item);
		item.Pose.Position.x = ox;
		item.Pose.Position.z = oz;

		* CheckObjectAhead causes issues when item collision boxes are too big, keeping it just in case it is salvageable.
		*/

		collide = 0;
		heading = ((h == y) && !collide && !floor->Stopper);
	}

	void CleanerToItemCollision(ItemInfo& item)
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
			long lp = 0;

			while (CollidedItems[lp])
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
		static auto wireEnds = std::array<int, 3>{ 5, 9, 13 };

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

				long joint = wireEnds[i];
				auto pos = GetJointPosition(&item, joint, Vector3i(-160, -8, 16));

				byte c = Random::GenerateInt(0, 64) + 128;
				TriggerDynamicLight(pos.x, pos.y, pos.z, 10, c >> 2, c >> 1, c);

				auto& spark = GetFreeSparkParticle();

				spark = {};
				spark.active = 1;
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
