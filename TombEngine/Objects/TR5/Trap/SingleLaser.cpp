#include "framework.h"
#include "Objects/TR5/Trap/SingleLaser.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/spark.h"
#include "Game/items.h"
#include "Game/control/los.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Renderer/Renderer.h"
#include "Math/Objects/EulerAngles.h"

using namespace TEN::Renderer;
using namespace TEN::Effects::Items;
using namespace TEN::Effects::Spark;
using namespace TEN::Math;

namespace TEN::Traps::TR5
{
	constexpr auto MAX_WIDTH = BLOCK(7);
	constexpr auto LIGHT_INTENSITY = 50.0f;
	constexpr auto LIGHT_AMPLITUDE = 31.0f;
	constexpr auto BEAM_HEIGHT = CLICK(0.05f);

	extern std::unordered_map<int, SingleLaser> LaserBeams = {};

	void SingleLaser::Initialize(const ItemInfo& item)
	{
		float barrierHeight = item.ItemFlags[0];
		int beamCount = 1;

		Color = item.Model.Color;
		Color.w = 0.0f;
		Beams.resize(1);
		IsLethal = (item.TriggerFlags > 0);
		IsHeavyActivator = (item.TriggerFlags <= 0);
		Update(item);
	}

	void TriggerLaserSpark(const GameVector& pos, short angle, int count, const Vector4& colorStart)
	{
		for (int i = 0; i < count; i++)
		{
			auto& s = GetFreeSparkParticle();
			s = {};
			s.age = 0;
			s.life = Random::GenerateFloat(10, 20);
			s.friction = 0.98f;
			s.gravity = 1.2f;
			s.width = 7.0f;
			s.height = 34.0f;
			s.room = pos.RoomNumber;
			s.pos = pos.ToVector3();
			float ang = TO_RAD(angle);
			Vector3 v = Vector3(sin(ang + Random::GenerateFloat(-PI_DIV_2, PI_DIV_2)), Random::GenerateFloat(-1, 1), cos(ang + Random::GenerateFloat(-PI_DIV_2, PI_DIV_2)));
			v += Vector3(Random::GenerateFloat(-64, 64), Random::GenerateFloat(-64, 64), Random::GenerateFloat(-64, 64));
			v.Normalize(v);
			s.velocity = v * Random::GenerateFloat(17, 24);
			s.sourceColor = colorStart;
			s.destinationColor = Vector4::Zero;
			s.active = true;
		}

	}

	void SingleLaser::Update(const ItemInfo& item)
	{
		// Fixed size for the laser beam
		
		
		float beamHeight = item.TriggerFlags < 0 ? BEAM_HEIGHT * abs(item.TriggerFlags) : BEAM_HEIGHT * item.TriggerFlags;
		const short BUFFER = -CLICK(0.5);
		GameVector origin;

		origin.x = item.Pose.Position.x;
		origin.y = item.Pose.Position.y;// -CLICK(0.6f);
		origin.z = item.Pose.Position.z;
		origin.RoomNumber = item.RoomNumber;

		float laserWidth = MAX_WIDTH;
		auto basePos = origin.ToVector3();

		auto rotMatrix = EulerAngles(item.Pose.Orientation.x + ANGLE(180.0f), item.Pose.Orientation.y , item.Pose.Orientation.z);
		GameVector target = Geometry::TranslatePoint(origin.ToVector3(), rotMatrix, MAX_WIDTH);

		auto color = Vector4(item.Model.Color.x, item.Model.Color.y, item.Model.Color.z, 1.0f);

		target.x = 3 * target.x - 2 * origin.x;
		target.y = 3 * target.y - 2 * origin.y;
		target.z = 3 * target.z - 2 * origin.z;

		bool los2 = LOS(&origin, &target);
		
		if (!los2)
		{
			if (item.TriggerFlags > 0)
			{
				TriggerLaserSpark(target, 2 * GetRandomControl(), 3, color);
				TriggerLaserSpark(target, 2 * GetRandomControl(), 3, color);
			}

			SpawnLaserBarrierLight(item, LIGHT_INTENSITY, LIGHT_AMPLITUDE, target);			
		}

		// Determine beam vertex base.
		auto baseVertices = std::array<Vector3, SingleLaserBeam::VERTEX_COUNT>
		{
			basePos + Vector3(0,  -beamHeight / 2, 0.0f),
			target.ToVector3() + Vector3(0,  -beamHeight / 2, 0.0f),
			target.ToVector3() + Vector3(0,  +beamHeight / 2, 0.0f),
			basePos + Vector3(0,  +beamHeight / 2, 0.0f),
		};

		// Set vertex positions.
		auto beamOffset = Vector3(0.0f, 0, 0.0f);
		for (auto& beam : Beams)
		{
			assertion(beam.VertexPoints.size() == baseVertices.size(), "Laser barrier beam vertex count out of sync.");

			for (int i = 0; i < beam.VertexPoints.size(); i++)
			{
				beam.VertexPoints[i] = baseVertices[i] + beamOffset;

				auto pointColl0 = GetCollision(beam.VertexPoints[1], item.RoomNumber);
				auto pointColl1 = GetCollision(beam.VertexPoints[2], item.RoomNumber);

				if (pointColl0.Block->IsWall(beam.VertexPoints[1].x, beam.VertexPoints[1].z) ||
					pointColl0.Block->IsWall(beam.VertexPoints[1].x, beam.VertexPoints[1].z))
				{
					beam.VertexPoints[1] = pointColl0.Coordinates.ToVector3();
				}
				
				if (pointColl1.Block->IsWall(beam.VertexPoints[2].x, beam.VertexPoints[2].z) ||
					pointColl1.Block->IsWall(beam.VertexPoints[2].x, beam.VertexPoints[2].z))
				{
					beam.VertexPoints[2] = pointColl1.Coordinates.ToVector3();
				}

				/*g_Renderer.AddDebugSphere(beam.VertexPoints[0], 23, Vector4(1, 1, 1, 1), RendererDebugPage::None, true);
				g_Renderer.AddDebugSphere(beam.VertexPoints[1], 23, Vector4(0.5f, 0.5f, 0.5f, 1), RendererDebugPage::None, true);
				g_Renderer.AddDebugSphere(beam.VertexPoints[2], 23, Vector4(0, 0, 1, 1), RendererDebugPage::None, true);
				g_Renderer.AddDebugSphere(beam.VertexPoints[3], 23, Vector4(1, 1, 0, 1), RendererDebugPage::None, true);*/

				//g_Renderer.AddDebugSphere(beam.VertexPoints[0], 23, Vector4(1, 1, 1, 1), RendererDebugPage::None, true);
			}

			//beamOffset.y -= beamHeight;
		}

		// Determine bounding box reference points.
		auto point0 = Beams.back().VertexPoints[0];
		auto point1 = Beams.back().VertexPoints[1];
		auto point2 = Beams.front().VertexPoints[2];
		auto point3 = Beams.front().VertexPoints[3];

		// Update bounding box.
		BoundingBox.Center = (point0 + point1 + point2 + point3) / 4;
		BoundingBox.Extents = Vector3(
			std::abs(point0.x - point1.x) / 2,
			std::abs(point0.y - point2.y) / 2,
			std::abs(point0.z - point2.z) / 2);
	}

