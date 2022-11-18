#include "framework.h"
#include "Game/effects/Blood.h"

#include "Game/collision/collide_room.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"

#include "lara.h"

#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"

using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Effects::Blood
{
	constexpr auto BLOOD_DRIP_LIFE_MAX			 = 5.0f * FPS;
	constexpr auto BLOOD_STAIN_LIFE_MAX			 = (4.0f * 60.0f)* FPS;
	constexpr auto BLOOD_STAIN_LIFE_START_FADING = 10.0f * FPS;

	constexpr auto BLOOD_DRIP_GRAVITY_MIN	  = 5.0f;
	constexpr auto BLOOD_DRIP_GRAVITY_MAX	  = 15.0f;
	constexpr auto BLOOD_DRIP_SPRAY_SEMIANGLE = 60.0f;

	constexpr auto BLOOD_STAIN_OPACITY_MAX		  = 0.8f;
	constexpr auto BLOOD_STAIN_POOLING_SCALE_RATE = 0.4f;
	constexpr auto BLOOD_STAIN_POOLING_TIME_DELAY = 10.0f;
	constexpr auto BLOOD_STAIN_HEIGHT_OFFSET	  = 4;
	constexpr auto BLOOD_STAIN_NUM_SPRITES		  = 11;

	constexpr auto BLOOD_COLOR_RED	 = Vector4(0.8f, 0.0f, 0.0f, 1.0f);
	constexpr auto BLOOD_COLOR_BROWN = Vector4(0.4f, 0.2f, 0.0f, 1.0f);

	std::array<BloodDrip, BLOOD_DRIP_NUM_MAX> BloodDrips  = {};
	std::deque<BloodStain>					  BloodStains = {};

	auto& GetFreeBloodDrip()
	{
		for (auto& drip : BloodDrips)
		{
			if (!drip.IsActive)
				return drip;
		}

		return BloodDrips[0];
	}

	std::array<Vector3, 4> GetBloodStainVertexPoints(const Vector3& normal, const Vector3& pos, short orient2D, float scale)
	{
		constexpr auto point0 = Vector3(SQRT_2, 0.0f, SQRT_2);
		constexpr auto point1 = Vector3(-SQRT_2, 0.0f, SQRT_2);
		constexpr auto point2 = Vector3(-SQRT_2, 0.0f, -SQRT_2);
		constexpr auto point3 = Vector3(SQRT_2, 0.0f, -SQRT_2);

		Color(4);
		// No scale; return early.
		if (scale == 0.0f)
			return std::array<Vector3, 4>{ pos, pos, pos, pos };

		// Determine surface angles.
		short aspectAngle = Geometry::GetSurfaceAspectAngle(normal);
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(normal);

		short deltaAngle = Geometry::GetShortestAngle(orient2D, aspectAngle);
		float sinDeltaAngle = phd_sin(deltaAngle);
		float cosDeltaAngle = phd_cos(deltaAngle);

		// Calculate rotation matrix.
		auto orient = EulerAngles(
			-slopeAngle * cosDeltaAngle,
			0,
			slopeAngle * sinDeltaAngle
		) + EulerAngles(0, orient2D, 0);
		auto rotMatrix = orient.ToRotationMatrix();

		return std::array<Vector3, 4>
		{
			pos + Vector3::Transform(point0 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(point1 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(point2 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(point3 * (scale / 2), rotMatrix)
		};
	}

	void SpawnBloodMist(const Vector3& pos, int roomNumber, const Vector3& direction, unsigned int count)
	{
		TriggerBlood(pos.x, pos.y, pos.z, direction.y, count);
	}

	void SpawnBloodMistCloud(const Vector3& pos, int roomNumber, const Vector3& direction, float velocity, unsigned int count)
	{
		static const auto box = BoundingOrientedBox(pos, Vector3::One * BLOCK(1.0f / 16), Quaternion(direction, 1.0f));

		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		for (int i = 0; i < count; i++)
		{
			auto randPos = pos + Random::GenerateVector3InBox(box);
			SpawnBloodMist(randPos, roomNumber, direction, count);
		}
	}

	void SpawnBloodMistCloudUnderwater(const Vector3& pos, int roomNumber, float velocity)
	{
		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;
		
		TriggerUnderwaterBlood(pos.x, pos.y, pos.z, velocity);
	}

	void SpawnBloodDrip(const Vector3& pos, int roomNumber, const Vector3& velocity, float scale)
	{
		auto& drip = GetFreeBloodDrip();

		drip.IsActive = true;
		drip.Position = pos;
		drip.RoomNumber = roomNumber;
		drip.Velocity = velocity;
		drip.Color = BLOOD_COLOR_RED;
		drip.Life = BLOOD_DRIP_LIFE_MAX;
		drip.Scale = scale;
		drip.Gravity = Random::GenerateFloat(BLOOD_DRIP_GRAVITY_MIN, BLOOD_DRIP_GRAVITY_MAX);
	}

	void SpawnBloodDripSpray(const Vector3& pos, int roomNumber, const Vector3& direction, const Vector3& baseVelocity, unsigned int maxCount)
	{
		SpawnBloodMistCloud(pos, roomNumber, baseVelocity + direction, Random::GenerateFloat(10.0f, 70.0f), maxCount * 3);

		unsigned int numDrips = Random::GenerateInt(0, maxCount);
		for (int i = 0; i < numDrips; i++)
		{
			float length = Random::GenerateFloat(40.0f, 80.0f);
			auto velocity = baseVelocity + Random::GenerateVector3InCone(direction, BLOOD_DRIP_SPRAY_SEMIANGLE, length);
			float scale = Random::GenerateFloat(10.0f, 20.0f);

			SpawnBloodDrip(pos, roomNumber, velocity, scale);
		}
	}

	void SpawnBloodStain(const Vector3& pos, int roomNumber, const Vector3& normal, float scaleMax, float scaleRate, float delayTimeInSec)
	{
		auto stain = BloodStain();

		stain.SpriteIndex = Random::GenerateInt(0, BLOOD_STAIN_NUM_SPRITES);
		stain.Position = pos;
		stain.RoomNumber = roomNumber;
		stain.Orientation2D = Random::GenerateAngle();
		stain.Normal = normal;
		stain.Color = BLOOD_COLOR_RED;
		stain.ColorStart = stain.Color;
		stain.ColorEnd = BLOOD_COLOR_BROWN;
		stain.VertexPoints = GetBloodStainVertexPoints(stain.Normal, stain.Position, stain.Orientation2D, 0.0f);
		stain.Life = BLOOD_STAIN_LIFE_MAX;
		stain.LifeStartFading = BLOOD_STAIN_LIFE_START_FADING;
		stain.Scale = 0.0f;
		stain.ScaleMax = scaleMax;
		stain.ScaleRate = scaleRate;
		stain.Opacity = BLOOD_STAIN_OPACITY_MAX;
		stain.OpacityStart = stain.Opacity;
		stain.DelayTime = std::round(delayTimeInSec * FPS); // NOTE: Converted to frame time.

		if (BloodStains.size() >= BLOOD_STAIN_NUM_MAX)
			BloodStains.pop_back();

		BloodStains.push_front(stain);
	}

	void SpawnBloodStain(ItemInfo& item)
	{
		auto pos = Vector3(item.Pose.Position.x, item.Pose.Position.y - BLOOD_STAIN_HEIGHT_OFFSET, item.Pose.Position.z);
		auto normal = Geometry::GetFloorNormal(GetCollision(&item).FloorTilt);

		auto bounds = GameBoundingBox(&item);
		float scaleMax = (bounds.GetWidth() + bounds.GetDepth()) / 2;

		SpawnBloodStain(pos, item.RoomNumber, normal, scaleMax, BLOOD_STAIN_POOLING_SCALE_RATE, BLOOD_STAIN_POOLING_TIME_DELAY);
	}

	//
	void UpdateBloodMists()
	{

	}
	
	void UpdateBloodDrips()
	{
		if (BloodDrips.empty())
			return;

		for (auto& drip : BloodDrips)
		{
			if (!drip.IsActive)
				continue;

			drip.Life -= 1.0f;

			// Despawn drip.
			if (drip.Life <= 0.0f)
			{
				drip.IsActive = false;
				continue;
			}

			// Update position.
			drip.Velocity.y += drip.Gravity;
			drip.Position += drip.Velocity;

			int vPos = drip.Position.y;
			auto pointColl = GetCollision(drip.Position.x, drip.Position.y, drip.Position.z, drip.RoomNumber);

			drip.RoomNumber = pointColl.RoomNumber;

			// Drip has hit wall; deactivate.
			if (pointColl.Position.Floor == NO_HEIGHT)
			{
				drip.IsActive = false;
				// TODO: Spawn stains on walls and objects.
			}
			// Drip has hit floor; spawn stain.
			else if ((pointColl.Position.Floor - vPos) <= 0)
			{
				drip.IsActive = false;

				auto pos = Vector3(drip.Position.x, pointColl.Position.Floor - BLOOD_STAIN_HEIGHT_OFFSET, drip.Position.z);
				auto normal = Geometry::GetFloorNormal(pointColl.FloorTilt);

				SpawnBloodStain(pos, drip.RoomNumber, normal, drip.Scale, drip.Velocity.Length());
			}
			// Drip has hit ceiling; spawn stain.
			else if ((pointColl.Position.Ceiling - vPos) >= 0)
			{
				drip.IsActive = false;

				auto pos = Vector3(drip.Position.x, pointColl.Position.Ceiling + BLOOD_STAIN_HEIGHT_OFFSET, drip.Position.z);
				auto normal = Geometry::GetCeilingNormal(pointColl.CeilingTilt);

				SpawnBloodStain(pos, drip.RoomNumber, normal, drip.Scale, drip.Velocity.Length());
			}

			// Spawn bloor mist cloud in water.
		}
	}

	void UpdateBloodStains()
	{
		if (BloodStains.empty())
			return;

		for (auto& stain : BloodStains)
		{
			// Update delay time.
			if (stain.DelayTime != 0.0f)
			{
				stain.DelayTime -= 1.0f;
				if (stain.DelayTime < 0.0f)
					stain.DelayTime = 0.0f;

				continue;
			}

			stain.Life -= 1.0f;

			// Despawn stain.
			if (stain.Life <= 0.0f)
			{
				BloodStains.pop_back();
				continue;
			}

			// Update scale.
			if (stain.ScaleRate != 0.0f)
			{
				if (stain.Scale >= stain.ScaleMax)
				{
					stain.ScaleRate *= 0.1f;
					if (abs(stain.Scale) <= FLT_EPSILON)
						stain.ScaleRate = 0.0f;
				}

				stain.Scale += stain.ScaleRate;
			}

			// Update vertex points.
			stain.VertexPoints = GetBloodStainVertexPoints(stain.Normal, stain.Position, stain.Orientation2D, stain.Scale);

			// Update opacity.
			if (stain.Life <= stain.LifeStartFading)
				stain.Opacity = Lerp(stain.OpacityStart, 0.0f, 1.0f - (stain.Life / BLOOD_STAIN_LIFE_START_FADING));

			// Update color.
			stain.Color = Vector4::Lerp(stain.ColorStart, stain.ColorEnd, 1.0f - (stain.Life / BLOOD_STAIN_LIFE_MAX));
			stain.Color.w = stain.Opacity;

		}
	}

	void DrawIdioticPlaceholders()
	{
		int numActiveDrips = 0;
		for (auto& drip : BloodDrips)
		{
			if (!drip.IsActive)
				continue;

			numActiveDrips++;
			g_Renderer.AddDebugSphere(drip.Position, drip.Scale, drip.Color, RENDERER_DEBUG_PAGE::NO_PAGE);
		}
		
		for (auto& stain : BloodStains)
		{
			//auto target = stain.Position + (stain.Normal * 128);
			//g_Renderer.AddLine3D(stain.Position, target, Vector4::One);

			/*constexpr auto numCircles = 5;
			for (int i = 1; i < numCircles; i++)
				g_Renderer.AddDebugCircle(stain.Position, ((stain.Scale / 2) / numCircles) * i, stain.Color, RENDERER_DEBUG_PAGE::NO_PAGE);*/
		}

		g_Renderer.PrintDebugMessage("Num blood drips: %d ", numActiveDrips);
		g_Renderer.PrintDebugMessage("Num blood stains: %d ", BloodStains.size());
	}
}
