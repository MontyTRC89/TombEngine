#include "newobjects.h"
#include "../Game/box.h"
#include "../Game/people.h"
#include "../Game/sphere.h"
#include "../Game/effect2.h"

BITE_INFO workerShotgun = { 0, 281, 40, 9 };
BITE_INFO workerMachineGun = { 0, 308, 32, 9 };
BITE_INFO workerDualGunL = { -2, 275, 23, 6 };
BITE_INFO workerDualGunR = { 2, 275, 23, 10 };
BITE_INFO workerFlameThrower = { 0, 250, 32, 9 };

static void ShotLara_WithShotgun(ITEM_INFO* item, AI_INFO* info, BITE_INFO* bite, short angle_y, int damage)
{
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
}

void __cdecl InitialiseWorkerShotgun(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;
	item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 5;

	InitialiseCreature(itemNum);

	anim = &Anims[item->animNumber];
	item->frameNumber = anim->frameBase;
	item->currentAnimState = anim->currentAnimState;
}

void __cdecl WorkerShotgunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* shotgun;
	AI_INFO info;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &Items[itemNum];
	shotgun = (CREATURE_INFO*)item->data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 18;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, shotgun->maximumTurn);

		switch (item->currentAnimState)
		{
			case 2:
				shotgun->flags = 0;
				shotgun->maximumTurn = 0;

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (shotgun->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 5;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance <= 0x900000 || info.zoneNumber != info.enemyZone)
					{
						item->goalAnimState = (GetRandomControl() >= 0x4000) ? 9 : 8;
					}
					else
					{
						item->goalAnimState = 1;
					}
				}
				else if (shotgun->mood == ATTACK_MOOD || !info.ahead)
				{
					item->goalAnimState = (info.distance <= 0x400000) ? 1 : 5;
				}
				else
				{
					item->goalAnimState = 3;
				}
				break;

			case 3:
				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (Targetable(item, &info))
				{
					item->goalAnimState = 4;
				}
				else if (shotgun->mood == ATTACK_MOOD || !info.ahead)
				{
					item->goalAnimState = 2;
				}
				break;

			case 1:
				shotgun->maximumTurn = 546;

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (shotgun->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 5;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
						item->goalAnimState = 2;
					else
						item->goalAnimState = 6;
				}
				else if (shotgun->mood == ATTACK_MOOD || !info.ahead)
				{
					if (info.distance > 0x400000)
						item->goalAnimState = 5;
				}
				else
				{
					item->goalAnimState = 2;
				}
				break;

			case 5:
				shotgun->maximumTurn = 910;
				tilt = (angle / 2);

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (shotgun->mood != ESCAPE_MOOD)
				{
					if (Targetable(item, &info))
					{
						item->goalAnimState = 1;
					}
					else if (shotgun->mood == BORED_MOOD || shotgun->mood == STALK_MOOD)
					{
						item->goalAnimState = 1;
					}
				}
				break;

			case 8:
			case 9:
				shotgun->flags = 0;

				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (Targetable(item, &info))
				{
					item->goalAnimState = (item->currentAnimState == 8) ? 4 : 10;
				}
				break;

			case 4:
			case 10:
				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (!shotgun->flags)
				{
					ShotLara_WithShotgun(item, &info, &workerShotgun, torso_y, 25);
					// TODO: later add item->firedWeapon = 1; for workerShotgun (in state id 6 too)
					shotgun->flags = 1;
				}

				if (item->currentAnimState == 4 && item->goalAnimState != 2 && (shotgun->mood == ESCAPE_MOOD || info.distance > 0x900000 || !Targetable(item, &info)))
				{
					item->goalAnimState = 2;
				}
				break;

			case 6:
				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (!shotgun->flags)
				{
					ShotLara_WithShotgun(item, &info, &workerShotgun, torso_y, 25);
					//item->firedWeapon = 1;
					shotgun->flags = 1;
				}
				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}