	void InitializeSingleLaser(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Initialize barrier height.
		auto pointColl = GetCollision(&item);
		float barrierHeight = item.Pose.Position.y - pointColl.Position.Ceiling;
		item.ItemFlags[0] = barrierHeight;

		// Initialize barrier effect.
		auto barrier = SingleLaser{};
		barrier.Initialize(item);
		LaserBeams.insert({ itemNumber, barrier });
	}

	void SpawnLaserBarrierLight(const ItemInfo& item, float intensity, float amplitude, const GameVector& pos)
	{
		if (pos == GameVector::Zero)
		{
			float intensityNorm = intensity - Random::GenerateFloat(0.0f, amplitude);
			TriggerDynamicLight(
				item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z,
				8,
				intensityNorm * (item.Model.Color.x / 2),
				intensityNorm * (item.Model.Color.y / 2),
				intensityNorm * (item.Model.Color.z / 2));
		}
		else
		{
			float intensityNorm = intensity - Random::GenerateFloat(0.0f, amplitude);
			TriggerDynamicLight(
				pos.x, pos.y, pos.z,
				8,
				intensityNorm * (item.Model.Color.x / 2),
				intensityNorm * (item.Model.Color.y / 2),
				intensityNorm * (item.Model.Color.z / 2));
		}
	}

	void ControlSingleLaser(short itemNumber)
	{
		constexpr auto LIGHT_INTENSITY = 150.0f;
		constexpr auto LIGHT_AMPLITUDE = 31.0f;

		if (!LaserBeams.count(itemNumber))
			return;

		

		auto& item = g_Level.Items[itemNumber];
		auto& barrier = LaserBeams.at(itemNumber);

		if (!TriggerActive(&item))
		{
			barrier.IsActive = false;
			barrier.Color.w = 0.0f;
			item.Model.Color.w = 0.0f;
			return;
		}


		//item.Pose.Orientation.x += 359;

		// Brightness fade-in and distortion.
		if (item.Model.Color.w < 1.0f)
			item.Model.Color.w += 0.02f;

		if (barrier.Color.w < 1.0f)
			barrier.Color.w += 0.02f;

		// TODO: Weird.
		if (item.Model.Color.w > 8.0f)
		{
			barrier.Color.w = 0.8f;
			item.Model.Color.w = 0.8f;
		}
			
		barrier.IsActive = true;
		barrier.Update(item);

		if (item.Model.Color.w >= 0.8f)
			//SpawnLaserBarrierLight(item, LIGHT_INTENSITY, LIGHT_AMPLITUDE, GameVector::Zero);

		SoundEffect(SFX_TR5_DOOR_BEAM, &item.Pose);
	}

	void CollideSingleLaser(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		constexpr auto LIGHT_INTENSITY = 255.0f;
		constexpr auto LIGHT_AMPLITUDE = 100.0f;

		if (!LaserBeams.count(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& barrier = LaserBeams.at(itemNumber);

		if (!barrier.IsActive)
			return;

		auto playerBox = GameBoundingBox(playerItem).ToBoundingOrientedBox(playerItem->Pose);
		if (barrier.BoundingBox.Intersects(playerBox))
		{
			if (barrier.IsLethal &&
				playerItem->HitPoints > 0 && playerItem->Effect.Type != EffectType::Smoke)
			{
				ItemRedLaserBurn(playerItem, 2.0f * FPS);
				DoDamage(playerItem, MAXINT);
			}
			else if (barrier.IsHeavyActivator)
			{
				TestTriggers(&item, true, item.Flags & IFLAG_ACTIVATION_MASK);
			}

			barrier.Color.w = Random::GenerateFloat(0.6f, 1.0f);
			SpawnLaserBarrierLight(item, LIGHT_INTENSITY, LIGHT_AMPLITUDE, GameVector::Zero);
		}		
	}

	void ClearSingleLaserEffects()
	{
		LaserBeams.clear();
	}
}
