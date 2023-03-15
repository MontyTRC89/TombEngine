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
	StreamerEffectController StreamerEffect = {};

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

		// TODO: Not working.
		// Update orientation.
		this->Orientation.SetAngle(Orientation.GetAngle() + Rotation);
		
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

		// If streamer was broken, set bool flag to track it.
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

		// Return most recent streamer iteration if it exists and is unbroken.
		if (!pool.empty())
		{
			auto& streamer = pool.back();
			if (!streamer.IsBroken)
				return streamer;
		}

		// Clear oldest streamer if pool is full.
		if (pool.size() == STREAMER_COUNT_MAX)
			pool.erase(pool.begin());

		// Add and return new streamer iteration.
		return pool.emplace_back();
	}

	void StreamerModule::ClearInactivePools()
	{
		for (auto it = Pools.begin(); it != Pools.end();)
		{
			const auto& pool = it->second;
			if (pool.empty())
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
			const auto& module = it->second;
			if (module.Pools.empty())
			{
				it = this->Modules.erase(it);
				continue;
			}
			
			++it;
		}
	}
}
