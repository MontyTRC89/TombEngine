#include "framework.h"
#include "Objects/TR1/Trap/DamoclesSword.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/effects/effects.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps::TR1
{
	constexpr auto DAMOCLES_SWORD_DAMAGE = 100;

	constexpr auto DAMOCLES_SWORD_VELOCITY_MIN = BLOCK(1.0f / 20);
	constexpr auto DAMOCLES_SWORD_VELOCITY_MAX = BLOCK(1.0f / 8);

	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_2D		  = BLOCK(3.0f / 2);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL = BLOCK(3);

	const auto DAMOCLES_SWORD_TURN_RATE_MAX = ANGLE(5.5f);

	void SetupDamoclesSword(ObjectInfo* object)
	{
		object->initialise = InitialiseDamoclesSword;
		object->control = ControlDamoclesSword;
		object->collision = CollisionDamoclesSword;
		//object->shadowSize = UNIT_SHADOW;
		object->savePosition = 1;
		object->saveAnim = 1;
		object->saveFlags = 1;
	}

	void InitialiseDamoclesSword(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.Pose.Orientation.y = Random::GenerateAngle();
		item.Animation.TargetState = Random::GenerateAngle(-DAMOCLES_SWORD_TURN_RATE_MAX, DAMOCLES_SWORD_TURN_RATE_MAX); // NOTE: TargetState stores random turn rate.
		item.Animation.Velocity.y = DAMOCLES_SWORD_VELOCITY_MIN;
	}

	void ControlDamoclesSword(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (item.Animation.IsAirborne)
		{
			// Calculate vertical velocity.
			item.Pose.Orientation.y += item.Animation.TargetState; // NOTE: TargetState stores random turn rate.
			item.Animation.Velocity.y += (item.Animation.Velocity.y < DAMOCLES_SWORD_VELOCITY_MAX) ? GRAVITY : 1.0f;

			// Translate sword toward player.
			short headingAngle = Geometry::GetOrientToPoint(item.Pose.Position.ToVector3(), LaraItem->Pose.Position.ToVector3()).y;
			TranslateItem(&item, headingAngle, item.Animation.ActiveState); // NOTE: ActiveState stores calculated forward velocity.
			item.Pose.Position.y += item.Animation.Velocity.y;

			if (item.Pose.Position.y > item.Floor)
			{
				SoundEffect(SFX_TR1_DAMOCLES_ROOM_SWORD, &item.Pose);
				item.Pose.Position.y = item.Floor + 10;
				item.Animation.IsAirborne = false;
				item.Status = ItemStatus::ITEM_DEACTIVATED;
				RemoveActiveItem(itemNumber);
			}
		}
		else if (item.Pose.Position.y != item.Floor)
		{
			item.Pose.Orientation.y += item.Animation.TargetState; // NOTE: TargetState stores random turn rate.

			float distanceV = LaraItem->Pose.Position.y - item.Pose.Position.y;
			float distance2D = Vector2i::Distance(
				Vector2i(item.Pose.Position.x, item.Pose.Position.z),
				Vector2i(LaraItem->Pose.Position.x, LaraItem->Pose.Position.z));

			// Check relative position to player.
			if (distanceV < DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL &&
				distance2D <= DAMOCLES_SWORD_ACTIVATE_RANGE_2D &&
				item.Pose.Position.y < LaraItem->Pose.Position.y)
			{
				item.Animation.ActiveState = distance2D / 32; // NOTE: ActiveState stores calculated forward velocity.
				item.Animation.IsAirborne = true;
			}
		}
	}

	void CollisionDamoclesSword(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TestBoundsCollide(&item, laraItem, coll->Setup.Radius))
			return;

		if (coll->Setup.EnableObjectPush)
			ItemPushItem(&item, laraItem, coll, false, true);

		if (item.Animation.IsAirborne)
		{
			DoDamage(&item, DAMOCLES_SWORD_DAMAGE);

			auto bloodPos = Vector3i(
				Random::GenerateInt(-BLOCK(1.0f / 16), BLOCK(1.0f / 16)),
				Random::GenerateInt(-BLOCK(1.0f / 16), BLOCK(1.0f / 16)),
				Random::GenerateInt(0, BLOCK(3.0f / 4))
			) + laraItem->Pose.Position;
			int direction = laraItem->Pose.Orientation.y + Random::GenerateAngle(ANGLE(-11.25f), ANGLE(-11.25f));
			DoLotsOfBlood(bloodPos.x, bloodPos.y, bloodPos.z, laraItem->Animation.Velocity.z, direction, laraItem->RoomNumber, 10);
		}
	}
}
