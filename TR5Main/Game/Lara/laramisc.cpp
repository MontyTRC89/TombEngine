#include "framework.h"
#include "laramisc.h"
#include "level.h"
#include "setup.h"
#include "lara.h"
#include "GameFlowScript.h"
#include "effect.h"
#include "collide.h"
#include "lara.h"
#include "lara_swim.h"
#include "lara_surface.h"
#include "effect2.h"

#include "misc.h"
#include "rope.h"
#include "draw.h"
#include "savegame.h"
#include "inventory.h"
#include "camera.h"
#include "input.h"
#include "sound.h"

extern GameFlow* g_GameFlow;

COLL_INFO lara_coll;
short SubsuitAir = 0;
short cheatHitPoints;


void GetLaraDeadlyBounds() // (F) (D)
{
	BOUNDING_BOX* bounds;
	BOUNDING_BOX tbounds;

	bounds = GetBoundsAccurate(LaraItem);
	phd_RotBoundingBoxNoPersp(&LaraItem->pos, bounds, &tbounds);

	DeadlyBounds[0] = LaraItem->pos.xPos + tbounds.X1;
	DeadlyBounds[1] = LaraItem->pos.xPos + tbounds.X2;
	DeadlyBounds[2] = LaraItem->pos.yPos + tbounds.Y1;
	DeadlyBounds[3] = LaraItem->pos.yPos + tbounds.Y2;
	DeadlyBounds[4] = LaraItem->pos.zPos + tbounds.Z1;
	DeadlyBounds[5] = LaraItem->pos.zPos + tbounds.Z2;
}

void DelsGiveLaraItemsCheat() // (AF) (D)
{
	int i;

	for (i = 0; i < 8; ++i)
	{
		if (Objects[ID_PUZZLE_ITEM1 + i].loaded)
			Lara.Puzzles[i] = 1;
		Lara.PuzzlesCombo[2 * i] = false;
		Lara.PuzzlesCombo[2 * i + 1] = false;
	}
	for (i = 0; i < 8; ++i)
	{
		if (Objects[ID_KEY_ITEM1 + i].loaded)
			Lara.Keys[i] = 1;
		Lara.KeysCombo[2 * i] = false;
		Lara.KeysCombo[2 * i + 1] = false;
	}
	for (i = 0; i < 3; ++i)
	{
		if (Objects[ID_PICKUP_ITEM1 + i].loaded)
			Lara.Pickups[i] = 1;
		Lara.PickupsCombo[2 * i] = false;
		Lara.PickupsCombo[2 * i + 1] = false;
	}

	g_Inventory.LoadObjects(false);

	/* Hardcoded code */
}

void LaraCheatGetStuff() // (F) (D)
{
	Lara.NumFlares = -1;
	Lara.NumSmallMedipacks = -1;
	Lara.NumLargeMedipacks = -1;

	if (Objects[ID_CROWBAR_ITEM].loaded)
		Lara.Crowbar = true;

	if (Objects[ID_LASERSIGHT_ITEM].loaded)
	Lara.Lasersight = true;

	if (Objects[ID_REVOLVER_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_REVOLVER].Present = true;
		Lara.Weapons[WEAPON_REVOLVER].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_REVOLVER].HasLasersight = false;
		Lara.Weapons[WEAPON_REVOLVER].HasSilencer = false;
		Lara.Weapons[WEAPON_REVOLVER].Ammo[WEAPON_AMMO1] = -1;
	}

	if (Objects[ID_UZI_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_UZI].Present = true;
		Lara.Weapons[WEAPON_UZI].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_UZI].HasLasersight = false;
		Lara.Weapons[WEAPON_UZI].HasSilencer = false;
		Lara.Weapons[WEAPON_UZI].Ammo[WEAPON_AMMO1] = -1;
	}

	if (Objects[ID_SHOTGUN_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_SHOTGUN].Present = true;
		Lara.Weapons[WEAPON_SHOTGUN].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_SHOTGUN].HasLasersight = false;
		Lara.Weapons[WEAPON_SHOTGUN].HasSilencer = false;
		Lara.Weapons[WEAPON_SHOTGUN].Ammo[WEAPON_AMMO1] = -1;
	}

