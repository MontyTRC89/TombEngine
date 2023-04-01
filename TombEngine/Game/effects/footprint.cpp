#include "framework.h"
#include "Game/effects/Footprint.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Math;

namespace TEN::Effects::Footprint
{
	const auto FootprintMaterials = std::vector<MaterialType>
	{
		MaterialType::Mud,
		MaterialType::Snow,
		MaterialType::Sand,
		MaterialType::Gravel,
		MaterialType::Custom1,
		MaterialType::Custom2,
		MaterialType::Custom3,
		MaterialType::Custom4
	};

	struct FootprintPositionData
	{
		bool	CanSpawn = false;
		Vector3 Position = Vector3::Zero;
	};

	std::vector<Footprint> Footprints = {};

	static SOUND_EFFECTS GetFootprintSfx(MaterialType material)
	{
		switch (material)
		{
		default:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case MaterialType::Mud:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MUD;

		case MaterialType::Snow:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SNOW;

		case MaterialType::Sand:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SAND;

		case MaterialType::Gravel:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRAVEL;

		case MaterialType::Ice:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_ICE;

		case MaterialType::Water:
			return SOUND_EFFECTS::SFX_TR4_LARA_WET_FEET;

		case MaterialType::Stone:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case MaterialType::Wood:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;

		case MaterialType::Metal:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;

		case MaterialType::Marble:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MARBLE;

		case MaterialType::Grass:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRASS;

		case MaterialType::Concrete:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case MaterialType::OldWood:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;

		case MaterialType::OldMetal:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;

		case MaterialType::Custom1:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_1;

		case MaterialType::Custom2:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_2;

		case MaterialType::Custom3:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_3;

		case MaterialType::Custom4:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_4;

		case MaterialType::Custom5:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_5;

		case MaterialType::Custom6:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_6;

		case MaterialType::Custom7:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_7;

		case MaterialType::Custom8:
			return SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_8;
		}
	}

	static std::array<Vector3, Footprint::VERTEX_COUNT> GetFootprintVertexPoints(const ItemInfo& item, const Vector3& pos, const Vector3& normal)
	{
		constexpr auto SCALE   = BLOCK(1 / 16.0f);
		constexpr auto POINT_0 = Vector3( SCALE, 0.0f,  SCALE);
		constexpr auto POINT_1 = Vector3(-SCALE, 0.0f,  SCALE);
		constexpr auto POINT_2 = Vector3(-SCALE, 0.0f, -SCALE);
		constexpr auto POINT_3 = Vector3( SCALE, 0.0f, -SCALE);

		// Determine rotation matrix.
		auto rotMatrix = Geometry::GetRelOrientToNormal(item.Pose.Orientation.y, normal).ToRotationMatrix();

		return std::array<Vector3, Footprint::VERTEX_COUNT>
		{
			pos + Vector3::Transform(POINT_0, rotMatrix),
			pos + Vector3::Transform(POINT_1, rotMatrix),
			pos + Vector3::Transform(POINT_2, rotMatrix),
			pos + Vector3::Transform(POINT_3, rotMatrix)
		};
	}

	static FootprintPositionData GetFootprintPositionData(const ItemInfo& item, int jointIndex)
	{
		constexpr auto SURFACE_OFFSET  = 4;
		constexpr auto ABS_FLOOR_BOUND = CLICK(0.25f);
		constexpr auto HEIGHT_OFFSET   = CLICK(0.25f);
		constexpr auto FOOT_OFFSET	   = Vector3i(0, HEIGHT_OFFSET, 0);

		auto footPos = GetJointPosition(item, jointIndex, FOOT_OFFSET);
		int floorHeight = GetCollision(footPos.x, footPos.y - CLICK(1), footPos.z, item.RoomNumber).Position.Floor;

		bool canSpawn = (abs(footPos.y - floorHeight) < ABS_FLOOR_BOUND);
		auto pos = Vector3(footPos.x, floorHeight - SURFACE_OFFSET, footPos.z);
		return FootprintPositionData{ canSpawn, pos };
	}

