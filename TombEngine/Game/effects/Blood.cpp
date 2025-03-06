#include "framework.h"
#include "Game/effects/Blood.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/collision/Point.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Utils/object_helper.h"
#include "Renderer/Renderer.h"
#include "Specific/clock.h"

// temp
#include "Game/Lara/lara.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Point;
using namespace TEN::Effects::Environment;
using namespace TEN::Math;
using namespace TEN::Renderer;

// TODO: Better position randomisation.

namespace TEN::Effects::Blood
{
	constexpr auto BLOOD_COLOR_RED	 = Vector4(0.8f, 0.0f, 0.0f, 1.0f);
	constexpr auto BLOOD_COLOR_BROWN = Vector4(0.3f, 0.1f, 0.0f, 1.0f);

	BloodDripEffectController			 BloodDripEffect			= {};
	BloodStainEffectController			 BloodStainEffect			= {};
	BloodBillboardEffectController		 BloodBillboardEffect		= {};
	BloodMistEffectController			 BloodMistEffect			= {};
	UnderwaterBloodCloudEffectController UnderwaterBloodCloudEffect = {};

	void BloodDripEffectParticle::Update()
	{
		constexpr auto SOUND_ID		   = SFX_TR4_LARA_FOOTSTEPS_MUD;
		constexpr auto SOUND_PITCH_MIN = 2.0f;
		constexpr auto SOUND_PITCH_MAX = 4.0f;
		constexpr auto SOUND_VOL	   = 1.0f;

		if (Life <= 0)
			return;

		// Update opacity.
		if (Life <= LifeStartFading)
			Opacity = Lerp(1.0f, 0.0f, 1.0f - ((float)Life / std::round(LIFE_START_FADING * FPS)));

		// Update color.
		Color.w = Opacity;

		// Update velocity.
		Velocity.y += Gravity; // TODO: Max?
		if (TestEnvironment(ENV_FLAG_WIND, RoomNumber))
			Velocity += Weather.Wind();

		// Update position.
		Position += Velocity;

		auto pointColl = GetPointCollision(Position, RoomNumber);

		RoomNumber = pointColl.GetRoomNumber();

		// Deactivate on wall hit.
		if (pointColl.IsWall())
		{
			Life = 0;
		}
		// Spawn underwater blood.
		else if (TestEnvironment(ENV_FLAG_WATER, RoomNumber))
		{
			Life = 0;

			float size = Size * 5;
			UnderwaterBloodCloudEffect.Spawn(Position, RoomNumber, size);
		}
		// Spawn stain on floor.
		else if (Position.y >= pointColl.GetFloorHeight() && Position.y <= (pointColl.GetFloorHeight() + Velocity.y))
		{
			Life = 0;
			BloodStainEffect.Spawn(*this, pointColl, true);
			SoundEffect(SOUND_ID, &Pose(Vector3i(Position)), SoundEnvironment::Always, Random::GenerateFloat(SOUND_PITCH_MIN, SOUND_PITCH_MAX), SOUND_VOL);
		}
		// Spawn stain on ceiling.
		else if (Position.y <= pointColl.GetCeilingHeight() && Position.y >= (pointColl.GetCeilingHeight() + Velocity.y))
		{
			Life = 0;
			BloodStainEffect.Spawn(*this, pointColl, false);
			SoundEffect(SOUND_ID, &Pose(Vector3i(Position)), SoundEnvironment::Always,	Random::GenerateFloat(SOUND_PITCH_MIN, SOUND_PITCH_MAX), SOUND_VOL);
		}

		// Update life.
		Life--;
	}

	const std::vector<BloodDripEffectParticle>& BloodDripEffectController::GetParticles()
	{
		return _particles;
	}

	void BloodDripEffectController::Spawn(const Vector3& pos, int roomNumber, const Vector3& vel, float size, float lifeInSec)
	{
		constexpr auto COUNT_MAX   = 1024;
		constexpr auto GRAVITY_MAX = 10.0f;
		constexpr auto GRAVITY_MIN = 5.0f;

		auto& part = GetNewEffect(_particles, COUNT_MAX);

		part.SpriteSeqID = ID_BLOOD_DRIP_SPRITE;
		part.SpriteID = 0;
		part.Position = pos;
		part.RoomNumber = roomNumber;
		part.Size = size;
		part.Velocity = vel;
		part.Color = BLOOD_COLOR_RED;
		part.Life = (int)std::round(lifeInSec * FPS);
		part.LifeStartFading = (int)std::round(BloodDripEffectParticle::LIFE_START_FADING * FPS);
		part.Opacity = 0.8f;
		part.Gravity = Random::GenerateFloat(GRAVITY_MIN, GRAVITY_MAX);
	}

