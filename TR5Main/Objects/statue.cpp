#include "newobjects.h"
#include "../Game/box.h"
#include "../Game/effect2.h"
#include "../Game/items.h"
#include "../Game/lot.h"
#include "../Game/effects.h"
#include "../Game/draw.h"
#include "../Game/sphere.h"

BITE_INFO swordBite = { 0, 37, 550, 15 };
BITE_INFO spearLeftBite = { 0, 0, 920, 11 };
BITE_INFO spearRightBite = { 0, 0, 920, 18 };
BITE_INFO shivaLeftBite = { 0, 0, 920, 13 };
BITE_INFO shivaRightBite = { 0, 0, 920, 22 };

// TODO: finish the statue render (drawanimatingitem not work anymore with the new render)
// TODO: crash with the explosion death !

// for TR2 and TR3 statue (compatible by any statue entity)
// the statue object need to be after the normal one:
// ex: ID_SWORD_GUARDIAN: 256, ID_SWORD_GUARDIAN_STATUE: 257
void __cdecl DrawStatue(ITEM_INFO* item)
{
	/*
	OBJECT_INFO* obj;
	CREATURE_INFO* creature;
	int* bones;
	int clip, i, poppush, frac, rate, bit;
	short* frames[2];
	short* extra_rotation;
	short* rotation1, *rotation2;
	short** normal, **statue;

	creature = (CREATURE_INFO*)item->data;
	frac = GetFrame_D2(item, frames, &rate);

	if (item->hitPoints <= 0 && item->status != ITEM_ACTIVE && item->meshBits != 0)
		item->meshBits = item->meshBits >> 1;

	obj = &Objects[item->objectNumber];
	//if (obj->shadowSize)
	//	S_PrintShadow(obj->shadowSize, frames[0], item, NULL);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.xPos, item->pos.yPos, item->pos.zPos);
	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);

	if (phd_ClipBoundingBox(frames[0]))
	{
		CalculateObjectLighting(item, frames[0]);

		if (item->data == NULL)
			extra_rotation = NullRotations;
		else
			extra_rotation = (short*)item->data;

		// all entity have the statue slot after it, (ex: ID_SWORD_GUARDIAN: 256, ID_SWORD_GUARDIAN_STATUE: 257)
		normal = &Meshes[Objects[obj->objectNumber].meshIndex];
		statue = &Meshes[Objects[obj->objectNumber + 1].meshIndex];
		bones = &Bones[obj->boneIndex];
		bit = 1;

		if (!frac)
		{
			phd_TranslateRel((int)*(frames[0] + 6), (int)*(frames[0] + 7), (int)*(frames[0] + 8)); // can be [0][6] etc.. ?
			rotation1 = (short*)(frames[0] + 9);
			gar_RotYXZsuperpack(&rotation1, 0);
			
			if (item->meshBits & bit)
				phd_PutPolygons(*normal);
			else
				phd_PutPolygons(*statue);
			normal++;
			statue++;

			for (i = (obj->nmeshes - 1); i > 0; i--, bones += 4, normal++, statue++)
			{
				poppush = *bones;

				if (poppush & 1)
				{
					phd_PopMatrix();
					phd_PopDxMatrix();
				}

				if (poppush & 2)
					phd_PushMatrix();

				phd_TranslateRel(*(bones + 1), *(bones + 2), *(bones + 3));
				gar_RotYXZsuperpack(&rotation1, 0);

				if (extra_rotation && (poppush & (ROT_X|ROT_Y|ROT_Z)))
				{
					if (poppush & ROT_Y)
						phd_RotY(*(extra_rotation++));
					if (poppush & ROT_X)
						phd_RotX(*(extra_rotation++));
					if (poppush & ROT_Z)
						phd_RotZ(*(extra_rotation++));
				}

				bit <<= 1;
				if (item->meshBits & bit)
					phd_PutPolygons(*normal);
				else
					phd_PutPolygons(*statue);
			}
		}
		else
		{
			InitInterpolate(frac, rate);
			phd_TranslateRel_ID((int)*(frames[0] + 6), (int)*(frames[0] + 7), (int)*(frames[0] + 8),
				                (int)*(frames[1] + 6), (int)*(frames[1] + 7), (int)*(frames[1] + 8));
			rotation1 = (short*)(frames[0] + 9);
			rotation2 = (short*)(frames[1] + 9);
			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

			if (item->meshBits & bit)
				phd_PutPolygons_I(*normal);
			else
				phd_PutPolygons_I(*statue);
			normal++;
			statue++;

			for (i = (obj->nmeshes - 1); i > 0; i--, bones += 4, normal++, statue++)
			{
				poppush = *bones;
				if (poppush & 1)
					phd_PopMatrix_I();

				if (poppush & 2)
					phd_PushMatrix_I();

				phd_TranslateRel_I(*(bones + 1), *(bones + 2), *(bones + 3));
				gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

				if (extra_rotation && (poppush & (ROT_X|ROT_Y|ROT_Z)))
				{
					if (poppush & ROT_Y)
						phd_RotY_I(*(extra_rotation++));
					if (poppush & ROT_X)
						phd_RotX_I(*(extra_rotation++));
					if (poppush & ROT_Z)
						phd_RotZ_I(*(extra_rotation++));
				}

				bit <<= 1;
				if (item->meshBits & bit)
					phd_PutPolygons_I(*normal);
				else
					phd_PutPolygons_I(*statue);
			}
		}
	}

	//PhdRight = PhdWidth;
	//PhdLeft = 0;
	//PhdTop = 0;
	//PhdBottom = PhdHeight;
	phd_PopMatrix();
	phd_PopDxMatrix();
	*/
}

