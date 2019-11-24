#include "newobjects.h"
#include "../Global/global.h"
#include "../Game/box.h"
#include "../Game/effect2.h"
#include "../Game/items.h"
#include "../Game/lot.h"
#include "../Game/effects.h"
#include "../Game/draw.h"

BITE_INFO swordBite = { 0, 37, 550, 15 };

// for TR2 and TR3 statue (compatible by any statue entity)
// the statue object need to be after the normal one:
// ex: ID_SWORD_GUARDIAN: 256, ID_SWORD_GUARDIAN_STATUE: 257
void __cdecl DrawStatue(ITEM_INFO* item)
{
	OBJECT_INFO* obj;
	CREATURE_INFO* creature;
	int* bones;
	int clip, i, poppush, frac, rate, bit;
	__int16* frames[2];
	__int16* extra_rotation;
	__int16* rotation1, *rotation2;
	__int16** normal, **statue;

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

		extra_rotation = (__int16*)item->data;

		// all entity have the statue slot after it, (ex: ID_SWORD_GUARDIAN: 256, ID_SWORD_GUARDIAN_STATUE: 257)
		normal = &Meshes[Objects[ID_SAS].meshIndex];
		statue = &Meshes[Objects[ID_SWAT].meshIndex];
		bones = Bones + obj->boneIndex;
		bit = 1;

		if (!frac)
		{
			phd_TranslateRel((int)*(frames[0] + 6), (int)*(frames[0] + 7), (int)*(frames[0] + 8)); // can be [0][6] etc.. ?
			rotation1 = (__int16*)(frames[0] + 9);
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

	PhdRight = PhdWidth;
	PhdLeft = 0;
	PhdTop = 0;
	PhdBottom = PhdHeight;
	phd_PopMatrix();
	phd_PopDxMatrix();
}

void __cdecl InitialiseSwordGuardian(__int16 itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex;
	
	InitialiseCreature(itemNum);

	anim = &Anims[item->animNumber];
	item->frameNumber = anim->frameBase;
	item->currentAnimState = anim->currentAnimState;
	//item->status |= ITEM_INACTIVE;
	item->status &= ~(ITEM_INVISIBLE);
	item->meshBits = 0;
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

void __cdecl SwordGuardianControl(__int16 itemNum)
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
	lara_alive = LaraItem->hitPoints > 0;

	if (item->hitPoints <= 0)
	{
		item->meshBits >>= 1;
		item->currentAnimState = 12;

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
			item->meshBits = 0xffffffff;

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

void __cdecl InitialiseSpearGuardian(__int16 itemNum)
{

}

void __cdecl SpearGuardianControl(__int16 itemNum)
{

}

void __cdecl InitialiseShivaGuardian(__int16 itemNum)
{

}

void __cdecl ShivaGuardianControl(__int16 itemNum)
{

}