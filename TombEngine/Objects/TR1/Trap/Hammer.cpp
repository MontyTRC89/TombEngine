#include "framework.h"
#include "Objects/TR1/Trap/DamoclesSword.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/level.h"

using namespace TEN::Collision::Point;
using namespace TEN::Math;

namespace TEN::Entities::Traps
{
	// NOTES:
	// ItemFlags[0] = random turn rate when active.
	// ItemFlags[1] = calculated forward velocity.

	constexpr auto DAMOCLES_SWORD_DAMAGE = 100;

	constexpr auto DAMOCLES_SWORD_VELOCITY_MIN = BLOCK(1 / 20.0f);
	constexpr auto DAMOCLES_SWORD_VELOCITY_MAX = BLOCK(1 / 8.0f);

	constexpr auto DAMOCLES_SWORD_IMPALE_DEPTH			  = -BLOCK(1 / 8.0f);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_2D		  = BLOCK(1.5f);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL = BLOCK(3);

	constexpr auto DAMOCLES_SWORD_TURN_RATE_MAX = ANGLE(5.0f);
	constexpr auto DAMOCLES_SWORD_TURN_RATE_MIN = ANGLE(1.0f);

	void InitializeDamoclesSword(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		int sign = Random::TestProbability(0.5f) ? 1 : -1;

		item.Pose.Orientation.y = Random::GenerateAngle();
		item.Animation.Velocity.y = DAMOCLES_SWORD_VELOCITY_MIN;
		item.ItemFlags[0] = Random::GenerateAngle(DAMOCLES_SWORD_TURN_RATE_MIN, DAMOCLES_SWORD_TURN_RATE_MAX) * sign;
	}

	void ControlDamoclesSword(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		const auto& laraItem = *LaraItem;

		// Fall toward player.
		if (item.Animation.IsAirborne)
		{
			item.Pose.Orientation.y += item.ItemFlags[0];

			// Calculate vertical velocity.
			item.Animation.Velocity.y += (item.Animation.Velocity.y < DAMOCLES_SWORD_VELOCITY_MAX) ? g_GameFlow->GetSettings()->Physics.Gravity : 1.0f;

			// Translate sword.
			short headingAngle = Geometry::GetOrientToPoint(item.Pose.Position.ToVector3(), laraItem.Pose.Position.ToVector3()).y;
			TranslateItem(&item, headingAngle, item.ItemFlags[1], item.Animation.Velocity.y);

			int vPos = item.Pose.Position.y;
			auto pointColl = GetPointCollision(item);

			// Impale floor.
			if ((pointColl.GetFloorHeight() - vPos) <= DAMOCLES_SWORD_IMPALE_DEPTH)
			{
				SoundEffect(SFX_TR1_DAMOCLES_ROOM_SWORD, &item.Pose);
				float distance = Vector3::Distance(item.Pose.Position.ToVector3(), Camera.pos.ToVector3());
				Camera.bounce = -((BLOCK(7.0f / 2) - distance) * abs(item.Animation.Velocity.y)) / BLOCK(7.0f / 2);

				item.Animation.IsAirborne = false;
				item.Status = ItemStatus::ITEM_DEACTIVATED;
				item.ItemFlags[0] = 0;

				RemoveActiveItem(itemNumber);
			}

			return;
		}
		
		// Scan for player.
		if (item.Pose.Position.y < GetPointCollision(item).GetFloorHeight())
		{
			item.Pose.Orientation.y += item.ItemFlags[0];

			// Check vertical position to player.
			if (item.Pose.Position.y >= laraItem.Pose.Position.y)
				return;

			// Check vertical distance.
			float distanceV = laraItem.Pose.Position.y - item.Pose.Position.y;
			if (distanceV > DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL)
				return;

			// Check 2D distance.
			float distance2D = Vector2i::Distance(
				Vector2i(item.Pose.Position.x, item.Pose.Position.z),
				Vector2i(laraItem.Pose.Position.x, laraItem.Pose.Position.z));
			if (distance2D > DAMOCLES_SWORD_ACTIVATE_RANGE_2D)
				return;

			// Drop sword.
			// TODO: Have 2D velocity also take vertical distance into account.
			item.Animation.IsAirborne = true;
			item.ItemFlags[1] = distance2D / 32;
			return;
		}
	}