	void BloodDripEffectController::Update()
	{
		if (_particles.empty())
			return;

		for (auto& part : _particles)
			part.Update();

		ClearInactiveEffects(_particles);
	}

	void BloodDripEffectController::Clear()
	{
		_particles.clear();
	}

	void BloodStainEffectParticle::Update()
	{
		constexpr auto ABS_SURF_HEIGHT_BOUND = CLICK(0.5f);

		if (Life <= 0)
			return;

		// Update delay time.
		if (DelayTime > 0)
		{
			DelayTime--;
			if (DelayTime < 0)
				DelayTime = 0;

			return;
		}

		// Update size.
		if (Scalar > 0.0f)
		{
			Size += Scalar;

			// Update scalar.
			if (Size >= (SizeMax * 0.8f))
			{
				Scalar *= 0.2f;
				if (abs(Scalar) <= EPSILON)
					Scalar = 0.0f;
			}

			// Update vertices.
			auto vertices = GetVertices();
			if (TestSurface())
			{
				Vertices = vertices;
			}
			else
			{
				Scalar = 0.0f;
			}
		}

		// Update opacity.
		if (Life <= LifeStartFading)
		{
			float alpha = 1.0f - ((float)Life / std::round(BloodStainEffectParticle::LIFE_START_FADING * FPS));
			Opacity = Lerp(OpacityMax, 0.0f, alpha);
		}

		// Update color.
		float alpha = 1.0f - ((float)Life / std::round(BloodStainEffectParticle::LIFE_MAX * FPS));
		Color = Vector4::Lerp(ColorStart, ColorEnd, alpha);
		Color.w = Opacity;

		// Update life.
		Life--;
		if (TestGlobalTimeInterval(COLL_CHECK_TIME_INTERVAL, CollCheckTimeOffset))
		{
			// TODO: Better handling of moving surfaces.
			// Vanish if floor disappreared.
			auto pointColl = GetPointCollision(Position, RoomNumber);
			if ((IsOnFloor && abs(pointColl.GetFloorHeight() - Position.y) > ABS_SURF_HEIGHT_BOUND) ||
				(!IsOnFloor && abs(pointColl.GetCeilingHeight() - Position.y) > ABS_SURF_HEIGHT_BOUND))
			{
				Life = 0;
			}
		}
	}

	std::array<Vector3, BloodStainEffectParticle::VERTEX_COUNT> BloodStainEffectParticle::GetVertices()
	{
		constexpr auto REL_VERTEX_0 = Vector3( SQRT_2, 0.0f,  SQRT_2);
		constexpr auto REL_VERTEX_1 = Vector3(-SQRT_2, 0.0f,  SQRT_2);
		constexpr auto REL_VERTEX_2 = Vector3(-SQRT_2, 0.0f, -SQRT_2);
		constexpr auto REL_VERTEX_3 = Vector3( SQRT_2, 0.0f, -SQRT_2);

		// No size; return vertices at singularity.
		if (Size <= EPSILON)
			return std::array<Vector3, VERTEX_COUNT>{ Position, Position, Position, Position };

		// Calculate and return vertices.
		int sign = IsOnFloor ? 1 : -1;
		auto rotMatrix = Geometry::GetRelOrientToNormal(Orientation, Normal * sign).ToRotationMatrix();
		return std::array<Vector3, VERTEX_COUNT>
		{
			Position + Vector3::Transform(REL_VERTEX_0 * (Size / 2), rotMatrix),
			Position + Vector3::Transform(REL_VERTEX_1 * (Size / 2), rotMatrix),
			Position + Vector3::Transform(REL_VERTEX_2 * (Size / 2), rotMatrix),
			Position + Vector3::Transform(REL_VERTEX_3 * (Size / 2), rotMatrix)
		};
	}

