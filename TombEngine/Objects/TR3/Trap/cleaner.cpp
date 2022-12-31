#include "framework.h"
#include "Objects/TR3/Trap/cleaner.h"

#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/effects/item_fx.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Game/effects/debris.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Spark;
using namespace TEN::Math::Random;

const auto CleanerDeadlyBits = std::vector<unsigned int>{ 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

/**************
* ItemFlags:
ItemFlags[0]: Rotation speed + direction
ItemFlags[1]: Flag that cleaner has reached a new sector so we can unblock the one behind us (unused right now but reserved just incase)
ItemFlags[2]: Movement speed. No speed = lose control
ItemFlags[3, 4, and 5]: Counters for the dynamic lights and sparks.
ItemFlags[6]: Target rotation angle.

* OCBs:
0: Stops when killing Lara.
Anything else: does not stop when killing Lara.
**************/

constexpr short CLEANER_TURN = 1024;
constexpr auto  CLEANER_SPEED = (SECTOR(1) / 16);

static bool NeedNewTarget(ItemInfo* item)
{
	// Checks if cleaner is in the centre of the block and facing the proper direction.

	if ((item->Pose.Position.z & (WALL_MASK)) == (SECTOR(1) / 2) && (item->Pose.Orientation.y == ANGLE(0) || item->Pose.Orientation.y == ANGLE(180)))
		return true;

	if ((item->Pose.Position.x & (WALL_MASK)) == (SECTOR(1) / 2) && (item->Pose.Orientation.y == ANGLE(90) || item->Pose.Orientation.y == ANGLE(-90)))
		return true;

	return false;
}

static bool CheckObjectAhead(ItemInfo* item)
{
	if (!GetCollidedObjects(item, CLICK(1), true, CollidedItems, CollidedMeshes, true))
		return false;

	long lp = 0;

	while (CollidedItems[lp])
	{
		if (CollidedItems[lp] == item || Objects[CollidedItems[lp]->ObjectNumber].intelligent)
		{
			lp++;
			continue;
		}

		return true;
	}

	return false;
}

static void CheckCleanerHeading(ItemInfo* item, long x, long y, long z, short roomNumber, bool& heading)
{
	FloorInfo* floor = GetFloor(x, y, z, &roomNumber);
	long h = GetFloorHeight(floor, x, y, z);
	ROOM_INFO* r = &g_Level.Rooms[roomNumber];
	floor = &r->floor[((z - r->z) / SECTOR(1)) + r->xSize * ((x - r->x) / SECTOR(1))];
	bool collide;

	/*
	long ox = item->Pose.Position.x;
	long oz = item->Pose.Position.z;
	item->Pose.Position.x = x;
	item->Pose.Position.z = z;
	collide = CheckObjectAhead(item);
	item->Pose.Position.x = ox;
	item->Pose.Position.z = oz;
	
	* CheckObjectAhead causes issues when item collision boxes are too big, keeping it just in case it is salvageable.
	*/
	collide = 0;
	heading = h == y && !collide && !floor->Stopper;
}

static void CleanerToItemCollision(ItemInfo* item)
{
	auto backupPos = item->Pose.Position;

	switch (item->Pose.Orientation.y)
	{
	case ANGLE(0):
		item->Pose.Position.z += 512;
		break;

	case ANGLE(90):
		item->Pose.Position.x += 512;
		break;

	case ANGLE(-90):
		item->Pose.Position.x -= 512;
		break;

	case ANGLE(-180):
		item->Pose.Position.z -= 512;
		break;
	}

	if (GetCollidedObjects(item, CLICK(1), true, CollidedItems, CollidedMeshes, true))
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

	item->Pose.Position = backupPos;
}

static void DoCleanerEffects(ItemInfo* item)
{
	static char WireEnds[3] = { 5, 9, 13 };

	SoundEffect(SFX_TR3_CLEANER_LOOP, &item->Pose);

	Vector3i vel
	(
		(GenerateInt(0, 255) * 4) - 512,
		 GenerateInt(0, 7) - 4,
		(GenerateInt(0, 255) * 4) - 512
	);

	for (int i = 0; i < 3; i++)
	{
		if ((!(GetRandomControl() & 7) && !item->ItemFlags[3 + i]) || item->ItemFlags[3 + i])
		{
			if (!item->ItemFlags[3 + i])
				item->ItemFlags[3 + i] = GenerateInt(0, 12) + 8;
			else
				item->ItemFlags[3 + i]--;

			long joint = WireEnds[i];
			auto pos = GetJointPosition(item, joint, Vector3i(-160, -8, 16));

			byte c = GenerateInt(0, 64) + 128;
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, c >> 2, c >> 1, c);

			auto& s = GetFreeSparkParticle();
			s = {};
			s.active = 1;
			s.age = 0;
			float color = (192.0F + GenerateFloat(0, 63.0F)) / 255.0F;
			s.sourceColor = Vector4(color / 4, color / 2, color, 1.0F);
			color = (192.0F + GenerateFloat(0, 63.0F)) / 255.0F;
			s.destinationColor = Vector4(color / 4, color / 2, color, 1.0F);
			s.life = GenerateFloat(20, 27);
			s.friction = 1.2f;
			s.gravity = 1.5f;
			s.width = 8.0f;
			s.height = 96.0f;
			auto v = vel.ToVector3();
			v.Normalize(v);
			s.velocity = v;
			s.pos = pos.ToVector3();
			s.room = item->RoomNumber;
		}
	}
}

