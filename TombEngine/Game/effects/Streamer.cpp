#include "framework.h"
#include "Game/effects/Streamer.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Math/Math.h"
#include "Specific/clock.h"

using namespace TEN::Math;

namespace TEN::Effects::Streamer
{
	void Streamer::StreamerSegment::InitializeVertices(const Vector3& pos, float width)
	{
		this->Vertices = { pos, pos };
		this->TransformVertices(0.0f, width / 2);
	}

	void Streamer::StreamerSegment::Update()
	{
		// Update opacity.
		if (Color.w > 0.0f)
			this->Color.w = InterpolateCos(0.0f, OpacityMax, Life / LifeMax);

		// Update orientation.
		this->Orientation.SetAngle(Orientation.GetAngle() + Rotation);
		// TODO: Directional bias like in the older version.
		
		// Update vertices.
		this->TransformVertices(Velocity, ScaleRate);

		// Update life.
		this->Life -= 1.0f;
	}

	void Streamer::StreamerSegment::TransformVertices(float vel, float scaleRate)
	{
		// Apply expansion.
		if (scaleRate != 0.0f)
		{
			auto direction = Orientation.ToDirection();
			this->Vertices[0] = Geometry::TranslatePoint(Vertices[0], -direction, scaleRate);
			this->Vertices[1] = Geometry::TranslatePoint(Vertices[1], direction, scaleRate);
		}

		// Apply directional velocity.
		if (vel != 0.0f)
		{
			auto direction = Orientation.GetAxis();
			this->Vertices[0] = Geometry::TranslatePoint(Vertices[0], direction, vel);
			this->Vertices[1] = Geometry::TranslatePoint(Vertices[1], direction, vel);
		}
	}

	void Streamer::AddSegment(const Vector3& pos, const Vector3& direction, short orient2D, const Vector4& color,
							  float width, float life, float vel, float scaleRate, float rot2D, int flags, unsigned int segmentCount)
	{
		auto& segment = this->GetNewSegment();

		float lifeMax = round(life * FPS);
		float opacityMax = InterpolateCos(0.0f, StreamerSegment::OPACITY_MAX, segmentCount / lifeMax);

		segment.Orientation = AxisAngle(direction, orient2D);
		segment.Color = Vector4(color.x, color.y, color.z, opacityMax);
		segment.Life =
		segment.LifeMax = lifeMax;
		segment.OpacityMax = opacityMax;
		segment.Velocity = vel;
		segment.ScaleRate = scaleRate;
		segment.Rotation = rot2D;
		segment.Flags = flags;
		segment.InitializeVertices(pos, width);
	}

	void Streamer::Update()
	{
		if (Segments.empty())
			return;

		// If streamer was broken, set flag to track it.
		const auto& newestSegment = Segments.back();
		if (newestSegment.Life != newestSegment.LifeMax)
			this->IsBroken = true;

		// Update segments.
		for (auto& segment : this->Segments)
			segment.Update();

		ClearInactiveEffects(this->Segments);
	}

	Streamer::StreamerSegment& Streamer::GetNewSegment()
	{
		assert(Segments.size() <= SEGMENT_COUNT_MAX);

		// Clear oldest segment if vector is full.
		if (Segments.size() == SEGMENT_COUNT_MAX)
			this->Segments.erase(Segments.begin());

		// Add and return new segment.
		return this->Segments.emplace_back();
	}

	void StreamerModule::AddStreamer(int tag, const Vector3& pos, const Vector3& direction, short orient2D, const Vector4& color,
									 float width, float life, float vel, float scaleRate, float rot2D, int flags)
	{
		assert(Pools.size() <= POOL_COUNT_MAX);

		// Return early if pool map is full and element with tag key doesn't already exist.
		if (Pools.size() == POOL_COUNT_MAX && !Pools.count(tag))
			return;

		// Get and extend streamer with new segment.
		auto& streamer = this->GetStreamer(tag);
		streamer.AddSegment(pos, direction, orient2D, color, width, life, vel, scaleRate, rot2D, flags, streamer.Segments.size());
	}

	void StreamerModule::Update()
	{
		if (Pools.empty())
			return;

		for (auto& [tag, pool] : this->Pools)
		{
			for (auto& streamer : pool)
				streamer.Update();

			this->ClearInactiveStreamers(tag);
		}

		this->ClearInactivePools();
	}

	std::vector<Streamer>& StreamerModule::GetPool(int tag)
	{
		// Get pool at tag key.
		this->Pools.insert({ tag, {} });
		auto& pool = this->Pools.at(tag);
		return pool;
	}

	Streamer& StreamerModule::GetStreamer(int tag)
	{
		auto& pool = this->GetPool(tag);

		assert(pool.size() <= STREAMER_COUNT_MAX);

		// Return unbroken streamer at back of vector if it exists.
		if (!pool.empty())
		{
			auto& streamer = pool.back();
			if (!streamer.IsBroken)
				return streamer;
		}

		// Clear oldest streamer if pool is full.
		if (pool.size() == STREAMER_COUNT_MAX)
			pool.erase(pool.begin());

		// Add and return new streamer.
		return pool.emplace_back();
	}

	void StreamerModule::ClearInactivePools()
	{
		for (auto it = Pools.begin(); it != Pools.end();)
		{
			if (it->second.empty())
			{
				it = this->Pools.erase(it);
				continue;
			}
			
			++it;
		}
	}

	void StreamerModule::ClearInactiveStreamers(int tag)
	{
		auto& pool = this->Pools.at(tag);

		pool.erase(
			std::remove_if(
				pool.begin(), pool.end(),
				[](const auto& streamer) { return streamer.Segments.empty(); }),
			pool.end());
	}