void __cdecl InitialiseWorkerMachineGun(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;
	item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 12;

	InitialiseCreature(itemNum);

	anim = &Anims[item->animNumber];
	item->frameNumber = anim->frameBase;
	item->currentAnimState = anim->currentAnimState;
}

void __cdecl WorkerMachineGunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* machinegun;
	AI_INFO info;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &Items[itemNum];
	machinegun = (CREATURE_INFO*)item->data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 19;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, machinegun->maximumTurn);

		switch (item->currentAnimState)
		{
			case 1:
				machinegun->flags = 0;
				machinegun->maximumTurn = 0;

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (machinegun->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
						item->goalAnimState = (GetRandomControl() < 0x4000) ? 8 : 10;
					else
						item->goalAnimState = 2;
				}
				else if (machinegun->mood == ATTACK_MOOD || !info.ahead)
				{
					if (info.distance <= 0x400000)
						item->goalAnimState = 2;
					else
						item->goalAnimState = 3;
				}
				else
				{
					item->goalAnimState = 4;
				}
				break;

			case 2:
				machinegun->maximumTurn = 546;

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (machinegun->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
						item->goalAnimState = 1;
					else
						item->goalAnimState = 6;
				}
				else if (machinegun->mood == ATTACK_MOOD || !info.ahead)
				{
					if (info.distance > 0x400000)
						item->goalAnimState = 3;
				}
				else
				{
					item->goalAnimState = 4;
				}
				break;

			case 3:
				machinegun->maximumTurn = 910;

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (machinegun->mood != ESCAPE_MOOD)
				{
					if (Targetable(item, &info))
					{
						item->goalAnimState = 2;
					}
					else if (machinegun->mood == BORED_MOOD || machinegun->mood == STALK_MOOD)
					{
						item->goalAnimState = 2;
					}
				}
				break;

			case 4:
				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (Targetable(item, &info))
				{
					item->goalAnimState = 5;
				}
				else
				{
					if (machinegun->mood == ATTACK_MOOD)
					{
						item->goalAnimState = 1;
					}
					else if (!info.ahead)
					{
						item->goalAnimState = 1;
					}
				}
				break;

			case 8:
			case 10:
				machinegun->flags = 0;

				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (Targetable(item, &info))
				{
					item->goalAnimState = (item->currentAnimState == 8) ? 5 : 11;
				}
				else
				{
					item->goalAnimState = 1;
				}
				break;

			case 9:
				machinegun->flags = 0;

				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (Targetable(item, &info))
				{
					item->goalAnimState = 6;
				}
				else
				{
					item->goalAnimState = 2;
				}
				break;

			case 5:
			case 11:
				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (machinegun->flags)
				{
					machinegun->flags--;
				}
				else
				{
					ShotLara(item, &info, &workerMachineGun, torso_y, 30);
					// TODO: item->firedWeapon = 1; for MachineGun (state id 6 too)
					machinegun->flags = 5;
				}

				if (item->goalAnimState != 1 && (machinegun->mood == ESCAPE_MOOD || info.distance > 0x900000 || !Targetable(item, &info)))
				{
					item->goalAnimState = 1;
				}
				break;

			case 6:
				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (machinegun->flags)
				{
					machinegun->flags--;
				}
				else
				{
					ShotLara(item, &info, &workerMachineGun, torso_y, 30);
					//item->firedWeapon = 1;
					machinegun->flags = 5;
				}
				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}

