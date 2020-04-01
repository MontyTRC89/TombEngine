#include "larafire.h"

#include "items.h"
#include "Lara.h"
#include "laraflar.h"
#include "lara1gun.h"
#include "lara2gun.h"

#include "..\Scripting\GameFlowScript.h"

#include <stdio.h>
#include "objects.h"
#include "effects.h"
#include "sphere.h"
#include "draw.h"
#include "effect2.h"
#include "flmtorch.h"
#include "..\Specific\roomload.h"
#include "lot.h"
#include "../Specific/setup.h"

WEAPON_INFO Weapons[NUM_WEAPONS] =
{
	/* No weapons */
	{
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},
	/* Pistols */
	{
		{ -ANGLE(60),  ANGLE(60),  -ANGLE(60), ANGLE(60) },
		{ -ANGLE(170), ANGLE(60),  -ANGLE(80), ANGLE(80) },
		{ -ANGLE(60),  ANGLE(170), -ANGLE(80), ANGLE(80) },
		ANGLE(10),
		ANGLE(8),
		650,
		8 * WALL_SIZE,
		1,
		9,
		3,
		0,
		SFX_LARA_FIRE
	},
	/* Revolver */
	{
		{ -ANGLE(60), ANGLE(60), -ANGLE(60), ANGLE(60) },
		{ -ANGLE(10), ANGLE(10), -ANGLE(80), ANGLE(80) },
		{  ANGLE(0),   ANGLE(0),   ANGLE(0),  ANGLE(0) },
		ANGLE(10),
		ANGLE(4),
		650,
		8 * WALL_SIZE,
		21,
		16,
		3,
		0,
		SFX_REVOLVER
	},
	/* Uzis */
	{
		{ -ANGLE(60),  ANGLE(60),  -ANGLE(60), ANGLE(60) },
		{ -ANGLE(170), ANGLE(60),  -ANGLE(80), ANGLE(80) },
		{ -ANGLE(60),  ANGLE(170), -ANGLE(80), ANGLE(80) },
		ANGLE(10),
		ANGLE(8),
		650,
		8 * WALL_SIZE,
		1,
		3,
		3,
		0,
		SFX_LARA_UZI_FIRE
	},
	/* Shotgun */
	{
		{ -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		ANGLE(10),
		0,
		500,
		8 * WALL_SIZE,
		3,
		9,
		3,
		10,
		SFX_LARA_SHOTGUN
	},
	/* HK */
	{
		{ -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		ANGLE(10),
		ANGLE(4),
		500,
		12 * WALL_SIZE,
		4,
		0,
		3,
		10,
		0     // FIRE/SILENCER_FIRE
	},
	/* Crossbow */
	{
		{ -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		5,
		0,
		2,
		10,
		SFX_LARA_CROSSBOW
	},
	/* Flare */
	{
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},
	/* Flare 2 */
	{
		{ -ANGLE(30), ANGLE(30), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(30), ANGLE(30), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(30), ANGLE(30), -ANGLE(55), ANGLE(55) },
		ANGLE(10),
		ANGLE(8),
		400,
		8 * WALL_SIZE,
		3,
		0,
		2,
		0,
		SFX_LARA_UZI_FIRE
	},
	/* Grenade launcher */
	{
		{ -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		20,
		0, 
		2,
		10,
		0
	}, 
	/* Harpoon gun */
	{
		{ -ANGLE(60), ANGLE(60), -ANGLE(65), ANGLE(65) },
		{ -ANGLE(20), ANGLE(20), -ANGLE(75), ANGLE(75) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(75), ANGLE(75) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		6,
		0,
		2,
		10,
		0
	},
	/* Rocket launcher */
	{
		{ -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },
		ANGLE(10),
		ANGLE(8),
		500,
		8 * WALL_SIZE,
		30,
		0,
		2,
		12,
		77
	},
	/* Snowmobile */
	{
		{ -ANGLE(30), ANGLE(30), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(30), ANGLE(30), -ANGLE(55), ANGLE(55) },
		{ -ANGLE(30), ANGLE(30), -ANGLE(55), ANGLE(55) },
		ANGLE(10),
		ANGLE(8),
		400,
		8 * WALL_SIZE,
		3,
		0,
		0,
		0,
		SFX_LARA_UZI_FIRE
	}
};

short HoldStates[] = {
	STATE_LARA_STOP,
	STATE_LARA_POSE,
	STATE_LARA_TURN_RIGHT_SLOW,
	STATE_LARA_TURN_LEFT_SLOW,
	STATE_LARA_WALK_BACK,
	STATE_LARA_TURN_FAST,
	STATE_LARA_WALK_RIGHT,
	STATE_LARA_WALK_LEFT,
	STATE_LARA_PICKUP,
	STATE_LARA_SWITCH_DOWN,
	STATE_LARA_SWITCH_UP,
	STATE_LARA_WADE_FORWARD,
	STATE_LARA_CROUCH_IDLE,
	STATE_LARA_CROUCH_TURN_LEFT,
	STATE_LARA_CROUCH_TURN_RIGHT,
	-1
};

extern GameFlow* g_GameFlow;
extern LaraExtraInfo g_LaraExtra;
bool MonksAttackLara;
ITEM_INFO* LastTargets[8];
ITEM_INFO* TargetList[8];

int WeaponObject(int weaponType) // (F) (D)
{
	switch (weaponType)
	{
	case WEAPON_UZI:
		return ID_UZI_ANIM;
	case WEAPON_SHOTGUN:
		return ID_SHOTGUN_ANIM;
	case WEAPON_REVOLVER:
		return ID_REVOLVER_ANIM;
	case WEAPON_CROSSBOW:
		return ID_CROSSBOW_ANIM;
	case WEAPON_HK:
		return ID_HK_ANIM;
	case WEAPON_FLARE:
		return ID_LARA_FLARE_ANIM;
	case WEAPON_GRENADE_LAUNCHER:
		return ID_GRENADE_ANIM;
	case WEAPON_ROCKET_LAUNCHER:
		return ID_ROCKET_ANIM;
	case WEAPON_HARPOON_GUN:
		return ID_HARPOON_ANIM;
	default:
		return ID_PISTOLS_ANIM;
	}
}

void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm) // (F) (D)
{
	short rotY, rotX, speed, x, y;

	speed = winfo->aimSpeed;

	if (arm->lock)
	{
		y = Lara.targetAngles[0];
		x = Lara.targetAngles[1];
	}
	else
	{
		y = 0;
		x = 0;
	}

	/* move y axis */
	rotY = arm->yRot;
	if ((rotY >= y - speed) && (rotY <= y + speed))
		rotY = y;
	else if (rotY < y)
		rotY += speed;
	else
		rotY -= speed;
	arm->yRot = rotY;

	/* move x axis */
	rotX = arm->xRot;
	if ((rotX >= x - speed) && (rotX <= x + speed))
		rotX = x;
	else if (rotX < x)
		rotX += speed;
	else
		rotX -= speed;
	arm->xRot = rotX;

	/* move z axis */
	arm->zRot = 0;
}

void SmashItem(short itemNum) // (F) (D)
{
	ITEM_INFO* item = &Items[itemNum];
	if (item->objectNumber >= ID_SMASH_OBJECT1 && item->objectNumber <= ID_SMASH_OBJECT8)
	{
		SmashObject(itemNum);
	}
}

void LaraGun() // (F) (D)
{
	if (Lara.leftArm.flash_gun > 0)
		--Lara.leftArm.flash_gun;
	if (Lara.rightArm.flash_gun > 0)
		--Lara.rightArm.flash_gun;

	if (Lara.gunType == WEAPON_TORCH)
	{
		DoFlameTorch();
		return;
	}

	if (LaraItem->hitPoints <= 0)
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (Lara.gunStatus == LG_NO_ARMS)
	{
		if (TrInput & IN_DRAW)
		{
			Lara.requestGunType = Lara.lastGunType;
		}
		else if (TrInput & IN_FLARE && (g_GameFlow->GetLevel(CurrentLevel)->LaraType != LARA_YOUNG))
		{
			if (LaraItem->currentAnimState == STATE_LARA_CROUCH_IDLE && LaraItem->animNumber != ANIMATION_LARA_CROUCH_IDLE)
				return;

			if (Lara.gunType == WEAPON_FLARE)
			{
				if (!Lara.leftArm.frameNumber)
				{
					Lara.gunStatus = LG_UNDRAW_GUNS;
				}
			}
			else if (g_LaraExtra.NumFlares)
			{
				if (g_LaraExtra.NumFlares != -1)
					g_LaraExtra.NumFlares--;
				Lara.requestGunType = WEAPON_FLARE;
			}
		}

		if ((Lara.requestGunType != Lara.gunType) || (TrInput & IN_DRAW))
		{
			if ((LaraItem->currentAnimState == STATE_LARA_CROUCH_IDLE
				|| LaraItem->currentAnimState == STATE_LARA_CROUCH_TURN_LEFT
				|| LaraItem->currentAnimState == STATE_LARA_CROUCH_TURN_RIGHT)
				&& (Lara.requestGunType == WEAPON_HK
				|| Lara.requestGunType == WEAPON_CROSSBOW
#if 1
				|| Lara.requestGunType == WEAPON_SHOTGUN))
#else
				|| Lara.requestGunType == WEAPON_SHOTGUN
				|| Lara.requestGunType == WEAPON_HARPOON_GUN))
#endif
			{
				if (Lara.gunType == WEAPON_FLARE)
					Lara.requestGunType = WEAPON_FLARE;
			}
			else if (Lara.requestGunType == WEAPON_FLARE
#if 0
				|| g_LaraExtra.Vehicle == NO_ITEM
				&& (Lara.requestGunType == WEAPON_HARPOON_GUN
#endif
				|| Lara.waterStatus == LW_ABOVE_WATER
				|| Lara.waterStatus == LW_WADE
				&& Lara.waterSurfaceDist > -Weapons[Lara.gunType].gunHeight)
			{
				if (Lara.gunType == WEAPON_FLARE)
				{
					CreateFlare(ID_FLARE_ITEM, 0);
					undraw_flare_meshes();
					Lara.flareControlLeft = false;
					Lara.flareAge = 0;
				}

				Lara.gunType = Lara.requestGunType;
				InitialiseNewWeapon();
				Lara.rightArm.frameNumber = 0;
				Lara.leftArm.frameNumber = 0;
				Lara.gunStatus = LG_DRAW_GUNS;
			}
			else
			{
				Lara.lastGunType = Lara.requestGunType;
				if (Lara.gunType != WEAPON_FLARE)
					Lara.gunType = Lara.requestGunType;
				else
					Lara.requestGunType = WEAPON_FLARE;
			}
		}
	}
	else if (Lara.gunStatus == LG_READY)
	{
#if 0
		if ((TrInput & IN_DRAW) || Lara.requestGunType != Lara.gunType || Lara.gunType != WEAPON_HARPOON_GUN && Lara.waterStatus != LW_ABOVE_WATER && Lara.waterStatus != LW_WADE || Lara.waterSurfaceDist < -Weapons[Lara.gunType].gunHeight)
#else
		if ((TrInput & IN_DRAW) || Lara.requestGunType != Lara.gunType || Lara.waterStatus != LW_ABOVE_WATER && Lara.waterStatus != LW_WADE || Lara.waterSurfaceDist < -Weapons[Lara.gunType].gunHeight)
#endif
			Lara.gunStatus = LG_UNDRAW_GUNS;
	}
	else if (Lara.gunStatus == LG_HANDS_BUSY && (TrInput & IN_FLARE) && LaraItem->currentAnimState == STATE_LARA_CRAWL_IDLE && LaraItem->animNumber == ANIMATION_LARA_CRAWL_IDLE)
	{
		Lara.requestGunType = WEAPON_FLARE;
	}

	switch (Lara.gunStatus)
	{
		case LG_DRAW_GUNS:
			if (Lara.gunType != WEAPON_FLARE && Lara.gunType != WEAPON_NONE)
				Lara.lastGunType = Lara.gunType;

		switch (Lara.gunType)
		{
			case WEAPON_PISTOLS:
			case WEAPON_REVOLVER:
			case WEAPON_UZI:
				if (Camera.type != CINEMATIC_CAMERA && Camera.type != LOOK_CAMERA && Camera.type != HEAVY_CAMERA)
					Camera.type = COMBAT_CAMERA;
				draw_pistols(Lara.gunType);
				break;

			case WEAPON_SHOTGUN:
			case WEAPON_CROSSBOW:
			case WEAPON_HK:
#if 0
			case WEAPON_GRENADE_LAUNCHER:
			case WEAPON_ROCKET_LAUNCHER:
			case WEAPON_HARPOON_GUN:
#endif
				if (Camera.type != CINEMATIC_CAMERA && Camera.type != LOOK_CAMERA && Camera.type != HEAVY_CAMERA)
					Camera.type = COMBAT_CAMERA;
				draw_shotgun(Lara.gunType);
				break;

			case WEAPON_FLARE:
				draw_flare();
				break;

			default:
				Lara.gunStatus = LG_NO_ARMS;
				break;
		}
		break;

		case LG_SPECIAL:
			draw_flare();
			break;

		case LG_UNDRAW_GUNS:
			LARA_MESHES(ID_LARA, LM_HEAD);

			switch (Lara.gunType)
			{
				case WEAPON_PISTOLS:
				case WEAPON_REVOLVER:
				case WEAPON_UZI:
					undraw_pistols(Lara.gunType);
					break;

				case WEAPON_SHOTGUN:
				case WEAPON_CROSSBOW:
				case WEAPON_HK:
#if 0
				case WEAPON_GRENADE_LAUNCHER:
				case WEAPON_ROCKET_LAUNCHER:
				case WEAPON_HARPOON_GUN:
#endif
					undraw_shotgun(Lara.gunType);
					break;

				case WEAPON_FLARE:
					undraw_flare();
					break;

				default:
					return;
			}
			break;

		case LG_READY:
			if (!(TrInput & IN_ACTION))
				LARA_MESHES(ID_LARA, LM_HEAD);
			else
				LARA_MESHES(ID_LARA_SCREAM, LM_HEAD);
		
			if (Camera.type != CINEMATIC_CAMERA && Camera.type != LOOK_CAMERA && Camera.type != HEAVY_CAMERA)
				Camera.type = COMBAT_CAMERA;

			if (TrInput & IN_ACTION)
			{
				if (!*GetAmmo(Lara.gunType))
				{
					Lara.requestGunType = Objects[ID_PISTOLS_ITEM].loaded ? WEAPON_PISTOLS : WEAPON_NONE;
					return;
				}
			}

			switch (Lara.gunType)
			{
				case WEAPON_PISTOLS:
				case WEAPON_UZI:
					PistolHandler(Lara.gunType);
					break;

				case WEAPON_SHOTGUN:
				case WEAPON_CROSSBOW:
				case WEAPON_HK:
#if 0
				case WEAPON_GRENADE_LAUNCHER:
				case WEAPON_ROCKET_LAUNCHER:
				case WEAPON_HARPOON_GUN:
#endif
				case WEAPON_REVOLVER:
					RifleHandler(Lara.gunType);
					break;

				default:
					return;
			}
			break;

		case LG_NO_ARMS:
			if (Lara.gunType == WEAPON_FLARE)
			{
#if 1
				if (CheckForHoldingState(LaraItem->currentAnimState))
#else
				if (g_LaraExtra.Vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState))
#endif
				{
					if (Lara.flareControlLeft)
					{
						if (Lara.leftArm.frameNumber)
						{
							if (++Lara.leftArm.frameNumber == 110)
								Lara.leftArm.frameNumber = 0;
						}
					}
					else
					{
						Lara.leftArm.frameNumber = 95;
						Lara.flareControlLeft = true;
					}
				}
				else
				{
					Lara.flareControlLeft = false;
				}

				DoFlareInHand(Lara.flareAge);
				set_flare_arm(Lara.leftArm.frameNumber);
			}
			break;

		case LG_HANDS_BUSY:
			if (Lara.gunType == WEAPON_FLARE)
			{
				if (CHECK_LARA_MESHES(ID_LARA_FLARE_ANIM, LM_LHAND))
				{
#if 0
					Lara.flareControlLeft = (g_LaraExtra.Vehicle != NO_ITEM || CheckForHoldingState(LaraItem->currentAnimState));
#else
					Lara.flareControlLeft = CheckForHoldingState(LaraItem->currentAnimState);
#endif
					DoFlareInHand(Lara.flareAge);
					set_flare_arm(Lara.leftArm.frameNumber);
				}
			}
			break;
	}
}

short* GetAmmo(int weaponType)
{
	return &g_LaraExtra.Weapons[weaponType].Ammo[g_LaraExtra.Weapons[weaponType].SelectedAmmo];
}

void InitialiseNewWeapon()
{
	Lara.rightArm.frameNumber = 0;
	Lara.leftArm.frameNumber = 0;
	Lara.leftArm.zRot = 0;
	Lara.leftArm.yRot = 0;
	Lara.leftArm.xRot = 0;
	Lara.rightArm.zRot = 0;
	Lara.rightArm.yRot = 0;
	Lara.rightArm.xRot = 0;
	Lara.target = NULL;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
	Lara.rightArm.flash_gun = 0;
	Lara.leftArm.flash_gun = 0;

	switch (Lara.gunType)
	{
	case WEAPON_PISTOLS:
	case WEAPON_UZI:
		Lara.rightArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		Lara.leftArm.frameBase = Objects[ID_PISTOLS_ANIM].frameBase;
		if (Lara.gunStatus != LG_NO_ARMS)
			draw_pistol_meshes(Lara.gunType);
		break;

	case WEAPON_SHOTGUN:
	case WEAPON_REVOLVER:
	case WEAPON_HK:
	case WEAPON_GRENADE_LAUNCHER:
	case WEAPON_HARPOON_GUN:
	case WEAPON_ROCKET_LAUNCHER:
		Lara.rightArm.frameBase = Objects[WeaponObject(Lara.gunType)].frameBase;
		Lara.leftArm.frameBase = Objects[WeaponObject(Lara.gunType)].frameBase;
		if (Lara.gunStatus != LG_NO_ARMS)
			draw_shotgun_meshes(Lara.gunType);
		break;

	case WEAPON_FLARE:
		Lara.rightArm.frameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		Lara.leftArm.frameBase = Objects[ID_LARA_FLARE_ANIM].frameBase;
		if (Lara.gunStatus != LG_NO_ARMS)
			draw_flare_meshes();
		break;

	default:
		Lara.rightArm.frameBase = Anims[LaraItem->animNumber].framePtr;
		Lara.leftArm.frameBase = Anims[LaraItem->animNumber].framePtr;
		break;
	}
}

int WeaponObjectMesh(int weaponType)
{
	switch (weaponType)
	{
	case WEAPON_REVOLVER:
		return (g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight == true ? ID_LARA_REVOLVER_LASER : ID_REVOLVER_ANIM);

	case WEAPON_UZI:
		return ID_UZI_ANIM;

	case WEAPON_SHOTGUN:
		return ID_SHOTGUN_ANIM;

	case WEAPON_HK:
		return ID_HK_ANIM;

	case WEAPON_CROSSBOW:
		return (g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight == true ? ID_LARA_CROSSBOW_LASER : ID_CROSSBOW_ANIM);
		
	case WEAPON_GRENADE_LAUNCHER:
		return ID_GRENADE_ANIM;

	case WEAPON_HARPOON_GUN:
		return ID_HARPOON_ANIM;

	case WEAPON_ROCKET_LAUNCHER:
		return ID_ROCKET_ANIM;

	default:
		return ID_PISTOLS_ANIM;

	}
}

void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitPos, int damage, int flag)
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	
	item->hitStatus = true;
	if (creature && item != LaraItem)
		creature->hurtByLara = true;

	if (hitPos)
	{
		if (obj->hitEffect)
		{
			switch (obj->hitEffect)
			{
				case 1:
					DoBloodSplat(hitPos->x, hitPos->y, hitPos->z, (GetRandomControl() & 3) + 3, item->pos.yRot, item->roomNumber);
					break;
				case 3:
					TriggerRicochetSpark(hitPos, LaraItem->pos.yRot, 3, 0);
					break;
				case 2:
					TriggerRicochetSpark(hitPos, LaraItem->pos.yRot, 3, -5);
					SoundEffect(SFX_SWORD_GOD_HITMET, &item->pos, 0);
					break;
			}
		}
	}
	if (!obj->undead || flag || item->hitPoints == -16384)
	{
		if (item->hitPoints > 0 && item->hitPoints <= damage)
			++Savegame.Level.Kills;
		item->hitPoints -= damage;
	}
}

