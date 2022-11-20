#include "framework.h"
#include "Game/effects/Blood.h"

#include "Game/collision/collide_room.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"
#include "Specific/setup.h"

#include "lara.h"

#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"

using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Effects::Blood
{
	constexpr auto BLOOD_DRIP_LIFE_MAX			 = 5.0f * FPS;
	constexpr auto BLOOD_STAIN_LIFE_MAX			 = (5.0f * 60.0f)* FPS;
	constexpr auto BLOOD_STAIN_LIFE_START_FADING = 20.0f * FPS;

	constexpr auto BLOOD_DRIP_GRAVITY_MIN	  = 5.0f;
	constexpr auto BLOOD_DRIP_GRAVITY_MAX	  = 15.0f;
	constexpr auto BLOOD_DRIP_SPRAY_SEMIANGLE = 60.0f;

	constexpr auto BLOOD_STAIN_OPACITY_MAX		  = 0.8f;
	constexpr auto BLOOD_STAIN_POOLING_SCALE_RATE = 0.4f;
	constexpr auto BLOOD_STAIN_POOLING_TIME_DELAY = 5.0f;
	constexpr auto BLOOD_STAIN_HEIGHT_OFFSET	  = 4;
	constexpr auto BLOOD_STAIN_NUM_SPRITES		  = 11; // TODO: Hardcoding.

	constexpr auto BLOOD_COLOR_RED	 = Vector4(0.8f, 0.0f, 0.0f, 1.0f);
	constexpr auto BLOOD_COLOR_BROWN = Vector4(0.4f, 0.2f, 0.0f, 1.0f);

	std::array<BloodDrip, BLOOD_DRIP_NUM_MAX> BloodDrips  = {};
	std::deque<BloodStain>					  BloodStains = {};

	BloodDrip& GetFreeBloodDrip()
	{
		for (auto& drip : BloodDrips)
		{
			if (!drip.IsActive)
				return drip;
		}

		return BloodDrips[0];
	}

	std::array<Vector3, 4> GetBloodStainVertexPoints(const Vector3& pos, short orient2D, const Vector3& normal, float scale)
	{
		constexpr auto point0 = Vector3(SQRT_2, 0.0f, SQRT_2);
		constexpr auto point1 = Vector3(-SQRT_2, 0.0f, SQRT_2);
		constexpr auto point2 = Vector3(-SQRT_2, 0.0f, -SQRT_2);
		constexpr auto point3 = Vector3(SQRT_2, 0.0f, -SQRT_2);

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
			slopeAngle * sinDeltaAngle) +
			EulerAngles(0, orient2D, 0);
		auto rotMatrix = orient.ToRotationMatrix();

		return std::array<Vector3, 4>
		{
			pos + Vector3::Transform(point0 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(point1 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(point2 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(point3 * (scale / 2), rotMatrix)
		};
	}

	bool TestBloodStainFloor(const BloodStain& stain)
	{
		static constexpr auto heightRange = CLICK(1.0f / 2);

		// Get point collision at every vertex point.
		auto pointColl0 = GetCollision(stain.VertexPoints[0].x, stain.Position.y - CLICK(1), stain.VertexPoints[0].z, stain.RoomNumber);
		auto pointColl1 = GetCollision(stain.VertexPoints[1].x, stain.Position.y - CLICK(1), stain.VertexPoints[1].z, stain.RoomNumber);
		auto pointColl2 = GetCollision(stain.VertexPoints[2].x, stain.Position.y - CLICK(1), stain.VertexPoints[2].z, stain.RoomNumber);
		auto pointColl3 = GetCollision(stain.VertexPoints[3].x, stain.Position.y - CLICK(1), stain.VertexPoints[3].z, stain.RoomNumber);

		// Stop scaling blood stain if floor heights at vertex points aren't within relative range.
		if ((abs(pointColl0.Position.Floor - pointColl1.Position.Floor) > heightRange) ||
			(abs(pointColl1.Position.Floor - pointColl2.Position.Floor) > heightRange) ||
			(abs(pointColl2.Position.Floor - pointColl3.Position.Floor) > heightRange) ||
			(abs(pointColl3.Position.Floor - pointColl0.Position.Floor) > heightRange))
		{
			return false;
		}

		return true;
	}

	void SpawnBloodMist(const Vector3& pos, int roomNumber, const Vector3& direction, unsigned int count)
	{
		TriggerBlood(pos.x, pos.y, pos.z, direction.y, count);
	}

	void SpawnBloodMistCloud(const Vector3& pos, int roomNumber, const Vector3& direction, float velocity, unsigned int count)
	{
		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		auto box = BoundingOrientedBox(pos, Vector3::One * BLOCK(1.0f / 16), Quaternion(direction, 1.0f));

		for (int i = 0; i < count; i++)
		{
			auto randPos = Random::GenerateVector3InBox(box);
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

		drip = {};
		drip.IsActive = true;
		drip.SpriteIndex = Objects[ID_BLOOD_STAIN_SPRITES].meshIndex + Random::GenerateInt(0, BLOOD_STAIN_NUM_SPRITES);
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
		SpawnBloodMistCloud(pos, roomNumber, baseVelocity + direction, Random::GenerateFloat(10.0f, 50.0f), maxCount * 3);

		unsigned int numDrips = Random::GenerateInt(0, maxCount);
		for (int i = 0; i < numDrips; i++)
		{
			float length = Random::GenerateFloat(20.0f, 40.0f);
			auto velocity = baseVelocity + Random::GenerateVector3InCone(direction, BLOOD_DRIP_SPRAY_SEMIANGLE, length);
			float scale = length / 2;

			SpawnBloodDrip(pos, roomNumber, velocity, scale);
		}
	}

	void SpawnBloodStain(const Vector3& pos, int roomNumber, const Vector3& normal, float scaleMax, float scaleRate, float delayTimeInSec)
	{
		auto stain = BloodStain();

		stain.SpriteIndex = Objects[ID_BLOOD_STAIN_SPRITES].meshIndex + Random::GenerateInt(0, BLOOD_STAIN_NUM_SPRITES);
		stain.Position = pos;
		stain.RoomNumber = roomNumber;
		stain.Orientation2D = Random::GenerateAngle();
		stain.Normal = normal;
		stain.Color = BLOOD_COLOR_RED;
		stain.ColorStart = stain.Color;
		stain.ColorEnd = BLOOD_COLOR_BROWN;
		stain.VertexPoints = GetBloodStainVertexPoints(stain.Position, stain.Orientation2D, stain.Normal, 0.0f);
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

	void SpawnBloodStainFromDrip(const BloodDrip& drip, const CollisionResult& pointColl)
	{
		auto pos = Vector3(drip.Position.x, pointColl.Position.Floor - BLOOD_STAIN_HEIGHT_OFFSET, drip.Position.z);
		auto normal = Geometry::GetFloorNormal(pointColl.FloorTilt);
		float scale = drip.Scale * 5;
		float scaleRate = std::min(drip.Velocity.Length() / 2, scale / 2);

		SpawnBloodStain(pos, drip.RoomNumber, normal, scale, scaleRate);
	}

	void SpawnBloodStainPool(ItemInfo& item)
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

			// Despawn drip.
			drip.Life -= 1.0f;
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

			// Drip hit wall or ceiling; deactivate.
			if (pointColl.Position.Floor == NO_HEIGHT || drip.Position.y <= pointColl.Position.Ceiling)
			{
				drip.IsActive = false;
			}
			// Drip hit floor; spawn stain.
			else if (drip.Position.y >= pointColl.Position.Floor)
			{
				drip.IsActive = false;
				SpawnBloodStainFromDrip(drip, pointColl);
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
			if (stain.DelayTime > 0.0f)
			{
				stain.DelayTime -= 1.0f;
				if (stain.DelayTime < 0.0f)
					stain.DelayTime = 0.0f;

				continue;
			}

			// Despawn.
			stain.Life -= 1.0f;
			if (stain.Life <= 0.0f)
			{
				BloodStains.pop_back();
				continue;
			}

			// Update scale.
			if (stain.ScaleRate > 0.0f)
			{
				if (!TestBloodStainFloor(stain))
					stain.ScaleRate = 0.0f;

				stain.Scale += stain.ScaleRate;
				if (stain.Scale >= (stain.ScaleMax * 0.8f))
				{
					stain.ScaleRate *= 0.2f;
					if (abs(stain.Scale) <= FLT_EPSILON)
						stain.ScaleRate = 0.0f;
				}
			}

			// Update vertex points.
			stain.VertexPoints = GetBloodStainVertexPoints(stain.Position, stain.Orientation2D, stain.Normal, stain.Scale);

			// Update opacity.
			if (stain.Life <= stain.LifeStartFading)
				stain.Opacity = Lerp(stain.OpacityStart, 0.0f, 1.0f - (stain.Life / BLOOD_STAIN_LIFE_START_FADING));

			// Update color.
			stain.Color = Vector4::Lerp(stain.ColorStart, stain.ColorEnd, 1.0f - (stain.Life / BLOOD_STAIN_LIFE_MAX));
			stain.Color.w = stain.Opacity;
		}
	}

	void ClearBloodMists()
	{

	}

	void ClearBloodDrips()
	{
		for (auto& drip : BloodDrips)
			drip = {};
	}

	void ClearBloodStains()
	{
		BloodStains.clear();
	}

	void DrawIdioticPlaceholders()
	{
		int numActiveDrips = 0;
		for (auto& drip : BloodDrips)
		{
			if (!drip.IsActive)
				continue;

			numActiveDrips++;
			//g_Renderer.AddDebugSphere(drip.Position, drip.Scale, drip.Color, RENDERER_DEBUG_PAGE::NO_PAGE);
		}
		
		for (auto& stain : BloodStains)
		{
			// Normal.
			//auto target = stain.Position + (stain.Normal * 128);
			//g_Renderer.AddLine3D(stain.Position, target, Vector4::One);

			// Heat stove.
			/*constexpr auto numCircles = 5;
			for (int i = 1; i < numCircles; i++)
				g_Renderer.AddDebugCircle(stain.Position, ((stain.Scale / 2) / numCircles) * i, stain.Color, RENDERER_DEBUG_PAGE::NO_PAGE);*/
		}

		g_Renderer.PrintDebugMessage("Num blood drips: %d ", numActiveDrips);
		g_Renderer.PrintDebugMessage("Num blood stains: %d ", BloodStains.size());
	}
}