	bool BloodStainEffectParticle::TestSurface() const
	{
		constexpr auto ABS_SURF_BOUND  = CLICK(0.5f);
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Get point collision at every vertex.
		int vPos = Position.y + (IsOnFloor ? -VERTICAL_OFFSET : VERTICAL_OFFSET);
		auto pointColl0 = GetPointCollision(Vector3i(Vertices[0].x, vPos, Vertices[0].z), RoomNumber);
		auto pointColl1 = GetPointCollision(Vector3i(Vertices[1].x, vPos, Vertices[1].z), RoomNumber);
		auto pointColl2 = GetPointCollision(Vector3i(Vertices[2].x, vPos, Vertices[2].z), RoomNumber);
		auto pointColl3 = GetPointCollision(Vector3i(Vertices[3].x, vPos, Vertices[3].z), RoomNumber);

		// Stop scaling blood stain if floor heights at vertices are beyond surface height bound.
		if (IsOnFloor)
		{
			if (abs(pointColl0.GetFloorHeight() - pointColl1.GetFloorHeight()) > ABS_SURF_BOUND ||
				abs(pointColl1.GetFloorHeight() - pointColl2.GetFloorHeight()) > ABS_SURF_BOUND ||
				abs(pointColl2.GetFloorHeight() - pointColl3.GetFloorHeight()) > ABS_SURF_BOUND ||
				abs(pointColl3.GetFloorHeight() - pointColl0.GetFloorHeight()) > ABS_SURF_BOUND)
			{
				return false;
			}
		}
		else
		{
			if (abs(pointColl0.GetCeilingHeight() - pointColl1.GetCeilingHeight()) > ABS_SURF_BOUND ||
				abs(pointColl1.GetCeilingHeight() - pointColl2.GetCeilingHeight()) > ABS_SURF_BOUND ||
				abs(pointColl2.GetCeilingHeight() - pointColl3.GetCeilingHeight()) > ABS_SURF_BOUND ||
				abs(pointColl3.GetCeilingHeight() - pointColl0.GetCeilingHeight()) > ABS_SURF_BOUND)
			{
				return false;
			}
		}

		return true;
	}

	const std::vector<BloodStainEffectParticle>& BloodStainEffectController::GetParticles()
	{
		return _particles;
	}

	void BloodStainEffectController::Spawn(const Vector3& pos, int roomNumber, const Vector3& normal, float size, float scalar, float delayInSec)
	{
		constexpr auto COUNT_MAX   = 512;
		constexpr auto OPACITY_MAX = 0.8f;

		const auto& object = Objects[ID_BLOOD_STAIN_SPRITES];

		auto& part = GetNewEffect(_particles, COUNT_MAX);

		part.SpriteSeqID = ID_BLOOD_STAIN_SPRITES;
		part.SpriteID = Random::GenerateInt(0, abs(object.nmeshes) - 1);
		part.IsOnFloor = (normal.y < 0.0f);
		part.CollCheckTimeOffset = Random::GenerateFloat(0.0f, BloodStainEffectParticle::COLL_CHECK_TIME_INTERVAL);
		part.Position = pos;
		part.RoomNumber = roomNumber;
		part.Orientation = Random::GenerateAngle();
		part.Normal = normal;
		part.Color =
		part.ColorStart = BLOOD_COLOR_RED;
		part.ColorEnd = BLOOD_COLOR_BROWN;
		part.Vertices = part.GetVertices();
		part.Life = (int)std::round(BloodStainEffectParticle::LIFE_MAX * FPS);
		part.LifeStartFading = (int)std::round(BloodStainEffectParticle::LIFE_START_FADING * FPS);
		part.Size = 0.0f;
		part.SizeMax = size;
		part.Scalar = scalar;
		part.Opacity =
		part.OpacityMax = OPACITY_MAX;
		part.DelayTime = (int)std::round(delayInSec * FPS);
	}

	void BloodStainEffectController::Spawn(const BloodDripEffectParticle& drip, PointCollisionData& pointColl, bool isOnFloor)
	{
		constexpr auto SIZE_COEFF = 5.0f;

		// Underwater; return early.
		if (TestEnvironment(ENV_FLAG_WATER, drip.RoomNumber))
			return;

		// Calculate position.
		auto normal = isOnFloor ? pointColl.GetFloorNormal() : pointColl.GetCeilingNormal();
		auto pos = Vector3(drip.Position.x, isOnFloor ? pointColl.GetFloorHeight() : pointColl.GetCeilingHeight(), drip.Position.z);
		pos = Geometry::TranslatePoint(pos, normal, BloodStainEffectParticle::SURFACE_OFFSET);

		float size = drip.Size * SIZE_COEFF;
		float scalar = std::min(drip.Velocity.Length() / 2, size / 2);

		Spawn(pos, drip.RoomNumber, normal, size, scalar);
	}

