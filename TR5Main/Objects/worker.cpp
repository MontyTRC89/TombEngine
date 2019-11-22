#include "newobjects.h"
#include "../Global/global.h"
#include "../Game/box.h"
#include "../Game/people.h"

static BITE_INFO workerShotgun = { 0, 281, 40, 9 };
static BITE_INFO workerMachineGun = { 0, 308, 32, 9 };

static void ShotLara_WithShotgun(ITEM_INFO* item, AI_INFO* info, BITE_INFO* bite, short angle_y, int damage)
{
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
}

void __cdecl InitialiseWorkerShotgun(__int16 itemNum)
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

void __cdecl WorkerShotgunControl(__int16 itemNum)
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

void __cdecl InitialiseWorkerMachineGun(__int16 itemNum)
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

void __cdecl WorkerMachineGunControl(__int16 itemNum)
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