void InitialiseCleaner(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];

	// Align item to the middle of the block it is on.
	item->Pose.Position.x = (item->Pose.Position.x & ~WALL_MASK) | (SECTOR(1) / 2);
	item->Pose.Position.z = (item->Pose.Position.z & ~WALL_MASK) | (SECTOR(1) / 2);

	// Init flags.
	item->ItemFlags[0] = CLEANER_TURN;
	item->ItemFlags[1] = 0;
	item->ItemFlags[2] = CLEANER_SPEED;
	item->Collidable = 1;
}

void CleanerControl(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
	{
		if (item->ItemFlags[2])
		{
			item->ItemFlags[2] = 0;
			SoundEffect(SFX_TR3_CLEANER_FUSEBOX, &item->Pose);
		}

		return;
	}

	if (!item->ItemFlags[2])
		return;

	static constexpr short QUADRANT_MASK = 0x3FFF;
	short rawAngle = item->Pose.Orientation.y & QUADRANT_MASK;

	// Not facing a quadrant, keep turning.
	if (rawAngle)
	{
		float coeff = std::max(0.25f, float((QUADRANT_MASK / 2) - abs(rawAngle - (QUADRANT_MASK / 2))) / float(QUADRANT_MASK / 2));

		float targetAngleDeg = TO_DEGREES(item->ItemFlags[6]);
		float turnAngleDeg   = TO_DEGREES(item->ItemFlags[0]) * coeff;
		float newAngle       = TO_DEGREES(item->Pose.Orientation.y) + turnAngleDeg;

		if ((item->ItemFlags[0] > 0 && abs(abs(newAngle) - abs(targetAngleDeg)) < 5.0f) ||
			(item->ItemFlags[0] < 0 && abs(abs(targetAngleDeg) - abs(newAngle)) < 5.0f))
		{
			newAngle = targetAngleDeg;
		}
		
		item->Pose.Orientation.y = ANGLE((newAngle));
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

			if (item->ItemFlags[1])
			{
				x = item->Pose.Position.x + (SECTOR(1) * phd_sin(item->Pose.Orientation.y + ANGLE(180.0F)));
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + (SECTOR(1) * phd_cos(item->Pose.Orientation.y + ANGLE(180.0F)));
				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				r = &g_Level.Rooms[roomNumber];
				r->floor[((z - r->z) / SECTOR(1)) + r->xSize * ((x - r->x) / SECTOR(1))].Stopper = 0;
				item->ItemFlags[1] = 0;
			}


			// Now check where we are heading and determine where to go next.
			bool left, ahead;
			y = item->Pose.Position.y;

			switch (item->Pose.Orientation.y)
			{
			case ANGLE(0):		// Facing Z+

				// Check if we can go left.
				x = item->Pose.Position.x - SECTOR(1);
				z = item->Pose.Position.z;
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, left);

				// Now check ahead.
				x = item->Pose.Position.x;
				z = item->Pose.Position.z + SECTOR(1);
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, ahead);

				if (!ahead && !left && item->ItemFlags[0] > 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->Pose.Orientation.y++;
					item->ItemFlags[0] = CLEANER_TURN;
				}
				else if (!ahead && !left && item->ItemFlags[0] < 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] -= CLEANER_TURN;
				}
				else if (left && item->ItemFlags[0] > 0)	// Prioritize left first.
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] = -CLEANER_TURN;
				}
				else
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->ItemFlags[0] = CLEANER_TURN;
					item->ItemFlags[1] = 1;
					item->Pose.Position.z += item->ItemFlags[2];
					x = item->Pose.Position.x;
					y = item->Pose.Position.y;
					z = item->Pose.Position.z + SECTOR(1);
					roomNumber = item->RoomNumber;
					floor = GetFloor(x, y, z, &roomNumber);
					r = &g_Level.Rooms[roomNumber];
					r->floor[((z - r->z) / SECTOR(1)) + r->xSize * ((x - r->x) / SECTOR(1))].Stopper = 1;
				}

				break;

			case ANGLE(90):		// Facing X+
				x = item->Pose.Position.x;
				z = item->Pose.Position.z + SECTOR(1);
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, left);

				x = item->Pose.Position.x + SECTOR(1);
				z = item->Pose.Position.z;
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, ahead);

				if (!ahead && !left && item->ItemFlags[0] > 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->Pose.Orientation.y++;
					item->ItemFlags[0] = CLEANER_TURN;
				}
				else if (!ahead && !left && item->ItemFlags[0] < 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] -= CLEANER_TURN;
				}
				else if (left && item->ItemFlags[0] > 0)	// Prioritize left first.
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] = -CLEANER_TURN;
				}
				else
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->ItemFlags[0] = CLEANER_TURN;
					item->ItemFlags[1] = 1;
					item->Pose.Position.x += item->ItemFlags[2];
					x = item->Pose.Position.x + SECTOR(1);
					y = item->Pose.Position.y;
					z = item->Pose.Position.z;
					roomNumber = item->RoomNumber;
					floor = GetFloor(x, y, z, &roomNumber);
					r = &g_Level.Rooms[roomNumber];
					r->floor[((z - r->z) / SECTOR(1)) + r->xSize * ((x - r->x) / SECTOR(1))].Stopper = 1;
				}

				break;

			case ANGLE(-90):	// Facing X-
				x = item->Pose.Position.x;
				z = item->Pose.Position.z - SECTOR(1);
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, left);

				x = item->Pose.Position.x - SECTOR(1);
				z = item->Pose.Position.z;
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, ahead);

				if (!ahead && !left && item->ItemFlags[0] > 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->Pose.Orientation.y++;
					item->ItemFlags[0] = CLEANER_TURN;
				}
				else if (!ahead && !left && item->ItemFlags[0] < 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] -= CLEANER_TURN;
				}
				else if (left && item->ItemFlags[0] > 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] = -CLEANER_TURN;
				}
				else
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->ItemFlags[0] = CLEANER_TURN;
					item->ItemFlags[1] = 1;
					item->Pose.Position.x -= item->ItemFlags[2];
					x = item->Pose.Position.x - SECTOR(1);
					y = item->Pose.Position.y;
					z = item->Pose.Position.z;
					roomNumber = item->RoomNumber;
					floor = GetFloor(x, y, z, &roomNumber);
					r = &g_Level.Rooms[roomNumber];
					r->floor[((z - r->z) / SECTOR(1)) + r->xSize * ((x - r->x) / SECTOR(1))].Stopper = 1;
				}

				break;

			case ANGLE(-180):	//facing Z-
				x = item->Pose.Position.x + SECTOR(1);
				z = item->Pose.Position.z;
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, left);

				x = item->Pose.Position.x;
				z = item->Pose.Position.z - SECTOR(1);
				CheckCleanerHeading(item, x, y, z, item->RoomNumber, ahead);

				if (!ahead && !left && item->ItemFlags[0] > 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->Pose.Orientation.y++;
					item->ItemFlags[0] = CLEANER_TURN;
				}
				else if (!ahead && !left && item->ItemFlags[0] < 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] -= CLEANER_TURN;
				}
				else if (left && item->ItemFlags[0] > 0)
				{
					item->ItemFlags[6] = item->Pose.Orientation.y - ANGLE(90);
					item->Pose.Orientation.y--;
					item->ItemFlags[0] = -CLEANER_TURN;
				}
				else
				{
					item->ItemFlags[6] = item->Pose.Orientation.y + ANGLE(90);
					item->ItemFlags[0] = CLEANER_TURN;
					item->ItemFlags[1] = 1;
					item->Pose.Position.z -= item->ItemFlags[2];
					x = item->Pose.Position.x;
					y = item->Pose.Position.y;
					z = item->Pose.Position.z - SECTOR(1);
					roomNumber = item->RoomNumber;
					floor = GetFloor(x, y, z, &roomNumber);
					r = &g_Level.Rooms[roomNumber];
					r->floor[((z - r->z) / SECTOR(1)) + r->xSize * ((x - r->x) / SECTOR(1))].Stopper = 1;
				}

				break;
			}

			TestTriggers(item, 1);
		}
		else
		{
			// No new target, so keep updating position.

			//No new target, so keep updating position.

			switch (item->Pose.Orientation.y)
			{
			case ANGLE(0):
				item->Pose.Position.z += item->ItemFlags[2];
				break;

			case ANGLE(90):
				item->Pose.Position.x += item->ItemFlags[2];
				break;

			case ANGLE(-90):
				item->Pose.Position.x -= item->ItemFlags[2];
				break;

			case ANGLE(-180):
				item->Pose.Position.z -= item->ItemFlags[2];
				break;
			}
		}
	}

	AnimateItem(item);

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

	if (item->RoomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	DoCleanerEffects(item);
	CleanerToItemCollision(item);
}

void CleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto item = &g_Level.Items[itemNumber];

	ObjectCollision(itemNumber, laraItem, coll);

	if (item->TouchBits.Test(CleanerDeadlyBits) && item->ItemFlags[2])
	{
		ItemElectricBurn(laraItem, -1);
		laraItem->HitPoints = 0;

		if (!item->TriggerFlags)
			item->ItemFlags[2] = 0;

		SoundEffect(SFX_TR3_CLEANER_FUSEBOX, &item->Pose);
	}
}
