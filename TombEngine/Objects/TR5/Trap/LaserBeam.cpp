#include "framework.h"
#include "Objects/TR5/Trap/LaserBeam.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/spark.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Specific/level.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Spark;
using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Traps::TR5
{
	constexpr auto LASER_BEAM_LIGHT_INTENSITY = 50.0f;
	constexpr auto LASER_BEAM_LIGHT_AMPLITUDE = 31.0f;
	constexpr auto LASER_BEAM_RADIUS		  = CLICK(0.05f);

	extern std::unordered_map<int, LaserBeamEffect> LaserBeams = {};

	void LaserBeamEffect::Initialize(const ItemInfo& item)
	{
		Color = item.Model.Color;
		Color.w = 0.0f;

		IsLethal = (item.TriggerFlags > 0);
		IsHeavyActivator = (item.TriggerFlags <= 0);

		Update(item);
	}

	static void SpawnLaserSpark(const GameVector& pos, short angle, int count, const Vector4& colorStart)
	{
		for (int i = 0; i < count; i++)
		{
			float ang = TO_RAD(angle);
			auto vel = Vector3(sin(ang + Random::GenerateFloat(-PI_DIV_2, PI_DIV_2)), Random::GenerateFloat(-1, 1), cos(ang + Random::GenerateFloat(-PI_DIV_2, PI_DIV_2)));
			vel += Vector3(Random::GenerateFloat(-64, 64), Random::GenerateFloat(-64, 64), Random::GenerateFloat(-64, 64));
			vel.Normalize(vel);

			auto& spark = GetFreeSparkParticle();

			spark = {};
			spark.age = 0;
			spark.life = Random::GenerateFloat(10, 20);
			spark.friction = 0.98f;
			spark.gravity = 1.2f;
			spark.width = 7.0f;
			spark.height = 34.0f;
			spark.room = pos.RoomNumber;
			spark.pos = pos.ToVector3();
			spark.velocity = vel * Random::GenerateFloat(17, 24);
			spark.sourceColor = colorStart;
			spark.destinationColor = Vector4::Zero;
			spark.active = true;
		}
	}

	static void SpawnLaserBeamLight(const Vector3& pos, int roomNumber, const Color& color, float intensity, float amplitude)
	{
		constexpr auto FALLOFF = 8;

		float intensityNorm = intensity - Random::GenerateFloat(0.0f, amplitude);
		TriggerDynamicLight(
			pos.x, pos.y, pos.z,
			FALLOFF,
			intensityNorm * (color.x / 2),
			intensityNorm * (color.y / 2),
			intensityNorm * (color.z / 2));
	}

	void LaserBeamEffect::Update(const ItemInfo& item)
	{
		float beamHeight = item.TriggerFlags < 0 ? LASER_BEAM_RADIUS * abs(item.TriggerFlags) : LASER_BEAM_RADIUS * item.TriggerFlags;
		GameVector origin;

		origin.x = item.Pose.Position.x;
		origin.y = item.Pose.Position.y;
		origin.z = item.Pose.Position.z;
		origin.RoomNumber = item.RoomNumber;

		auto basePos = origin.ToVector3();

		auto rotMatrix = EulerAngles(item.Pose.Orientation.x + ANGLE(180.0f), item.Pose.Orientation.y , item.Pose.Orientation.z);
		GameVector target = Geometry::TranslatePoint(origin.ToVector3(), rotMatrix, MAX_VISIBILITY_DISTANCE);

		auto color = Vector4(item.Model.Color.x, item.Model.Color.y, item.Model.Color.z, 1.0f);

		target.x = 3 * target.x - 2 * origin.x;
		target.y = 3 * target.y - 2 * origin.y;
		target.z = 3 * target.z - 2 * origin.z;

		bool los2 = LOS(&origin, &target);

		auto pointColl = GetCollision(target.ToVector3i(), item.RoomNumber);
		if (pointColl.RoomNumber != target.RoomNumber)
			target.RoomNumber = pointColl.RoomNumber;

		if (!los2)
		{
			if (item.TriggerFlags > 0)
			{
				SpawnLaserSpark(target, 2 * GetRandomControl(), 3, color);
				SpawnLaserSpark(target, 2 * GetRandomControl(), 3, color);
			}

			SpawnLaserBeamLight(target.ToVector3(), target.RoomNumber, item.Model.Color, LASER_BEAM_LIGHT_INTENSITY, LASER_BEAM_LIGHT_AMPLITUDE);
		}

		// Determine beam vertex base.
		auto baseVertices = std::array<Vector3, LaserBeamEffect::VERTEX_COUNT>
		{
			basePos + Vector3(0.0f, -beamHeight / 2, 0.0f),
			target.ToVector3() + Vector3(0.0f, -beamHeight / 2, 0.0f),
			target.ToVector3() + Vector3(0.0f, beamHeight / 2, 0.0f),
			basePos + Vector3(0.0f, beamHeight / 2, 0.0f),
		};

		// Set vertex positions.
		auto beamOffset = Vector3::Zero;
		assertion(VertexPoints.size() == baseVertices.size(), "Laser beam vertex count out of sync.");

		for (int i = 0; i < VertexPoints.size(); i++)
		{
			VertexPoints[i] = baseVertices[i] + beamOffset;

			auto pointColl0 = GetCollision(VertexPoints[1], item.RoomNumber);
			auto pointColl1 = GetCollision(VertexPoints[2], item.RoomNumber);

			if (pointColl0.Block->IsWall(VertexPoints[1].x, VertexPoints[1].z) ||
				pointColl0.Block->IsWall(VertexPoints[1].x, VertexPoints[1].z))
			{
				VertexPoints[1] = pointColl0.Coordinates.ToVector3();
			}
				
			if (pointColl1.Block->IsWall(VertexPoints[2].x, VertexPoints[2].z) ||
				pointColl1.Block->IsWall(VertexPoints[2].x, VertexPoints[2].z))
			{
				VertexPoints[2] = pointColl1.Coordinates.ToVector3();
			}
		}

		// TODO: Bounding box.

		// Determine bounding box reference points.
		/*auto point0 = Beams.back().VertexPoints[0];
		auto point1 = Beams.back().VertexPoints[1];
		auto point2 = Beams.front().VertexPoints[2];
		auto point3 = Beams.front().VertexPoints[3];

		// Update bounding box.
		BoundingBox.Center = (point0 + point1 + point2 + point3) / 4;
		BoundingBox.Extents = Vector3(
			std::abs(point0.x - point1.x) / 2,
			std::abs(point0.y - point2.y) / 2,
			std::abs(point0.z - point2.z) / 2);*/
	}

	void InitializeLaserBeam(short itemNumber)
	{
		const auto& item = g_Level.Items[itemNumber];

		auto laser = LaserBeamEffect{};
		laser.Initialize(item);

		LaserBeams.insert({ itemNumber, laser });
	}

	void ControlLaserBeam(short itemNumber)
	{
		if (!LaserBeams.count(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& laser = LaserBeams.at(itemNumber);

		if (!TriggerActive(&item))
		{
			laser.IsActive = false;
			laser.Color.w = 0.0f;
			item.Model.Color.w = 0.0f;
			return;
		}

		// Brightness fade-in and distortion.
		if (item.Model.Color.w < 1.0f)
			item.Model.Color.w += 0.02f;

		if (laser.Color.w < 1.0f)
			laser.Color.w += 0.02f;

		// TODO: Weird.
		if (item.Model.Color.w > 8.0f)
		{
			laser.Color.w = 0.8f;
			item.Model.Color.w = 0.8f;
		}
			
		laser.IsActive = true;
		laser.Update(item);

		if (item.Model.Color.w >= 0.8f)
			SpawnLaserBeamLight(item.Pose.Position.ToVector3(), item.RoomNumber, item.Model.Color, LASER_BEAM_LIGHT_INTENSITY, LASER_BEAM_LIGHT_AMPLITUDE);

		SoundEffect(SFX_TR5_DOOR_BEAM, &item.Pose);
	}

	void CollideLaserBeam(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		constexpr auto LASER_BEAM_LIGHT_INTENSITY_MODIFY = 255.0f;
		constexpr auto LASER_BEAM_LIGHT_AMPLITUDE_MODIFY = 100.0f;

		if (!LaserBeams.count(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& laser = LaserBeams.at(itemNumber);

		if (!laser.IsActive)
			return;

		auto origin = GameVector(item.Pose.Position, item.RoomNumber);
		auto basePos = origin.ToVector3();

		auto rotMatrix = EulerAngles(item.Pose.Orientation.x + ANGLE(180.0f), item.Pose.Orientation.y, item.Pose.Orientation.z);
		auto target = GameVector(Geometry::TranslatePoint(origin.ToVector3(), rotMatrix, MAX_VISIBILITY_DISTANCE), 0);

		auto pointColl = GetCollision(target.ToVector3i(),item.RoomNumber);
		if (pointColl.RoomNumber != target.RoomNumber)
			target.RoomNumber = pointColl.RoomNumber;

		bool los2 = LOS(&origin, &target);

		auto hitPos = Vector3i::Zero;
		if (ObjectOnLOS2(&origin, &target, &hitPos, nullptr, ID_LARA) == LaraItem->Index && !los2)
		{
			if (laser.IsLethal &&
				playerItem->HitPoints > 0 && playerItem->Effect.Type != EffectType::Smoke)
			{
				ItemRedLaserBurn(playerItem, 2.0f * FPS);
				DoDamage(playerItem, MAXINT);
			}
			else if (laser.IsHeavyActivator)
			{
				TestTriggers(&item, true, item.Flags & IFLAG_ACTIVATION_MASK);
			}

			laser.Color.w = Random::GenerateFloat(0.6f, 1.0f);
			SpawnLaserBeamLight(item.Pose.Position.ToVector3(), item.RoomNumber, item.Model.Color, LASER_BEAM_LIGHT_INTENSITY_MODIFY, LASER_BEAM_LIGHT_AMPLITUDE_MODIFY);
		}		
	}

	void ClearLaserBeamEffects()
	{
		LaserBeams.clear();
	}
}