void __cdecl InitialiseSwordGuardian(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	item = &Items[itemNum];
	
	InitialiseCreature(itemNum);

	//item->status = ITEM_INACTIVE;
	//item->meshBits = 0;
}

void __cdecl SwordGuardianFly(ITEM_INFO* item)
{
	PHD_VECTOR pos;

	pos.x = (GetRandomControl() << 8 >> 15) + item->pos.xPos - 128;
	pos.y = (GetRandomControl() << 8 >> 15) + item->pos.yPos - 256;
	pos.z = (GetRandomControl() << 8 >> 15) + item->pos.zPos - 128;

	TriggerGunSmoke(pos.x, pos.y, pos.z, 1, 1, 1, 1, WEAPON_GRENADE_LAUNCHER, 32);
	SoundEffect(SFX_SWORD_GUARDIAN_FLYING_ID312, &item->pos, 0);
}

void __cdecl SwordGuardianControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* sword;
	AI_INFO info;
	short angle, head, torso;
	bool lara_alive;

	item = &Items[itemNum];
	sword = (CREATURE_INFO*)item->data;
	angle = head = torso = 0;
	lara_alive = (LaraItem->hitPoints > 0);

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 12)
		{
			//item->meshBits >>= 1;
			SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, 0);
			SoundEffect(SFX_EXPLOSION2, &LaraItem->pos, 0);
			//item->meshBits = 0xFFFFFFFF;
			//item->objectNumber = ID_SAS;
			ExplodingDeath(itemNum, -1, 256);
			//item->objectNumber = ID_SWAT;
			DisableBaddieAI(itemNum);
			KillItem(itemNum);
			//item->status = ITEM_DEACTIVATED;
			//item->flags |= ONESHOT;
			item->currentAnimState = 12;

			/*
			if (!item->meshBits)
			{
				SoundEffect(105, NULL, 0);
				item->meshBits = 0xFFFFFFFF;
				item->objectNumber = ID_SAS;
				ExplodingDeath(itemNum, -1, 256);
				item->objectNumber = ID_SWAT;
				DisableBaddieAI(itemNum);
				KillItem(itemNum);
				item->status = ITEM_DEACTIVATED;
				item->flags |= ONESHOT;
			}
			*/
		}
		return;
	}
	else
	{
		/* Get ground based information */
		sword->LOT.step = STEP_SIZE;
		sword->LOT.drop = -STEP_SIZE;
		sword->LOT.fly = NO_FLYING;
		sword->LOT.zone = 1;
		CreatureAIInfo(item, &info);

		if (item->currentAnimState == 8)
		{
			/* If flying and not in same zone, then use fly zone */
			if (info.zoneNumber != info.enemyZone)
			{
				sword->LOT.step = WALL_SIZE * 20;
				sword->LOT.drop = -WALL_SIZE * 20;
				sword->LOT.fly = STEP_SIZE / 4;
				sword->LOT.zone = 4;
				CreatureAIInfo(item, &info);
			}
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, sword->maximumTurn);

		if (item->currentAnimState != 9) // for reload
			item->meshBits = 0xFFFFFFFF;

		switch (item->currentAnimState)
		{
		case 9:
			sword->maximumTurn = 0;

			if (!sword->flags)
			{
				item->meshBits = (item->meshBits << 1) + 1;
				sword->flags = 3;
			}
			else
			{
				sword->flags--;
			}
			break;

		case 1:
			sword->maximumTurn = 0;

			if (info.ahead)
				head = info.angle;

			if (lara_alive)
			{
				if (info.bite && info.distance < 0x100000)
				{
					if (GetRandomControl() >= 0x4000)
						item->goalAnimState = 5;
					else
						item->goalAnimState = 3;
				}
				else
				{
					if (info.zoneNumber == info.enemyZone)
						item->goalAnimState = 2;
					else
						item->goalAnimState = 8;
				}
			}
			else
			{
				item->goalAnimState = 7;
			}
			break;

		case 2:
			sword->maximumTurn = ANGLE(9);

			if (info.ahead)
				head = info.angle;

			if (lara_alive)
			{
				if (info.bite && info.distance < 0x400000)
					item->goalAnimState = 10;
				else if (info.zoneNumber != info.enemyZone)
					item->goalAnimState = 1;
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;

		case 3:
			sword->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (!info.bite || info.distance > 0x100000)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 4;
			break;

		case 5:
			sword->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (!info.bite || info.distance > 0x100000)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 6;
			break;

		case 10:
			sword->flags = 0;

			if (info.ahead)
				torso = info.angle;

			if (!info.bite || info.distance > 0x400000)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 11;
			break;

		case 8:
			sword->maximumTurn = ANGLE(7);

			if (info.ahead)
				head = info.angle;

			SwordGuardianFly(item);

			if (!sword->LOT.fly)
				item->goalAnimState = 1;
			break;

		case 4:
		case 6:
		case 11:
			if (info.ahead)
				torso = info.angle;

			if (!sword->flags && (item->touchBits & 0xC000))
			{
				LaraItem->hitPoints -= 300;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &swordBite, DoBloodSplat);
				sword->flags = 1;
			}
			break;
		}
	}

	if (item->hitPoints > 0)
	{
		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNum, angle, 0);
	}
}

