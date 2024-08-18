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

namespace TEN::Effects::Blood
{
	constexpr auto BLOOD_COLOR_RED	 = Vector4(0.8f, 0.0f, 0.0f, 1.0f);
	constexpr auto BLOOD_COLOR_BROWN = Vector4(0.3f, 0.1f, 0.0f, 1.0f);

	BloodDripEffectController		BloodDripEffect		  = {};
	BloodStainEffectController		BloodStainEffect	  = {};
	BloodMistEffectController		BloodMistEffect		  = {};
	UnderwaterBloodEffectController UnderwaterBloodEffect = {};

	void BloodDripEffectParticle::Update()
	{
		if (Life <= 0.0f)
			return;

		// Update opacity.
		if (Life <= LifeStartFading)
			Opacity = Lerp(1.0f, 0.0f, 1.0f - (Life / std::round(LIFE_START_FADING * FPS)));

		// Update color.
		Color.w = Opacity;

		// Update velocity.
		Velocity.y += Gravity;
		if (TestEnvironment(ENV_FLAG_WIND, RoomNumber))
			Velocity += Weather.Wind();

		// Update position.
		Position += Velocity;

		auto pointColl = GetPointCollision(Position, RoomNumber);

		RoomNumber = pointColl.GetRoomNumber();

		// Hit water; spawn underwater blood.
		if (TestEnvironment(ENV_FLAG_WATER, RoomNumber))
		{
			Life = 0.0f;

			if (CanSpawnStain)
			{
				float size = ((Size.x + Size.y) / 2) * 5;
				UnderwaterBloodEffect.Spawn(Position, RoomNumber, size);
			}
		}
		// Hit wall or ceiling; deactivate.
		if (pointColl.GetFloorHeight() == NO_HEIGHT || Position.y <= pointColl.GetCeilingHeight())
		{
			Life = 0.0f;
		}
		// Hit floor; spawn 
		else if (Position.y >= pointColl.GetFloorHeight())
		{
			Life = 0.0f;
			BloodStainEffect.Spawn(*this, pointColl, true);
		}
		// Hit ceiling; spawn 
		else if (Position.y <= pointColl.GetCeilingHeight())
		{
			Life = 0.0f;
			BloodStainEffect.Spawn(*this, pointColl, false);
		}

		// Update life.
		Life -= 1.0f;
	}

	const std::vector<BloodDripEffectParticle>& BloodDripEffectController::GetParticles()
	{
		return Particles;
	}

	void BloodDripEffectController::Spawn(const Vector3& pos, int roomNumber, const Vector3& vel, const Vector2& size, float lifeInSec, bool canSpawnStain)
	{
		constexpr auto COUNT_MAX   = 1024;
		constexpr auto GRAVITY_MAX = 10.0f;
		constexpr auto GRAVITY_MIN = 5.0f;

		// Sprite missing; return early.
		if (!CheckIfSlotExists(ID_DRIP_SPRITE, "Drip sprite missing."))
			return;

		auto& part = GetNewEffect(Particles, COUNT_MAX);

		part.SpriteID = Objects[ID_DRIP_SPRITE].meshIndex;
		part.CanSpawnStain = canSpawnStain;
		part.Position = pos;
		part.RoomNumber = roomNumber;
		part.Size = size;
		part.Velocity = vel;
		part.Color = BLOOD_COLOR_RED;
		part.Life = std::round(lifeInSec * FPS);
		part.LifeStartFading = std::round(BloodDripEffectParticle::LIFE_START_FADING * FPS);
		part.Opacity = 1.0f;
		part.Gravity = Random::GenerateFloat(GRAVITY_MIN, GRAVITY_MAX);
	}

	void BloodDripEffectController::Update()
	{
		if (Particles.empty())
			return;

		for (auto& part : Particles)
			part.Update();

		ClearInactiveEffects(Particles);
	}

	void BloodDripEffectController::Clear()
	{
		Particles.clear();
	}

