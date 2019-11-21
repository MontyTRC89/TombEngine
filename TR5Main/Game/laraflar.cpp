#include "laraflar.h"
#include "..\Global\global.h"

void __cdecl DrawFlareMeshes()
{
	Lara.meshPtrs[13] = Meshes[Objects[ID_FLARE_ANIM].meshIndex + 26];
}

void FlareControl(short item_number)//4A418, 4A87C
{
	UNIMPLEMENTED();
}

void ready_flare()//4A3E4(<), 4A848(<) (F)
{
	Lara.gun_status = LG_NO_ARMS;
	Lara.left_arm.z_rot = 0;
	Lara.left_arm.yRot = 0;
	Lara.left_arm.xRot = 0;
	Lara.right_arm.z_rot = 0;
	Lara.right_arm.yRot = 0;
	Lara.right_arm.xRot = 0;
	Lara.right_arm.lock = 0;
	Lara.left_arm.lock = 0;
	Lara.target = NULL;
}

void undraw_flare_meshes()//4A3BC, 4A820 (F)
{
	Lara.mesh_ptrs[LM_LHAND] = meshes[objects[LARA].mesh_index + 0x68];
}

void draw_flare_meshes()//4A394(<), 4A7F8(<) (F)
{
	Lara.mesh_ptrs[LM_LHAND] = meshes[objects[FLARE_ANIM].mesh_index + 0x68];
}

void undraw_flare()//4A108, 4A56C
{
	UNIMPLEMENTED();
}

void draw_flare()//49F74, 4A3D8 (F)
{
	short ani;

	if (lara_item->currentAnimState == STATE_LARA_FLARE_PICKUP ||
		lara_item->currentAnimState == STATE_LARA_PICKUP)
	{
		DoFlareInHand(Lara.flare_age);
		Lara.flare_control_left = false;
		ani = 93;
	}
	else
	{
		ani = Lara.left_arm.frameNumber + 1;
		Lara.flare_control_left = TRUE;

		if (ani < 33 || ani > 94)
		{
			ani = 33;
		}
		else if (ani == 46)
		{
			draw_flare_meshes();
		}
		else if (ani >= 72 && ani <= 93)
		{
			if (ani == 72)
			{
				SoundEffect(SFX_RAVESTICK, &lara_item->pos, room[lara_item->room_number].flags & RF_FILL_WATER);
				Lara.flare_age = 1;
			}

			DoFlareInHand(Lara.flare_age);
		}
		else
		{
			if (ani == 94)
			{
				ready_flare();
				ani = 0;
				DoFlareInHand(Lara.flare_age);
			}
		}
	}

	Lara.left_arm.frameNumber = ani;
	set_flare_arm(ani);
}

void set_flare_arm(int frame)//49ED4, 4A338 (F)
{
	int anim_base = objects[FLARE_ANIM].anim_index;

	if (frame >= 95)
	{
		anim_base += 4;
	}
	else if (frame >= 72)
	{
		anim_base += 3;
	}
	else if (frame >= 33)
	{
		anim_base += 2;
	}
	else if (frame >= 1)
	{
		anim_base += 1;
	}

	Lara.left_arm.animNumber = anim_base;
	Lara.left_arm.frameBase = Anims[anim_base].frame_ptr;
}

void CreateFlare(short object, int thrown)//49BBC, 4A020
{
	UNIMPLEMENTED();
}

void DrawFlareInAir(struct ITEM_INFO* item)//49A8C, 49EF0
{
	UNIMPLEMENTED();
}

void DoFlareInHand(int flare_age)//49984, 49DE8
{
	struct PHD_VECTOR pos;

	pos.x = 11;
	pos.y = 32;
	pos.z = 41;

	GetLaraJointPos(&pos, LJ_LHAND);
	DoFlareLight(&pos, flare_age);

	if (gfLevelFlags & GF_LVOP_MIRROR_USED && lara_item->room_number == gfMirrorRoom)
	{
		pos.z = 2 * gfMirrorZPlane - pos.z;

		DoFlareLight(&pos, flare_age);
	}

	if (Lara.flare_age >= 900)
	{
		if (Lara.gun_status == LG_NO_ARMS)
		{
			Lara.gun_status = LG_UNDRAW_GUNS;
		}
	}
	else if (Lara.flare_age != 0)
	{
		Lara.flare_age++;
	}
}

int DoFlareLight(struct PHD_VECTOR* pos, int flare_age)//49708, 49B6C (F)
{
	int x, y, z;
	int r, g, b;
	int random;
	int falloff;

	if (flare_age >= 900 || flare_age == 0)
		return 0;

	random = GetRandomControl();

	x = pos->x + (random << 3 & 120);
	y = pos->y + (random >> 1 & 120) - 256;
	z = pos->z + (random >> 5 & 120);

	if (flare_age < 4)
	{
		falloff = (random & 3) + 4 * (flare_age + 1);

		r = (random >> 4 & 0x1F) + 8 * (flare_age + 4);
		g = (random & 0x1F) + 16 * (flare_age + 10);
		b = (random >> 8 & 0x1F) + 16 * flare_age;

		TriggerDynamic(x, y, z, falloff, r, g, b);
		return 1;
	}
	else if (flare_age < 16)
	{
		falloff = (random & 1) + flare_age + 2;

		r = (random >> 4 & 0x1F) + 4 * flare_age + 64;
		g = (random & 0x3F) + 4 * flare_age + 128;
		b = (random >> 8 & 0x1F) + 4 * flare_age + 16;

		TriggerDynamic(x, y, z, falloff, r, g, b);
		return 1;
	}
	else if (flare_age < 810 || random > 8192)
	{
		falloff = 16;

		r = (random >> 4 & 0x1F) + 128;
		g = (random & 0x3F) + 192;
		b = (random >> 8 & 0x20) + (random >> 8 & 0x3F);

		TriggerDynamic(x, y, z, falloff, r, g, b);
		return 1;
	}
	else if (flare_age >= 876)
	{
		falloff = 16 - ((flare_age - 876) >> 1);

		r = (GetRandomControl() & 0x3F) + 64;
		g = (GetRandomControl() & 0x3F) + 192;
		b = GetRandomControl() & 0x1F;

		TriggerDynamic(x, y, z, falloff, r, g, b);
		return random & 1;
	}
	else
	{
		falloff = (GetRandomControl() & 6) + 8;

		r = (GetRandomControl() & 0x3F) + 64;
		g = (GetRandomControl() & 0x3F) + 192;
		b = GetRandomControl() & 0x7F;

		TriggerDynamic(x, y, z, falloff, r, g, b);
		return 0;
	}
}

void Inject_LaraFlar()
{
	INJECT(0x004553B0, DrawFlareMeshes);
}