/*	if (Objects[ID_HARPOON_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_HARPOON_GUN].Present = true;
		Lara.Weapons[WEAPON_HARPOON_GUN].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_HARPOON_GUN].HasLasersight = false;
		Lara.Weapons[WEAPON_HARPOON_GUN].HasSilencer = false;
		Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1] = -1;
	}

	if (Objects[ID_GRENADE_GUN_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Present = true;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].HasSilencer = false;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO1] = -1;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO2] = -1;
		Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[WEAPON_AMMO3] = -1;
	}

	if (Objects[ID_ROCKET_LAUNCHER_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Present = true;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].HasLasersight = false;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].HasSilencer = false;
		Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[WEAPON_AMMO1] = -1;
	}*/

	if (Objects[ID_HK_ITEM].loaded)
	{
		Lara.Weapons[WEAPON_HK].Present = true;
		Lara.Weapons[WEAPON_HK].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[WEAPON_HK].HasLasersight = false;
		Lara.Weapons[WEAPON_HK].HasSilencer = false;
		Lara.Weapons[WEAPON_HK].Ammo[WEAPON_AMMO1] = -1;
	}

/*	if (Objects[ID_CROSSBOW_ITEM].loaded)
	{
		Lara.Weapons[ID_CROSSBOW_ITEM].Present = true;
		Lara.Weapons[ID_CROSSBOW_ITEM].SelectedAmmo = WEAPON_AMMO1;
		Lara.Weapons[ID_CROSSBOW_ITEM].HasLasersight = false;
		Lara.Weapons[ID_CROSSBOW_ITEM].HasSilencer = false;
		Lara.Weapons[ID_CROSSBOW_ITEM].Ammo[WEAPON_AMMO1] = -1;
		Lara.Weapons[ID_CROSSBOW_ITEM].Ammo[WEAPON_AMMO2] = -1;
		Lara.Weapons[ID_CROSSBOW_ITEM].Ammo[WEAPON_AMMO3] = -1;
	}*/

	/*Commented out the blocks for weapons that don't work ingame, after they're fixed, it'll be okay to uncomment*/

	g_Inventory.LoadObjects(false);
}

void LaraCheatyBits() // (F) (D)
{
	if (g_GameFlow->FlyCheat)
	{
#ifdef _DEBUG
		if (TrInput & IN_PAUSE)
#else
		if (TrInput & IN_D)
#endif
		{
			LaraCheatGetStuff();
			LaraItem->hitPoints = 1000;
		}

#ifdef _DEBUG
		if (TrInput & IN_PAUSE)
#else
		if (TrInput & IN_CHEAT)
#endif
		{
			DelsGiveLaraItemsCheat();
			LaraItem->pos.yPos -= 128;
			if (Lara.waterStatus != LW_FLYCHEAT)
			{
				Lara.waterStatus = LW_FLYCHEAT;
				LaraItem->animNumber = LA_DOZY;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = LA_ONWATER_IDLE_TO_SWIM;
				LaraItem->goalAnimState = LA_ONWATER_IDLE_TO_SWIM;
				LaraItem->gravityStatus = false;
				LaraItem->pos.xRot = ANGLE(30);
				LaraItem->fallspeed = 30;
				Lara.air = 1800;
				Lara.deathCount = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				cheatHitPoints = LaraItem->hitPoints;
			}
		}
	}
}

