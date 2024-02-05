#include "framework.h"
#include "Objects/TR4/Trap/SpikyWall.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps
{
	constexpr auto SPIKY_WALL_HARM_DAMAGE = 15;

	void InitializeSpikyWall(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[0] = item.TriggerFlags;
		item.ItemFlags[1] = 0;
	}

	void ControlSpikyWall(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		item.ItemFlags[0] = item.TriggerFlags;
		
		if (item.ItemFlags[1] && 
			item.ItemFlags[0])
		{
			item.ItemFlags[1] -= 1;
		}

		int forwardVel = item.ItemFlags[0];
		auto bounds = GameBoundingBox(&item);

		auto pointColl = GetCollision(&item, item.Pose.Orientation.y, (forwardVel >= 0) ? bounds.Z2 : bounds.Z1, bounds.Y2);
		auto pointColl2 = GetCollision(&item, item.Pose.Orientation.y, (forwardVel >= 0) ? bounds.Z2 : bounds.Z1, bounds.Y2, (bounds.X2 - bounds.X1) /2);

		if (GetCollidedObjects(&item, CLICK(1), true, CollidedItems, CollidedMeshes, true))
		{
			int lp = 0;
			while (CollidedItems[lp] != nullptr)
			{
				if (Objects[CollidedItems[lp]->ObjectNumber].intelligent)
				{
					CollidedItems[lp]->HitPoints = 0;
					
				}
				else if (CollidedItems[lp]->ObjectNumber == ID_SPIKY_WALL && !item.ItemFlags[1])
				{
					CollidedItems[lp]->TriggerFlags = 0;
					item.TriggerFlags = 0;
					item.Status = ITEM_DEACTIVATED;
					StopSoundEffect(SFX_TR4_ROLLING_BALL);
					item.ItemFlags[1] = 10;
				}

				lp++;
			}
		}

		// Stop moving.
		if (!item.TriggerFlags ||
			pointColl.Block->IsWall(item.Pose.Position.x, item.Pose.Position.z) || 
			pointColl.Block->Stopper ||
			pointColl2.Block->IsWall(item.Pose.Position.x, item.Pose.Position.z) ||
			pointColl2.Block->Stopper )
		{
			auto& room = g_Level.Rooms[item.RoomNumber];
			auto pos = item.Pose.Position;

			for (auto& mesh : room.mesh)
			{
				if ((abs(pointColl.Coordinates.x - mesh.pos.Position.x) < BLOCK(1) &&
					abs(pointColl.Coordinates.z - mesh.pos.Position.z) < BLOCK(1)) ||
					abs(pointColl2.Coordinates.x - mesh.pos.Position.x) < BLOCK(1) &&
					abs(pointColl2.Coordinates.z - mesh.pos.Position.z) < BLOCK(1) &&
					!(StaticObjects[mesh.staticNumber].shatterType == ShatterType::None))
				{					
					if (mesh.HitPoints == 0)
					{
						mesh.HitPoints -= 1;
						ShatterObject(nullptr, &mesh, -64, LaraItem->RoomNumber, 0);
						SoundEffect(SFX_TR4_SMASH_ROCK, &item.Pose);
						TestTriggers(pos.x, pos.y, pos.z, item.RoomNumber, true);
					}
				}
			}

			item.Status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		// Move.
		else
		{
			item.Status = ITEM_ACTIVE;

			item.Pose.Position = Geometry::TranslatePoint(item.Pose.Position, item.Pose.Orientation.y, forwardVel);

			if (pointColl.RoomNumber != item.RoomNumber)
				ItemNewRoom(itemNumber, pointColl.RoomNumber);

			SoundEffect(SFX_TR4_ROLLING_BALL, &item.Pose);
		}
	}

	void CollideSpikyWall(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (item.ObjectNumber != ID_SPIKY_WALL)
			return;

		// Collide with objects.
		if (item.Status == ITEM_ACTIVE)
		{
			if (!TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
				return;

			TestCollision(&item, playerItem);
		}
		else if (item.Status != ITEM_INVISIBLE)
		{
			ObjectCollision(itemNumber, playerItem, coll);
		}

		// Damage entity.
		if (TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
		{
			DoDamage(playerItem, SPIKY_WALL_HARM_DAMAGE);
			DoLotsOfBlood(playerItem->Pose.Position.x, playerItem->Pose.Position.y + CLICK(2), playerItem->Pose.Position.z, 4, playerItem->Pose.Orientation.y, playerItem->RoomNumber, 3);
			playerItem->TouchBits.ClearAll();

			SoundEffect(SFX_TR4_LARA_GRABFEET, &playerItem->Pose);
			//pushes Lara along with the wall.
			int velocity = playerItem->Animation.Velocity.z;

			playerItem->Pose.Position = Geometry::TranslatePoint(playerItem->Pose.Position, item.Pose.Orientation.y, (item.ItemFlags[0] > 0) ? (item.ItemFlags[0] + velocity) : (item.ItemFlags[0] - velocity));
		}
	}
}
