#include "framework.h"
#include "Game/effects/Blood.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Effects::Blood
{
	constexpr auto BLOOD_DRIP_COUNT_MAX  = 256;
	constexpr auto BLOOD_STAIN_COUNT_MAX = 192;
	constexpr auto BLOOD_MIST_COUNT_MAX  = 256;
	constexpr auto UW_BLOOD_COUNT_MAX	 = 256;

	constexpr auto BLOOD_DRIP_LIFE_START_FADING = 0.5f;
	
	constexpr auto BLOOD_STAIN_LIFE_MAX			 = 5.0f * 60.0f;
	constexpr auto BLOOD_STAIN_LIFE_START_FADING = 30.0f;
	constexpr auto BLOOD_STAIN_SURFACE_OFFSET	 = 4;

	constexpr auto BLOOD_COLOR_RED	 = Vector4(0.8f, 0.0f, 0.0f, 1.0f);
	constexpr auto BLOOD_COLOR_BROWN = Vector4(0.3f, 0.1f, 0.0f, 1.0f);

	std::deque<BloodDrip>		BloodDrips				 = {};
	std::deque<BloodStain>		BloodStains				 = {};
	std::deque<BloodMist>		BloodMists				 = {};
	std::deque<UnderwaterBlood> UnderwaterBloodParticles = {};

	static std::array<Vector3, 4> GetBloodStainVertexPoints(const Vector3& pos, short orient2D, const Vector3& normal, float scale)
	{
		constexpr auto POINT_0 = Vector3( SQRT_2, 0.0f,  SQRT_2);
		constexpr auto POINT_1 = Vector3(-SQRT_2, 0.0f,  SQRT_2);
		constexpr auto POINT_2 = Vector3(-SQRT_2, 0.0f, -SQRT_2);
		constexpr auto POINT_3 = Vector3( SQRT_2, 0.0f, -SQRT_2);

		// No scale; return early.
		if (scale == 0.0f)
			return std::array<Vector3, 4>{ pos, pos, pos, pos };

		// Determine rotation matrix.
		auto rotMatrix = Geometry::GetRelOrientToNormal(orient2D, normal).ToRotationMatrix();

		return std::array<Vector3, 4>
		{
			pos + Vector3::Transform(POINT_0 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(POINT_1 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(POINT_2 * (scale / 2), rotMatrix),
			pos + Vector3::Transform(POINT_3 * (scale / 2), rotMatrix)
		};
	}

	static bool TestBloodStainFloor(const Vector3& pos, int roomNumber, const std::array<Vector3, 4>& vertexPoints)
	{
		static constexpr auto heightRange = CLICK(0.5f);

		// Get point collision at every vertex point.
		auto pointColl0 = GetCollision(vertexPoints[0].x, pos.y - CLICK(1), vertexPoints[0].z, roomNumber);
		auto pointColl1 = GetCollision(vertexPoints[1].x, pos.y - CLICK(1), vertexPoints[1].z, roomNumber);
		auto pointColl2 = GetCollision(vertexPoints[2].x, pos.y - CLICK(1), vertexPoints[2].z, roomNumber);
		auto pointColl3 = GetCollision(vertexPoints[3].x, pos.y - CLICK(1), vertexPoints[3].z, roomNumber);

		// Stop scaling blood stain if floor heights at vertex points are outside relative range.
		if ((abs(pointColl0.Position.Floor - pointColl1.Position.Floor) > heightRange) ||
			(abs(pointColl1.Position.Floor - pointColl2.Position.Floor) > heightRange) ||
			(abs(pointColl2.Position.Floor - pointColl3.Position.Floor) > heightRange) ||
			(abs(pointColl3.Position.Floor - pointColl0.Position.Floor) > heightRange))
		{
			return false;
		}

		return true;
	}

	void SpawnBloodDrip(const Vector3& pos, int roomNumber, const Vector3& velocity, float lifeInSec, float scale, bool canSpawnStain)
	{
		constexpr auto GRAVITY_MAX = 15.0f;
		constexpr auto GRAVITY_MIN = 5.0f;

		auto& drip = GetNewEffect(BloodDrips, BLOOD_DRIP_COUNT_MAX);

		drip.SpriteIndex = Objects[ID_DRIP_SPRITE].meshIndex;
		drip.CanSpawnStain = canSpawnStain;
		drip.Position = pos;
		drip.RoomNumber = roomNumber;
		drip.Velocity = velocity;
		drip.Color = BLOOD_COLOR_RED;
		drip.Life = std::round(lifeInSec * FPS);
		drip.LifeStartFading = std::round(BLOOD_DRIP_LIFE_START_FADING * FPS);
		drip.Opacity = 1.0f;
		drip.Scale = scale;
		drip.Gravity = Random::GenerateFloat(GRAVITY_MIN, GRAVITY_MAX);
	}

	void SpawnBloodDripSpray(const Vector3& pos, int roomNumber, const Vector3& direction, const Vector3& baseVelocity, unsigned int count)
	{
		constexpr auto LIFE_MAX			  = 5.0f;
		constexpr auto SPRAY_VELOCITY_MAX = 45.0f;
		constexpr auto SPRAY_VELOCITY_MIN = 15.0f;
		constexpr auto SPRAY_SEMIANGLE	  = 50.0f;

		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		// Spawn mist.
		SpawnBloodMistCloud(pos, roomNumber, direction, count * 4);

		// Spawn decorative drips.
		for (int i = 0; i < count * 12; i++)
		{
			float length = Random::GenerateFloat(SPRAY_VELOCITY_MIN, SPRAY_VELOCITY_MAX);
			auto velocity = Random::GenerateDirectionInCone(-direction, SPRAY_SEMIANGLE) * length;
			float scale = length * 0.25f;

			SpawnBloodDrip(pos, roomNumber, velocity, BLOOD_DRIP_LIFE_START_FADING, scale, false);
		}

		// Spawn special drips capable of creating stains.
		for (int i = 0; i < count; i++)
		{
			float length = Random::GenerateFloat(SPRAY_VELOCITY_MIN, SPRAY_VELOCITY_MAX);
			auto velocity = baseVelocity + Random::GenerateDirectionInCone(direction, SPRAY_SEMIANGLE) * length;
			float scale = length * 0.5f;

			SpawnBloodDrip(pos, roomNumber, velocity, LIFE_MAX, scale, true);
		}
	}

	void SpawnBloodStain(const Vector3& pos, int roomNumber, const Vector3& normal, float scaleMax, float scaleRate, float delayTimeInSec)
	{
		constexpr auto OPACITY_MAX		= 0.8f;
		constexpr auto SPRITE_INDEX_MAX = 7; // TODO: Dehardcode index range.

		auto& stain = GetNewEffect(BloodStains, BLOOD_STAIN_COUNT_MAX);

		stain.SpriteIndex = Objects[ID_BLOOD_STAIN_SPRITES].meshIndex + Random::GenerateInt(0, SPRITE_INDEX_MAX);
		stain.Position = pos;
		stain.RoomNumber = roomNumber;
		stain.Orientation2D = Random::GenerateAngle();
		stain.Normal = normal;
		stain.Color =
		stain.ColorStart = BLOOD_COLOR_RED;
		stain.ColorEnd = BLOOD_COLOR_BROWN;
		stain.VertexPoints = GetBloodStainVertexPoints(stain.Position, stain.Orientation2D, stain.Normal, 0.0f);
		stain.Life = std::round(BLOOD_STAIN_LIFE_MAX * FPS);
		stain.LifeStartFading = std::round(BLOOD_STAIN_LIFE_START_FADING * FPS);
		stain.Scale = 0.0f;
		stain.ScaleMax = scaleMax;
		stain.ScaleRate = scaleRate;
		stain.Opacity =
		stain.OpacityMax = OPACITY_MAX;
		stain.DelayTime = std::round(delayTimeInSec * FPS);
	}

	void SpawnBloodStainFromDrip(const BloodDrip& drip, const CollisionResult& pointColl)
	{
		if (!drip.CanSpawnStain)
			return;

		if (TestEnvironment(ENV_FLAG_WATER, drip.RoomNumber))
			return;

		auto pos = Vector3(drip.Position.x, pointColl.Position.Floor - BLOOD_STAIN_SURFACE_OFFSET, drip.Position.z);
		auto normal = Geometry::GetFloorNormal(pointColl.FloorTilt);
		float scale = drip.Scale * 4;
		float scaleRate = std::min(drip.Velocity.Length() / 2, scale / 2);

		SpawnBloodStain(pos, drip.RoomNumber, normal, scale, scaleRate);
	}

	void SpawnBloodStainPool(ItemInfo& item)
	{
		constexpr auto SCALE_RATE = 0.4f;
		constexpr auto DELAY_TIME = 5.0f;

		auto pointColl = GetCollision(&item);
		auto pos = Vector3(item.Pose.Position.x, pointColl.Position.Floor - BLOOD_STAIN_SURFACE_OFFSET, item.Pose.Position.z);
		auto normal = Geometry::GetFloorNormal(pointColl.FloorTilt);

		auto bounds = GameBoundingBox(&item);
		float scaleMax = (bounds.GetWidth() + bounds.GetDepth()) / 2;

		SpawnBloodStain(pos, item.RoomNumber, normal, scaleMax, SCALE_RATE, DELAY_TIME);
	}
	
	void SpawnBloodMist(const Vector3& pos, int roomNumber, const Vector3& direction)
	{
		constexpr auto LIFE_MAX		 = 0.75f;
		constexpr auto LIFE_MIN		 = 0.25f;
		constexpr auto VELOCITY_MAX	 = 16.0f;
		constexpr auto SCALE_MAX	 = 128.0f;
		constexpr auto SCALE_MIN	 = 64.0f;
		constexpr auto OPACITY_MAX	 = 0.7f;
		constexpr auto GRAVITY_MAX	 = 2.0f;
		constexpr auto GRAVITY_MIN	 = 1.0f;
		constexpr auto FRICTION		 = 4.0f;
		constexpr auto ROTATION_MAX	 = ANGLE(10.0f);
		constexpr auto SPHERE_RADIUS = BLOCK(1 / 8.0f);
		constexpr auto SEMIANGLE	 = 20.0f;

		auto& mist = GetNewEffect(BloodMists, BLOOD_MIST_COUNT_MAX);

		auto sphere = BoundingSphere(pos, SPHERE_RADIUS);

		mist.SpriteIndex = Objects[ID_BLOOD_MIST_SPRITES].meshIndex;
		mist.Position = Random::GeneratePointInSphere(sphere);
		mist.RoomNumber = roomNumber;
		mist.Orientation2D = Random::GenerateAngle();
		mist.Velocity = Random::GenerateDirectionInCone(direction, SEMIANGLE) * Random::GenerateFloat(0.0f, VELOCITY_MAX);
		mist.Color = BLOOD_COLOR_RED;
		mist.Life = std::round(Random::GenerateFloat(LIFE_MIN, LIFE_MAX) * FPS);
		mist.LifeMax = mist.Life;
		mist.Scale = Random::GenerateFloat(SCALE_MIN, SCALE_MAX);
		mist.ScaleMax = mist.Scale * 4;
		mist.ScaleMin = mist.Scale;
		mist.Opacity =
		mist.OpacityMax = OPACITY_MAX;
		mist.Gravity = Random::GenerateFloat(GRAVITY_MIN, GRAVITY_MAX);
		mist.Friction = FRICTION;
		mist.Rotation = Random::GenerateAngle(-ROTATION_MAX, ROTATION_MAX);
	}

	void SpawnBloodMistCloud(const Vector3& pos, int roomNumber, const Vector3& direction, unsigned int count)
	{
		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		for (int i = 0; i < count; i++)
			SpawnBloodMist(pos, roomNumber, direction);
	}
	
	void SpawnUnderwaterBlood(const Vector3& pos, int roomNumber, float scale)
	{
		constexpr auto LIFE_MAX		 = 8.5f;
		constexpr auto LIFE_MIN		 = 8.0f;
		constexpr auto SPHERE_RADIUS = BLOCK(0.25f);

		auto& uwBlood = GetNewEffect(UnderwaterBloodParticles, UW_BLOOD_COUNT_MAX);

		auto sphere = BoundingSphere(pos, SPHERE_RADIUS);

		uwBlood.SpriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		uwBlood.Position = Random::GeneratePointInSphere(sphere);
		uwBlood.Life = std::round(Random::GenerateFloat(LIFE_MIN, LIFE_MAX) * FPS);
		uwBlood.Init = 1.0f;
		uwBlood.Scale = scale;
	}

	void SpawnUnderwaterBloodCloud(const Vector3& pos, int roomNumber, float scaleMax, unsigned int count)
	{
		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		for (int i = 0; i < count; i++)
			SpawnUnderwaterBlood(pos, roomNumber, scaleMax);
	}

	void UpdateBloodDrips()
	{
		for (auto& drip : BloodDrips)
		{
			if (drip.Life <= 0.0f)
				continue;

			// Update opacity.
			if (drip.Life <= drip.LifeStartFading)
				drip.Opacity = Lerp(1.0f, 0.0f, 1.0f - (drip.Life / std::round(BLOOD_DRIP_LIFE_START_FADING * FPS)));

			// Update color.
			drip.Color.w = drip.Opacity;

			// Update velocity.
			drip.Velocity.y += drip.Gravity;
			if (TestEnvironment(ENV_FLAG_WIND, drip.RoomNumber))
				drip.Velocity += Weather.Wind();

			// Update position.
			drip.Position += drip.Velocity;

			auto pointColl = GetCollision(drip.Position.x, drip.Position.y, drip.Position.z, drip.RoomNumber);

			drip.RoomNumber = pointColl.RoomNumber;

			// Hit water; spawn underwater blood.
			if (TestEnvironment(ENV_FLAG_WATER, drip.RoomNumber))
			{
				drip.Life = 0.0f;

				if (drip.CanSpawnStain)
					SpawnUnderwaterBlood(drip.Position, drip.RoomNumber, drip.Scale * 5);
			}
			// Hit wall or ceiling; deactivate.
			if (pointColl.Position.Floor == NO_HEIGHT || drip.Position.y <= pointColl.Position.Ceiling)
			{
				drip.Life = 0.0f;
			}
			// Hit floor; spawn stain.
			else if (drip.Position.y >= pointColl.Position.Floor)
			{
				drip.Life = 0.0f;
				SpawnBloodStainFromDrip(drip, pointColl);
			}

			// Update life.
			drip.Life -= 1.0f;
		}

		ClearInactiveEffects(BloodDrips);
	}

	void UpdateBloodStains()
	{
		if (BloodStains.empty())
			return;

		for (auto& stain : BloodStains)
		{
			if (stain.Life <= 0.0f)
				continue;

			// Update delay time.
			if (stain.DelayTime > 0.0f)
			{
				stain.DelayTime -= 1.0f;
				if (stain.DelayTime < 0.0f)
					stain.DelayTime = 0.0f;

				continue;
			}

			// Update scale.
			if (stain.ScaleRate > 0.0f)
			{
				stain.Scale += stain.ScaleRate;

				// Update scale rate.
				if (stain.Scale >= (stain.ScaleMax * 0.8f))
				{
					stain.ScaleRate *= 0.2f;
					if (abs(stain.ScaleRate) <= FLT_EPSILON)
						stain.ScaleRate = 0.0f;
				}

				// Update vertex points.
				auto vertexPoints = GetBloodStainVertexPoints(stain.Position, stain.Orientation2D, stain.Normal, stain.Scale);
				if (TestBloodStainFloor(stain.Position, stain.RoomNumber, vertexPoints))
					stain.VertexPoints = vertexPoints;
				else
					stain.ScaleRate = 0.0f;
			}

			// Update opacity.
			if (stain.Life <= stain.LifeStartFading)
				stain.Opacity = Lerp(stain.OpacityMax, 0.0f, 1.0f - (stain.Life / std::round(BLOOD_STAIN_LIFE_START_FADING * FPS)));

			// Update color.
			stain.Color = Vector4::Lerp(stain.ColorStart, stain.ColorEnd, 1.0f - (stain.Life / std::round(BLOOD_STAIN_LIFE_MAX * FPS)));
			stain.Color.w = stain.Opacity;

			// Update life.
			stain.Life -= 1.0f;
		}

		ClearInactiveEffects(BloodStains);
	}

	void UpdateBloodMists()
	{
		for (auto& mist : BloodMists)
		{
			if (mist.Life <= 0.0f)
				continue;

			// Update velocity.
			mist.Velocity.y += mist.Gravity;
			mist.Velocity -= mist.Velocity / mist.Friction;

			// Update position.
			mist.Position += mist.Velocity;
			mist.Orientation2D += mist.Rotation;

			// Update scale.
			mist.Scale = Lerp(mist.ScaleMin, mist.ScaleMax, 1.0f - (mist.Life / mist.LifeMax));

			// Update opacity.
			mist.Opacity = Lerp(mist.OpacityMax, 0.0f, 1.0f - (mist.Life / mist.LifeMax));
			mist.Color.w = mist.Opacity;

			// Update life.
			mist.Life -= 1.0f;
		}

		ClearInactiveEffects(BloodMists);
	}

	void UpdateUnderwaterBloodParticles()
	{
		constexpr auto SCALE_MAX = BLOCK(0.25f);

		for (auto& uwBlood : UnderwaterBloodParticles)
		{
			if (uwBlood.Life <= 0.0f)
				continue;

			// Update scale.
			if (uwBlood.Scale < SCALE_MAX)
				uwBlood.Scale += 4.0f;

			// Update life.
			if (uwBlood.Init == 0.0f)
			{
				uwBlood.Life -= 3.0f;
			}
			else if (uwBlood.Init < uwBlood.Life)
			{
				uwBlood.Init += 4.0f;

				if (uwBlood.Init >= uwBlood.Life)
					uwBlood.Init = 0.0f;
			}
		}

		ClearInactiveEffects(UnderwaterBloodParticles);
	}

	void ClearUnderwaterBloodParticles()
	{
		UnderwaterBloodParticles.clear();
	}
	
	void ClearBloodMists()
	{
		BloodMists.clear();
	}

	void ClearBloodDrips()
	{
		BloodDrips.clear();
	}

	void ClearBloodStains()
	{
		BloodStains.clear();
	}

	void DrawBloodDebug()
	{
		g_Renderer.PrintDebugMessage("Blood drip count: %d ", BloodDrips.size());
		g_Renderer.PrintDebugMessage("Blood stain count: %d ", BloodStains.size());
		g_Renderer.PrintDebugMessage("Blood mist count: %d ", BloodMists.size());
		g_Renderer.PrintDebugMessage("UW. blood count: %d ", UnderwaterBloodParticles.size());
	}
}