void __cdecl XianDamage(ITEM_INFO* item, CREATURE_INFO* xian, int damage)
{
	if (!(xian->flags & 1) && (item->touchBits & 0x40000))
	{
		LaraItem->hitPoints -= damage;
		LaraItem->hitStatus = true;
		CreatureEffect(item, &spearRightBite, DoBloodSplat);
		xian->flags |= 1;
		SoundEffect(SFX_SWORD_HITTARGET_ID318, &item->pos, 0);
	}

	if (!(xian->flags & 2) && (item->touchBits & 0x800))
	{
		LaraItem->hitPoints -= damage;
		LaraItem->hitStatus = true;
		CreatureEffect(item, &spearLeftBite, DoBloodSplat);
		xian->flags |= 2;
		SoundEffect(SFX_SWORD_HITTARGET_ID318, &item->pos, 0);
	}
}

void __cdecl InitialiseSpearGuardian(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	InitialiseCreature(itemNum);

	item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 48;

	anim = &Anims[item->animNumber];

	item->frameNumber = anim->frameBase;
	item->currentAnimState = anim->currentAnimState;
	
	//item->status = ITEM_INACTIVE;
	//item->meshBits = 0;
}

void __cdecl SpearGuardianControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* xian;
	short angle, head, neck, tilt;
	int random, lara_alive;
	AI_INFO info;

	item = &Items[itemNum];
	xian = (CREATURE_INFO*)item->data;
	head = neck = angle = tilt = 0;
	lara_alive = (LaraItem->hitPoints > 0);

	if (item->hitPoints <= 0)
	{
		item->currentAnimState = 17;
		item->meshBits >>= 1;

		if (!item->meshBits)
		{
			SoundEffect(105, NULL, 0);
			/*
			item->meshBits = 0xffffffff;
			item->objectNumber = ID_SPEAR_GUARDIAN_STATUE; // just to fool ExplodingDeath to produce jade chunks
			ExplodingDeath(itemNum, 0xffffffff, 0);
			item->objectNumber = ID_SPEAR_GUARDIAN;
			DisableBaddieAI(itemNum);
			KillItem(itemNum);
			item->status = ITEM_DEACTIVATED;
			item->flags |= ONESHOT;
			*/
		}
		return;
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, xian->maximumTurn);

		if (item->currentAnimState != 18) // for reload
			item->meshBits = 0xFFFFFFFF;

		switch (item->currentAnimState)
		{
		case 18:
			/* Make jade man come to life!! */
			if (!xian->flags)
			{
				item->meshBits = (item->meshBits << 1) + 1;
				xian->flags = 3;
			}
			else
				xian->flags--;
			break;

		case 1:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = 0;

			if (xian->mood == BORED_MOOD)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->goalAnimState = 2;
				else if (random < 0x400)
					item->goalAnimState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 5;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = 0;

			if (xian->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (xian->mood == BORED_MOOD)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->goalAnimState = 1;
				else if (random < 0x400)
					item->goalAnimState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 13;
			else
				item->goalAnimState = 3;
			break;

		case 3:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = ANGLE(3);

			if (xian->mood == ESCAPE_MOOD)
				item->goalAnimState = 4;
			else if (xian->mood == BORED_MOOD)
			{
				random = GetRandomControl();
				if (random < 0x200)
					item->goalAnimState = 1;
				else if (random < 0x400)
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE*2))
			{
				if (info.distance < SQUARE(WALL_SIZE * 3 / 2))
					item->goalAnimState = 7;
				else if (GetRandomControl() < 0x4000)
					item->goalAnimState = 9;
				else
					item->goalAnimState = 11;
			}
			else if (!info.ahead || info.distance > SQUARE(WALL_SIZE*3))
				item->goalAnimState = 4;
			break;

		case 4:
			if (info.ahead)
				neck = info.angle;

			xian->maximumTurn = ANGLE(5);

			if (xian->mood == ESCAPE_MOOD)
				break;
			else if (xian->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE*2)) // the only way he'll ever break out of a run is to attack Lara
				item->goalAnimState = 15;
			break;

		case 5:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE))
				item->goalAnimState = 1;
			else
				item->goalAnimState = 6;
			break;

		case 7:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 3 / 2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 8;
			break;

		case 9:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE*2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 8;
			break;

		case 11:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE*2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 8;
			break;

		case 13:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE))
				item->goalAnimState = 2;
			else
				item->goalAnimState = 14;
			break;

		case 15:
			if (info.ahead)
				head = info.angle;

			xian->flags = 0;
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 4;
			else
				item->goalAnimState = 16;
			break;

		case 6:
			XianDamage(item, xian, 75);
			break;

		case 8:
		case 10:
		case 12:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 75);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 2;
			}
			else
				item->goalAnimState = 3;
			break;

		case 14:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 75);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 1;
			else
				item->goalAnimState = 2;
			break;

		case 16:
			if (info.ahead)
				head = info.angle;

			XianDamage(item, xian, 120);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 3;
			else
				item->goalAnimState = 4;
			break;
		}
	}

	if (lara_alive && LaraItem->hitPoints <= 0)
	{
		CreatureKill(item, 49, 19, 2); // uses EXTRA_YETIKILL slot
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(itemNum, angle, tilt);
}