int DetectCrouchWhenFiring(ITEM_INFO* src, WEAPON_INFO* weapon)
{
	if (src->currentAnimState == STATE_LARA_CROUCH_IDLE || src->currentAnimState == STATE_LARA_CROUCH_TURN_LEFT || src->currentAnimState == STATE_LARA_CROUCH_TURN_RIGHT)
		return STEP_SIZE;
	else
		return int(weapon->gunHeight);
}

int FireWeapon(int weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles) // (F) (D)
{
	short* ammo = GetAmmo(weaponType);
	if (!*ammo)
		return 0;
	if (*ammo != -1)
		(*ammo)--;

	WEAPON_INFO* weapon = &Weapons[weaponType];
	int r;

	PHD_3DPOS pos;
	pos.xPos = 0;
	pos.yPos = 0;
	pos.zPos = 0;
	GetLaraJointPosition((PHD_VECTOR*)&pos, LJ_RHAND);

	pos.xPos = src->pos.xPos;
	pos.zPos = src->pos.zPos;

	pos.xRot = angles[1] + (GetRandomControl() - 16384) * weapon->shotAccuracy / 65536;
	pos.yRot = angles[0] + (GetRandomControl() - 16384) * weapon->shotAccuracy / 65536;
	pos.zRot = 0;

	phd_GenerateW2V(&pos);
	
	int num = GetSpheres(target, SphereList, 0);
	int best = -1;
	int bestDistance = 0x7FFFFFFF;

	for (int i = 0; i < num; i++)
	{
		SPHERE* sphere = &SphereList[i];

		r = sphere->r;									 
		if ((abs(sphere->x)) < r && (abs(sphere->y)) < r &&  sphere->z > r && SQUARE(sphere->x) + SQUARE(sphere->y) <= SQUARE(r))
		{
			if (sphere->z - r < bestDistance)
			{
				bestDistance = sphere->z - r;
				best = i;                 			 
			}
		}
	}

	Lara.hasFired = true;
	Lara.fired = true;
	
	GAME_VECTOR vSrc;
	vSrc.x = pos.xPos;
	vSrc.y = pos.yPos;
	vSrc.z = pos.zPos;
	
	short roomNumber = src->roomNumber;
	GetFloor(pos.xPos, pos.yPos, pos.zPos, &roomNumber);
	vSrc.roomNumber = roomNumber;

	if (best < 0)
	{
		GAME_VECTOR vDest;
		vDest.x = vSrc.x + (MatrixPtr[M20] * 5 >> 2);
		vDest.y = vSrc.y + (MatrixPtr[M21] * 5 >> 2);
		vDest.z = vSrc.z + (MatrixPtr[M22] * 5 >> 2);

		GetTargetOnLOS(&vSrc, &vDest, 0, 1);
		
		return -1;
	}
	else
	{
		Savegame.Game.AmmoHits++;

		GAME_VECTOR vDest;
		vDest.x = vSrc.x + ((MatrixPtr[M20] * bestDistance) >> W2V_SHIFT);
		vDest.y = vSrc.y + ((MatrixPtr[M21] * bestDistance) >> W2V_SHIFT);
		vDest.z = vSrc.z + ((MatrixPtr[M22] * bestDistance) >> W2V_SHIFT);

		// TODO: enable it when the slot is created !
		/*
		if (target->objectNumber == ID_TRIBEBOSS)
		{
			long dx, dy, dz;

			dx = (vDest.x - vSrc.x) >> 5;
			dy = (vDest.y - vSrc.y) >> 5;
			dz = (vDest.z - vSrc.z) >> 5;
			FindClosestShieldPoint(vDest.x - dx, vDest.y - dy, vDest.z - dz, target);
		}
		else if (target->objectNumber == ID_ARMY_WINSTON || target->objectNumber == ID_LONDONBOSS) //Don't want blood on Winston - never get the stains out
		{
			short ricochet_angle;
			target->hitStatus = true; //need to do this to maintain defence state
			target->hitPoints--;
			ricochet_angle = (mGetAngle(LaraItem->pos.zPos, LaraItem->pos.xPos, target->pos.zPos, target->pos.xPos) >> 4) & 4095;
			TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
			SoundEffect(SFX_LARA_RICOCHET, &target->pos, 0);		// play RICOCHET Sample
		}
		else if (target->objectNumber == ID_SHIVA) //So must be Shiva
		{
			z = target->pos.zPos - lara_item->pos.zPos;
			x = target->pos.xPos - lara_item->pos.xPos;
			angle = 0x8000 + ATAN(z, x) - target->pos.yRot;

			if ((target->currentAnimState > 1 && target->currentAnimState < 5) && angle < 0x4000 && angle > -0x4000)
			{
				target->hitStatus = true; //need to do this to maintain defence state
				ricochet_angle = (mGetAngle(LaraItem->pos.zPos, LaraItem->pos.xPos, target->pos.zPos, target->pos.xPos) >> 4) & 4095;
				TriggerRicochetSparks(&vDest, ricochet_angle, 16, 0);
				SoundEffect(SFX_LARA_RICOCHET, &target->pos, 0); // play RICOCHET Sample
			}
			else //Shiva's not in defence mode or has its back to Lara
				HitTarget(target, &vDest, weapon->damage, 0);
		}
		else
		{*/
			if (!GetTargetOnLOS(&vSrc, &vDest, 0, 1))
				HitTarget(target, &vDest, weapon->damage, 0);
		//}
		
		return 1;
	}
}

