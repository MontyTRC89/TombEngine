#include "framework.h"
#include "Objects/TR3/Trap/WallMountedBlade.h"

#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"

constexpr auto WALL_MOUNTED_BLADE_HARM_DAMAGE = 200;
constexpr auto WALL_MOUNTED_BLADE_JOINT = MESH_BITS(1);

enum WallMountedBladeState
{
	WALL_MOUNTED_BLADE_STATE_NONE = 0,
	WALL_MOUNTED_BLADE_STATE_DISABLED = 1,
	WALL_MOUNTED_BLADE_STATE_ENABLED = 2
};

enum WallMountedBladeAnim
{
	WALL_MOUNTED_BLADE_ANIM_IDLE_TO_ATTACK = 0,
	WALL_MOUNTED_BLADE_ANIM_IDLE = 1,
	WALL_MOUNTED_BLADE_ANIM_ATTACK = 2,
	WALL_MOUNTED_BLADE_ANIM_ATTACK_TO_IDLE = 3
};

void InitialiseWallMountedBlade(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	SetAnimation(*item, WALL_MOUNTED_BLADE_ANIM_IDLE);

	// Used for GenericSphereBoxCollision
	item->ItemFlags[0] = WALL_MOUNTED_BLADE_JOINT;
	item->ItemFlags[3] = WALL_MOUNTED_BLADE_HARM_DAMAGE;
}

void WallMountedBladeControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->Animation.ActiveState == WALL_MOUNTED_BLADE_STATE_DISABLED)
	{
		item->Animation.TargetState = WALL_MOUNTED_BLADE_STATE_ENABLED;
		item->ItemFlags[3] = WALL_MOUNTED_BLADE_HARM_DAMAGE;
	}
	else
	{
		item->Animation.TargetState = WALL_MOUNTED_BLADE_STATE_DISABLED;
		item->ItemFlags[3] = 0; // NOTE: reset damage to avoid GenericSphereBoxCollision() damaging lara when disabled !
	}

	AnimateItem(item);
}