void __cdecl TriggerShivaSmoke(long x, long y, long z, long uw)
{
	long size;
	SPARKS* sptr;
	long dx, dz;

	dx = LaraItem->pos.xPos - x;
	dz = LaraItem->pos.zPos - z;

	if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
		return;

	sptr = &Sparks[GetFreeSpark()];

	sptr->on = 1;
	if (uw)
	{
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;
		sptr->dR = 192;
		sptr->dG = 192;
		sptr->dB = 208;
	}
	else
	{
		sptr->sR = 144;
		sptr->sG = 144;
		sptr->sB = 144;
		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;
	}

	sptr->colFadeSpeed = 8;
	sptr->fadeToBlack = 64;
	sptr->sLife = sptr->life = (GetRandomControl() & 31) + 96;

	if (uw)
		sptr->transType = 2;
	else
		sptr->transType = 2;

	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x + (GetRandomControl() & 31) - 16;
	sptr->y = y + (GetRandomControl() & 31) - 16;
	sptr->z = z + (GetRandomControl() & 31) - 16;
	sptr->xVel = ((GetRandomControl() & 4095) - 2048) >> 2;
	sptr->yVel = (GetRandomControl() & 255) - 128;
	sptr->zVel = ((GetRandomControl() & 4095) - 2048) >> 2;

	if (uw)
	{
		sptr->yVel >>= 4;
		sptr->y += 32;
		sptr->friction = 4 | (1 << 4);
	}
	else
	{
		sptr->friction = 6;
	}

	sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	sptr->rotAng = GetRandomControl() & 4095;
	if (GetRandomControl() & 1)
		sptr->rotAdd = -(GetRandomControl() & 15) - 16;
	else
		sptr->rotAdd = (GetRandomControl() & 15) + 16;

	sptr->scalar = 3;
	if (uw)
	{
		sptr->gravity = sptr->maxYvel = 0;
	}
	else
	{
		sptr->gravity = -(GetRandomControl() & 3) - 3;
		sptr->maxYvel = -(GetRandomControl() & 3) - 4;
	}
	size = (GetRandomControl() & 31) + 128;
	sptr->size = sptr->sSize = size >> 2;
	sptr->dSize = size;
	size += (GetRandomControl() & 31) + 32;
	sptr->size = sptr->sSize = size >> 3;
	sptr->dSize = size;
}