void find_target_point(ITEM_INFO* item, GAME_VECTOR* target) // (F) (D)
{
	ANIM_FRAME* bounds = (ANIM_FRAME*) GetBestFrame(item);

	int x = (bounds->MinX + bounds->MaxX) / 2;
	int y = bounds->MinY + (bounds->MaxY - bounds->MinY) / 3;
	int z = (bounds->MinZ + bounds->MaxZ) / 2;

	int c = COS(item->pos.yRot);
	int s = SIN(item->pos.yRot);

	target->x = item->pos.xPos + ((c * x + s * z) >> W2V_SHIFT);
	target->y = item->pos.yPos + y;
	target->z = item->pos.zPos + ((c * z - s * x) >> W2V_SHIFT);

	target->roomNumber = item->roomNumber;
}

void LaraTargetInfo(WEAPON_INFO* weapon) // (F) (D)
{
	if (!Lara.target)
	{
		Lara.rightArm.lock = 0;
		Lara.leftArm.lock = 0;
		Lara.targetAngles[1] = 0;
		Lara.targetAngles[0] = 0;
		return;
	}

	GAME_VECTOR pos;

	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	GetLaraJointPosition((PHD_VECTOR*)&pos, LJ_RHAND);

	pos.x = LaraItem->pos.xPos;
	pos.z = LaraItem->pos.zPos;
	pos.roomNumber = LaraItem->roomNumber;
	
	GAME_VECTOR targetPoint;
	find_target_point(Lara.target, &targetPoint);

	short angles[2];
	phd_GetVectorAngles(targetPoint.x - pos.x, targetPoint.y - pos.y, targetPoint.z - pos.z, angles);

	angles[0] -= LaraItem->pos.yRot;
	angles[1] -= LaraItem->pos.xRot;

	if (LOS(&pos, &targetPoint))
	{
		if (angles[0] >= weapon->lockAngles[0]
		&&  angles[0] <= weapon->lockAngles[1]
		&&  angles[1] >= weapon->lockAngles[2]
		&&  angles[1] <= weapon->lockAngles[3])
		{
			Lara.rightArm.lock = 1;
			Lara.leftArm.lock = 1;
		}
		else
		{
			if (Lara.leftArm.lock)
			{
				if ((angles[0] < weapon->leftAngles[0] ||
					 angles[0] > weapon->leftAngles[1] ||
					 angles[1] < weapon->leftAngles[2] ||
					 angles[1] > weapon->leftAngles[3]))
					Lara.leftArm.lock = 0;
			}

			if (Lara.rightArm.lock)
			{
				if ((angles[0] < weapon->rightAngles[0] ||
					 angles[0] > weapon->rightAngles[1] ||
					 angles[1] < weapon->rightAngles[2] ||
					 angles[1] > weapon->rightAngles[3]))
					Lara.rightArm.lock = 0;
			}
		}
	}
	else
	{
		Lara.rightArm.lock = 0;
		Lara.leftArm.lock = 0;
	}

	Lara.targetAngles[0] = angles[0];
	Lara.targetAngles[1] = angles[1];
}

