#include "framework.h"
#include "Objects/TR3/Trap/WallMountedBlade.h"

#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"

constexpr auto WALL_MOUNTED_BLADE_HARM_DAMAGE = 200;

enum SlammingDoorsState
{
	WALL_MOUNTED_BLADE_STATE_NONE = 0,
	WALL_MOUNTED_BLADE_STATE_DISABLED = 1,
	WALL_MOUNTED_BLADE_STATE_ENABLED = 2
};

enum SlammingDoorsAnim
{
	WALL_MOUNTED_BLADE_ANIM_RETRACT = 0,
	WALL_MOUNTED_BLADE_ANIM_ENABLED = 1,
	WALL_MOUNTED_BLADE_ANIM_DISABLED = 2,
	WALL_MOUNTED_BLADE_ANIM_TRIGGERED = 3
};

void InitialiseWallMountedBlade(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	SetAnimation(*item, WALL_MOUNTED_BLADE_ANIM_DISABLED);

}

void WallMountedBladeControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item) && item->Animation.ActiveState == WALL_MOUNTED_BLADE_STATE_ENABLED)
		item->Animation.TargetState = WALL_MOUNTED_BLADE_STATE_ENABLED;
	else
		item->Animation.TargetState = WALL_MOUNTED_BLADE_STATE_DISABLED;

	if (item->TouchBits.Test(1) && item->Animation.ActiveState == WALL_MOUNTED_BLADE_STATE_DISABLED)
	{
		DoDamage(LaraItem, WALL_MOUNTED_BLADE_HARM_DAMAGE);
	}

	AnimateItem(item);
}