	void CollideDamoclesSword(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TestBoundsCollide(&item, laraItem, coll->Setup.Radius))
			return;

		if (coll->Setup.EnableObjectPush)
			ItemPushItem(&item, laraItem, coll, false, 1);

		if (item.Animation.IsAirborne)
		{
			DoDamage(laraItem, DAMOCLES_SWORD_DAMAGE);

			auto bloodBox = GameBoundingBox(laraItem).ToBoundingOrientedBox(laraItem->Pose);
			auto bloodPos = Vector3i(Random::GeneratePointInBox(bloodBox));

			auto orientToSword = Geometry::GetOrientToPoint(laraItem->Pose.Position.ToVector3(), item.Pose.Position.ToVector3());
			short randAngleOffset = Random::GenerateAngle(ANGLE(-11.25f), ANGLE(11.25f));
			short bloodHeadingAngle = orientToSword.y + randAngleOffset;
			
			DoLotsOfBlood(bloodPos.x, bloodPos.y, bloodPos.z, laraItem->Animation.Velocity.z, bloodHeadingAngle, laraItem->RoomNumber, 20);
		}
	}
}

/*#include "game/items.h"
#include "game/lara/common.h"
#include "game/objects/common.h"
#include "game/room.h"
#include "global/vars.h"

#include <libtrx/debug.h>

typedef enum {
    THOR_HAMMER_STATE_SET = 0,
    THOR_HAMMER_STATE_TEASE = 1,
    THOR_HAMMER_STATE_ACTIVE = 2,
    THOR_HAMMER_STATE_DONE = 3,
} THOR_HAMMER_STATE;

static void M_SetupHandle(OBJECT *obj);
static void M_InitialiseHandle(int16_t item_num);
static void M_ControlHandle(int16_t item_num);
static void M_CollisionHandle(
    int16_t item_num, ITEM *lara_item, COLL_INFO *coll);

static void M_SetupHead(OBJECT *obj);
static void M_CollisionHead(int16_t item_num, ITEM *lara_item, COLL_INFO *coll);

static void M_SetupHandle(OBJECT *const obj)
{
    obj->initialise_func = M_InitialiseHandle;
    obj->control_func = M_ControlHandle;
    obj->draw_func = Object_DrawUnclippedItem;
    obj->collision_func = M_CollisionHandle;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

static void M_InitialiseHandle(const int16_t item_num)
{
    ITEM *const hand_item = Item_Get(item_num);
    const int16_t head_item_num = Item_CreateLevelItem();
    ASSERT(head_item_num != NO_ITEM);
    ITEM *const head_item = Item_Get(head_item_num);
    head_item->object_id = O_THORS_HEAD;
    head_item->room_num = hand_item->room_num;
    head_item->pos = hand_item->pos;
    head_item->rot = hand_item->rot;
    head_item->shade.value_1 = hand_item->shade.value_1;
    Item_Initialise(head_item_num);
    hand_item->data = head_item;
}

static void M_ControlHandle(const int16_t item_num)
{
    ITEM *const item = Item_Get(item_num);

    switch (item->current_anim_state) {
    case THOR_HAMMER_STATE_SET:
        if (Item_IsTriggerActive(item)) {
            item->goal_anim_state = THOR_HAMMER_STATE_TEASE;
        } else {
            Item_RemoveActive(item_num);
            item->status = IS_INACTIVE;
        }
        break;

    case THOR_HAMMER_STATE_TEASE:
        if (Item_IsTriggerActive(item)) {
            item->goal_anim_state = THOR_HAMMER_STATE_ACTIVE;
        } else {
            item->goal_anim_state = THOR_HAMMER_STATE_SET;
        }
        break;

    case THOR_HAMMER_STATE_ACTIVE: {
        const int32_t frame_num = Item_GetRelativeFrame(item);
        if (frame_num > 30) {
            int32_t x = item->pos.x;
            int32_t z = item->pos.z;

            switch (item->rot.y) {
            case 0:
                z += WALL_L * 3;
                break;
            case DEG_90:
                x += WALL_L * 3;
                break;
            case -DEG_90:
                x -= WALL_L * 3;
                break;
            case -DEG_180:
                z -= WALL_L * 3;
                break;
            }

            if (g_LaraItem->hit_points >= 0 && g_LaraItem->pos.x > x - 520
                && g_LaraItem->pos.x < x + 520 && g_LaraItem->pos.z > z - 520
                && g_LaraItem->pos.z < z + 520) {
                g_LaraItem->hit_points = -1;
                g_LaraItem->pos.y = item->pos.y;
                g_LaraItem->gravity = 0;
                g_LaraItem->current_anim_state = LS_SPECIAL;
                g_LaraItem->goal_anim_state = LS_SPECIAL;
                Item_SwitchToAnim(g_LaraItem, LA_ROLLING_BALL_DEATH, 0);
            }
        }
        break;
    }

    case THOR_HAMMER_STATE_DONE: {
        int32_t x = item->pos.x;
        int32_t z = item->pos.z;
        int32_t old_x = x;
        int32_t old_z = z;

        Room_TestTriggers(item);

        switch (item->rot.y) {
        case 0:
            z += WALL_L * 3;
            break;
        case DEG_90:
            x += WALL_L * 3;
            break;
        case -DEG_90:
            x -= WALL_L * 3;
            break;
        case -DEG_180:
            z -= WALL_L * 3;
            break;
        }

        item->pos.x = x;
        item->pos.z = z;
        if (g_LaraItem->hit_points >= 0) {
            Room_AlterFloorHeight(item, -WALL_L * 2);
        }
        item->pos.x = old_x;
        item->pos.z = old_z;

        Item_RemoveActive(item_num);
        item->status = IS_DEACTIVATED;
        break;
    }
    }
    Item_Animate(item);

    ITEM *const head_item = item->data;
    const int16_t relative_anim = Item_GetRelativeAnim(item);
    const int16_t relative_frame = Item_GetRelativeFrame(item);
    Item_SwitchToAnim(head_item, relative_anim, relative_frame);
    head_item->current_anim_state = item->current_anim_state;
}

static void M_CollisionHandle(
    const int16_t item_num, ITEM *const lara_item, COLL_INFO *const coll)
{
    ITEM *const item = Item_Get(item_num);
    if (!Lara_TestBoundsCollide(item, coll->radius)) {
        return;
    }
    if (coll->enable_baddie_push) {
        Lara_Push(item, coll, false, true);
    }
}

static void M_SetupHead(OBJECT *const obj)
{
    obj->collision_func = M_CollisionHead;
    obj->draw_func = Object_DrawUnclippedItem;
    obj->save_flags = 1;
    obj->save_anim = 1;
}

static void M_CollisionHead(
    const int16_t item_num, ITEM *const lara_item, COLL_INFO *const coll)
{
    ITEM *const item = Item_Get(item_num);
    if (!Lara_TestBoundsCollide(item, coll->radius)) {
        return;
    }
    if (coll->enable_baddie_push
        && item->current_anim_state != THOR_HAMMER_STATE_ACTIVE) {
        Lara_Push(item, coll, false, true);
    }
}

REGISTER_OBJECT(O_THORS_HANDLE, M_SetupHandle)
REGISTER_OBJECT(O_THORS_HEAD, M_SetupHead)*/