	void BloodStainEffectParticle::Update()
	{
		if (Life <= 0.0f)
			return;

		// Update delay time.
		if (DelayTime > 0.0f)
		{
			DelayTime -= 1.0f;
			if (DelayTime < 0.0f)
				DelayTime = 0.0f;

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

			// Update vertex points.
			auto vertexPoints = GetVertexPoints();
			if (TestSurface())
			{
				VertexPoints = vertexPoints;
			}
			else
			{
				Scalar = 0.0f;
			}
		}

		// Update opacity.
		if (Life <= LifeStartFading)
		{
			float alpha = 1.0f - (Life / std::round(BloodStainEffectParticle::LIFE_START_FADING * FPS));
			Opacity = Lerp(OpacityMax, 0.0f, alpha);
		}

		// Update color.
		float alpha = 1.0f - (Life / std::round(BloodStainEffectParticle::LIFE_MAX * FPS));
		Color = Vector4::Lerp(ColorStart, ColorEnd, alpha);
		Color.w = Opacity;

		// Update life.
		Life -= 1.0f;
	}

	std::array<Vector3, BloodStainEffectParticle::VERTEX_COUNT> BloodStainEffectParticle::GetVertexPoints()
	{
		constexpr auto POINT_0 = Vector3( SQRT_2, 0.0f,  SQRT_2);
		constexpr auto POINT_1 = Vector3(-SQRT_2, 0.0f,  SQRT_2);
		constexpr auto POINT_2 = Vector3(-SQRT_2, 0.0f, -SQRT_2);
		constexpr auto POINT_3 = Vector3( SQRT_2, 0.0f, -SQRT_2);

		// No size; return vertices at singularity.
		if (Size == 0.0f)
			return std::array<Vector3, VERTEX_COUNT>{ Position, Position, Position, Position };

		// Determine rotation matrix.
		auto rotMatrix = Geometry::GetRelOrientToNormal(Orientation2D, Normal).ToRotationMatrix();

		// Calculate and return vertex points.
		return std::array<Vector3, VERTEX_COUNT>
		{
			Position + Vector3::Transform(POINT_0 * (Size / 2), rotMatrix),
			Position + Vector3::Transform(POINT_1 * (Size / 2), rotMatrix),
			Position + Vector3::Transform(POINT_2 * (Size / 2), rotMatrix),
			Position + Vector3::Transform(POINT_3 * (Size / 2), rotMatrix)
		};
	}

	// TODO: Ceilings.
	bool BloodStainEffectParticle::TestSurface()
	{
		constexpr auto ABS_FLOOR_BOUND = CLICK(0.5f);
		constexpr auto VERTICAL_OFFSET = CLICK(1);

		// Get point collision at every vertex point.
		int vPos = Position.y - VERTICAL_OFFSET;
		auto pointColl0 = GetPointCollision(Vector3i(VertexPoints[0].x, vPos, VertexPoints[0].z), RoomNumber);
		auto pointColl1 = GetPointCollision(Vector3i(VertexPoints[1].x, vPos, VertexPoints[1].z), RoomNumber);
		auto pointColl2 = GetPointCollision(Vector3i(VertexPoints[2].x, vPos, VertexPoints[2].z), RoomNumber);
		auto pointColl3 = GetPointCollision(Vector3i(VertexPoints[3].x, vPos, VertexPoints[3].z), RoomNumber);

		// Stop scaling blood stain if floor heights at vertex points are beyond lower/upper floor height bound.
		if (abs(pointColl0.GetFloorHeight() - pointColl1.GetFloorHeight()) > ABS_FLOOR_BOUND ||
			abs(pointColl1.GetFloorHeight() - pointColl2.GetFloorHeight()) > ABS_FLOOR_BOUND ||
			abs(pointColl2.GetFloorHeight() - pointColl3.GetFloorHeight()) > ABS_FLOOR_BOUND ||
			abs(pointColl3.GetFloorHeight() - pointColl0.GetFloorHeight()) > ABS_FLOOR_BOUND)
		{
			return false;
		}

		return true;
	}

	const std::vector<BloodStainEffectParticle>& BloodStainEffectController::GetParticles()
	{
		return Particles;
	}

