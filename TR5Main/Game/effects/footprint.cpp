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

		// Don't process material if foot has hit bridge object
		if (result.Position.Bridge >= 0)
			return;

		// Choose material for footstep sound
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

		// Calculate footprint tilts
		auto plane = floor->FloorCollision.Planes[floor->SectorPlane(position.x, position.z)];
		auto c = phd_cos(item->pos.yRot + ANGLE(180));
		auto s = phd_sin(item->pos.yRot + ANGLE(180));
		auto yRot = TO_RAD(item->pos.yRot);
		auto xRot = plane.x * s + plane.y * c;
		auto zRot = plane.y * s - plane.x * c;

		// Don't process actual footprint placement if foot isn't on floor
		auto footPos = Vector3();
		if (!CheckFootOnFloor(*item, foot, footPos))
			return;

		// Calculate footprint positions
		auto p0 = Vector3( FOOTPRINT_SIZE, 0,  FOOTPRINT_SIZE);
		auto p1 = Vector3(-FOOTPRINT_SIZE, 0,  FOOTPRINT_SIZE);
		auto p2 = Vector3(-FOOTPRINT_SIZE, 0, -FOOTPRINT_SIZE);
		auto p3 = Vector3( FOOTPRINT_SIZE, 0, -FOOTPRINT_SIZE);
		auto rot = Matrix::CreateFromYawPitchRoll(yRot, xRot, zRot);
		p0 = XMVector3Transform(p0, rot);
		p1 = XMVector3Transform(p1, rot);
		p2 = XMVector3Transform(p2, rot);
		p3 = XMVector3Transform(p3, rot);
		p0 += Vector3(footPos.x, footPos.y, footPos.z);
		p1 += Vector3(footPos.x, footPos.y, footPos.z);
		p2 += Vector3(footPos.x, footPos.y, footPos.z);
		p3 += Vector3(footPos.x, footPos.y, footPos.z);

		// Get blocks for every footprint corner
		auto c1 = GetCollisionResult(p0.x, position.y - STEP_SIZE, p0.z, item->roomNumber);
		auto c2 = GetCollisionResult(p1.x, position.y - STEP_SIZE, p1.z, item->roomNumber);
		auto c3 = GetCollisionResult(p2.x, position.y - STEP_SIZE, p2.z, item->roomNumber);
		auto c4 = GetCollisionResult(p3.x, position.y - STEP_SIZE, p3.z, item->roomNumber);

		// Don't process actual footprint placement if all foot corners aren't on the same tilt level
		if ((c1.TiltX != c2.TiltX) || (c2.TiltX != c3.TiltX) || (c3.TiltX != c4.TiltX))
			return;
		if ((c1.TiltZ != c2.TiltZ) || (c2.TiltZ != c3.TiltZ) || (c3.TiltZ != c4.TiltZ))
			return;

		// Construct footprint
		FOOTPRINT_STRUCT footprint = {};
		footprint.Position[0] = p0;
		footprint.Position[1] = p1;
		footprint.Position[2] = p2;
		footprint.Position[3] = p3;
		footprint.LifeStartFading = 30 * 10;
		footprint.StartOpacity = 0.25f;
		footprint.Life = 30 * 20;
		footprint.Active = true;
		footprint.RightFoot = rightFoot;

		// Add footprint
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