	void BloodStainEffectController::Spawn(const ItemInfo& item)
	{
		constexpr auto SCALAR	  = 0.4f;
		constexpr auto DELAY_TIME = 5.0f;

		auto pointColl = GetPointCollision(item);

		// Calculate position.
		auto pos = Vector3(item.Pose.Position.x, pointColl.GetFloorHeight(), item.Pose.Position.z);
		pos = Geometry::TranslatePoint(pos, pointColl.GetFloorNormal(), BloodStainEffectParticle::SURFACE_OFFSET);

		// Calculate size.
		auto obb = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
		float size = ((obb.Extents.x + obb.Extents.z) / 2) * 2;

		Spawn(pos, item.RoomNumber, pointColl.GetFloorNormal(), size, SCALAR, DELAY_TIME);
	}

	void BloodStainEffectController::Update()
	{
		if (_particles.empty())
			return;

		for (auto& part : _particles)
			part.Update();

		ClearInactiveEffects(_particles);
	}

	void BloodStainEffectController::Clear()
	{
		_particles.clear();
	}

	void BloodBillboardEffectParticle::Update()
	{
		const auto& object = Objects[SpriteSeqID];

		// Update sprite ID.
		SpriteID++;

		// Update life.
		if (SpriteID > (abs(object.nmeshes) - 1))
			Life = 0.0f;
	}

	void BloodBillboardEffectController::Spawn(const Vector3& pos, int roomNumber, float size)
	{
		constexpr auto	  COUNT_MAX				= 128;
		constexpr auto	  SPHERE_RADIUS			= BLOCK(1 / 16.0f);
		static const auto SPRITE_SEQ_OBJECT_IDS = std::vector{ ID_BLOOD_SPLASH_SPRITES_1, ID_BLOOD_SPLASH_SPRITE_2, ID_BLOOD_SPLASH_SPRITE_3 };

		auto sphere = BoundingSphere(pos, SPHERE_RADIUS);

		auto& part = GetNewEffect(_particles, COUNT_MAX);

		part.SpriteSeqID = SPRITE_SEQ_OBJECT_IDS[Random::GenerateInt(0, (int)SPRITE_SEQ_OBJECT_IDS.size() - 1)];
		part.SpriteID = 0;
		part.Position = Random::GeneratePointOnSphere(sphere);
		part.RoomNumber = roomNumber;
		part.Color = BLOOD_COLOR_RED;
		part.Life = 1;
		part.Size = size;
	}

	void BloodBillboardEffectController::Update()
	{
		if (_particles.empty())
			return;

		for (auto& part : _particles)
			part.Update();

		ClearInactiveEffects(_particles);
	}

	void BloodBillboardEffectController::Clear()
	{
		_particles.clear();
	}

	const std::vector<BloodBillboardEffectParticle>& BloodBillboardEffectController::GetParticles()
	{
		return _particles;
	}

	void BloodMistEffectParticle::Update()
	{
		if (Life <= 0)
			return;

		// Update velocity.
		Velocity.y += Gravity;
		Velocity -= Velocity / Friction;

		// Update position.
		Position += Velocity;
		Orientation2D += Rotation;

		// Update size.
		Size = Lerp(SizeMin, SizeMax, 1.0f - ((float)Life / (float)LifeMax));

		// Update opacity.
		Opacity = Lerp(OpacityMax, 0.0f, 1.0f - ((float)Life / (float)LifeMax));
		Color.w = Opacity;

		// Update life.
		Life--;
	}

	const std::vector<BloodMistEffectParticle>& BloodMistEffectController::GetParticles()
	{
		return _particles;
	}

