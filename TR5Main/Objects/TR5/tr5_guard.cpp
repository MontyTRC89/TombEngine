#include "../newobjects.h"
#include "../../Game/items.h"
#include "../../Game/sphere.h"
#include "../../Game/Box.h"
#include "../../Game/effect2.h"
#include "../../Game/people.h"
#include "../../Game/draw.h"

BITE_INFO swatGun = { 0x50, 0xC8, 0x0D, 0 };

void InitialiseGuard(short itemNum)
{
    ITEM_INFO* item, *item2;
    short anim;
    short roomItemNumber;

    item = &Items[itemNum];
    ClearItem(itemNum);
    anim = Objects[ID_SWAT].animIndex;
    if (!Objects[ID_SWAT].loaded)
        anim = Objects[ID_BLUE_GUARD].animIndex;

    switch (item->triggerFlags)
    {
        case 0:
        case 10:
            item->animNumber = anim;
            item->goalAnimState = 1;
            break;
        case 1:
            item->goalAnimState = 11;
            item->animNumber = anim + 23;
            break;
        case 2:
            item->goalAnimState = 13;
            item->animNumber = anim + 25;
            // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
            break;
        case 3:
            item->goalAnimState = 15;
            item->animNumber = anim + 28;
            *item->pad2 = 9216;
            roomItemNumber = Rooms[item->roomNumber].itemNumber;
            if (roomItemNumber != NO_ITEM)
            {
                while (true)
                {
                    item2 = &Items[roomItemNumber];
                    if (item2->objectNumber >= ID_ANIMATING1 && item2->objectNumber <= ID_ANIMATING15 && item2->roomNumber == item->roomNumber && item2->triggerFlags == 3)
                        break;
                    roomItemNumber = item2->nextItem;
                    if (roomItemNumber == NO_ITEM)
                    {
                        item->frameNumber = Anims[item->animNumber].frameBase;
                        item->currentAnimState = item->goalAnimState;
                        break;
                    }
                }
                item2->meshBits = -5;
            }
            break;
        case 4:
            item->goalAnimState = 17;
            *item->pad2 = 8192;
            item->animNumber = anim + 30;
            break;
        case 5:
            FLOOR_INFO *floor;
            short roomNumber;

            item->animNumber = anim + 26;
            item->goalAnimState = 14;
            roomNumber = item->roomNumber;
            floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
            GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
            item->pos.yPos = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) - SECTOR(2);
            break;
        case 6:
            item->goalAnimState = 19;
            item->animNumber = anim + 32;
            break;
        case 7:
        case 9:
            item->goalAnimState = 38;
            item->animNumber = anim + 59;
            item->pos.xPos -= SIN(item->pos.yRot); // 4 * not exist there ??
            item->pos.zPos -= COS(item->pos.yRot); // 4 * not exist there ??
            break;
        case 8:
            item->goalAnimState = 31;
            item->animNumber = anim + 46;
            break;
        case 11:
            item->goalAnimState = 7;
            item->animNumber = anim + 12;
            break;
        default:
            break;
    }
}

void InitialiseGuardM16(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
    item->pos.yPos += STEP_SIZE*2;
    item->pos.xPos += SIN(item->pos.yRot);
    item->pos.zPos += COS(item->pos.yRot);
}

void InitialiseGuardLaser(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex + 6;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
}

