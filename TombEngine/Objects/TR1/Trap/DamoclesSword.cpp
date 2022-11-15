#include "framework.h"
#include "Objects/TR1/Trap/DamoclesSword.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps::TR1
{
	constexpr auto DAMOCLES_SWORD_DAMAGE = 100;

	constexpr auto DAMOCLES_SWORD_VELOCITY_MIN = BLOCK(1.0f / 20);
	constexpr auto DAMOCLES_SWORD_VELOCITY_MAX = BLOCK(1.0f / 8);

	constexpr auto DAMOCLES_SWORD_IMPALE_DEPTH			  = -BLOCK(1.0f / 8);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_2D		  = BLOCK(3.0f / 2);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL = BLOCK(3);

	const auto DAMOCLES_SWORD_TURN_RATE_MAX = ANGLE(5.0f);

	void SetupDamoclesSword(ObjectInfo* object)
	{
		object->initialise = InitialiseDamoclesSword;
		object->control = ControlDamoclesSword;
		object->collision = CollideDamoclesSword;
		//object->shadowSize = UNIT_SHADOW;
		object->savePosition = true;
		object->saveAnim = true;
		object->saveFlags = true;
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
		const auto& laraItem = *LaraItem;

		// Fall toward player.
		if (item.Animation.IsAirborne)
		{
			item.Pose.Orientation.y += item.Animation.TargetState; // NOTE: TargetState stores random turn rate.

			// Calculate vertical velocity.
			item.Animation.Velocity.y += (item.Animation.Velocity.y < DAMOCLES_SWORD_VELOCITY_MAX) ? GRAVITY : 1.0f;

			// Translate sword.
			short headingAngle = Geometry::GetOrientToPoint(item.Pose.Position.ToVector3(), laraItem.Pose.Position.ToVector3()).y;
			TranslateItem(&item, headingAngle, item.Animation.ActiveState); // NOTE: ActiveState stores calculated 2D velocity.
			item.Pose.Position.y += item.Animation.Velocity.y;

			int vPos = item.Pose.Position.y;
			auto pointColl = GetCollision(&item);

			// Impale floor.
			if ((pointColl.Position.Floor - vPos) <= DAMOCLES_SWORD_IMPALE_DEPTH)
			{
				SoundEffect(SFX_TR1_DAMOCLES_ROOM_SWORD, &item.Pose);
				float distance = Vector3::Distance(item.Pose.Position.ToVector3(), Camera.pos.ToVector3());
				Camera.bounce = -((BLOCK(4) - distance) * abs(item.Animation.Velocity.y)) / BLOCK(4);

				item.Animation.TargetState = 0; // NOTE: TargetState stores random turn rate.
				item.Animation.IsAirborne = false;
				item.Status = ItemStatus::ITEM_DEACTIVATED;

				RemoveActiveItem(itemNumber);
			}

			return;
		}
		
		// Scan for player.
		if (item.Pose.Position.y < GetCollision(&item).Position.Floor)
		{
			item.Pose.Orientation.y += item.Animation.TargetState; // NOTE: TargetState stores random turn rate.

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

			// Check relative position to player.
			if (item.Pose.Position.y < laraItem.Pose.Position.y)
			{
				// TODO: Have 2D velocity also take vertical distance into account.
				item.Animation.ActiveState = distance2D / 32; // NOTE: ActiveState stores calculated 2D velocity.
				item.Animation.IsAirborne = true;
			}

			return;
		}
	}

	void CollideDamoclesSword(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TestBoundsCollide(&item, laraItem, coll->Setup.Radius))
			return;

		if (coll->Setup.EnableObjectPush)
			ItemPushItem(&item, laraItem, coll, false, true);

		if (item.Animation.IsAirborne)
		{
			DoDamage(laraItem, DAMOCLES_SWORD_DAMAGE);

			auto bloodBounds = GameBoundingBox(laraItem).ToBoundingOrientedBox(laraItem->Pose);
			auto bloodPos = Vector3i(Random::GenerateVector3InBox(bloodBounds) + bloodBounds.Center);

			auto orientToSword = Geometry::GetOrientToPoint(laraItem->Pose.Position.ToVector3(), item.Pose.Position.ToVector3());
			short randAngleOffset = Random::GenerateAngle(ANGLE(-11.25f), ANGLE(11.25f));
			short bloodHeadingAngle = orientToSword.y + randAngleOffset;
			
			DoLotsOfBlood(bloodPos.x, bloodPos.y, bloodPos.z, laraItem->Animation.Velocity.z, bloodHeadingAngle, laraItem->RoomNumber, 20);
		}
	}
}