void __cdecl WorkerDualGunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* dual;
	AI_INFO info;
	short angle, head_x, head_y, torso_x, torso_y, tilt;

	item = &Items[itemNum];
	dual = (CREATURE_INFO*)item->data;
	angle = head_x = head_y = torso_x = torso_y = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 11)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 32;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 11;
		}
	}
	else if (LaraItem->hitPoints <= 0)
	{
		item->goalAnimState = 2;
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, dual->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
		case 2:
			dual->maximumTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (dual->mood == ATTACK_MOOD || LaraItem->hitPoints > 0)
			{
				if (Targetable(item, &info))
				{
					if (info.distance <= 0x900000)
						item->goalAnimState = 9;
					else
						item->goalAnimState = 3;
				}
				else
				{
					switch (dual->mood)
					{
					case ATTACK_MOOD:
						if (info.distance > 0x19000000 || !info.ahead)
							item->goalAnimState = 4;
						else
							item->goalAnimState = 3;
						break;
					case ESCAPE_MOOD:
						item->goalAnimState = 4;
						break;
					case STALK_MOOD:
						item->goalAnimState = 3;
						break;

					default:
						if (!info.ahead)
							item->goalAnimState = 3;
						break;
					}
				}
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;
		case 3:
			dual->maximumTurn = ANGLE(3);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
				{
					item->goalAnimState = 1;
				}
				else
				{
					if (info.angle >= 0)
						item->goalAnimState = 6;
					else
						item->goalAnimState = 5;
				}
			}

			if (dual->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
			}
			else if (dual->mood == ATTACK_MOOD || dual->mood == STALK_MOOD)
			{
				if (info.distance > 0x19000000 || !info.ahead)
					item->goalAnimState = 4;
			}
			else if (LaraItem->hitPoints > 0)
			{
				if (info.ahead)
					item->goalAnimState = 1;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;
		case 4:
			dual->maximumTurn = ANGLE(6);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			tilt = angle / 4;

			if (Targetable(item, &info))
			{
				if (info.zoneNumber == info.enemyZone)
				{
					if (info.angle >= 0)
						item->goalAnimState = 6;
					else
						item->goalAnimState = 5;
				}
				else
				{
					item->goalAnimState = 3;
				}
			}
			else if (dual->mood == ATTACK_MOOD)
			{
				if (info.ahead && info.distance < 0x19000000)
					item->goalAnimState = 3;
			}
			else if (LaraItem->hitPoints > 0)
			{
				item->goalAnimState = 1;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;
		case 5:
			dual->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 7;
			else
				item->goalAnimState = 3;
			break;
		case 6:
			dual->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 8;
			else
				item->goalAnimState = 3;
			break;
		case 7:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->flags)
			{
				ShotLara(item, &info, &workerDualGunL, torso_y, 50);
				dual->flags = 1;
			}
			break;
		case 8:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->flags)
			{
				ShotLara(item, &info, &workerDualGunR, torso_y, 50);
				dual->flags = 1;
			}
			break;
		case 9:
			dual->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->goalAnimState = 10;
			else
				item->goalAnimState = 1;
			break;
		case 10:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->flags)
			{
				ShotLara(item, &info, &workerDualGunL, torso_y, 50);
				ShotLara(item, &info, &workerDualGunR, torso_y, 50);
				dual->flags = 1;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}

/*
void Flame(DWORD x, int y, DWORD z, int speed, WORD yrot, WORD room_number)
{
	short fx_number;
	short cam_rot;
	FX_INFO* fx;

	fx_number = CreateEffect(room_number);
	if (fx_number != NO_ITEM)
	{
		fx = &Ptr_VetEffects[fx_number];
		fx->pos.x_pos = x;
		fx->pos.y_pos = y;
		fx->pos.z_pos = z;
		fx->room_number = room_number;
		phd_GetVectorAngles(fx->pos.x_pos - Trng.pGlobTomb4->pAdr->Camera.pCameraSrc->CordX,
			fx->pos.y_pos - Trng.pGlobTomb4->pAdr->Camera.pCameraSrc->CordY,
			fx->pos.z_pos - Trng.pGlobTomb4->pAdr->Camera.pCameraSrc->CordZ,
			&cam_rot);
		fx->pos.x_rot = fx->pos.z_rot = 0;
		fx->pos.y_rot = cam_rot;
		fx->speed = 200;
		fx->object_number = Utils.getObjects(TR2_DRAGON_FIRE);
		fx->shade = 14 * 256;
		fx->counter = 40;
		Utils.ShootAtLara(fx);
	}
}
*/

