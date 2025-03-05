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
    Offset,
    Speed,
    StartPositionX,
    StartPositionY,
    StartPositionZ,
    Damage,
    PauseCounter,
    PauseFrames
    };

	constexpr auto MOVING_LASER_DAMAGE = 100;
    constexpr float maxSpeed = 128.0f;
    constexpr float minSpeed = 1.0f;
    constexpr float acceleration = 32.0f;
    constexpr float wallThreshold = 512.0f;
    constexpr int pauseFrames = 30;

	void InitializeMovingLaser(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
        item.ItemFlags[Offset] = 1;
        item.ItemFlags[StartPositionX] = item.Pose.Position.x;
        item.ItemFlags[StartPositionY] = item.Pose.Position.y;
        item.ItemFlags[StartPositionZ] = item.Pose.Position.z;
        
	}

	void ControlMovingLaser(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;
        
        auto multiplier = 0;

        if (item.TriggerFlags)
        multiplier = item.TriggerFlags;

        float moveDistance = CLICK(1) * multiplier;

        if (!item.ItemFlags[Damage])
            item.ItemFlags[Damage] = MOVING_LASER_DAMAGE;

        item.Animation.ActiveState = 0;
        SpawnDynamicLight(item.Pose.Position.x, item.Pose.Position.y - 64, item.Pose.Position.z, (GetRandomControl() % 2) + 8, (GetRandomControl() % 4) + 24, GetRandomControl() % 4, GetRandomControl() % 2);
        item.MeshBits = -1 - (GetRandomControl() & 0x14); // To make lasers flicker

        if (multiplier > 0)
        {
            if (!item.ItemFlags[PauseFrames])
                item.ItemFlags[PauseFrames] = pauseFrames;

            auto startPosition = Vector3(item.ItemFlags[StartPositionX], item.ItemFlags[StartPositionY], item.ItemFlags[StartPositionZ]);

            float forwardX = std::sin(item.Pose.Orientation.y);
            float forwardZ = std::cos(item.Pose.Orientation.y);

            // Direction of movement, 1 for forward, -1 for backward
            int direction = item.ItemFlags[Offset];

            // Create a unit vector representing the forward direction
            Vector3 forwardDirection = Vector3(forwardX, 0.0f, forwardZ);
 
            // Calculate the target position by adding the direction vector scaled by moveDistance
            Vector3 targetPosition = startPosition + forwardDirection * moveDistance * direction;

            // Move towards the target position
            Vector3 directionToTarget = targetPosition - item.Pose.Position.ToVector3();
            float distanceToTarget = directionToTarget.Length();

            // Normalize the direction
            directionToTarget.Normalize();

            float speed = item.ItemFlags[Speed] > 0 ? item.ItemFlags[Speed] : minSpeed;

            // Handle 30-frame pause at the destination
            if (item.ItemFlags[PauseCounter] > 0)
            {
                item.ItemFlags[PauseCounter]--;
                AnimateItem(&item);
                return; // Skip movement during the pause
            }

            // Slow down if near a wall
            if (distanceToTarget < wallThreshold && speed > minSpeed)
                speed = std::max(minSpeed, (speed - acceleration)); // Gradual slowdown
            else if (distanceToTarget >= wallThreshold && speed < maxSpeed)
                speed = std::min(static_cast<float>(maxSpeed), (speed + acceleration)); // Gradual speed increase

            // Move the laser based on speed and direction
            item.Pose.Position.x += directionToTarget.x * speed * direction;
            item.Pose.Position.z += directionToTarget.z * speed * direction;

            // Calculate total distance moved from the start position
            Vector3 currentPosition = item.Pose.Position.ToVector3();
            float distanceTraveled = (currentPosition - startPosition).Length();

            // Reverse direction when reaching target position
            if (distanceToTarget < 256.0f || distanceTraveled > moveDistance)
            {
                // Start pause and reset speed
                item.ItemFlags[PauseCounter] = item.ItemFlags[PauseFrames];
                item.ItemFlags[Offset] = -direction;
                item.ItemFlags[Speed] = minSpeed;

                // Update start position
                item.ItemFlags[StartPositionX] = item.Pose.Position.x;
                item.ItemFlags[StartPositionY] = item.Pose.Position.y;
                item.ItemFlags[StartPositionZ] = item.Pose.Position.z;
            }

            // Store the updated speed
            item.ItemFlags[Speed] = speed;

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
			DoDamage(playerItem, item.ItemFlags[Damage]);
			DoLotsOfBlood(playerItem->Pose.Position.x, playerItem->Pose.Position.y + CLICK(3), playerItem->Pose.Position.z, 4, playerItem->Pose.Orientation.y, playerItem->RoomNumber, 3);
			playerItem->TouchBits.ClearAll();
		}
	}
}
