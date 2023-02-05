#include "framework.h"
#include "Game/effects/Footprint.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Effects::Footprint
{
	constexpr auto FOOTPRINT_COUNT_MAX = 32;

	const auto FootprintMaterials = std::vector<FLOOR_MATERIAL>
	{
		FLOOR_MATERIAL::Mud,
		FLOOR_MATERIAL::Snow,
		FLOOR_MATERIAL::Sand,
		FLOOR_MATERIAL::Gravel,
		FLOOR_MATERIAL::Custom1,
		FLOOR_MATERIAL::Custom2,
		FLOOR_MATERIAL::Custom3,
		FLOOR_MATERIAL::Custom4
	};

	std::deque<Footprint> Footprints = {};

	static SOUND_EFFECTS GetFootprintSfx(FLOOR_MATERIAL material)
	{
		switch (material)
		{
		default:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case FLOOR_MATERIAL::Mud:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MUD;

		case FLOOR_MATERIAL::Snow:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SNOW;

		case FLOOR_MATERIAL::Sand:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SAND;

		case FLOOR_MATERIAL::Gravel:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRAVEL;

		case FLOOR_MATERIAL::Ice:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_ICE;

		case FLOOR_MATERIAL::Water:
			return SOUND_EFFECTS::SFX_TR4_LARA_WET_FEET;

		case FLOOR_MATERIAL::Stone:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case FLOOR_MATERIAL::Wood:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;

		case FLOOR_MATERIAL::Metal:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;

		case FLOOR_MATERIAL::Marble:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MARBLE;

		case FLOOR_MATERIAL::Grass:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRASS;

		case FLOOR_MATERIAL::Concrete:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case FLOOR_MATERIAL::OldWood:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;

		case FLOOR_MATERIAL::OldMetal:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;

		case FLOOR_MATERIAL::Custom1:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_1;

		case FLOOR_MATERIAL::Custom2:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_2;

		case FLOOR_MATERIAL::Custom3:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_3;

		case FLOOR_MATERIAL::Custom4:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_4;

		case FLOOR_MATERIAL::Custom5:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_5;

		case FLOOR_MATERIAL::Custom6:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_6;

		case FLOOR_MATERIAL::Custom7:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_7;

		case FLOOR_MATERIAL::Custom8:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_8;
		}
	}

	static std::array<Vector3, 4> GetFootprintVertexPoints(const ItemInfo& item, const Vector3& pos, const Vector3& normal)
	{
		constexpr auto SCALE   = BLOCK(1 / 16.0f);
		constexpr auto POINT_0 = Vector3( SCALE, 0.0f,  SCALE);
		constexpr auto POINT_1 = Vector3(-SCALE, 0.0f,  SCALE);
		constexpr auto POINT_2 = Vector3(-SCALE, 0.0f, -SCALE);
		constexpr auto POINT_3 = Vector3( SCALE, 0.0f, -SCALE);

		// Determine rotation matrix.
		auto rotMatrix = Geometry::GetRelOrientToNormal(item.Pose.Orientation.y, normal).ToRotationMatrix();

		return std::array<Vector3, 4>
		{
			pos + Vector3::Transform(POINT_0, rotMatrix),
			pos + Vector3::Transform(POINT_1, rotMatrix),
			pos + Vector3::Transform(POINT_2, rotMatrix),
			pos + Vector3::Transform(POINT_3, rotMatrix)
		};
	}

	static bool TestMaterial(FLOOR_MATERIAL refMaterial, const std::vector<FLOOR_MATERIAL>& materialList)
	{
		for (const auto& material : materialList)
		{
			if (material == refMaterial)
				return true;
		}

		return false;
	}

	static bool TestFootHeight(const ItemInfo& item, int meshIndex, Vector3& outFootprintPos)
	{
		constexpr auto SURFACE_OFFSET = 4;
		constexpr auto HEIGHT_OFFSET  = CLICK(0.25f);
		constexpr auto HEIGHT_RANGE	  = CLICK(0.25f);

		static const auto FOOT_OFFSET = Vector3i(0, HEIGHT_OFFSET, 0);

		auto footPos = GetJointPosition(LaraItem, meshIndex, FOOT_OFFSET);
		int floorHeight = GetCollision(footPos.x, footPos.y - CLICK(1), footPos.z, item.RoomNumber).Position.Floor;

		outFootprintPos = Vector3(footPos.x, floorHeight - SURFACE_OFFSET, footPos.z);
		return (abs(footPos.y - floorHeight) < HEIGHT_RANGE);
	}

	static bool TestFootprintFloor(const ItemInfo& item, const Vector3& pos, const std::array<Vector3, 4>& vertexPoints)
	{
		constexpr auto HEIGHT_RANGE = CLICK(0.5f);

		// Get point collision at every vertex point.
		auto pointColl0 = GetCollision(vertexPoints[0].x, pos.y - CLICK(1), vertexPoints[0].z, item.RoomNumber);
		auto pointColl1 = GetCollision(vertexPoints[1].x, pos.y - CLICK(1), vertexPoints[1].z, item.RoomNumber);
		auto pointColl2 = GetCollision(vertexPoints[2].x, pos.y - CLICK(1), vertexPoints[2].z, item.RoomNumber);
		auto pointColl3 = GetCollision(vertexPoints[3].x, pos.y - CLICK(1), vertexPoints[3].z, item.RoomNumber);

		// Don't spawn footprint if floor heights at vertex points are outside relative range.
		if ((abs(pointColl0.Position.Floor - pointColl1.Position.Floor) > HEIGHT_RANGE) ||
			(abs(pointColl1.Position.Floor - pointColl2.Position.Floor) > HEIGHT_RANGE) ||
			(abs(pointColl2.Position.Floor - pointColl3.Position.Floor) > HEIGHT_RANGE) ||
			(abs(pointColl3.Position.Floor - pointColl0.Position.Floor) > HEIGHT_RANGE))
		{
			return false;
		}

		return true;
	}

	void SpawnFootprint(bool isRightFoot, const std::array<Vector3, 4>& vertexPoints)
	{
		constexpr auto LIFE_MAX			 = 20.0f;
		constexpr auto LIFE_START_FADING = 10.0f;
		constexpr auto OPACITY_MAX		 = 0.5f;

		auto footprint = GetNewEffect(Footprints, FOOTPRINT_COUNT_MAX);

		footprint.SpriteIndex = Objects[ID_MISC_SPRITES].meshIndex + 1 + (int)footprint.IsRightFoot;
		footprint.IsRightFoot = isRightFoot;
		footprint.VertexPoints = vertexPoints;
		footprint.Life = std::round(LIFE_MAX * FPS);
		footprint.LifeStartFading = std::round(LIFE_START_FADING * FPS);
		footprint.Opacity =
		footprint.OpacityStart = OPACITY_MAX;
	}

	void SpawnFootprint(const ItemInfo& item, bool isRightFoot)
	{
		if (!item.IsLara())
			return;

		int footJoint = isRightFoot ? LM_RFOOT : LM_LFOOT;

		// Don't process actual footprint placement if foot isn't on floor.
		auto footPos = Vector3::Zero;
		if (!TestFootHeight(item, footJoint, footPos))
			return;

		// Slightly randomize 2D position.
		footPos += Vector3(Random::GenerateFloat(-5.0f, 5.0f), 0.0f, Random::GenerateFloat(-5.0f, 5.0f));

		auto pointColl = GetCollision(footPos.x, footPos.y - CLICK(1), footPos.z, item.RoomNumber);

		// Don't process material if foot hit bridge object.
		if (pointColl.Position.Bridge >= 0)
			return;

		// Get and emit footstep sound for floor material.
		auto sfx = GetFootprintSfx(pointColl.BottomBlock->Material);
		if (sfx != SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS) // HACK: Must be here until reference WAD2 is revised.
		{
			auto poseCopy = item.Pose;
			SoundEffect(sfx, &poseCopy);
		}

		// Check floor material.
		if (!TestMaterial(pointColl.BottomBlock->Material, FootprintMaterials))
			return;

		auto vertexPoints = GetFootprintVertexPoints(item, footPos, Geometry::GetFloorNormal(pointColl.FloorTilt));

		// Check floor continuity.
		if (!TestFootprintFloor(item, footPos, vertexPoints))
			return;

		SpawnFootprint(isRightFoot, vertexPoints);
	}

	void UpdateFootprints()
	{
		if (Footprints.empty())
			return;

		for (auto& footprint: Footprints)
		{
			if (footprint.Life <= 0.0f)
				continue;

			// Update opacity.
			if (footprint.Life <= footprint.LifeStartFading)
			{
				float opacity = Lerp(footprint.OpacityStart, 0.0f, 1.0f - (footprint.Life / footprint.LifeStartFading));
				footprint.Opacity = opacity;
			}

			// Update life.
			footprint.Life -= 1.0f;
		}

		ClearInactiveEffects(Footprints);
	}

	void ClearFootprints()
	{
		Footprints.clear();
	}
}