	void BloodStainEffectController::Spawn(const Vector3& pos, int roomNumber, const Vector3& normal, float size, float scalar, float delayInSec)
	{
		constexpr auto COUNT_MAX   = 192;
		constexpr auto OPACITY_MAX = 0.8f;

		// Sprite missing; return early.
		if (!CheckIfSlotExists(ID_BLOOD_STAIN_SPRITES, "Blood stain sprite missing."))
			return;

		auto& part = GetNewEffect(Particles, COUNT_MAX);
		const auto& object = Objects[ID_BLOOD_STAIN_SPRITES];

		// TODO: Dehardcoding value doesn't work.
		part.SpriteID = object.meshIndex + Random::GenerateInt(0, 7);
		part.Position = pos;
		part.RoomNumber = roomNumber;
		part.Orientation2D = Random::GenerateAngle();
		part.Normal = normal;
		part.Color =
		part.ColorStart = BLOOD_COLOR_RED;
		part.ColorEnd = BLOOD_COLOR_BROWN;
		part.VertexPoints = part.GetVertexPoints();
		part.Life = std::round(BloodStainEffectParticle::LIFE_MAX * FPS);
		part.LifeStartFading = std::round(BloodStainEffectParticle::LIFE_START_FADING * FPS);
		part.Size = 0.0f;
		part.SizeMax = size;
		part.Scalar = scalar;
		part.Opacity =
		part.OpacityMax = OPACITY_MAX;
		part.DelayTime = std::round(delayInSec * FPS);
	}

	void BloodStainEffectController::Spawn(const BloodDripEffectParticle& drip, PointCollisionData& pointColl, bool isOnFloor)
	{
		constexpr auto SIZE_COEFF = 4.2f;

		// Drip can't spawn stain; return early.
		if (!drip.CanSpawnStain)
			return;

		// Underwater; return early.
		if (TestEnvironment(ENV_FLAG_WATER, drip.RoomNumber))
			return;

		auto normal = isOnFloor ? pointColl.GetFloorNormal() : pointColl.GetCeilingNormal();

		auto pos = Vector3(
			drip.Position.x,
			(isOnFloor ? pointColl.GetFloorHeight() : pointColl.GetCeilingHeight()),
			drip.Position.z);
		pos = Geometry::TranslatePoint(pos, normal, BloodStainEffectParticle::SURFACE_OFFSET);

		float size = ((drip.Size.x + drip.Size.y) / 2) * SIZE_COEFF;
		float scalar = std::min(drip.Velocity.Length() / 2, size / 2);

		Spawn(pos, drip.RoomNumber, normal, size, scalar);
	}

	void BloodStainEffectController::Spawn(const ItemInfo& item)
	{
		constexpr auto SCALAR	  = 0.4f;
		constexpr auto DELAY_TIME = 5.0f;

		auto pointColl = GetPointCollision(item);
		auto pos = Vector3(item.Pose.Position.x, pointColl.GetFloorHeight() - BloodStainEffectParticle::SURFACE_OFFSET, item.Pose.Position.z);

		auto obb = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
		float sizeMax = obb.Extents.x + obb.Extents.z;

		Spawn(pos, item.RoomNumber, pointColl.GetFloorNormal(), sizeMax, SCALAR, DELAY_TIME);
	}

	void BloodStainEffectController::Update()
	{
		if (Particles.empty())
			return;

		for (auto& part : Particles)
			part.Update();

		ClearInactiveEffects(Particles);
	}

	void BloodStainEffectController::Clear()
	{
		Particles.clear();
	}

	void BloodMistEffectParticle::Update()
	{
		if (Life <= 0.0f)
			return;

		// Update velocity.
		Velocity.y += Gravity;
		Velocity -= Velocity / Friction;

		// Update position.
		Position += Velocity;
		Orientation2D += Rotation;

		// Update size.
		Size = Lerp(SizeMin, SizeMax, 1.0f - (Life / LifeMax));

		// Update opacity.
		Opacity = Lerp(OpacityMax, 0.0f, 1.0f - (Life / LifeMax));
		Color.w = Opacity;

		// Update life.
		Life -= 1.0f;
	}

	const std::vector<BloodMistEffectParticle>& BloodMistEffectController::GetParticles()
	{
		return Particles;
	}

