#include "framework.h"
#include "Objects/Effects/Fireflies.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/effects/effects.h"
#include "Game/effects/Streamer.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Specific/clock.h"
#include "Specific/level.h"

using namespace TEN::Collision::Point;
using namespace TEN::Effects::Streamer;
using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Effects::Fireflies
{
	constexpr auto FIREFLY_COHESION_FACTOR			  = 600.1f;
	constexpr auto FIREFLY_SPACING_FACTOR			  = 600.0f;
	constexpr auto FIREFLY_CATCH_UP_FACTOR			  = 0.2f;
	constexpr auto FIREFLY_TARGET_DISTANCE_MAX		  = SQUARE(BLOCK(1));
	constexpr auto FIREFLY_BASE_SEPARATION_DISTANCE	  = 10.0f;
	constexpr auto FIREFLY_FLEE_DISTANCE			  = BLOCK(0.7);
	constexpr auto FIREFLY_COUNT_MAX				  = 92;
	constexpr auto FIREFLY_COUNT_DEFAULT			  = 20;
	constexpr auto FIREFLY_ASCENT_FACTOR			  = 200;
	constexpr auto FIREFLY_RANGE_MAX				  = 8;
	constexpr auto FIREFLY_LIGHT_ALPHA_CYCLE_DURATION = 120.0f;

	std::vector<FireflyData> FireflySwarm;

	static auto NextFireflyIDMap = std::unordered_map<int, int>{}; // For streamer effect.

	void InitializeFireflySwarm(short itemNumber)
	{
		constexpr auto VEL_MAX = 160.0f;
		constexpr auto VEL_MIN = 32.0f;

		auto& item = g_Level.Items[itemNumber];

		item.Animation.Velocity.z = Random::GenerateFloat(VEL_MIN, VEL_MAX);

		item.HitPoints = FIREFLY_COUNT_DEFAULT;

		item.ItemFlags[(int)FirefliesItemFlags::TargetItemPtr] = item.Index;
		item.ItemFlags[(int)FirefliesItemFlags::Light] = 1; // 1 = light effect, 0 = no light effect.
		item.ItemFlags[(int)FirefliesItemFlags::TriggerFlags] = std::clamp((int)item.TriggerFlags, -FIREFLY_RANGE_MAX, FIREFLY_RANGE_MAX);
		item.ItemFlags[(int)FirefliesItemFlags::SpawnCounter] = 0;
		item.ItemFlags[(int)FirefliesItemFlags::RemoveFliesEffect] = 0;

		// Firefly IDs with light.
		item.ItemFlags[(int)FirefliesItemFlags::LightID0] = NO_VALUE;
		item.ItemFlags[(int)FirefliesItemFlags::LightID1] = NO_VALUE;
	}

	void SpawnFireflySwarm(ItemInfo& item, int triggerFlags)
	{
		constexpr auto VEL_MAX			 = 34.0f;
		constexpr auto VEL_MIN			 = 6.0f;
		constexpr auto FIREFLY_SPRITE_ID = 0;
		constexpr auto FLY_SPRITE_ID	 = 1;
		constexpr auto FIREFLY_SIZE		 = 10.0f; // TODO: Check.
		constexpr auto FLY_SIZE			 = 8.0f; // TODO: Check.

		// Create new firefly.
		auto& firefly = GetNewEffect(FireflySwarm, FIREFLY_COUNT_MAX);

		unsigned char r = 255;
		unsigned char g = 255;
		unsigned char b = 255;

		if (triggerFlags >= 0)
		{
			float brightnessShift = Random::GenerateFloat(-0.1f, 0.1f);
			r = std::clamp(item.Model.Color.x / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;
			g = std::clamp(item.Model.Color.y / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;
			b = std::clamp(item.Model.Color.z / 2.0f + brightnessShift, 0.0f, 1.0f) * UCHAR_MAX;

			firefly.SpriteSeqID = ID_FIREFLY_SPRITES;
			firefly.SpriteID = FIREFLY_SPRITE_ID;
			firefly.BlendMode = BlendMode::Additive;
			firefly.Size = FIREFLY_SIZE;
		}
		else
		{
			firefly.SpriteSeqID = ID_FIREFLY_SPRITES;
			firefly.SpriteID = FLY_SPRITE_ID;
			firefly.BlendMode = BlendMode::Subtractive;
			firefly.Size = FLY_SIZE;
		}

		// TODO: Use proper normalised colour in range [0.0f, 1.0f].
		firefly.r = firefly.rB = r;
		firefly.g = firefly.gB = g;
		firefly.b = firefly.bB = b;

		firefly.Position = item.Pose.Position.ToVector3();
		firefly.RoomNumber = item.RoomNumber;
		firefly.Orientation = item.Pose.Orientation;
		firefly.Velocity = Random::GenerateFloat(VEL_MIN, VEL_MAX);
		firefly.zVel = 0.3f; // TODO: Demagic.

		// TODO: Demagic.
		firefly.Life = Random::GenerateFloat(1.0f, 400.0f);

		firefly.TargetItem = &g_Level.Items[item.ItemFlags[(int)FirefliesItemFlags::TargetItemPtr]];

		int& nextFireflyID = NextFireflyIDMap[item.Index];
		firefly.ID = nextFireflyID++; // TODO: Increment should be on separate line for clarity.
	}

	void ControlFireflySwarm(short itemNumber)
	{
		constexpr auto ALPHA_PAUSE_DURATION = 2.0f;

		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
		{
			// Remove all fireflies associated with this item.
			ClearInactiveFireflies(item);

			// Reset ItemFlags.
			if (item.HitPoints == NOT_TARGETABLE)
				item.HitPoints = item.ItemFlags[(int)FirefliesItemFlags::SpawnCounter];

			item.ItemFlags[(int)FirefliesItemFlags::SpawnCounter] = 0;
			item.ItemFlags[(int)FirefliesItemFlags::LightID0] = NO_VALUE;
			item.ItemFlags[(int)FirefliesItemFlags::LightID1] = NO_VALUE;

			return;
		}

		static int frameCounter = 0;

		// Increment game frame counter.
		frameCounter++;

		if (item.HitPoints != NOT_TARGETABLE)
		{
			int fireflyCount = item.HitPoints - item.ItemFlags[(int)FirefliesItemFlags::SpawnCounter];

			if (fireflyCount < 0)
			{
				int firefliesToTurnOff = -fireflyCount;
				for (auto& firefly : FireflySwarm)
				{
					if (firefly.TargetItem == &item && firefly.Life > 0.0f)
					{
						firefly.Life = 0.0f;
						firefliesToTurnOff--;

						if (firefliesToTurnOff == 0)
							break;
					}
				}
			}
			else if (fireflyCount > 0)
			{
				for (int i = 0; i < fireflyCount; i++)
				{
					SpawnFireflySwarm(item, item.TriggerFlags);
				}
			}

			item.ItemFlags[(int)FirefliesItemFlags::SpawnCounter] = item.HitPoints;
			item.HitPoints = NOT_TARGETABLE;
		}
	
		// Update color values for blinking effect.
		float alphaFactor;

		for (auto& firefly : FireflySwarm)
		{
			auto targetItem = firefly.TargetItem;

			if (targetItem == &item)
			{
				// Choose one available firefly ID that has light.
				if (targetItem->ItemFlags[(int)FirefliesItemFlags::LightID0] == NO_VALUE && targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] >= 0)
				{
					targetItem->ItemFlags[(int)FirefliesItemFlags::LightID0] = Random::GenerateInt(0, targetItem->TriggerFlags);
				}
				// Two lights max for each cluster.
				if (targetItem->ItemFlags[(int)FirefliesItemFlags::LightID1] == NO_VALUE && targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] >= 0)
				{
					targetItem->ItemFlags[(int)FirefliesItemFlags::LightID1] = Random::GenerateInt(0, targetItem->TriggerFlags);
				}

				auto posBase = firefly.Position;
				auto rotMatrix = firefly.Orientation.ToRotationMatrix();
				auto pos = posBase + Vector3::Transform(Vector3(0, 0, 30), rotMatrix);
				auto dir0 = Geometry::RotatePoint(posBase, EulerAngles::Identity);
				short orient2D = firefly.Orientation.z;

				if (targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] >= 0)
				{
					StreamerEffect.Spawn(
						targetItem->Index, firefly.ID, pos, dir0, orient2D,
						Vector4(firefly.r / (float)UCHAR_MAX, firefly.g / (float)UCHAR_MAX, firefly.b / (float)UCHAR_MAX, 1.0f),
						Vector4::Zero,
						6.3f - (firefly.zVel / 12), ((firefly.Velocity / 8) + firefly.zVel * 3) / (float)UCHAR_MAX, 0.0f, -0.1f, 90.0f, StreamerFeatherMode::None, BlendMode::Additive);
				}
				else if (targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] < 0)
				{
					StreamerEffect.Spawn(
						targetItem->Index, firefly.ID, pos, dir0, orient2D,
						Vector4((firefly.r / 2) / (float)UCHAR_MAX, (firefly.g / 2) / (float)UCHAR_MAX, (firefly.b / 2) / (float)UCHAR_MAX, 0.2f),
						Vector4((firefly.r / 3) / (float)UCHAR_MAX, (firefly.g / 3) / (float)UCHAR_MAX, (firefly.b / 3) / (float)UCHAR_MAX, 0.2f),
						0.0f, 0.4f, 0.0f, 0.2f, 0.0f, StreamerFeatherMode::None, BlendMode::Subtractive);
				}

				if ((targetItem->ItemFlags[(int)FirefliesItemFlags::LightID0] == firefly.ID || targetItem->ItemFlags[(int)FirefliesItemFlags::LightID1] == firefly.ID) &&
					targetItem->ItemFlags[(int)FirefliesItemFlags::Light] == 1)
				{
					float totalCycleDuration = 2 * (FIREFLY_LIGHT_ALPHA_CYCLE_DURATION + ALPHA_PAUSE_DURATION);
					float alphaTime = fmod(frameCounter, totalCycleDuration);

					if (alphaTime < ALPHA_PAUSE_DURATION)
					{
						alphaFactor = 1.0f; // Pause on Alpha 1.
					}
					else if (alphaTime < ALPHA_PAUSE_DURATION + FIREFLY_LIGHT_ALPHA_CYCLE_DURATION)
					{
						alphaFactor = 1.0f - ((alphaTime - ALPHA_PAUSE_DURATION) / FIREFLY_LIGHT_ALPHA_CYCLE_DURATION);
					}
					else if (alphaTime < 2 * ALPHA_PAUSE_DURATION + FIREFLY_LIGHT_ALPHA_CYCLE_DURATION)
					{
						alphaFactor = 0.0f; // Pause on Alpha 0.
						targetItem->ItemFlags[(int)FirefliesItemFlags::LightID0] = NO_VALUE;
						targetItem->ItemFlags[(int)FirefliesItemFlags::LightID1] = NO_VALUE;
					}
					else
					{
						alphaFactor = (alphaTime - 2 * ALPHA_PAUSE_DURATION - FIREFLY_LIGHT_ALPHA_CYCLE_DURATION) / FIREFLY_LIGHT_ALPHA_CYCLE_DURATION;
					}

					SpawnDynamicLight(firefly.Position.x, firefly.Position.y, firefly.Position.z, 3,
						unsigned char(std::clamp(firefly.r * alphaFactor, 0.0f, (float)firefly.r)),
						unsigned char(std::clamp(firefly.g * alphaFactor, 0.0f, (float)firefly.g)),
						unsigned char(std::clamp(firefly.b * alphaFactor, 0.0f, (float)firefly.b)));
				}
			}
		}
	}

	void UpdateFireflySwarm()
	{
		constexpr auto FLEE_VEL				= 1.5f;
		constexpr auto ALPHA_PAUSE_DURATION = 100.0f;

		static const auto SPHERE = BoundingSphere(Vector3::Zero, BLOCK(1 / 8.0f));

		if (FireflySwarm.empty())
			return;

		const auto& playerItem = *LaraItem;

		static int frameCounter = 0;

		// Increment game frame counter.
		frameCounter++;

		for (auto& firefly : FireflySwarm)
		{
			if (firefly.Life <= 0.0f)
				continue;

			auto targetItem = firefly.TargetItem;

			if (targetItem->ItemFlags[(int)FirefliesItemFlags::RemoveFliesEffect])
			{
				firefly.r = 0;
				firefly.g = 0;
				firefly.b = 0;
				continue;
			}

			firefly.StoreInterpolationData();

			firefly.PositionTarget = Random::GeneratePointInSphere(SPHERE);

			auto spheroidAxis = Vector3(
				CLICK(targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] * 2),
				CLICK(targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] * 4),
				CLICK(targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] * 2));

			auto itemPos = Vector3i(targetItem->Pose.Position.x, targetItem->Pose.Position.y - FIREFLY_ASCENT_FACTOR, targetItem->Pose.Position.z);

			// Calculate desired position based on target object and random offsets.
			auto desiredPos = itemPos + Random::GeneratePointInSpheroid(firefly.PositionTarget, EulerAngles::Identity, spheroidAxis);
			auto dir = desiredPos - firefly.Position;

			auto dirs = dir.ToVector3();
			dirs.Normalize();
			auto dirNorm = dirs;

			// Define cohesion factor to keep fireflies close together.
			float distToTarget = dirs.Length();

			float targetVel = (distToTarget * FIREFLY_COHESION_FACTOR) + Random::GenerateFloat(3.0f, 5.0f);
			firefly.Velocity = std::min(targetVel, targetItem->Animation.Velocity.z - 21.0f);

			// Firefly is too far from target; increase velocity to catch up.
			if (distToTarget > FIREFLY_TARGET_DISTANCE_MAX)
				firefly.Velocity += FIREFLY_CATCH_UP_FACTOR;

			// Translate.
			auto moveDir = firefly.Orientation.ToDirection();
			moveDir.Normalize();
			firefly.Position += (moveDir * firefly.Velocity) / 26.0f;
			firefly.Position += (moveDir * FIREFLY_SPACING_FACTOR) / 26.0f;

			auto orientTo = Geometry::GetOrientToPoint(firefly.Position, desiredPos.ToVector3());
			firefly.Orientation.Lerp(orientTo, 0.1f);

			// Update color values for blinking effect.
			float totalCycleDuration = 2 * (FIREFLY_LIGHT_ALPHA_CYCLE_DURATION + ALPHA_PAUSE_DURATION);
			float alphaTime = fmod(frameCounter + firefly.Life, totalCycleDuration);

			float alpha = 0.0f;
			if (alphaTime < ALPHA_PAUSE_DURATION)
			{
				alpha = 1.0f;
			}
			else if (alphaTime < ALPHA_PAUSE_DURATION + FIREFLY_LIGHT_ALPHA_CYCLE_DURATION)
			{
				alpha = 1.0f - ((alphaTime - ALPHA_PAUSE_DURATION) / FIREFLY_LIGHT_ALPHA_CYCLE_DURATION);
			}
			else if (alphaTime < 2 * ALPHA_PAUSE_DURATION + FIREFLY_LIGHT_ALPHA_CYCLE_DURATION)
			{
				alpha = 0.0f;
			}
			else
			{
				alpha = (alphaTime - 2 * ALPHA_PAUSE_DURATION - FIREFLY_LIGHT_ALPHA_CYCLE_DURATION) / FIREFLY_LIGHT_ALPHA_CYCLE_DURATION;
			}

			firefly.r = unsigned char(firefly.rB * alpha);
			firefly.g = unsigned char(firefly.gB * alpha);
			firefly.b = unsigned char(firefly.bB * alpha);

			for (const auto& otherFirefly : FireflySwarm)
			{
				if (&firefly == &otherFirefly)
					continue;

				float distToOtherFirefly = Vector3::Distance(firefly.Position, otherFirefly.Position);
				float distToPlayer = Vector3::Distance(firefly.Position, playerItem.Pose.Position.ToVector3());

				// Player is too close; flee.
				if (distToPlayer < FIREFLY_FLEE_DISTANCE && playerItem.Animation.ActiveState != 2)
				{
					auto separationDir = firefly.Position - playerItem.Pose.Position.ToVector3();
					separationDir.Normalize();

					// Reduce Y component of escape direction.
					separationDir.y *= Random::GenerateFloat(0.0f, 0.4f);
					separationDir.Normalize();

					firefly.Position += separationDir * FLEE_VEL;

					auto orientTo = Geometry::GetOrientToPoint(firefly.Position, separationDir);
					firefly.Orientation.Lerp(orientTo, 0.05f);

					firefly.Velocity -= std::min(FLEE_VEL, firefly.TargetItem->Animation.Velocity.z - 1.0f);

					if (Random::TestProbability(1.0f / 700.0f) &&
						targetItem->ItemFlags[(int)FirefliesItemFlags::Light] == 1 &&
						targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] >= 0)
					{
						if (firefly.zVel == 0.3f)
						{
							firefly.zVel = 50.0f;
						}
					}

					if (firefly.zVel > 50.0f)
						firefly.zVel = 0.3f;
				}

				if (distToOtherFirefly < FIREFLY_BASE_SEPARATION_DISTANCE)
				{
					auto separationDir = firefly.Position - otherFirefly.Position;
					separationDir.Normalize();

					firefly.Position += separationDir * (FIREFLY_BASE_SEPARATION_DISTANCE - distToOtherFirefly);
				}
			}

			auto pointColl = GetPointCollision(firefly.Position, firefly.RoomNumber);

			// Update firefly room number.
			if (pointColl.GetRoomNumber() != firefly.RoomNumber &&
				pointColl.GetRoomNumber() != NO_VALUE)
			{
				firefly.RoomNumber = pointColl.GetRoomNumber();
			}

			if (targetItem->ItemFlags[(int)FirefliesItemFlags::Light] == 1 &&
				targetItem->ItemFlags[(int)FirefliesItemFlags::TriggerFlags] >= 0)
			{
				if (Random::TestProbability(1.0f / (700.0f - float(targetItem->ItemFlags[(int)FirefliesItemFlags::SpawnCounter] * 2))))
					firefly.zVel = 100.0f;

				if (firefly.zVel > 1.0f)
					firefly.zVel -= 2.0f;
				if (firefly.zVel <= 1.0f)
					firefly.zVel = 0.3f;
			}
		}
	}

	void ClearInactiveFireflies(ItemInfo& item)
	{
		FireflySwarm.erase(
			std::remove_if(
				FireflySwarm.begin(), FireflySwarm.end(),
				[&item](FireflyData& firefly)
				{
					if (firefly.TargetItem == &item)
					{
						firefly.Life = 0.0f;
						return true;
					}

					return false;
				}),
			FireflySwarm.end());

		NextFireflyIDMap.erase(item.Index);
	}

	void ClearFireflySwarm()
	{
		FireflySwarm.clear();
		NextFireflyIDMap.clear();
	}
}