	void BloodMistEffectController::Spawn(const Vector3& pos, int roomNumber, const Vector3& dir, unsigned int count)
	{
		constexpr auto COUNT_MAX		  = 256;
		constexpr auto LIFE_MAX			  = 1.0f; // Life in seconds.
		constexpr auto LIFE_MIN			  = 0.5f; // Life in seconds.
		constexpr auto VEL_MAX			  = 16.0f;
		constexpr auto MIST_SIZE_MAX	  = 128.0f;
		constexpr auto MIST_SIZE_MIN	  = 64.0f;
		constexpr auto MIST_SIZE_MAX_MULT = 4.0f;
		constexpr auto OPACITY_MAX		  = 0.6f;
		constexpr auto GRAVITY_MAX		  = 2.0f;
		constexpr auto GRAVITY_MIN		  = 1.0f;
		constexpr auto FRICTION			  = 4.0f;
		constexpr auto ROT_MAX			  = ANGLE(10.0f);
		constexpr auto SPHERE_RADIUS	  = BLOCK(1 / 8.0f);
		constexpr auto CONE_SEMIANGLE	  = 20.0f;

		// Underwater; return early.
		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		auto sphere = BoundingSphere(pos, SPHERE_RADIUS);

		for (int i = 0; i < count; i++)
		{
			auto& part = GetNewEffect(_particles, COUNT_MAX);

			part.SpriteSeqID = ID_BLOOD_MIST_SPRITES;
			part.SpriteID = 0;
			part.Position = Random::GeneratePointInSphere(sphere);
			part.RoomNumber = roomNumber;
			part.Orientation2D = Random::GenerateAngle();
			part.Velocity = Random::GenerateDirectionInCone(dir, CONE_SEMIANGLE) * Random::GenerateFloat(0.0f, VEL_MAX);
			part.Color = BLOOD_COLOR_RED;
			part.Life =
			part.LifeMax = (int)std::round(Random::GenerateFloat(LIFE_MIN, LIFE_MAX) * FPS);
			part.Size = Random::GenerateFloat(MIST_SIZE_MIN, MIST_SIZE_MAX);
			part.SizeMax = part.Size * MIST_SIZE_MAX_MULT;
			part.SizeMin = part.Size;
			part.Opacity =
			part.OpacityMax = OPACITY_MAX;
			part.Gravity = Random::GenerateFloat(GRAVITY_MIN, GRAVITY_MAX);
			part.Friction = FRICTION;
			part.Rotation = Random::GenerateAngle(-ROT_MAX, ROT_MAX);
		}
	}

	void BloodMistEffectController::Update()
	{
		if (_particles.empty())
			return;

		for (auto& part : _particles)
			part.Update();

		ClearInactiveEffects(_particles);
	}

	void BloodMistEffectController::Clear()
	{
		_particles.clear();
	}

	const std::vector<UnderwaterBloodCloudEffectParticle>& UnderwaterBloodCloudEffectController::GetParticles()
	{
		return _particles;
	}

	void UnderwaterBloodCloudEffectParticle::Update()
	{
		constexpr auto PART_SIZE_MAX = BLOCK(0.25f);

		if (Life <= 0)
			return;

		// Update size.
		if (Size < PART_SIZE_MAX)
			Size += 4.0f;

		// Update life.
		if (Init == 0.0f)
		{
			Life -= 3;
		}
		else if (Init < Life)
		{
			Init += 4.0f;

			if (Init >= Life)
				Init = 0.0f;
		}
	}

	void UnderwaterBloodCloudEffectController::Spawn(const Vector3& pos, int roomNumber, float size, unsigned int count)
	{
		constexpr auto COUNT_MAX	= 512;
		constexpr auto LIFE_MAX		= 8.5f;
		constexpr auto LIFE_MIN		= 8.0f;
		constexpr auto SPAWN_RADIUS = BLOCK(0.25f);

		auto sphere = BoundingSphere(pos, SPAWN_RADIUS);

		for (int i = 0; i > count; i++)
		{
			auto& part = GetNewEffect(_particles, COUNT_MAX);

			part.SpriteSeqID = ID_BLOOD_MIST_SPRITES;
			part.SpriteID = 0;
			part.Position = Random::GeneratePointInSphere(sphere);
			part.RoomNumber = roomNumber;
			part.Life = (int)std::round(Random::GenerateFloat(LIFE_MIN, LIFE_MAX) * FPS);
			part.Init = 1.0f;
			part.Size = size;
		}
	}

	void UnderwaterBloodCloudEffectController::Update()
	{
		if (_particles.empty())
			return;

		for (auto& part : _particles)
			Update();

		ClearInactiveEffects(_particles);
	}

	void UnderwaterBloodCloudEffectController::Clear()
	{
		_particles.clear();
	}

