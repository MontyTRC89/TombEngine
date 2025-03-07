#include "framework.h"
#include "Objects/TR1/Trap/tr1_hammer.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/collision/Sphere.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/level.h"

using namespace TEN::Collision::Point;
using namespace TEN::Collision::Sphere;
using namespace TEN::Math;

//OCB 0 = Default TR1 behaviour
//OCB 1 = Retract after crashing once
//OCB 2 = Continuous crashing

namespace TEN::Entities::Traps
{
    constexpr auto HAMMER_HIT_FRAME = 30;

    enum HammerState
    {
        THOR_HAMMER_STATE_SET = 0,
        THOR_HAMMER_STATE_TEASE = 1,
        THOR_HAMMER_STATE_ACTIVE = 2,
        THOR_HAMMER_STATE_DONE = 3,
        THOR_HAMMER_STATE_RETRACT = 4,
    };

    enum HammerAnim
    {
        THOR_HAMMER_ANIM_SET = 0,
        THOR_HAMMER_ANIM_TEASE = 1,
        THOR_HAMMER_ANIM_ACTIVE = 2,
        THOR_HAMMER_ANIM_DONE = 3,
        THOR_HAMMER_ANIM_RETRACT = 4
    };

	static void InitializeHammer(ItemInfo& frontItem)
    {
        int backItemNumber = SpawnItem(frontItem, ID_HAMMER_BLOCK);

        if (backItemNumber == NO_VALUE)
        {
            TENLog("Failed to create hammer handle segment.", LogLevel::Warning);
            return;
        }

        auto& backItem = g_Level.Items[backItemNumber];

        // Store hammer segment item number.
        frontItem.ItemFlags[0] = backItemNumber;
        backItem.ItemFlags[0] = NO_VALUE;
    }

    void InitializeHandle(short itemNumber)
    {
        auto& item = g_Level.Items[itemNumber];

        // Initialize back body segment.
        InitializeHammer(item);
    }

    static void SyncHammerSegment(ItemInfo& frontItem)
    {
        short& backItemNumber = frontItem.ItemFlags[0];
        auto& backItem = g_Level.Items[backItemNumber];

        // Sync destruction.
        backItem.Status = frontItem.Status;

        // Sync animation.
        SetAnimation(backItem, GetAnimNumber(frontItem), GetFrameNumber(frontItem));

        // Sync position.
        backItem.Pose = frontItem.Pose;
        if (backItem.RoomNumber != frontItem.RoomNumber)
            ItemNewRoom(backItem.Index, frontItem.RoomNumber);
    }

	void ControlHandle(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		const auto& playerItem = *LaraItem;
        
        switch (item.Animation.ActiveState) {
        case THOR_HAMMER_STATE_SET:
            if (TriggerActive(&item)) {
                if (item.TriggerFlags == 1 && item.ItemFlags[1] == 1)
                {
                    item.Status = ITEM_NOT_ACTIVE;
                    break;

                }
                if (item.TriggerFlags == 2)
                {
                    item.Animation.TargetState = THOR_HAMMER_STATE_ACTIVE;
                    break;
                    
                }
                item.Animation.TargetState = THOR_HAMMER_STATE_TEASE;
            }
            else {
                RemoveActiveItem(itemNumber);
                item.Status = ITEM_NOT_ACTIVE;
            }
            break;

        case THOR_HAMMER_STATE_TEASE:
            if (TriggerActive(&item)) {
                item.Animation.TargetState = THOR_HAMMER_STATE_ACTIVE;
            }
            else {
                item.Animation.TargetState = THOR_HAMMER_STATE_SET;
            }
            break;

        case THOR_HAMMER_STATE_ACTIVE: {
            break;
        }

        case THOR_HAMMER_STATE_DONE: {
            if (item.TriggerFlags > 0)
            {
                item.Animation.TargetState = THOR_HAMMER_STATE_RETRACT;
                if (item.TriggerFlags == 1)
                {
                    item.ItemFlags[1] = 1;

                }
            }
            else {
                item.Status = ITEM_NOT_ACTIVE;
                RemoveActiveItem(itemNumber);
            }
            break;
        }
        }

        AnimateItem(&item);
        SyncHammerSegment(item);
	}
	
    void CollideHandle(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
    {
        auto& item = g_Level.Items[itemNumber];

        if (!TestBoundsCollide(&item, laraItem, coll->Setup.Radius))
            return;

        if (coll->Setup.EnableObjectPush)
            ItemPushItem(&item, laraItem, coll, false, 1);
    }

    void CollideHammer(short itemNumber, ItemInfo * playerItem, CollisionInfo * coll)
    {
        auto& item = g_Level.Items[itemNumber];

        if (!TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
            return;

        if (!HandleItemSphereCollision(item, *playerItem))
            return;

        if (item.Animation.ActiveState == THOR_HAMMER_STATE_ACTIVE && (item.Animation.FrameNumber - GetAnimData(item).frameBase) <= HAMMER_HIT_FRAME)
        {
            auto pointColl = GetPointCollision(*playerItem);
            int floorHeight = pointColl.GetFloorHeight();
            playerItem->Pose.Position.y = floorHeight;
            playerItem-> Animation.IsAirborne = false;
            playerItem->Animation.Velocity.y = 0.0f;
            playerItem->Animation.Velocity.z = 0.0f;
            playerItem->Pose.Scale = Vector3(1.0f, 0.1f, 1.0f);

            DoDamage(playerItem, INT_MAX);
            SetAnimation(playerItem, LA_BOULDER_DEATH);

            

        }
        else if (playerItem->HitPoints > 0)
        {
            ItemPushItem(&item, playerItem, coll, false, 1);
        }
	}
}