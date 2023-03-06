#include "framework.h"
#include "Objects/TR3/Object/corpse.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define TRAIN_VEL	260
#define LARA_TRAIN_DEATH_ANIM 3;

namespace TEN::Entities
{

	void InitialiseCorpse(short itemNumber)
	{

	}

	void CorpseControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TriggerActive(item))
			return;

		if (item->TriggerFlags == 1)
		{
			old_room = item->room_number;

			item->pos.y_pos += item->fallspeed;

			room_number = item->room_number;
			floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
			h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

			if (item->room_number != room_number)
				ItemNewRoom(item_number, room_number);

			if (item->pos.y_pos >= h)
			{
				item->pos.y_pos = h;
				item->fallspeed = 0;
				item->pos.z_rot = 0x4000;
				return;
			}

			item->pos.z_rot += item->fallspeed;
			item->fallspeed += (room[room_number].flags & UNDERWATER) ? 1 : 8;
			maxfs = (room[old_room].flags & UNDERWATER) ? 64 : 512;
			if (item->fallspeed > maxfs)
				item->fallspeed = maxfs;

			if ((room[room_number].flags & UNDERWATER) && !(room[old_room].flags & UNDERWATER))
			{
				splash_setup.x = item->pos.x_pos;
				splash_setup.y = room[room_number].y;
				splash_setup.z = item->pos.z_pos;
				splash_setup.InnerXZoff = 16;
				splash_setup.InnerXZsize = 16;
				splash_setup.InnerYsize = -96;
				splash_setup.InnerXZvel = 0xa0;
				splash_setup.InnerYvel = -item->fallspeed * 72;
				splash_setup.InnerGravity = 0x80;
				splash_setup.InnerFriction = 7;
				splash_setup.MiddleXZoff = 24;
				splash_setup.MiddleXZsize = 32;
				splash_setup.MiddleYsize = -64;
				splash_setup.MiddleXZvel = 0xe0;
				splash_setup.MiddleYvel = -item->fallspeed * 36;
				splash_setup.MiddleGravity = 0x48;
				splash_setup.MiddleFriction = 8;
				splash_setup.OuterXZoff = 32;
				splash_setup.OuterXZsize = 32;
				splash_setup.OuterXZvel = 0x110;
				splash_setup.OuterFriction = 9;
				SetupSplash(&splash_setup);
				item->fallspeed = 16;
				item->pos.y_pos = room[room_number].y + 1;
			}

		}


}