// TODO: add flame effect when shooting
void __cdecl WorkerFlamethrower(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* flame;
	AI_INFO info;
	PHD_VECTOR pos;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &Items[itemNum];
	flame = (CREATURE_INFO*)item->data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	// get the exact flame start position
	pos.x = workerFlameThrower.x;
	pos.y = workerFlameThrower.y;
	pos.z = workerFlameThrower.z;
	GetJointAbsPosition(item, &pos, workerFlameThrower.meshNum);

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 19;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 7;
		}
	}
	else
	{
		if (item->currentAnimState != 5 && item->currentAnimState != 6)
		{
			//TriggerDynamic(pos.RelX, pos.RelY, pos.RelZ, (GetRandomControl() & 4) + 10, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
			AddFire(pos.x, pos.y, pos.z, 0, item->roomNumber, 0);
		}
		else
		{
			//TriggerDynamic(pos.RelX, pos.RelY, pos.RelZ, (GetRandomControl() & 4) + 14, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
		}

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, flame->maximumTurn);

		switch (item->currentAnimState)
		{
			case 1:
				flame->flags = 0;
				flame->maximumTurn = 0;

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (flame->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(WALL_SIZE*4) || info.zoneNumber != info.enemyZone)
						item->goalAnimState = 8;
					else
						item->goalAnimState = 2;
				}
				else if (flame->mood == ATTACK_MOOD || !info.ahead)
				{
					if (info.distance <= SQUARE(WALL_SIZE*2))
						item->goalAnimState = 2;
					else
						item->goalAnimState = 3;
				}
				else
				{
					item->goalAnimState = 4;
				}
				break;

			case 2:
				flame->maximumTurn = ANGLE(5);

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (flame->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(WALL_SIZE*4) || info.zoneNumber != info.enemyZone)
						item->goalAnimState = 1;
					else
						item->goalAnimState = 6;
				}
				else if (flame->mood == ATTACK_MOOD || !info.ahead)
				{
					if (info.distance > SQUARE(WALL_SIZE*2))
						item->goalAnimState = 3;
				}
				else
				{
					item->goalAnimState = 4;
				}
				break;

			case 3:
				flame->maximumTurn = ANGLE(10);

				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (flame->mood != ESCAPE_MOOD)
				{
					if (Targetable(item, &info))
					{
						item->goalAnimState = 2;
					}
					else if (flame->mood == BORED_MOOD || flame->mood == STALK_MOOD)
					{
						item->goalAnimState = 2;
					}
				}
				break;

			case 4:
				if (info.ahead)
				{
					head_y = info.angle;
					head_x = info.xAngle;
				}

				if (Targetable(item, &info))
				{
					item->goalAnimState = 5;
				}
				else
				{
					if (flame->mood == ATTACK_MOOD)
					{
						item->goalAnimState = 1;
					}
					else if (!info.ahead)
					{
						item->goalAnimState = 1;
					}
				}
				break;

			case 5:
			case 6:
				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;

					//CreatureEffect(item, &workerFlameThrower, Flame);
				}

				if (item->goalAnimState != 1 && (flame->mood == ESCAPE_MOOD || info.distance > SQUARE(WALL_SIZE*10) || !Targetable(item, &info)))
				{
					item->goalAnimState = 1;
				}
				break;

			case 8:
			case 9:
				flame->flags = 0;

				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
				}

				if (Targetable(item, &info))
				{
					item->goalAnimState = (item->currentAnimState == 8) ? 5 : 11;
				}
				else
				{
					item->goalAnimState = 1;
				}
				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}

void __cdecl InitialiseWorkerFlamethrower(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 12;

	InitialiseCreature(itemNum);

	anim = &Anims[item->animNumber];
	item->frameNumber = anim->frameBase;
	item->currentAnimState = anim->currentAnimState;
}