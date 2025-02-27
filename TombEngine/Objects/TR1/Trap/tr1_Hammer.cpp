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
        if (backItem.Status == ITEM_DEACTIVATED)
        {
            KillItem(backItem.Index);
            backItemNumber = NO_VALUE;
            return;
        }

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
                item.Animation.TargetState = THOR_HAMMER_STATE_TEASE;
            }
            else {
               item.Status = ITEM_NOT_ACTIVE;
            }
            break;

        case THOR_HAMMER_STATE_TEASE:
            if (TriggerActive(&item)) {
                item.Animation.TargetState = THOR_HAMMER_STATE_ACTIVE;
            }
            else {
                item.Animation.TargetState = THOR_HAMMER_STATE_SET;
                TENLog("State 0 set", LogLevel::Warning);
            }
            break;

        case THOR_HAMMER_STATE_ACTIVE: {
            break;
        }

        case THOR_HAMMER_STATE_DONE: {
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
            DoDamage(playerItem, INT_MAX);
            SetAnimation(playerItem, LA_BOULDER_DEATH);
            playerItem->Animation.Velocity.y = 0.0f;
            playerItem->Animation.Velocity.z = 0.0f;

            auto bloodBox = GameBoundingBox(playerItem).ToBoundingOrientedBox(playerItem->Pose);
            auto bloodPos = Vector3i(Random::GeneratePointInBox(bloodBox));

            auto orientToHammer = Geometry::GetOrientToPoint(playerItem->Pose.Position.ToVector3(), item.Pose.Position.ToVector3());
            short randAngleOffset = Random::GenerateAngle(ANGLE(-11.25f), ANGLE(11.25f));
            short bloodHeadingAngle = orientToHammer.y + randAngleOffset;

            DoLotsOfBlood(bloodPos.x, bloodPos.y, bloodPos.z, playerItem->Animation.Velocity.z, bloodHeadingAngle, playerItem->RoomNumber, 20);
        }
        else if (playerItem->HitPoints > 0)
        {
            ItemPushItem(&item, playerItem, coll, false, 1);
        }
	}
}