	static bool TestFootprintFloor(const ItemInfo& item, const Vector3& pos, const std::array<Vector3, Footprint::VERTEX_COUNT>& vertexPoints)
	{
		constexpr auto ABS_FLOOR_BOUND = CLICK(0.5f);

		// Get point collision at every vertex point.
		auto pointColl0 = GetCollision(vertexPoints[0].x, pos.y - CLICK(1), vertexPoints[0].z, item.RoomNumber);
		auto pointColl1 = GetCollision(vertexPoints[1].x, pos.y - CLICK(1), vertexPoints[1].z, item.RoomNumber);
		auto pointColl2 = GetCollision(vertexPoints[2].x, pos.y - CLICK(1), vertexPoints[2].z, item.RoomNumber);
		auto pointColl3 = GetCollision(vertexPoints[3].x, pos.y - CLICK(1), vertexPoints[3].z, item.RoomNumber);

		// Don't spawn footprint if floor heights at vertex points are outside lower/upper floor height bound.
		if ((abs(pointColl0.Position.Floor - pointColl1.Position.Floor) > ABS_FLOOR_BOUND) ||
			(abs(pointColl1.Position.Floor - pointColl2.Position.Floor) > ABS_FLOOR_BOUND) ||
			(abs(pointColl2.Position.Floor - pointColl3.Position.Floor) > ABS_FLOOR_BOUND) ||
			(abs(pointColl3.Position.Floor - pointColl0.Position.Floor) > ABS_FLOOR_BOUND))
		{
			return false;
		}

		return true;
	}

	void SpawnFootprint(bool isRight, const std::array<Vector3, Footprint::VERTEX_COUNT>& vertexPoints)
	{
		constexpr auto LIFE_MAX			 = 20.0f;
		constexpr auto LIFE_START_FADING = 10.0f;
		constexpr auto OPACITY_MAX		 = 0.5f;

		auto& footprint = GetNewEffect(Footprints, Footprint::COUNT_MAX);

		footprint.SpriteIndex = Objects[ID_MISC_SPRITES].meshIndex + (1 + (int)isRight);
		footprint.IsRight = isRight;
		footprint.VertexPoints = vertexPoints;
		footprint.Life = std::round(LIFE_MAX * FPS);
		footprint.LifeStartFading = std::round(LIFE_START_FADING * FPS);
		footprint.Opacity =
		footprint.OpacityStart = OPACITY_MAX;
	}

	void SpawnFootprint(const ItemInfo& item, bool isRight)
	{
		if (!item.IsLara())
			return;

		// Don't spawn footprint if foot isn't on floor.
		int jointIndex = isRight ? LM_RFOOT : LM_LFOOT;
		auto posData = GetFootprintPositionData(item, jointIndex);
		if (!posData.CanSpawn)
			return;

		// Slightly randomize 2D position.
		posData.Position += Vector3(Random::GenerateFloat(-5.0f, 5.0f), 0.0f, Random::GenerateFloat(-5.0f, 5.0f));

		auto pointColl = GetCollision(posData.Position.x, posData.Position.y - CLICK(1), posData.Position.z, item.RoomNumber);

		// Don't process material if foot hit bridge object.
		// TODO: Handle bridges once bridge collision is less stupid.
		if (pointColl.Position.Bridge >= 0)
			return;

		// Get and emit footstep sound for floor material.
		auto sfx = GetFootprintSfx(pointColl.BottomBlock->Material);
		if (sfx != SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS) // HACK: Must be here until reference WAD2 is revised.
		{
			auto pose = item.Pose;
			SoundEffect(sfx, &pose);
		}

		// Check floor material.
		if (!TestMaterial(pointColl.BottomBlock->Material, FootprintMaterials))
			return;

		auto vertexPoints = GetFootprintVertexPoints(item, posData.Position, GetSurfaceNormal(pointColl.FloorTilt, true));

		// Test floor continuity.
		if (!TestFootprintFloor(item, posData.Position, vertexPoints))
			return;

		SpawnFootprint(isRight, vertexPoints);
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
				float alpha = 1.0f - (footprint.Life / footprint.LifeStartFading);
				footprint.Opacity = Lerp(footprint.OpacityStart, 0.0f, alpha);
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