int CheckForHoldingState(int state) // (F) (D)
{
	short* holdState = HoldStates;

#if 0
	if (g_LaraExtra.ExtraAnim)
		return 0;
#endif

	while (*holdState >= 0)
	{
		if (state == *holdState)
			return 1;
		holdState++;
	}
	
	return 0;
}

void LaraGetNewTarget(WEAPON_INFO* winfo) // (F) (D)
{
	GAME_VECTOR source, target;
	int bestDistance, maxDistance, targets, slot, x, y, z, distance;
	ITEM_INFO* bestItem, *item;
	short bestYrot, angle[2], match;
	bool flag, loop;

	if (BinocularRange)
	{
		Lara.target = NULL;
		return;
	}
	bestItem = NULL;
	bestYrot = 0x7FFF;
	bestDistance = 0x7FFFFFFF;
	source.x = LaraItem->pos.xPos;
	source.y = LaraItem->pos.yPos - 650;
	source.z = LaraItem->pos.zPos;
	source.roomNumber = LaraItem->roomNumber;
	maxDistance = winfo->targetDist;
	targets = 0;
	for (slot = 0; slot < 5; ++slot)
	{
		if (BaddieSlots[slot].itemNum != NO_ITEM)
		{
			item = &Items[BaddieSlots[slot].itemNum];
			if (item->hitPoints > 0)
			{
				x = item->pos.xPos - source.x;
				y = item->pos.yPos - source.y;
				z = item->pos.zPos - source.z;
				if (abs(x) <= maxDistance && abs(y) <= maxDistance && abs(z) <= maxDistance)
				{
					distance = SQUARE(x) + SQUARE(y) + SQUARE(z);
					if (distance < SQUARE(maxDistance))
					{
						find_target_point(item, &target);
						if (LOS(&source, &target))
						{
							phd_GetVectorAngles(target.x - source.x, target.y - source.y, target.z - source.z, angle);
							angle[0] -= LaraItem->pos.yRot + Lara.torsoYrot;
							angle[1] -= LaraItem->pos.xRot + Lara.torsoXrot;
							if (angle[0] >= winfo->lockAngles[0] && angle[0] <= winfo->lockAngles[1] && angle[1] >= winfo->lockAngles[2] && angle[1] <= winfo->lockAngles[3])
							{
								TargetList[targets] = item;
								++targets;
								if (abs(angle[0]) < bestYrot + ANGLE(15) && distance < bestDistance)
								{
									bestDistance = distance;
									bestYrot = abs(angle[0]);
									bestItem = item;
								}
							}
						}
					}
				}
			}
		}
	}
	TargetList[targets] = NULL;
	if (!TargetList[0])
	{
		Lara.target = NULL;
	}
	else
	{
		for (slot = 0; slot < 8; ++slot)
		{
			if (!TargetList[slot])
				Lara.target = NULL;
			if (TargetList[slot] == Lara.target)
				break;
		}
		if (Lara.gunStatus != LG_NO_ARMS || TrInput & IN_LOOKSWITCH)
		{
			if (!Lara.target)
			{
				Lara.target = bestItem;
				LastTargets[0] = NULL;
			}
			else if (TrInput & IN_LOOKSWITCH)
			{
				Lara.target = NULL;
				flag = true;
				for (match = 0; match < 8 && TargetList[match]; ++match)
				{
					loop = false;
					for (slot = 0; slot < 8 && LastTargets[slot]; ++slot)
					{
						if (LastTargets[slot] == TargetList[match])
						{
							loop = true;
							break;
						}
					}
					if (!loop)
					{
						Lara.target = TargetList[match];
						if (Lara.target)
							flag = false;
						break;
					}
				}
				if (flag)
				{
					Lara.target = bestItem;
					LastTargets[0] = NULL;
				}
			}
		}
	}
	if (Lara.target != LastTargets[0])
	{
		for (slot = 7; slot > 0; --slot)
			LastTargets[slot] = LastTargets[slot - 1];
		LastTargets[0] = Lara.target;
	}
	LaraTargetInfo(winfo);
}

void Inject_LaraFire()
{
	INJECT(0x00453490, AimWeapon);
	INJECT(0x00453AE0, WeaponObject);
	INJECT(0x00452430, LaraGun);
	INJECT(0x004546C0, GetAmmo);
	INJECT(0x00452B30, InitialiseNewWeapon);
	INJECT(0x00453B50, WeaponObjectMesh);
	//INJECT(0x00453A90, SmashItem);
}