void __cdecl ShivaDamage(ITEM_INFO* item, CREATURE_INFO* shiva, int damage)
{
	if (!(shiva->flags) && (item->touchBits & 0x2400000))
	{
		LaraItem->hitPoints -= damage;
		LaraItem->hitStatus = true;
		CreatureEffect(item, &shivaRightBite, DoBloodSplat);
		shiva->flags = 1;
		SoundEffect(SFX_SWORD_HITTARGET_ID318, &item->pos, 0);
	}

	if (!(shiva->flags) && (item->touchBits & 0x2400))
	{
		LaraItem->hitPoints -= damage;
		LaraItem->hitStatus = true;
		CreatureEffect(item, &shivaLeftBite, DoBloodSplat);
		shiva->flags = 1;
		SoundEffect(SFX_SWORD_HITTARGET_ID318, &item->pos, 0);
	}
}

void __cdecl InitialiseShiva(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	InitialiseCreature(itemNum);

	item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 14;

	anim = &Anims[item->animNumber];

	item->frameNumber = anim->frameBase;
	item->currentAnimState = anim->currentAnimState;
	//item->status = ITEM_INACTIVE;
	//item->meshBits = 0;
}

void __cdecl ShivaControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* shiva;
	short angle, head_x, head_y, torso_x, torso_y, tilt, room_number;
	int x, z;
	int random, lara_alive;
	AI_INFO info;
	PHD_VECTOR	pos;
	FLOOR_INFO* floor;
	int effect_mesh = 0;

	item = &Items[itemNum];
	shiva = (CREATURE_INFO*)item->data;
	head_x = head_y = torso_x = torso_y = angle = tilt = 0;
	lara_alive = (LaraItem->hitPoints > 0);
	pos.x = 0;             // Copy Offsets from mesh
	pos.y = 0;             // Pivot
	pos.z = 256;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 22;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (shiva->mood == ESCAPE_MOOD)
		{
			shiva->target.x = LaraItem->pos.xPos;
			shiva->target.z = LaraItem->pos.zPos;
		}

		angle = CreatureTurn(item, shiva->maximumTurn);

		if (item->currentAnimState != 4)
			item->meshBits = 0xFFFFFFFF;

		switch (item->currentAnimState)
		{
			case 4:
				shiva->maximumTurn = 0;

				if (!shiva->flags)
				{
					if (item->meshBits == 0)
						effect_mesh = 0;
					item->meshBits = (item->meshBits << 1) + 1;
					shiva->flags = 1;

					GetJointAbsPosition(item, &pos, effect_mesh++);
					TriggerExplosionSparks(pos.x, pos.y, pos.z, 2, 0, 0, item->roomNumber);
					TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);

				}
				else
				{
					shiva->flags--;
				}

				if (item->meshBits == 0x7FFFFFFF)
				{
					item->goalAnimState = 0;
					effect_mesh = 0;
					shiva->flags = -45; //set up the delay before actually moving
				}
				break;

			case 0:
				if (info.ahead)
					head_y = info.angle;

				if (shiva->flags < 0)   //delay before moving
				{
					shiva->flags++;
					TriggerShivaSmoke(item->pos.xPos + (GetRandomControl() & 0x5FF) - 0x300, pos.y - (GetRandomControl() & 0x5FF), item->pos.zPos + (GetRandomControl() & 0x5FF) - 0x300, 1);
					break;
				}

				if (shiva->flags == 1)
					shiva->flags = 0;

				shiva->maximumTurn = 0;

				if (shiva->mood == ESCAPE_MOOD)
				{
					room_number = item->roomNumber;
					x = item->pos.xPos + (WALL_SIZE * SIN(item->pos.yRot + 0x8000) >> W2V_SHIFT);
					z = item->pos.zPos + (WALL_SIZE * COS(item->pos.yRot + 0x8000) >> W2V_SHIFT);
					floor = GetFloor(x, item->pos.yPos, z, &room_number);

					if (!shiva->flags && floor->box != NO_BOX && !(Boxes[floor->box].overlapIndex & BLOCKABLE))
						item->goalAnimState = 8;
					else
						item->goalAnimState = 2;
				}
				else if (shiva->mood == BORED_MOOD)
				{
					random = GetRandomControl();
					if (random < 0x400)
						item->goalAnimState = 1;
				}
				else if (info.bite && info.distance < SQUARE(WALL_SIZE * 5 / 4))
				{
					item->goalAnimState = 5;
					shiva->flags = 0;
				}
				else if (info.bite && info.distance < SQUARE(WALL_SIZE * 4 / 3))
				{
					item->goalAnimState = 7;
					shiva->flags = 0;
				}
				else if (item->hitStatus && info.ahead)
				{
					shiva->flags = 4;
					item->goalAnimState = 2;
				}
				else
				{
					item->goalAnimState = 1;
				}
				break;

			case 2:
				if (info.ahead)
					head_y = info.angle;

				shiva->maximumTurn = 0;
				if (item->hitStatus || shiva->mood == ESCAPE_MOOD)
					shiva->flags = 4;

				if ((info.bite && info.distance < SQUARE(WALL_SIZE * 4 / 3)) || (item->frameNumber == Anims[item->animNumber].frameBase && !shiva->flags) || !info.ahead)
				{
					item->goalAnimState = 0;
					shiva->flags = 0;
				}
				else if (shiva->flags)
				{
					item->goalAnimState = 2;
				}

				if (item->frameNumber == Anims[item->animNumber].frameBase && shiva->flags > 1)
					shiva->flags -= 2;
				break;

			case 1:
				if (info.ahead)
					head_y = info.angle;

				shiva->maximumTurn = ANGLE(4);

				if (shiva->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 0;
				}
				else if (shiva->mood == BORED_MOOD)
				{
					item->goalAnimState = 0;
				}
				else if (info.bite && info.distance < SQUARE(WALL_SIZE * 4 / 3))
				{
					item->goalAnimState = 0;
					shiva->flags = 0;
				}
				else if (item->hitStatus)
				{
					shiva->flags = 4;
					item->goalAnimState = 3;
				}
				break;

			case 3:
				if (info.ahead)
					head_y = info.angle;

				shiva->maximumTurn = ANGLE(4);

				if (item->hitStatus)
					shiva->flags = 4;

				if ((info.bite && info.distance < SQUARE(WALL_SIZE * 5 / 4)) || (item->frameNumber == Anims[item->animNumber].frameBase && !shiva->flags))
				{
					item->goalAnimState = 1;
					shiva->flags = 0;
				}
				else if (shiva->flags)
				{
					item->goalAnimState = 3;
				}
				
				if (item->frameNumber == Anims[item->animNumber].frameBase)
				{
					shiva->flags = 0;
				}
				break;

			case 8:
				if (info.ahead)
					head_y = info.angle;

				shiva->maximumTurn = ANGLE(4);
				if ((info.ahead && info.distance < SQUARE(WALL_SIZE * 4 / 3)) || (item->frameNumber == Anims[item->animNumber].frameBase && !shiva->flags))
				{
					item->goalAnimState = 0;
				}
				else if (item->hitStatus)
				{
					shiva->flags = 4;
					item->goalAnimState = 0;
				}
				break;


			case 5:
				if (info.ahead)
				{
					torso_y = info.angle;
					torso_x = info.xAngle;
					head_y = info.angle;
				}

				shiva->maximumTurn = ANGLE(4);

				ShivaDamage(item, shiva, 150);
				break;

			case 7:
				torso_y = info.angle;
				head_y = info.angle;
				if (info.xAngle > 0)
					torso_x = info.xAngle;

				shiva->maximumTurn = ANGLE(4);

				ShivaDamage(item, shiva, 180);
				break;

			case 6:
				torso_y = torso_x = head_x = head_y = shiva->maximumTurn = 0;
				if (item->frameNumber == Anims[item->animNumber].frameBase + 10 || item->frameNumber == Anims[item->animNumber].frameBase + 21 || item->frameNumber == Anims[item->animNumber].frameBase + 33)
				{
					CreatureEffect(item, &shivaRightBite, DoBloodSplat);
					CreatureEffect(item, &shivaLeftBite, DoBloodSplat);
				}
				break;
		}
	}

	if (lara_alive && LaraItem->hitPoints <= 0)
	{
		CreatureKill(item, 18, 6, 2); // uses EXTRA_YETIKILL slot
		return;
	}

	CreatureTilt(item, tilt);
	head_y -= torso_y;
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}