	void StreamerEffectController::Spawn(int entityNumber, int tag, const Vector3& pos, const Vector3& direction, short orient2D, const Vector4& color,
								   float width, float life, float vel, float scaleRate, float rot2D, int flags)
	{
		assert(Modules.size() <= MODULE_COUNT_MAX);

		// Return early if module map is full and element with entityNumber key doesn't already exist.
		if (Modules.size() == MODULE_COUNT_MAX && !Modules.count(entityNumber))
			return;

		// Get module and extend streamer within pool.
		auto& module = this->GetModule(entityNumber);
		module.AddStreamer(tag, pos, direction, orient2D, color, width, life, vel, scaleRate, rot2D, flags);
	}

	void StreamerEffectController::Update()
	{
		if (Modules.empty())
			return;

		for (auto& [entityNumber, module] : this->Modules)
			module.Update();

		this->ClearInactiveModules();
	}

	void StreamerEffectController::Clear()
	{
		*this = {};
	}

	StreamerModule& StreamerEffectController::GetModule(int entityNumber)
	{
		// Get module at entityNumber key.
		this->Modules.insert({ entityNumber, {} });
		auto& module = this->Modules.at(entityNumber);
		return module;
	}

	void StreamerEffectController::ClearInactiveModules()
	{
		for (auto it = Modules.begin(); it != Modules.end();)
		{
			if (it->second.Pools.empty())
			{
				it = this->Modules.erase(it);
				continue;
			}
			
			++it;
		}
	}

	StreamerEffectController StreamerEffect = {};

	//------------------------

	std::array<std::vector<StreamerSegmentOld>, (int)StreamerType::Count> Streamers = {};

	static StreamerSegmentOld& GetNewStreamerSegment(StreamerType type)
	{
		constexpr auto COUNT_MAX = 256;

		// Add and return new segment.
		if (Streamers[(int)type].size() < COUNT_MAX)
			return Streamers[(int)type].emplace_back();

		// Clear and return oldest segment.
		auto& segment = Streamers[(int)type][0];
		segment = {};
		return segment;
	}

	void ClearInactiveStreamerSegments()
	{
		for (auto& streamer : Streamers)
			ClearInactiveEffects(streamer);
	}

	void SpawnStreamerSegment(const Vector3& pos, ItemInfo* item, int type, float width, float life, float fade)
	{
		constexpr auto OPACITY_MAX = 0.7f;

		auto& segment = GetNewStreamerSegment((StreamerType)type);

		segment.Type = (StreamerType)type;
		segment.Direction = -EulerAngles(0, item->Pose.Orientation.y, 0).ToDirection();
		segment.Life = life;
		segment.Opacity = OPACITY_MAX;
		segment.ScaleRate = width;
		segment.FadeOut = fade;

		auto leftDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(-90.0f), 0));
		auto rightDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(90.0f), 0));

		auto leftVertex = Geometry::TranslatePoint(pos, leftDirection, width);
		auto rightVertex = Geometry::TranslatePoint(pos, rightDirection, width);

		if ((StreamerType)type == StreamerType::Left)
		{
			segment.Vertices[0] = leftVertex;
			segment.Vertices[1] = rightVertex;
		}
		else
		{
			segment.Vertices[1] = leftVertex;
			segment.Vertices[0] = rightVertex;
		}
	}

	void SpawnStreamer(ItemInfo* item, int xOffset, int yOffset, int zOffset, int type, bool isOnWater, float width, float life, float fade)
	{
		float sinY = phd_sin(item->Pose.Orientation.y);
		float cosY = phd_cos(item->Pose.Orientation.y);

		//auto collPos = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, )
		int x = item->Pose.Position.x + (zOffset * sinY) + (xOffset * cosY);
		int z = item->Pose.Position.z + (zOffset * cosY) - (xOffset * sinY);

		int probedRoomNumber = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).RoomNumber;
		int waterHeight = GetWaterHeight(x, item->Pose.Position.y, z, probedRoomNumber);

		if (isOnWater)
		{
			if (waterHeight != NO_HEIGHT)
			{
				auto pos = Vector3(x, yOffset, z);
				SpawnStreamerSegment(pos, item, type, width, life, fade);
			}
		}
		else
		{
			auto pos = Vector3(xOffset, yOffset, zOffset);
			SpawnStreamerSegment(pos, item, type, width, life, fade);
		}
	}

	void UpdateStreamers()
	{
		for (auto& streamer : Streamers)
		{
			for (auto& segment : streamer)
			{
				if (segment.Opacity > 0.0f)
					segment.Opacity -= 0.1f / segment.FadeOut;

				auto leftDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(-90.0f), 0));
				auto rightDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(90.0f), 0));

				/*auto leftVertex = Geometry::TranslatePoint(segment.Vertices[0], leftDirection, segment.Width);
				auto rightVertex = Geometry::TranslatePoint(segment.Vertices[1], rightDirection, segment.Width);;

				segment.Vertices[0] = leftVertex;
				segment.Vertices[1] = rightVertex;*/

				int zOffset = 0;
				float sinY = phd_sin(EulerAngles(-segment.Direction).y);
				float cosY = phd_cos(EulerAngles(-segment.Direction).y);

				switch (segment.Type)
				{
				case StreamerType::Center:
					segment.Vertices[1] += Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[0] -= Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					break;

				case StreamerType::Left:
					segment.Vertices[0] -= Vector3((zOffset * sinY) + ((segment.ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment.ScaleRate / 2) * sinY));
					segment.Vertices[1] -= Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					break;

				case StreamerType::Right:
					segment.Vertices[1] += Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[0] += Vector3((zOffset * sinY) + ((segment.ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment.ScaleRate / 2) * sinY));
					break;
				}

				segment.Life -= 1.0f;
			}
		}

		ClearInactiveStreamerSegments();
	}
}
