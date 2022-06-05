#include "framework.h"
#include "Game/effects/footprint.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"

namespace TEN {
namespace Effects {
namespace Footprints {

	std::deque<FOOTPRINT_STRUCT> footprints = std::deque<FOOTPRINT_STRUCT>();

	bool CheckFootOnFloor(ItemInfo const & item, int mesh, Vector3& outFootprintPosition) 
	{
		int x = item.Pose.Position.x;
		int y = item.Pose.Position.y;
		int z = item.Pose.Position.z;
		short roomNumber = item.RoomNumber;

		auto floor = GetFloor(x, y, z, &roomNumber);
		auto pos = Vector3Int(0, FOOT_HEIGHT_OFFSET, 0);

		GetLaraJointPosition(&pos, mesh);
		int height = GetFloorHeight(floor, pos.x, pos.y - STEP_SIZE, pos.z);

		outFootprintPosition.x = pos.x;
		outFootprintPosition.y = height - 8;
		outFootprintPosition.z = pos.z;

		return abs(pos.y - height) < 64;
	}

	void AddFootprint(ItemInfo* item, bool rightFoot)
	{
		if (item != LaraItem)
			return;

		auto foot = rightFoot ? LM_RFOOT : LM_LFOOT;

		// Don't process actual footprint placement if foot isn't on floor
		auto footPos = Vector3();
		if (!CheckFootOnFloor(*item, foot, footPos))
			return;

		// Randomize foot position slightly to avoid patterns
		footPos.x += (GetRandomControl() & 10) - 5;
		footPos.z += (GetRandomControl() & 10) - 5;

		auto result = GetCollision(footPos.x, footPos.y - STEP_SIZE, footPos.z, item->RoomNumber);
		auto floor = result.BottomBlock;

		// Don't process material if foot has hit bridge object
		if (result.Position.Bridge >= 0)
			return;

		auto fx = SOUND_EFFECTS::SFX_TR4_LARA_FEET;
		// Choose material for footstep sound
		switch (floor->Material)
		{
		case FLOOR_MATERIAL::Concrete:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FEET;
			break;

		case FLOOR_MATERIAL::Grass:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRASS;
			break;

		case FLOOR_MATERIAL::Gravel:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_GRAVEL;
			break;

		case FLOOR_MATERIAL::Ice:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_ICE;
			break;

		case FLOOR_MATERIAL::Marble:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MARBLE;
			break;

		case FLOOR_MATERIAL::Metal:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;
			break;

		case FLOOR_MATERIAL::Mud:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_MUD;
			break;

		case FLOOR_MATERIAL::OldMetal:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_METAL;
			break;

		case FLOOR_MATERIAL::OldWood:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;
			break;

		case FLOOR_MATERIAL::Sand:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SAND;
			break;

		case FLOOR_MATERIAL::Snow:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_SNOW;
			break;

		case FLOOR_MATERIAL::Stone:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FEET;
			break;

		case FLOOR_MATERIAL::Water:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_WET_FEET;
			break;

		case FLOOR_MATERIAL::Wood:
			fx = SOUND_EFFECTS::SFX_TR4_LARA_FOOTSTEPS_WOOD;
			break;

		case FLOOR_MATERIAL::Custom_Sound_1:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_1;
		break;

		case FLOOR_MATERIAL::Custom_Sound_2:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_2;
		break; 
		
		case FLOOR_MATERIAL::Custom_Sound_3:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_3;
		break;
		
		case FLOOR_MATERIAL::Custom_Sound_4:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_4;
		break; 
		
		case FLOOR_MATERIAL::Custom_Sound_5:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_5;
			break;

		case FLOOR_MATERIAL::Custom_Sound_6:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_6;
			break;

		case FLOOR_MATERIAL::Custom_Sound_7:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_7;
			break;

		case FLOOR_MATERIAL::Custom_Sound_8:
			fx = SOUND_EFFECTS::SFX_CUSTOM_FOOTSTEP_SOUNDS_8;
			break;
		}

		// HACK: must be here until reference wad2 is revised
		if (fx != SOUND_EFFECTS::SFX_TR4_LARA_FEET)
			SoundEffect(fx, &item->Pose);

		if (floor->Material != FLOOR_MATERIAL::Sand &&
			floor->Material != FLOOR_MATERIAL::Snow &&
			floor->Material != FLOOR_MATERIAL::Gravel &&
			floor->Material != FLOOR_MATERIAL::Mud)
			return;

		// Calculate footprint tilts
		auto plane = floor->FloorCollision.Planes[floor->SectorPlane(footPos.x, footPos.z)];
		auto c = phd_cos(item->Pose.Orientation.y + ANGLE(180));
		auto s = phd_sin(item->Pose.Orientation.y + ANGLE(180));
		auto yRot = TO_RAD(item->Pose.Orientation.y);
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
		auto c0 = GetCollision(p0.x, footPos.y - STEP_SIZE, p0.z, item->RoomNumber);
		auto c1 = GetCollision(p1.x, footPos.y - STEP_SIZE, p1.z, item->RoomNumber);
		auto c2 = GetCollision(p2.x, footPos.y - STEP_SIZE, p2.z, item->RoomNumber);
		auto c3 = GetCollision(p3.x, footPos.y - STEP_SIZE, p3.z, item->RoomNumber);

		// Don't process footprint placement if all foot corners aren't on the same tilt level
		if ((c0.FloorTilt.x != c1.FloorTilt.x) || (c1.FloorTilt.x != c2.FloorTilt.x) || (c2.FloorTilt.x != c3.FloorTilt.x))
			return;
		if ((c0.FloorTilt.y != c1.FloorTilt.y) || (c1.FloorTilt.y != c2.FloorTilt.y) || (c2.FloorTilt.y != c3.FloorTilt.y))
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
		footprint.StartOpacity = 0.25f;
		footprint.LifeStartFading = FPS * 10;
		footprint.Life = FPS * 20;
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