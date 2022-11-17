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
	constexpr auto FOOTPRINTS_NUM_MAX = 32;
	constexpr auto FOOTPRINT_SIZE	  = 64.0f;
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
		constexpr auto p0 = Vector3(FOOTPRINT_SIZE, 0, FOOTPRINT_SIZE);
		constexpr auto p1 = Vector3(-FOOTPRINT_SIZE, 0, FOOTPRINT_SIZE);
		constexpr auto p2 = Vector3(-FOOTPRINT_SIZE, 0, -FOOTPRINT_SIZE);
		constexpr auto p3 = Vector3(FOOTPRINT_SIZE, 0, -FOOTPRINT_SIZE);

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
			pos + Vector3::Transform(p0, rotMatrix),
			pos + Vector3::Transform(p1, rotMatrix),
			pos + Vector3::Transform(p2, rotMatrix),
			pos + Vector3::Transform(p3, rotMatrix)
		};
	}

	bool TestFootprintMaterial(FLOOR_MATERIAL refMaterial, const std::vector<FLOOR_MATERIAL>& materialList)
	{
		for (const auto& material : materialList)
		{
			if (material == refMaterial)
				return true;
		}

		return false;
	}

	void AddFootprint(ItemInfo* item, bool isRightFoot)
	{
		if (!item->IsLara())
			return;

		auto footJoint = isRightFoot ? LM_RFOOT : LM_LFOOT;

		// Don't process actual footprint placement if foot isn't on floor.
		auto footPos = Vector3::Zero;
		if (!TestFootOnFloor(*item, footJoint, footPos))
			return;

		// Slightly randomize foot position to avoid patterns.
		footPos.x += Random::GenerateInt(-5, 5);
		footPos.z += Random::GenerateInt(-5, 5);

		auto pointColl = GetCollision(footPos.x, footPos.y - CLICK(1), footPos.z, item->RoomNumber);
		auto* floor = pointColl.BottomBlock;

		// Don't process material if foot hit bridge object.
		if (pointColl.Position.Bridge >= 0)
			return;

		// Get footstep sound for floor material.
		auto fx = GetFootprintSoundEffect(floor->Material);

		// HACK: Must be here until reference WAD2 is revised.
		if (fx != SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS)
			SoundEffect(fx, &item->Pose);

		//if (!TestFootprintMaterial(floor->Material, FootprintMaterials))
		//	return;

		auto vertexPoints = GetFootprintVertexPoints(*item, footPos, Geometry::GetFloorNormal(pointColl.FloorTilt));

		// Get point collision for every vertex point.
		auto c0 = GetCollision(vertexPoints[0].x, footPos.y - CLICK(1), vertexPoints[0].z, item->RoomNumber);
		auto c1 = GetCollision(vertexPoints[1].x, footPos.y - CLICK(1), vertexPoints[1].z, item->RoomNumber);
		auto c2 = GetCollision(vertexPoints[2].x, footPos.y - CLICK(1), vertexPoints[2].z, item->RoomNumber);
		auto c3 = GetCollision(vertexPoints[3].x, footPos.y - CLICK(1), vertexPoints[3].z, item->RoomNumber);

		// Don't process footprint placement if all vertex points are outside relative height range.
		if ((abs(c0.Position.Floor - c1.Position.Floor) > CLICK(1.0f / 2)) ||
			(abs(c1.Position.Floor - c2.Position.Floor) > CLICK(1.0f / 2)) ||
			(abs(c2.Position.Floor - c3.Position.Floor) > CLICK(1.0f / 2)) ||
			(abs(c3.Position.Floor - c0.Position.Floor) > CLICK(1.0f / 2)))
		{
			return;
		}

		// Construct footprint.
		auto footprint = Footprint();

		footprint.IsActive = true;
		footprint.IsRightFoot = isRightFoot;
		footprint.VertexPoints = vertexPoints;
		footprint.Life = 20.0f * FPS;
		footprint.LifeStartFading = 10.0f * FPS;
		footprint.Opacity = 0.0f;
		footprint.StartOpacity = 0.5f;

		// Add footprint.
		if (Footprints.size() >= FOOTPRINTS_NUM_MAX)
			Footprints.pop_back();
		Footprints.push_front(footprint);
	}

	bool TestFootOnFloor(ItemInfo& item, int mesh, Vector3& outFootprintPos)
	{
		static const auto footOffset = Vector3i(0, FOOT_HEIGHT_OFFSET, 0);

		auto footPos = GetJointPosition(LaraItem, mesh, footOffset);
		int height = GetCollision(footPos.x, footPos.y - CLICK(1), footPos.z, item.RoomNumber).Position.Floor;

		outFootprintPos = Vector3(footPos.x, height - 4, footPos.z);
		return (abs(footPos.y - height) < CLICK(1.0f / 4));
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
			if (footprint.Life > footprint.LifeStartFading) 
			{
				footprint.Opacity = footprint.StartOpacity;
			}
			else 
			{
				float opacity = Lerp(0.0f, footprint.StartOpacity, fmax(0, fmin(1, footprint.Life / (float)footprint.LifeStartFading)));
				footprint.Opacity = opacity;
			}
		}

		for (int i = 0; i < numInvalidFootprints; i++) 
			Footprints.pop_back();
	}
}