	void BloodMistEffectController::Spawn(const Vector3& pos, int roomNumber, const Vector3& dir, unsigned int count)
	{
		constexpr auto COUNT_MAX		  = 256;
		constexpr auto LIFE_MAX			  = 0.75f;
		constexpr auto LIFE_MIN			  = 0.25f;
		constexpr auto VEL_MAX			  = 16.0f;
		constexpr auto MIST_SIZE_MAX	  = 128.0f;
		constexpr auto MIST_SIZE_MIN	  = 64.0f;
		constexpr auto MIST_SIZE_MAX_MULT = 4.0f;
		constexpr auto OPACITY_MAX		  = 0.7f;
		constexpr auto GRAVITY_MAX		  = 2.0f;
		constexpr auto GRAVITY_MIN		  = 1.0f;
		constexpr auto FRICTION			  = 4.0f;
		constexpr auto ROT_MAX			  = ANGLE(10.0f);
		constexpr auto SPHERE_RADIUS	  = BLOCK(1 / 8.0f);
		constexpr auto CONE_SEMIANGLE	  = 20.0f;

		// Sprite missing; return early.
		if (!CheckIfSlotExists(ID_BLOOD_MIST_SPRITES, "Blood mist sprite missing."))
			return;

		// Underwater; return early.
		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
			return;

		auto sphere = BoundingSphere(pos, SPHERE_RADIUS);

		for (int i = 0; i < count; i++)
		{
			auto& part = GetNewEffect(Particles, COUNT_MAX);

			part.SpriteID = Objects[ID_SMOKE_SPRITES].meshIndex;
			part.Position = Random::GeneratePointInSphere(sphere);
			part.RoomNumber = roomNumber;
			part.Orientation2D = Random::GenerateAngle();
			part.Velocity = Random::GenerateDirectionInCone(dir, CONE_SEMIANGLE) * Random::GenerateFloat(0.0f, VEL_MAX);
			part.Color = BLOOD_COLOR_RED;
			part.Life =
			part.LifeMax = std::round(Random::GenerateFloat(LIFE_MIN, LIFE_MAX) * FPS);
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
		if (Particles.empty())
			return;

		for (auto& part : Particles)
			part.Update();

		ClearInactiveEffects(Particles);
	}

	void BloodMistEffectController::Clear()
	{
		Particles.clear();
	}

	const std::vector<UnderwaterBloodEffectParticle>& UnderwaterBloodEffectController::GetParticles()
	{
		return Particles;
	}

	void UnderwaterBloodEffectParticle::Update()
	{
		constexpr auto PART_SIZE_MAX = BLOCK(0.25f);

		if (Life <= 0.0f)
			return;

		// Update size.
		if (Size < PART_SIZE_MAX)
			Size += 4.0f;

		// Update life.
		if (Init == 0.0f)
		{
			Life -= 3.0f;
		}
		else if (Init < Life)
		{
			Init += 4.0f;

			if (Init >= Life)
				Init = 0.0f;
		}
	}

	void UnderwaterBloodEffectController::Spawn(const Vector3& pos, int roomNumber, float size, unsigned int count)
	{
		constexpr auto COUNT_MAX	= 512;
		constexpr auto LIFE_MAX		= 8.5f;
		constexpr auto LIFE_MIN		= 8.0f;
		constexpr auto SPAWN_RADIUS = BLOCK(0.25f);

		// Sprite missing; return early.
		if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Underwater blood sprite missing."))
			return;

		auto sphere = BoundingSphere(pos, SPAWN_RADIUS);

		for (int i = 0; i > count; i++)
		{
			auto& part = GetNewEffect(Particles, COUNT_MAX);

			part.SpriteID = Objects[ID_DEFAULT_SPRITES].meshIndex;
			part.Position = Random::GeneratePointInSphere(sphere);
			part.RoomNumber = roomNumber;
			part.Life = std::round(Random::GenerateFloat(LIFE_MIN, LIFE_MAX) * FPS);
			part.Init = 1.0f;
			part.Size = size;
		}
	}

	void UnderwaterBloodEffectController::Update()
	{
		if (Particles.empty())
			return;

		for (auto& part : Particles)
			Update();

		ClearInactiveEffects(Particles);
	}

	void UnderwaterBloodEffectController::Clear()
	{
		Particles.clear();
	}

	void SpawnBloodSplatEffect(const Vector3& pos, int roomNumber, const Vector3& dir, const Vector3& baseVel, unsigned int baseCount)
	{
		constexpr auto LIFE			   = 2.0f;
		constexpr auto WIDTH_MAX	   = 20.0f;
		constexpr auto WIDTH_MIN	   = WIDTH_MAX / 4;
		constexpr auto HEIGHT_MAX	   = WIDTH_MAX * 2;
		constexpr auto HEIGHT_MIN	   = WIDTH_MAX;
		constexpr auto VEL_MAX		   = 35.0f;
		constexpr auto VEL_MIN		   = 15.0f;
		constexpr auto DRIP_COUNT_MULT = 12;
		constexpr auto MIST_COUNT_MULT = 8;
		constexpr auto CONE_SEMIANGLE  = 50.0f;

		// Spawn underwater blood.
		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
		{
			UnderwaterBloodEffect.Spawn(pos, roomNumber, BLOCK(0.5f), baseCount);
			return;
		}

		// Spawn drips.
		unsigned int dripCount = baseCount * DRIP_COUNT_MULT;
		for (int i = 0; i < dripCount; i++)
		{
			float vel = Random::GenerateFloat(VEL_MIN, VEL_MAX);
			auto velVector = baseVel + (Random::GenerateDirectionInCone(dir, CONE_SEMIANGLE) * vel);
			auto size = Vector2(
				Random::GenerateFloat(WIDTH_MIN, WIDTH_MAX),
				Random::GenerateFloat(HEIGHT_MIN, HEIGHT_MAX));

			bool canSpawnStain = (i < baseCount);
			BloodDripEffect.Spawn(pos, roomNumber, velVector, size, LIFE, canSpawnStain);
		}

		// Spawn mists.
		unsigned int mistCount = baseCount * MIST_COUNT_MULT;
		BloodMistEffect.Spawn(pos, roomNumber, dir, mistCount);
	}

	void SpawnPlayerBloodEffect(const ItemInfo& item)
	{
		constexpr auto BASE_COUNT_MAX	 = 3;
		constexpr auto BASE_COUNT_MIN	 = 1;
		constexpr auto SPHERE_RADIUS_MAX = 16.0f;

		auto rotMatrix = item.Pose.Orientation.ToRotationMatrix();
		auto baseVel = Vector3::Transform(item.Animation.Velocity, rotMatrix);
		unsigned int baseCount = Random::GenerateInt(BASE_COUNT_MIN, BASE_COUNT_MAX);

		int node = 1;
		for (int i = 0; i < LARA_MESHES::LM_HEAD; i++)
		{
			if (node & item.TouchBits.ToPackedBits())
			{
				auto sphere = BoundingSphere(Vector3::Zero, Random::GenerateFloat(0.0f, SPHERE_RADIUS_MAX));
				auto relOffset = Random::GeneratePointInSphere(sphere);
				auto pos = GetJointPosition(item, i, relOffset);

				SpawnBloodSplatEffect(pos.ToVector3(), item.RoomNumber, -Vector3::UnitY, baseVel, baseCount);
				//DoBloodSplat(pos.x, pos.y, pos.z, Random::GenerateInt(8, 16), Random::GenerateAngle(), LaraItem->RoomNumber);
			}

			node *= 2;
		}
	}

	// TODO: Room number.
	void TriggerBlood(const Vector3& pos, short headingAngle, unsigned int count)
	{
		SpawnBloodSplatEffect(pos, LaraItem->RoomNumber, -Vector3::UnitY, Vector3::Zero, count / 4);
		//BloodMistEffect.Spawn(Vector3(x, y, z), 0, Vector3::Zero, count);
	}

	short DoBloodSplat(int x, int y, int z, short vel, short headingAngle, short roomNumber)
	{
		int probedRoomNumber = GetPointCollision(Vector3i(x, y, z), roomNumber).GetRoomNumber();
		if (TestEnvironment(ENV_FLAG_WATER, probedRoomNumber))
		{
			UnderwaterBloodEffect.Spawn(Vector3(x, y, z), probedRoomNumber, vel);
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
		g_Renderer.PrintDebugMessage("Blood drips: %d ", BloodDripEffect.GetParticles().size());
		g_Renderer.PrintDebugMessage("Blood stains: %d ", BloodStainEffect.GetParticles().size());
		g_Renderer.PrintDebugMessage("Blood mists: %d ", BloodMistEffect.GetParticles().size());
		g_Renderer.PrintDebugMessage("UW. blood particles: %d ", UnderwaterBloodEffect.GetParticles().size());
	}
}
