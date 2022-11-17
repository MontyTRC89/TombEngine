#include "framework.h"
#include "Game/effects/footprint.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Effects::Footprints
{
	constexpr auto FOOTPRINT_LIFE_MAX		   = 20.0f * FPS;
	constexpr auto FOOTPRINT_LIFE_START_FADING = FOOTPRINT_LIFE_MAX - (10.0f * FPS);
	constexpr auto FOOTPRINT_OPACITY_MAX	   = 0.5f;

	constexpr auto FOOTPRINT_SCALE	  = 64.0f;
	constexpr auto FOOT_HEIGHT_OFFSET = CLICK(1.0f / 4);

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

	std::deque<Footprint> Footprints = std::deque<Footprint>();

	void AddFootprint(ItemInfo* item, bool isRightFoot)
	{
		if (!item->IsLara())
			return;

		auto footJoint = isRightFoot ? LM_RFOOT : LM_LFOOT;

		// Don't process actual footprint placement if foot isn't on floor.
		auto footPos = Vector3::Zero;
		if (!TestFootHeight(*item, footJoint, footPos))
			return;

		// Slightly randomize foot position to avoid patterns.
		footPos += Vector3(Random::GenerateFloat(-5.0f, 5.0f), 0.0f, Random::GenerateFloat(-5.0f, 5.0f));

		auto pointColl = GetCollision(footPos.x, footPos.y - CLICK(1), footPos.z, item->RoomNumber);
		auto* floor = pointColl.BottomBlock;

		// Don't process material if foot hit bridge object.
		if (pointColl.Position.Bridge >= 0)
			return;

		// Get footstep sound for floor material.
		auto soundEffectID = GetFootprintSoundEffect(floor->Material);

		// HACK: Must be here until reference WAD2 is revised.
		if (soundEffectID != SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS)
			SoundEffect(soundEffectID, &item->Pose);

		// Check floor material.
		if (!TestMaterial(floor->Material, FootprintMaterials))
			return;

		auto vertexPoints = GetFootprintVertexPoints(*item, footPos, Geometry::GetFloorNormal(pointColl.FloorTilt));

		// Check floor continuity.
		if (!TestFootprintFloor(*item, footPos, vertexPoints))
			return;

		SpawnFootprint(vertexPoints, isRightFoot);
	}

	SOUND_EFFECTS GetFootprintSoundEffect(FLOOR_MATERIAL material)
	{
		switch (material)
		{
		default:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case FLOOR_MATERIAL::Concrete:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case FLOOR_MATERIAL::Grass:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRASS;

		case FLOOR_MATERIAL::Gravel:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRAVEL;

		case FLOOR_MATERIAL::Ice:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_ICE;

		case FLOOR_MATERIAL::Marble:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MARBLE;

		case FLOOR_MATERIAL::Metal:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;

		case FLOOR_MATERIAL::Mud:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MUD;

		case FLOOR_MATERIAL::OldMetal:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;

		case FLOOR_MATERIAL::OldWood:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;

		case FLOOR_MATERIAL::Sand:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SAND;

		case FLOOR_MATERIAL::Snow:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SNOW;

		case FLOOR_MATERIAL::Stone:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS;

		case FLOOR_MATERIAL::Water:
			return SOUND_EFFECTS::SFX_TR4_LARA_WET_FEET;

		case FLOOR_MATERIAL::Wood:
			return SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;

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

	std::array<Vector3, 4> GetFootprintVertexPoints(const ItemInfo& item, const Vector3& pos, const Vector3& normal)
	{
		constexpr auto point0 = Vector3(FOOTPRINT_SCALE, 0, FOOTPRINT_SCALE);
		constexpr auto point1 = Vector3(-FOOTPRINT_SCALE, 0, FOOTPRINT_SCALE);
		constexpr auto point2 = Vector3(-FOOTPRINT_SCALE, 0, -FOOTPRINT_SCALE);
		constexpr auto point3 = Vector3(FOOTPRINT_SCALE, 0, -FOOTPRINT_SCALE);

		// Determine surface angles.
		short aspectAngle = Geometry::GetSurfaceAspectAngle(normal);
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(normal);

		short deltaAngle = Geometry::GetShortestAngle(item.Pose.Orientation.y, aspectAngle);
		float sinDeltaAngle = phd_sin(deltaAngle);
		float cosDeltaAngle = phd_cos(deltaAngle);

		// Calculate rotation matrix.
		auto orient = EulerAngles(
			-slopeAngle * cosDeltaAngle,
			0,
			slopeAngle * sinDeltaAngle
		) + EulerAngles(0, item.Pose.Orientation.y, 0);
		auto rotMatrix = orient.ToRotationMatrix();

		return std::array<Vector3, 4>
		{
			pos + Vector3::Transform(point0, rotMatrix),
			pos + Vector3::Transform(point1, rotMatrix),
			pos + Vector3::Transform(point2, rotMatrix),
			pos + Vector3::Transform(point3, rotMatrix)
		};
	}

	bool TestMaterial(FLOOR_MATERIAL refMaterial, const std::vector<FLOOR_MATERIAL>& materialList)
	{
		for (const auto& material : materialList)
		{
			if (material == refMaterial)
				return true;
		}

		return false;
	}

	bool TestFootHeight(ItemInfo& item, int mesh, Vector3& outFootprintPos)
	{
		static constexpr auto heightRange = CLICK(1.0f / 4);
		static const auto footOffset = Vector3i(0, FOOT_HEIGHT_OFFSET, 0);

		auto footPos = GetJointPosition(LaraItem, mesh, footOffset);
		int floorHeight = GetCollision(footPos.x, footPos.y - CLICK(1), footPos.z, item.RoomNumber).Position.Floor;

		outFootprintPos = Vector3(footPos.x, floorHeight - 4, footPos.z);
		return (abs(footPos.y - floorHeight) < heightRange);
	}

	bool TestFootprintFloor(const ItemInfo& item, const Vector3& pos, const std::array<Vector3, 4>& vertexPoints)
	{
		static constexpr auto heightRange = CLICK(1.0f / 2);

		// Get point collision for every vertex point.
		auto pointColl0 = GetCollision(vertexPoints[0].x, pos.y - CLICK(1), vertexPoints[0].z, item.RoomNumber);
		auto pointColl1 = GetCollision(vertexPoints[1].x, pos.y - CLICK(1), vertexPoints[1].z, item.RoomNumber);
		auto pointColl2 = GetCollision(vertexPoints[2].x, pos.y - CLICK(1), vertexPoints[2].z, item.RoomNumber);
		auto pointColl3 = GetCollision(vertexPoints[3].x, pos.y - CLICK(1), vertexPoints[3].z, item.RoomNumber);

		// Don't spawn footprint if all vertex points are outside relative height range.
		if ((abs(pointColl0.Position.Floor - pointColl1.Position.Floor) > heightRange) ||
			(abs(pointColl1.Position.Floor - pointColl2.Position.Floor) > heightRange) ||
			(abs(pointColl2.Position.Floor - pointColl3.Position.Floor) > heightRange) ||
			(abs(pointColl3.Position.Floor - pointColl0.Position.Floor) > heightRange))
		{
			return false;
		}

		return true;
	}

	void SpawnFootprint(const std::array<Vector3, 4>& vertexPoints, bool isRightFoot)
	{
		auto footprint = Footprint();

		footprint.IsActive = true;
		footprint.IsRightFoot = isRightFoot;
		footprint.VertexPoints = vertexPoints;
		footprint.Life = FOOTPRINT_LIFE_MAX;
		footprint.LifeStartFading = FOOTPRINT_LIFE_START_FADING;
		footprint.Opacity = FOOTPRINT_OPACITY_MAX;
		footprint.OpacityStart = footprint.Opacity;

		if (Footprints.size() >= FOOTPRINTS_NUM_MAX)
			Footprints.pop_back();

		Footprints.push_front(footprint);
	}

	void UpdateFootprints()
	{
		if (Footprints.empty())
			return;

		unsigned int numInvalidFootprints = 0;

		for (auto& footprint: Footprints)
		{
			footprint.Life -= 1.0f;

			// Despawn footprint.
			if (footprint.Life <= 0.0f)
			{
				numInvalidFootprints++;
				continue;
			}

			// Update opacity.
			if (footprint.Life <= footprint.LifeStartFading)
			{
				float opacity = Lerp(0.0f, footprint.OpacityStart, fmax(0, fmin(1, footprint.Life / (float)footprint.LifeStartFading)));
				footprint.Opacity = opacity;
			}
		}

		for (int i = 0; i < numInvalidFootprints; i++)
			Footprints.pop_back();
	}
}