/*
void ControlGuard(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	int animIndex = 0;
	if (Objects[ID_SWAT].loaded)
		animIndex= Objects[ID_SWAT].animIndex;
	else
		animIndex = Objects[ID_BLUE_GUARD].animIndex;

	HIWORD(v2) = HIWORD(items);

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	LOWORD(v2) = item->roomNumber;

	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	int x = item->pos.xPos;
	int z = item->pos.zPos;

	int dx = 870 * SIN(item->pos.yRot) >> W2V_SHIFT;
	int dz = 870 * COS(item->pos.yRot) >> W2V_SHIFT;

	x += dx;
	z += dz;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, item->pos.yPos, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, item->pos.yPos, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, item->pos.yPos, z);

	bool canJump1block;
	if (item->boxNumber == LaraItem->boxNumber
		|| item->pos.yPos >= height1 - 384
		|| item->pos.yPos >= height2 + 256
		|| item->pos.yPos <= height2 - 256)
		canJump1block = false;
	else
		canJump1block = true;

	bool canJump2blocks;
	if (item->boxNumber == LaraItem->boxNumber 
		|| item->pos.yPos >= height1 - 384 
		|| item->pos.yPos >= height2 - 384
		|| item->pos.yPos >= height3 + 256 
		|| item->pos.yPos <= height3 - 256)
		canJump2blocks = false;
	else
		canJump2blocks = true;

	if (item->firedWeapon)
	{
		PHD_VECTOR pos;
		pos.x = swatGun.x;
		pos.y = swatGun.y;
		pos.z = swatGun.z;
		GetJointAbsPosition(item, &pos, swatGun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->firedWeapon + 10, 192, 128, 32);
		item->firedWeapon--;
	}

	if (item->aiBits)
		GetAITarget(creature);
	else
		creature->enemy = LaraItem;

	AI_INFO info;
	AI_INFO laraInfo;

	CreatureAIInfo(item, &info);

	if (creature->enemy == LaraItem)
	{
		laraInfo.angle = info.angle;
		laraInfo.distance = info.distance;
	}
	else
	{
		int dx = LaraItem->pos.xPos - item->pos.xPos;
		int dz = LaraItem->pos.zPos - item->pos.zPos;

		laraInfo.angle = ATAN(dz, dx) - item->pos.yRot;
		laraInfo.distance = SQUARE(dx) + SQUARE(dz);
	}
	
	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 8 && item->currentAnimState != 6)
		{
			if (laraInfo.angle >= 12288 || laraInfo.angle <= -12288)
			{
				item->currentAnimState = 8;
				item->animNumber = animIndex + 16;;
				item->pos.yRot += laraInfo.angle + -ANGLE(180);
			}
			else
			{
				item->currentAnimState = 6;
				item->animNumber = animIndex + 11;
				item->pos.yRot += laraInfo.angle;
			}
			item->frameNumber = Anims[item->animNumber].frameBase;
		}
	}
	else
	{

	}


	GetCreatureMood(item, &info, creature->enemy != LaraItem);

	if (item->objectNumber == ID_SCIENTIST)
	{
		if (item->hitPoints >= Objects[ID_SCIENTIST].hitPoints)
		{
			if (creature->enemy == LaraItem)
				creature->mood = BORED_MOOD;
		}
		else
		{
			creature->mood = ESCAPE_MOOD;
		}
	}

	if (Rooms[item->roomNumber].flags & ENV_FLAG_NO_LENSFLARE) // CHECK
	{
		if (item->objectNumber == ID_SWAT_PLUS)
		{
			item->itemFlags[0]++;
			if (item->itemFlags[0] > 60 && !(GetRandomControl() & 0xF))
			{
				SoundEffect(SFX_BIO_BREATHE_OUT, &item->pos, 0);
				item->itemFlags[0] = 0;
			}
		}
		else
		{
			if (!(GlobalCounter & 7))
				item->hitPoints--;
			creature->mood = ESCAPE_MOOD;
			if (item->hitPoints <= 0)
			{
				item->currentAnimState = 8;
				item->animNumber = animIndex + 16;
				item->frameNumber = Anims[item->animNumber].frameBase;
			}
		}
	}

	CreatureMood(item, &info, creature->enemy != LaraItem);

	ITEM_INFO* enemy = creature->enemy;
	angle = CreatureTurn(item, creature->maximumTurn);
	creature->enemy = LaraItem;

	if (laraInfo.distance < 0x400000 && LaraItem->speed > 20 
		|| item->hitStatus 	
		|| TargetVisible(item, &laraInfo))
	{
		if (!(item->aiBits & FOLLOW) && item->objectNumber != ID_SCIENTIST && abs(item->pos.yPos - LaraItem->pos.yPos) < 1280)
		{
			creature->enemy = LaraItem;
			AlertAllGuards(itemNum);
		}
	}

	creature->enemy = enemy;

	GAME_VECTOR src;
	src.x = item->pos.xPos;
	src.y = item->pos.yPos - 384;
	src.z = item->pos.zPos;

	short* frame = GetBestFrame(LaraItem);
	
	GAME_VECTOR dest;
	dest.x = LaraItem->pos.xPos;
	dest.y = LaraItem->pos.yPos + ((frame[3] + 3 * frame[2]) >> 2);
	dest.z = LaraItem->pos.zPos;

	bool los = !LOS(&src, &dest) && item->triggerFlags != 10;
	
	creature->maximumTurn = 0;

	switch (item->currentAnimState)
	{
	case 1:
		creature->LOT.isJumping = false;
		joint2 = laraInfo.angle;
		creature->flags = 0;

		if (info.ahead)
		{
			if (!(item->aiBits & FOLLOW))
			{
				joint0 = info.angle >> 1;
				joint1 = info.xAngle;
			}
		}

		if (item->objectNumber == ID_SCIENTIST && item == Lara.target)
		{
			item->goalAnimState = 39;
		}
		else if (item->requiredAnimState)
		{
			item->goalAnimState = item->requiredAnimState;
		}
		else if (item->aiBits & GUARD)
		{
			if (item->aiBits & MODIFY)
				joint2 = 0;
			else
				joint2 = AIGuard(creature);

			if (item->aiBits & PATROL1)
			{
				item->triggerFlags--;
				if (item->triggerFlags < 1)
					item->aiBits &= ~GUARD;
			}
		}
		else if (creature->enemy == LaraItem && (laraInfo.angle > 20480 || laraInfo.angle < -20480) && item->objectNumber != ID_SCIENTIST)
		{
			item->goalAnimState = 2;
		}
		else if (item->aiBits & PATROL1)
		{
			item->goalAnimState = 5;
		}
		else if (item->aiBits & AMBUSH)
		{
			item->goalAnimState = 7;
		}
		else if (Targetable(item, &info) && item->objectNumber != ID_SCIENTIST)
		{
			if (info.distance >= 0x1000000 && info.zoneNumber == info.enemyZone)
			{
				if (!(item->aiBits & MODIFY))
					item->goalAnimState = 5;
			}
			else
				item->goalAnimState = 4;
		}
		else if (canJump1block || canJump2blocks)
		{
			creature->maximumTurn = 0;
			item->animNumber = animIndex + 41;
			item->currentAnimState = 26;
			item->frameNumber = Anims[item->animNumber].frameBase;
			if (canJump1block)
				item->goalAnimState = 27;
			else
				item->goalAnimState = 28;
			creature->LOT.isJumping = true;
		}
		else if (los)
		{
			item->goalAnimState = 31;
		}
		else if (creature->mood)
		{
			if (info.distance < 0x900000 || item->aiBits & FOLLOW)
			{
				item->goalAnimState = 5;
			}
			else
				item->goalAnimState = 7;
		}
		else
		{
			item->goalAnimState = 1;
		}

		if (item->triggerFlags == 11)
			item->triggerFlags = 0;
		break;

	case 2:
		creature->flags = 0;
		if (info.angle >= 0)
			item->pos.yRot -= ANGLE(2);
		else
			item->pos.yRot += ANGLE(2);
		if (item->frameNumber == Anims[item->animNumber].frameEnd)
			item->pos.yRot += -ANGLE(180);
		break;

	case 3:
	case 35:
		joint0 = laraInfo.angle >> 1;
		joint2 = laraInfo.angle >> 1;
		if (info.ahead)
			joint1 = info.xAngle;
		if (abs(info.angle) >= ANGLE(2))
		{
			if (info.angle >= 0)
				item->pos.yRot += ANGLE(2);
			else
				item->pos.yRot -= ANGLE(2);
		}
		else
		{
			item->pos.yRot += info.angle;
		}

		if (item->currentAnimState == 35)
		{
			if (creature->flags)
			{
				if (item->frameNumber < Anims[item->animNumber].frameBase + 10 
					&& (item->frameNumber - Anims[item->animNumber].frameBase) & 1)
					creature->flags = 0;
			}
		}

		if (!creature->flags)
		{
			creature->flags = 1;
			item->firedWeapon = 2;
			if (item->currentAnimState == 3)
				ShotLara(item, &info, &swatGun, joint0, 30);
			else
				ShotLara(item, &info, &swatGun, joint0, 10);
		}
		break;

	case 4:
		creature->flags = 0;
		joint0 = laraInfo.angle >> 1;
		joint2 = laraInfo.angle >> 1;
		if (info.ahead)
			joint1 = info.xAngle;
		if (abs(info.angle) >= ANGLE(2))
		{
			if (info.angle >= 0)
				item->pos.yRot += ANGLE(2);
			else
				item->pos.yRot -= ANGLE(2);
		}
		else
		{
			item->pos.yRot += info.angle;
		}

		if (!Targetable(item, &info))
			item->goalAnimState = 1;
		else if (item->objectNumber == ID_BLUE_GUARD || item->objectNumber == ID_CRANE_GUY)
			item->goalAnimState = 3;
		else
			item->goalAnimState = 35;
		break;

	case 5:
		creature->LOT.isJumping = false;
		creature->maximumTurn = ANGLE(5);
		if (!Targetable(item, &info)
			|| info.distance >= 0x1000000 && info.zoneNumber == info.enemyZone
			|| item->objectNumber == ID_SCIENTIST
			|| item->aiBits & AMBUSH || item->aiBits & PATROL1) // CHECK
		{
			if (canJump1block || canJump2blocks)
			{
				creature->maximumTurn = 0;
				item->animNumber = animIndex + 41;
				item->currentAnimState = 26;
				item->frameNumber = Anims[item->animNumber].frameBase;
				if (canJump1block)
					item->goalAnimState = 27;
				else
					item->goalAnimState = 28;
				creature->LOT.isJumping = true;
			}
			else if (info.distance >= 0x100000)
			{
				if (!los || item->aiBits)
				{
					if (info.distance > 0x900000)
					{
						if (!(item->InDrawRoom))
							item->goalAnimState = 7;
					}
				}
				else
				{
					item->goalAnimState = 1;
				}
			}
			else
			{
				item->goalAnimState = 1;
			}
		}
		else
		{
			item->goalAnimState = 4;
		}
		break;

	case 7:
		creature->LOT.isJumping = false;
		creature->maximumTurn = ANGLE(10);
		if (Targetable(item, &info) && (info.distance < 0x1000000 || info.enemyZone == info.zoneNumber) && item->objectNumber != ID_SCIENTIST)
		{
			item->goalAnimState = 4;
		}
		else if (v119 || v117)
		{
			v64 = animIndex;
			creature->maximumTurn = 0;
			v57 = v117 == 0;
			item->animNumber = v64 + 50;
			v65 = Anims[(v64 + 50)].frameBase;
			item->currentAnimState = 26;
			item->frameNumber = v65;
			if (v57)
				item->goalAnimState = 27;
			else
				item->goalAnimState = 28;
			LOBYTE(creature->LOT.flags) |= 8u;
		}
		else if (v45)
		{
			item->goalAnimState = 1;
		}
		else if (info.distance < &unk_900000)
		{
			item->goalAnimState = 5;
		}
		if (item->triggerFlags == 11)
		{
			LOBYTE(creature->LOT.flags) |= 8u;
			creature->maximumTurn = 0;
		}
		goto LABEL_255;
	case 14:
		v68 = item->pos.yPos;
		joint2 = laraInfo.angle;
		v69 = item->floor;
		if (v68 <= item->floor - 2048 || item->triggerFlags != 5)
		{
			if (v68 >= v69 - 512)
				item->goalAnimState = 4;
		}
		else
		{
			LOWORD(v69) = item->roomNumber;
			item->triggerFlags = 0;
			*a4 = v69;
			v70 = GetFloor(*v42, item->pos.yPos, item->pos.zPos, a4);
			GetHeight(v70, *v42, item->pos.yPos, item->pos.zPos);
			TestTriggers(trigger_index, 1, 0);
			SoundEffect(340, &item->pos, 0);
		}
		if (abs(info.angle) >= 364)
		{
			if ((info.angle & 0x8000u) == 0)
				item->pos.yRot += 364;
			else
				item->pos.yRot -= 364;
		}
		else
		{
			item->pos.yRot += info.angle;
		}
		goto LABEL_255;
	case 15:
		LOWORD(v72) = AIGuard(creature);
		joint2 = v72;
		if (creature->bitfield & 1)
			item->goalAnimState = 16;
		goto LABEL_255;
	case 16:
	case 18:
		v74 = Anims[item->animNumber].frameBase;
		v75 = item->frameNumber;
		if (v75 == v74)
			goto LABEL_254;
		if (v75 == v74 + 44)
		{
			v76 = item->roomNumber;
			*item->pad2 = 0;
			v77 = room[v76].item_number;
			if (v77 == -1)
				goto LABEL_255;
			while (1)
			{
				v78 = v77;
				v79 = items[v78].object_number;
				v80 = &items[v78];
				if (v79 >= 416 && v79 <= 444 && v80->roomNumber == v76)
				{
					v81 = v80->triggerFlags;
					if (v81 > 2 && v81 < 5)
						break;
				}
				v77 = v80->next_item;
				if (v77 == -1)
					goto LABEL_255;
			}
			v80->mesh_bits = -3;
		}
		else if (v75 == Anims[item->animNumber].frame_end)
		{
			item->pos.yRot -= 0x4000;
		}
		goto LABEL_255;
	case 17:
		v71 = item->_bf15ea;
		joint2 = 0;
		if (!(v71 & 0x10) && LaraItem->speed < 40 && !(*(&lara + 69) & 0x10))
			creature->bitfield &= 0xFFFEu;
		if (creature->bitfield & 1)
			item->goalAnimState = 18;
		goto LABEL_255;
	case 19:
		LOWORD(v73) = AIGuard(creature);
		joint2 = v73;
		if (creature->bitfield & 1)
			item->goalAnimState = 1;
		goto LABEL_255;
	case 30:
		goto LABEL_176;
	case 31:
		if (item->triggerFlags != 8 || !v45 || item->_bf15ea & 0x10)
			item->goalAnimState = 30;
	LABEL_176:
		creature->LOT.flags &= 0xFFF7u;
		v57 = v119 == 0;
		creature->maximumTurn = 910;
		if (!v57 || v117 || info.distance < 0x100000 || !v45 || item->_bf15ea & 0x10)
			item->goalAnimState = 1;
		goto LABEL_255;
	case 36:
		goto LABEL_253;
	case 37:
		v82 = 0;
		for (i = room[item->roomNumber].item_number; i != -1; i = v82->next_item)
		{
			v82 = &items[i];
			if (items[i].object_number == 249)
				break;
		}
		v84 = item->frameNumber;
		v85 = &Anims[item->animNumber];
		creature2 = v85;
		LOWORD(v85) = HIWORD(v85->aiTarget.floor);
		if (v84 == v85)
		{
			v86 = v82->pos.yRot;
			v82->mesh_bits = 0x1FFF;
			item->pos.yRot = v86;
			item->pos.xPos = v82->pos.xPos - 256;
			v87 = v82->pos.zPos;
			*item->pad2 = 1024;
			item->pos.zPos = v87 + 128;
		}
		else
		{
			v85 = v85;
			v88 = v84;
			if (v84 == v85 + 32)
			{
				v82->mesh_bits = 16381;
			}
			else if (v84 == v85->aiTarget.item_flags)
			{
				v82->mesh_bits = 278461;
			}
			else if (v84 == &v85->aiTarget.pad_ex_light[16])
			{
				v82->mesh_bits = 802621;
			}
			else if (v84 == (&v85->aiTarget.draw_room + 1))
			{
				v82->mesh_bits = 819001;
			}
			else if (v84 == &v85->aiTarget.pad1[30])
			{
				v82->mesh_bits = 17592121;
			}
			else if (v84 == LOWORD(creature2->aiTarget.touch_bits))
			{
				v82->mesh_bits = 0x1FFF;
				LOWORD(v88) = item->roomNumber;
				*a4 = v88;
				v89 = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, a4);
				GetHeight(v89, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				TestTriggers(trigger_index, 1, 0);
				item->requriedAnimState = 5;
				*item->pad2 = 0;
			}
		}
		goto LABEL_255;
	case 38:
		if ((v120 != 69 || item != lara.target)
			&& (GetRandomControl() & 0x7F || (v90 = item->triggerFlags, v90 >= 10) || v90 == 9))
		{
			v91 = item->_bf15ea;
			if (v91 & 0x200)
			{
				LOWORD(v92) = AIGuard(creature);
				joint2 = v92;
				v93 = item->_bf15ea;
				if (v93 & 0x800)
				{
					if (--item->triggerFlags < 1)
					{
						BYTE1(v93) = BYTE1(v93) & 0xC9 | 8;
						item->_bf15ea = v93;
					}
				}
			}
		}
		else
		{
		LABEL_242:
			item->goalAnimState = 1;
		}
		goto LABEL_255;
	case 39:
		if (item != lara.target && !(GetRandomControl() & 0x3F))
		{
			v94 = item->triggerFlags;
			if (v94 == 7 || v94 == 9)
				item->requriedAnimState = 38;
			item->goalAnimState = 1;
		}
	LABEL_253:
		HIWORD(v44) = HIWORD(anims);
		if (item->frameNumber == Anims[item->animNumber].frameBase + 39)
		{
		LABEL_254:
			LOWORD(v44) = item->roomNumber;
			*a4 = v44;
			v95 = GetFloor(*v42, item->pos.yPos, item->pos.zPos, a4);
			GetHeight(v95, *v42, item->pos.yPos, item->pos.zPos);
			TestTriggers(trigger_index, 1, 0);
		}
		goto LABEL_255;
	default:
		goto LABEL_255;
	}


LABEL_255:
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	if (!(creature->bitfield & 8))
		goto LABEL_269;
	v97 = creature->enemy;
	if (!v97)
		goto LABEL_269;
	v98 = v97->flags;
	if (v98 != 4)
	{
		if (v98 & 0x10)
		{
			v99 = item->_bf15ea;
			item->goalAnimState = 1;
			item->requriedAnimState = 38;
			item->triggerFlags = 300;
			BYTE1(v99) &= 0xCBu;
		}
		else
		{
			if (v98 & 0x20)
			{
				v100 = item->_bf15ea;
				item->goalAnimState = 1;
				item->requriedAnimState = 36;
				BYTE1(v100) = BYTE1(v100) & 0xC9 | 8;
				item->_bf15ea = v100;
				goto LABEL_268;
			}
			LOWORD(v96) = v97->roomNumber;
			*a4 = v96;
			v101 = GetFloor(creature->enemy->pos.xPos, creature->enemy->pos.yPos, creature->enemy->pos.zPos, a4);
			GetHeight(v101, creature->enemy->pos.xPos, creature->enemy->pos.yPos, creature->enemy->pos.zPos);
			TestTriggers(trigger_index, 1, 0);
			item->requriedAnimState = 5;
			if (creature->enemy->flags & 2)
				item->item_flags[3] = item->pad2[6] - 1;
			if (!(creature->enemy->flags & 8))
				goto LABEL_268;
			v99 = item->_bf15ea;
			item->requriedAnimState = 1;
			item->triggerFlags = 300;
		}
		BYTE1(v99) |= 0xAu;
		item->_bf15ea = v99;
		goto LABEL_268;
	}
	item->goalAnimState = 1;
	item->requriedAnimState = 37;
LABEL_268:
	++item->item_flags[3];
	creature->bitfield &= 0xFFF7u;
	creature->enemy = 0;
LABEL_269:
	v102 = item->currentAnimState;
	if ((v102 >= 20 || v102 == 6 || v102 == 8) && v102 != 30)
	{
		CreatureAnimation(itemNum, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNum, angle, 2, 256) + 4)
		{
		case 0:
			v113 = animIndex;
			creature->maximumTurn = 0;
			item->animNumber = v113 + 38;
			v114 = Anims[(v113 + 38)].frameBase;
			item->currentAnimState = 23;
			item->frameNumber = v114;
			break;
		case 1:
			v111 = animIndex;
			creature->maximumTurn = 0;
			v111 += 39;
			item->animNumber = v111;
			v112 = Anims[v111].frameBase;
			item->currentAnimState = 24;
			item->frameNumber = v112;
			break;
		case 2:
			v109 = animIndex;
			creature->maximumTurn = 0;
			item->animNumber = v109 + 40;
			v110 = Anims[(v109 + 40)].frameBase;
			item->currentAnimState = 25;
			item->frameNumber = v110;
			break;
		case 6:
			v103 = animIndex;
			creature->maximumTurn = 0;
			item->animNumber = v103 + 35;
			v104 = Anims[(v103 + 35)].frameBase;
			item->currentAnimState = 20;
			item->frameNumber = v104;
			break;
		case 7:
			v105 = animIndex;
			creature->maximumTurn = 0;
			v105 += 36;
			item->animNumber = v105;
			v106 = Anims[v105].frameBase;
			item->currentAnimState = 21;
			item->frameNumber = v106;
			break;
		case 8:
			v107 = animIndex;
			creature->maximumTurn = 0;
			item->animNumber = v107 + 37;
			v108 = Anims[(v107 + 37)].frameBase;
			item->currentAnimState = 22;
			item->frameNumber = v108;
			break;
		default:
			return;
		}
	}
	return;
}*/