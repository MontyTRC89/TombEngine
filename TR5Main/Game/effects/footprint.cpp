#include "framework.h"
#include "control/control.h"
#include "lara.h"
#include "animation.h"
#include "collide.h"
#include "sound.h"
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

		auto floor = GetFloor(x, y, z, &roomNumber);
		auto pos = PHD_VECTOR(0, FOOT_HEIGHT_OFFSET, 0);

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

		// Don't process actual footprint placement if foot isn't on floor
		auto footPos = Vector3();
		if (!CheckFootOnFloor(*item, foot, footPos))
			return;

		auto result = GetCollisionResult(footPos.x, footPos.y - STEP_SIZE, footPos.z, item->roomNumber);
		auto floor = result.BottomBlock;

		// Don't process material if foot has hit bridge object
		if (result.Position.Bridge >= 0)
			return;

		auto fx = SOUND_EFFECTS::SFX_TR4_LARA_FEET;
		// Choose material for footstep sound
		switch (floor->Material)
		{
		case GroundMaterial::Concrete:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FEET;
			break;

		case GroundMaterial::Grass:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_SAND__AND__GRASS;
			break;

		case GroundMaterial::Gravel:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_GRAVEL;
			break;

		case GroundMaterial::Ice:
			fx = SOUND_EFFECTS::SFX_TR3_FOOTSTEPS_ICE;
			break;

		case GroundMaterial::Marble:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_MARBLE;
			break;

		case GroundMaterial::Metal:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_METAL;
			break;

		case GroundMaterial::Mud:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_MUD;
			break;

		case GroundMaterial::OldMetal:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_METAL;
			break;

		case GroundMaterial::OldWood:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_WOOD;
			break;

		case GroundMaterial::Sand:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_SAND__AND__GRASS;
			break;

		case GroundMaterial::Snow:
			fx = SOUND_EFFECTS::SFX_TR3_FOOTSTEPS_SNOW;
			break;

		case GroundMaterial::Stone:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FEET;
			break;

		case GroundMaterial::Water:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_WET_FEET;
			break;

		case GroundMaterial::Wood:
			fx = SOUND_EFFECTS::SFX_TR4_FOOTSTEPS_WOOD;
			break;
		}

		// HACK: must be here until reference wad2 is revised
		if (fx != SOUND_EFFECTS::SFX_TR4_LARA_FEET)
			SoundEffect(fx, &item->pos, 0);

		// Calculate footprint tilts
		auto plane = floor->FloorCollision.Planes[floor->SectorPlane(footPos.x, footPos.z)];
		auto c = phd_cos(item->pos.yRot + ANGLE(180));
		auto s = phd_sin(item->pos.yRot + ANGLE(180));
		auto yRot = TO_RAD(item->pos.yRot);
		auto xRot = plane.x * s + plane.y * c;
		auto zRot = plane.y * s - plane.x * c;

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
		auto c0 = GetCollisionResult(p0.x, footPos.y - STEP_SIZE, p0.z, item->roomNumber);
		auto c1 = GetCollisionResult(p1.x, footPos.y - STEP_SIZE, p1.z, item->roomNumber);
		auto c2 = GetCollisionResult(p2.x, footPos.y - STEP_SIZE, p2.z, item->roomNumber);
		auto c3 = GetCollisionResult(p3.x, footPos.y - STEP_SIZE, p3.z, item->roomNumber);

		// Don't process footprint placement if all foot corners aren't on the same tilt level
		if ((c0.TiltX != c1.TiltX) || (c1.TiltX != c2.TiltX) || (c2.TiltX != c3.TiltX))
			return;
		if ((c0.TiltZ != c1.TiltZ) || (c1.TiltZ != c2.TiltZ) || (c2.TiltZ != c3.TiltZ))
			return;

		// Don't process footprint placement if all foot corners aren't on the same height
		if ((abs(c0.Position.Floor - c1.Position.Floor) > STEP_SIZE / 2) ||
			(abs(c1.Position.Floor - c2.Position.Floor) > STEP_SIZE / 2) ||
			(abs(c2.Position.Floor - c3.Position.Floor) > STEP_SIZE / 2) ||
			(abs(c3.Position.Floor - c0.Position.Floor) > STEP_SIZE / 2))
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