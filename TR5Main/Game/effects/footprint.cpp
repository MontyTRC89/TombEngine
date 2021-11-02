#include "framework.h"
#include "control/control.h"
#include "lara.h"
#include "animation.h"
#include "collide.h"
#include "sound.h"
#include "effects/groundfx.h"
#include "effects/footprint.h"
#include "level.h"
#include "items.h"

namespace TEN {
namespace Effects {
namespace Footprints {

	std::deque<FOOTPRINT_STRUCT> footprints = std::deque<FOOTPRINT_STRUCT>();

	bool CheckFootOnFloor(ITEM_INFO const & item, int mesh, Vector3& outFootprintPosition) 
	{
		int x = item.pos.xPos;
		int y = item.pos.yPos;
		int z = item.pos.zPos;
		short roomNumber = item.roomNumber;

		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
		if (!(floor->Material == GroundMaterial::Sand   ||
			  floor->Material == GroundMaterial::Snow   ||
			  floor->Material == GroundMaterial::Gravel ||
			  floor->Material == GroundMaterial::Mud)) 
		{
			return false;
		}

		PHD_VECTOR pos;

		pos.x = pos.z = 0;
		pos.y = FOOT_HEIGHT_OFFSET;

		GetLaraJointPosition(&pos, mesh);
		int height = GetFloorHeight(floor, pos.x, pos.y - STEP_SIZE, pos.z);

		outFootprintPosition.x = pos.x;
		outFootprintPosition.y = height - 8;
		outFootprintPosition.z = pos.z;

		return abs(pos.y - height) < 64;
	}

	void AddFootprint(ITEM_INFO* item, bool rightFoot)
	{
		if (item != LaraItem)
			return;

		auto foot = rightFoot ? LM_RFOOT : LM_LFOOT;
		PHD_VECTOR position;
		GetLaraJointPosition(&position, foot);

		auto fx = sound_effects::SFX_TR4_LARA_FEET;
		auto result = GetCollisionResult(position.x, position.y - STEP_SIZE, position.z, item->roomNumber);
		auto floor = result.BottomBlock;

		if (result.Position.Bridge >= 0)
			return;

		switch (floor->Material)
		{
		case GroundMaterial::Concrete:
			fx = sound_effects::SFX_TR4_LARA_FEET;
			break;

		case GroundMaterial::Grass:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_SAND__AND__GRASS;
			break;

		case GroundMaterial::Gravel:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_GRAVEL;
			break;

		case GroundMaterial::Ice:
			fx = sound_effects::SFX_TR3_FOOTSTEPS_ICE;
			break;

		case GroundMaterial::Marble:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_MARBLE;
			break;

		case GroundMaterial::Metal:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_METAL;
			break;

		case GroundMaterial::Mud:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_MUD;
			break;

		case GroundMaterial::OldMetal:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_METAL;
			break;

		case GroundMaterial::OldWood:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_WOOD;
			break;

		case GroundMaterial::Sand:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_SAND__AND__GRASS;
			break;

		case GroundMaterial::Snow:
			fx = sound_effects::SFX_TR3_FOOTSTEPS_SNOW;
			break;

		case GroundMaterial::Stone:
			fx = sound_effects::SFX_TR4_LARA_FEET;
			break;

		case GroundMaterial::Water:
			fx = sound_effects::SFX_TR4_LARA_WET_FEET;
			break;

		case GroundMaterial::Wood:
			fx = sound_effects::SFX_TR4_FOOTSTEPS_WOOD;
			break;
		}

		// HACK: must be here until reference wad2 is revised
		if (fx != sound_effects::SFX_TR4_LARA_FEET)
			SoundEffect(fx, &item->pos, 0);

		auto plane = floor->FloorCollision.Planes[floor->SectorPlane(position.x, position.z)];

		auto c = phd_cos(item->pos.yRot + ANGLE(180));
		auto s = phd_sin(item->pos.yRot + ANGLE(180));
		auto yRot = TO_RAD(item->pos.yRot);
		auto xRot = plane.x * s + plane.y * c;
		auto zRot = plane.y * s - plane.x * c;

		auto footPos = Vector3();
		if (!CheckFootOnFloor(*item, foot, footPos))
			return;

		FOOTPRINT_STRUCT footprint = {};
		footprint.Position = footPos;
		footprint.Rotation = Vector3(xRot, yRot, zRot);
		footprint.LifeStartFading = 30 * 10;
		footprint.StartOpacity = 0.25f;
		footprint.Life = 30 * 20;
		footprint.Active = true;
		footprint.RightFoot = rightFoot;

		if (footprints.size() >= MAX_FOOTPRINTS)
			footprints.pop_back();
		footprints.push_front(footprint);
	}

	void UpdateFootprints()
	{
		if (footprints.size() == 0)
			return;

		int numInvalidFootprints = 0;

		for (auto i = footprints.begin(); i != footprints.end(); i++) 
		{
			FOOTPRINT_STRUCT& footprint = *i;
			footprint.Life--;

			if (footprint.Life <= 0) 
			{
				numInvalidFootprints++;
				continue;
			}

			if (footprint.Life > footprint.LifeStartFading) 
			{
				footprint.Opacity = footprint.StartOpacity;
			}
			else 
			{
				float opacity = lerp(0, footprint.StartOpacity, fmax(0, fmin(1, footprint.Life / (float)footprint.LifeStartFading)));
				footprint.Opacity = opacity;
			}
		}

		for (int i = 0; i < numInvalidFootprints; i++) 
		{
			footprints.pop_back();
		}
	}
}
}
}