void DelAlignLaraToRope(ITEM_INFO* item) // (F) (D)
{
	ROPE_STRUCT* rope;
	short ropeY;
	PHD_VECTOR vec, vec2, vec3, vec4, vec5, pos, pos2, diff, diff2;
	int matrix[12];
	short angle[3];
	ANIM_FRAME* frame;

	vec.x = 4096;
	vec.y = 0;
	vec.z = 0;
	frame = (ANIM_FRAME*) GetBestFrame(item);
	ropeY = Lara.ropeY - ANGLE(90);
	rope = &Ropes[Lara.ropePtr];
	_0x0046D130(rope, (Lara.ropeSegment - 1 << 7) + frame->offsetY, &pos.x, &pos.y, &pos.z);
	_0x0046D130(rope, (Lara.ropeSegment - 1 << 7) + frame->offsetY - 192, &pos2.x, &pos2.y, &pos2.z);
	diff.x = pos.x - pos2.x << 16;
	diff.y = pos.y - pos2.y << 16;
	diff.z = pos.z - pos2.z << 16;
	NormaliseRopeVector(&diff);
	diff.x >>= 2;
	diff.y >>= 2;
	diff.z >>= 2;
	ScaleVector(&diff, DotProduct(&vec, &diff), &vec2);
	vec2.x = vec.x - vec2.x;
	vec2.y = vec.y - vec2.y;
	vec2.z = vec.z - vec2.z;
	vec3.x = vec2.x;
	vec3.y = vec2.y;
	vec3.z = vec2.z;
	vec4.x = vec2.x;
	vec4.y = vec2.y;
	vec4.z = vec2.z;
	diff2.x = diff.x;
	diff2.y = diff.y;
	diff2.z = diff.z;
	ScaleVector(&vec3, phd_cos(ropeY), &vec3);
	ScaleVector(&diff2, DotProduct(&diff2, &vec2), &diff2);
	ScaleVector(&diff2, 4096 - phd_cos(ropeY), &diff2);
	CrossProduct(&diff, &vec2, &vec4);
	ScaleVector(&vec4, phd_sin(ropeY), &vec4);
	diff2.x += vec3.x;
	diff2.y += vec3.y;
	diff2.z += vec3.z;
	vec2.x = diff2.x + vec4.x << 16;
	vec2.y = diff2.y + vec4.y << 16;
	vec2.z = diff2.z + vec4.z << 16;
	NormaliseRopeVector(&vec2);
	vec2.x >>= 2;
	vec2.y >>= 2;
	vec2.z >>= 2;
	CrossProduct(&diff, &vec2, &vec5);
	vec5.x <<= 16;
	vec5.y <<= 16;
	vec5.z <<= 16;
	NormaliseRopeVector(&vec5);
	vec5.x >>= 2;
	vec5.y >>= 2;
	vec5.z >>= 2;
	matrix[M00] = vec5.x;
	matrix[M01] = diff.x;
	matrix[M02] = vec2.x;
	matrix[M10] = vec5.y;
	matrix[M11] = diff.y;
	matrix[M12] = vec2.y;
	matrix[M20] = vec5.z;
	matrix[M21] = diff.z;
	matrix[M22] = vec2.z;
	_0x0046D420(matrix, angle);
	item->pos.xPos = rope->position.x + (rope->meshSegment[Lara.ropeSegment].x >> 16);
	item->pos.yPos = rope->position.y + (rope->meshSegment[Lara.ropeSegment].y >> 16) + Lara.ropeOffset;
	item->pos.zPos = rope->position.z + (rope->meshSegment[Lara.ropeSegment].z >> 16);

	Matrix rotMatrix = Matrix::CreateFromYawPitchRoll(
		TO_DEGREES(angle[1]),
		TO_DEGREES(angle[0]),
		TO_DEGREES(angle[2])
	);
	
	// PHD_MATH!
	item->pos.xPos += -112 * rotMatrix.m[0][2]; // MatrixPtr[M02] >> W2V_SHIFT;
	item->pos.yPos += -112 * rotMatrix.m[1][2]; // MatrixPtr[M12] >> W2V_SHIFT;
	item->pos.zPos += -112 * rotMatrix.m[2][2]; // MatrixPtr[M22] >> W2V_SHIFT;
	
	item->pos.xRot = angle[0];
	item->pos.yRot = angle[1];
	item->pos.zRot = angle[2];
}
