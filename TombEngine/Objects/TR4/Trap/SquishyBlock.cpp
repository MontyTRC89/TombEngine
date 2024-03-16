#include "framework.h"
#include "Objects/TR4/Trap/SquishyBlock.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"

namespace TEN::Entities::Traps
{
	
	void ControlLRSquishyBlock(short item_number)
	{
		ITEM_INFO* item;
		ushort ang;
		short frame;

		item = &items[item_number];

		if (!TriggerActive(item))
			return;

		frame = item->frame_number - anims[item->anim_number].frame_base;

		if (item->touch_bits)
		{
			ang = (ushort)phd_atan(item->pos.z_pos - lara_item->pos.z_pos, item->pos.x_pos - lara_item->pos.x_pos) - item->pos.y_rot;

			if (!frame && ang > 0xA000 && ang < 0xE000)
			{
				item->item_flags[0] = 9;
				lara_item->hit_points = 0;
				lara_item->pos.y_rot = item->pos.y_rot - 0x4000;
			}
			else if (frame == 33 && ang > 0x2000 && ang < 0x6000)
			{
				item->item_flags[0] = 42;
				lara_item->hit_points = 0;
				lara_item->pos.y_rot = item->pos.y_rot + 0x4000;
			}
		}

		if (!item->item_flags[0] || frame != item->item_flags[0])
			AnimateItem(item);
	}




	void FallingSquishyBlockCollision(short item_number, ITEM_INFO* l, COLL_INFO* coll)
	{
		ItemInfo* item;

		item = &items[item_number];

		if (TestBoundsCollide(item, l, coll->radius) && TestCollision(item, l))
		{
			if (item->frame_number - anims[item->anim_number].frame_base <= 8)
			{
				item->frame_number += 2;
				l->hit_points = 0;
				l->current_anim_state = AS_DEATH;
				l->goal_anim_state = AS_DEATH;
				l->anim_number = ANIM_FBLOCK_DEATH;
				l->frame_number = anims[ANIM_FBLOCK_DEATH].frame_base + 50;
				l->fallspeed = 0;
				l->speed = 0;

				for (int i = 0; i < 12; i++)
					TriggerBlood(l->pos.x_pos, l->pos.y_pos - 128, l->pos.z_pos, GetRandomControl() << 1, 3);
			}
			else if (l->hit_points > 0)
				ItemPushLara(item, l, coll, 0, 1);
		}
	}

	void ControlFallingSquishyBlock(short item_number)
	{
		ITEM_INFO* item;

		item = &items[item_number];

		if (TriggerActive(item))
		{
			if (item->item_flags[0] < 60)
			{
				SoundEffect(SFX_EARTHQUAKE_LOOP, &item->pos, SFX_DEFAULT);
				camera.bounce = (item->item_flags[0] - 92) >> 1;
				item->item_flags[0]++;
			}
			else
			{
				if (item->frame_number - anims[item->anim_number].frame_base == 8)
					camera.bounce = -96;

				AnimateItem(item);
			}
		}
	}

}
