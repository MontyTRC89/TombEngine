#include "framework.h"
#include "Objects/TR5/Trap/tr5_movinglaser.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/collision/Sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/light.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Math/Utils.h"
#include "Specific/level.h"

using namespace TEN::Collision::Sphere;

namespace TEN::Entities::Traps
{
    enum MovingLaser {
    Speed,
    PauseCounter,
    Direction,
    DistanceTravelled
    };

	constexpr auto MOVING_LASER_DAMAGE = 100;
    constexpr int PAUSE_FRAMES = 30;
    constexpr float MAX_SPEED_THRESHOLD = 0.9f;
    constexpr float MIN_SPEED = 1.0f;
    constexpr float ACCELERATION = 8.0f;

	void InitializeMovingLaser(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
        item.ItemFlags[Direction] = 1;
        item.ItemFlags[Speed] = 10;
        item.Pose.Translate(item.Pose.Orientation, -CLICK(1)); //Offset by one click to make it dangerous at the edges of the block.
	}

	void ControlMovingLaser(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

        float moveDistance = (BLOCK(1) * item.TriggerFlags) + CLICK(2); //Use OCB to calculate the distance and add 2 clicks.
                
        float distancePerFrame = (static_cast<float>(CLICK(1)) * item.ItemFlags[Speed]) / FPS; // Calculate distance per frame
               
        item.Animation.ActiveState = 0;
        SpawnDynamicLight(item.Pose.Position.x, item.Pose.Position.y - 64, item.Pose.Position.z, (GetRandomControl() % 2) + 8, (GetRandomControl() % 4) + 24, GetRandomControl() % 4, GetRandomControl() % 2);
        item.MeshBits = -1 - (GetRandomControl() & 0x14); // To make lasers flicker

        if (item.TriggerFlags == 0)
        {
            AnimateItem(&item);
            return;
        }
        
        if (item.ItemFlags[PauseCounter] > 0)
        {
            item.ItemFlags[PauseCounter]--;

            if (item.ItemFlags[PauseCounter] == 0) 
            {
                item.ItemFlags[Direction] *= -1;
                item.ItemFlags[DistanceTravelled] = 0;
            }

            AnimateItem(&item);
            return;
        }

        item.Pose.Translate(item.Pose.Orientation, (item.ItemFlags[Direction] * distancePerFrame));
        
        item.ItemFlags[DistanceTravelled] += distancePerFrame;

        if (item.ItemFlags[DistanceTravelled] >= moveDistance)
        {
            item.ItemFlags[PauseCounter] = PAUSE_FRAMES;
        }

        // Update room if necessary
        short new_room = item.RoomNumber;
        GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &new_room);
        if (new_room != item.RoomNumber)
            ItemNewRoom(itemNumber, new_room);

        AnimateItem(&item);
    }

	void CollideMovingLaser(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		// Collide with objects.
		if (item.Status == ITEM_ACTIVE)
		{
			if (!TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
				return;

			HandleItemSphereCollision(item, *playerItem);
		}
		else if (item.Status != ITEM_INVISIBLE)
		{
			ObjectCollision(itemNumber, playerItem, coll);
		}
		
		// Damage entity.
		if (TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
		{
			DoDamage(playerItem, MOVING_LASER_DAMAGE);
			DoLotsOfBlood(playerItem->Pose.Position.x, playerItem->Pose.Position.y + CLICK(3), playerItem->Pose.Position.z, 4, playerItem->Pose.Orientation.y, playerItem->RoomNumber, 3);
			playerItem->TouchBits.ClearAll();
		}
	}
}