	void SpawnBloodSplatEffect(const Vector3& pos, int roomNumber, const Vector3& dir, const Vector3& baseVel, unsigned int count)
	{
		constexpr auto DRIP_LIFE		   = 2.0f; // Life in seconds.
		constexpr auto DRIP_SIZE_MAX	   = BLOCK(1 / 64.0f);
		constexpr auto DRIP_SIZE_MIN	   = DRIP_SIZE_MAX / 2;
		constexpr auto DRIP_VEL_MAX		   = BLOCK(1 / 32.0f);
		constexpr auto DRIP_VEL_MIN		   = DRIP_VEL_MAX / 2;
		constexpr auto BILLBOARD_SIZE	   = BLOCK(1 / 5.0f);
		constexpr auto MIST_COUNT_MULT	   = 2;
		constexpr auto UW_CLOUD_SIZE	   = BLOCK(0.5f);
		constexpr auto DRIP_CONE_SEMIANGLE = 50.0f;

		// Spawn underwater cloud.
		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
		{
			UnderwaterBloodCloudEffect.Spawn(pos, roomNumber, UW_CLOUD_SIZE, count);
		}
		else
		{
			// Spawn drips.
			for (int i = 0; i < count; i++)
			{
				float vel = Random::GenerateFloat(DRIP_VEL_MIN, DRIP_VEL_MAX);
				auto velVector = baseVel + (Random::GenerateDirectionInCone(dir, DRIP_CONE_SEMIANGLE) * vel);
				auto size = Random::GenerateFloat(DRIP_SIZE_MIN, DRIP_SIZE_MAX);

				BloodDripEffect.Spawn(pos, roomNumber, velVector, size, DRIP_LIFE);
			}

			// Spawn billboard.
			BloodBillboardEffect.Spawn(pos, roomNumber, BILLBOARD_SIZE);

			// Spawn mists.
			BloodMistEffect.Spawn(pos, roomNumber, dir, std::max<int>(count * MIST_COUNT_MULT, 2));
		}
	}

	void SpawnPlayerBloodEffect(const ItemInfo& item)
	{
		constexpr auto COUNT_MAX	 = 2;
		constexpr auto SPHERE_RADIUS = 16.0f;

		auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
		auto baseVel = Vector3::Transform(item.Animation.Velocity, rotMatrix);

		auto sphere = BoundingSphere(Vector3::Zero, SPHERE_RADIUS);

		int node = 1;
		for (int i = 0; i < LARA_MESHES::LM_HEAD; i++)
		{
			if (node & item.TouchBits.ToPackedBits())
			{
				auto relOffset = Random::GeneratePointInSphere(sphere);
				auto pos = GetJointPosition(item, i, relOffset).ToVector3();
				unsigned int count = Random::GenerateInt(0, COUNT_MAX);

				SpawnBloodSplatEffect(pos, item.RoomNumber, -Vector3::UnitY, baseVel, count);
			}

			node *= 2;
		}
	}

	// TODO: Room number.
	void TriggerBlood(const Vector3& pos, short headingAngle, unsigned int count)
	{
		SpawnBloodSplatEffect(pos, LaraItem->RoomNumber, -Vector3::UnitY, Vector3::Zero, count / 6);
	}

	short DoBloodSplat(int x, int y, int z, short vel, short headingAngle, short roomNumber)
	{
		int probedRoomNumber = GetPointCollision(Vector3i(x, y, z), roomNumber).GetRoomNumber();
		if (TestEnvironment(ENV_FLAG_WATER, probedRoomNumber))
		{
			UnderwaterBloodCloudEffect.Spawn(Vector3(x, y, z), probedRoomNumber, vel);
		}
		else
		{
			TriggerBlood(Vector3(x, y, z), headingAngle / 16, vel);
		}

		return 0;
	}

	void DoLotsOfBlood(const Vector3& pos, int vel, short headingAngle, int roomNumber, unsigned int count)
	{
		constexpr auto SPHERE_RADIUS = BLOCK(0.25f);

		auto sphere = BoundingSphere(pos, SPHERE_RADIUS);
		for (int i = 0; i < count; i++)
		{
			auto bloodPos = Random::GeneratePointInSphere(sphere);
			DoBloodSplat(bloodPos.x, bloodPos.y, bloodPos.z, vel, headingAngle, roomNumber);
		}
	}

	void DrawBloodDebug()
	{
		g_Renderer.PrintDebugMessage("BLOOD EFFECT DEBUG");
		g_Renderer.PrintDebugMessage("Drips: %d ", BloodDripEffect.GetParticles().size());
		g_Renderer.PrintDebugMessage("Stains: %d ", BloodStainEffect.GetParticles().size());
		g_Renderer.PrintDebugMessage("Billboards: %d ", BloodBillboardEffect.GetParticles().size());
		g_Renderer.PrintDebugMessage("Mists: %d ", BloodMistEffect.GetParticles().size());
		g_Renderer.PrintDebugMessage("Underwater clouds: %d ", UnderwaterBloodCloudEffect.GetParticles().size());
	}
}
