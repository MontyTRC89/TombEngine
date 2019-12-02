#include "swat.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\effect2.h"
#include "..\Game\people.h"
#include "..\Game\Lara.h"

/*
void InitialiseGuard(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	
	ClearItem(itemNum);
	
	short animIndex = Objects[ID_SWAT].animIndex;
	if (!Objects[ID_SWAT].loaded)
		animIndex = Objects[ID_BLUE_GUARD].animIndex;

	switch (item->triggerFlags)
	{
	case 0:
	case 10:
		item->animNumber = animIndex;
		item->goalAnimState = 1;
		break;
	case 1:
		item->goalAnimState = 11;
		item->animNumber = v4 + 23;
		break;
	case 2:
		item->goalAnimState = 13;
		item->animNumber = v5 + 25;
		item->_bf15ea ^= (item->_bf15ea ^ ((item->_bf15ea & 0xFE) + 2)) & 6;
		break;
	case 3:
		v6 = item->roomNumber;
		v7 = itemNum + 28;
		item->goalAnimState = 15;
		item->animNumber = v7;
		*item->pad2 = 9216;
		v8 = Rooms[v6].item_number;
		if (v8 != -1)
		{
			while (1)
			{
				v9 = v8;
				v10 = Items[v9].object_number;
				v11 = &Items[v9];
				if (v10 >= 416 && v10 <= 444 && v11->roomNumber == v6 && v11->triggerFlags == 3)
					break;
				v8 = v11->next_item;
				if (v8 == -1)
					goto LABEL_21;
			}
			v11->meshBits = -5;
		}
		break;
	case 4:
		v12 = itemNum;
		item->goalAnimState = 17;
		*item->pad2 = 0x2000;
		item->animNumber = v12 + 30;
		break;
	case 5:
		LOWORD(v3) = item->roomNumber;
		item->animNumber = itemNum + 26;
		v13 = item->pos.zPos;
		itemNum = v3;
		v14 = item->pos.yPos;
		v15 = item->pos.xPos;
		item->goalAnimState = 14;
		v16 = GetFloor(v15, v14, v13, &itemNum);
		GetFloorHeight(v16, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		item->pos.yPos = GetCeiling(v16, item->pos.xPos, item->pos.yPos, item->pos.zPos) - 2048;
		break;
	case 6:
		v17 = itemNum;
		item->goalAnimState = 19;
		item->animNumber = v17 + 32;
		break;
	case 7:
	case 9:
		v18 = item->pos.xPos;
		v19 = itemNum + 59;
		item->goalAnimState = 38;
		item->animNumber = v19;
		LOWORD(v19) = item->pos.yRot;
		v20 = (v19 >> 3) & 0x1FFE;
		item->pos.xPos = v18 - (rcossin_tbl[v20] << 9 >> 14);
		item->pos.zPos -= rcossin_tbl[v20 + 1] << 9 >> 14;
		break;
	case 8:
		v21 = itemNum;
		item->goalAnimState = 31;
		item->animNumber = v21 + 46;
		break;
	case 11:
		v22 = itemNum;
		item->goalAnimState = 7;
		item->animNumber = v22 + 12;
		break;
	default:
		break;
	}
LABEL_21:
	result = anims;
	v24 = item->goalAnimState;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = v24;
	return result;
}

void ControlGuard(int itemNum)
{
	int v1; // eax
	int v2; // ecx
	ITEM_INFO* item; // esi
	int v4; // eax
	int v5; // edi
	int v6; // ebp
	int v7; // eax
	int v8; // ebx
	int v9; // ecx
	int v10; // edi
	int v11; // ebp
	struct FLOOR_INFO* v12; // eax
	int v13; // eax
	int v14; // ebp
	int v15; // edi
	struct FLOOR_INFO* v16; // eax
	int v17; // eax
	int v18; // edx
	int v19; // edi
	int v20; // ebp
	struct FLOOR_INFO* v21; // eax
	int v22; // eax
	unsigned short v23; // cx
	unsigned short v24; // dx
	int v25; // edi
	int v26; // eax
	CREATURE_INFO* v27; // ebp
	ITEM_INFO* v28; // ecx
	short v29; // ax
	int v30; // edi
	int v31; // ebx
	short v32; // bx
	short v33; // cx
	short v34; // bx
	short v35; // bx
	int v36; // edi
	int v37; // eax
	int v38; // eax
	enum mood_type v39; // edi
	ITEM_INFO* v40; // ecx
	int v41; // eax
	int* v42; // edi
	int v43; // eax
	int v44; // ecx
	_BOOL2 v45; // bx
	short v46; // cx
	int v47; // eax
	int v48; // eax
	int v49; // eax
	short v50; // di
	short v51; // ax
	int v52; // eax
	int v53; // eax
	int v54; // eax
	int v55; // eax
	short v56; // dx
	bool v57; // zf
	short v58; // dx
	int v59; // ecx
	int v60; // ecx
	int v61; // eax
	short v62; // dx
	short v63; // dx
	short v64; // dx
	short v65; // dx
	int v66; // eax
	int v67; // ecx
	int v68; // ecx
	int v69; // eax
	struct FLOOR_INFO* v70; // eax
	char v71; // al
	int v72; // eax
	int v73; // eax
	short v74; // cx
	short v75; // ax
	short v76; // di
	short v77; // ax
	int v78; // eax
	short v79; // cx
	ITEM_INFO* v80; // eax
	short v81; // cx
	ITEM_INFO* v82; // eax
	short i; // cx
	short v84; // di
	CREATURE_INFO_FINAL* v85; // ecx
	short v86; // cx
	int v87; // eax
	int v88; // edx
	struct FLOOR_INFO* v89; // eax
	short v90; // ax
	int v91; // eax
	int v92; // eax
	int v93; // eax
	short v94; // ax
	struct FLOOR_INFO* v95; // eax
	int v96; // edx
	int v97; // eax
	short v98; // cx
	int v99; // eax
	int v100; // ecx
	struct FLOOR_INFO* v101; // eax
	short v102; // ax
	short v103; // dx
	short v104; // dx
	short v105; // ax
	short v106; // ax
	short v107; // cx
	short v108; // cx
	short v109; // dx
	short v110; // dx
	short v111; // ax
	short v112; // ax
	short v113; // cx
	short v114; // cx
	int a4; // [esp+Ch] [ebp-8Ch]
	int animIndex; // [esp+10h] [ebp-88h]
	int v117; // [esp+14h] [ebp-84h]
	int v118; // [esp+18h] [ebp-80h]
	int v119; // [esp+1Ch] [ebp-7Ch]
	int v120; // [esp+20h] [ebp-78h]
	int v121; // [esp+24h] [ebp-74h]
	int a2; // [esp+28h] [ebp-70h]
	int v123; // [esp+2Ch] [ebp-6Ch]
	int v124; // [esp+30h] [ebp-68h]
	int v125; // [esp+38h] [ebp-60h]
	int v126; // [esp+40h] [ebp-58h]
	CREATURE_INFO_FINAL* creature; // [esp+44h] [ebp-54h]
	int v128; // [esp+48h] [ebp-50h]
	int v129; // [esp+4Ch] [ebp-4Ch]
	int v130; // [esp+50h] [ebp-48h]
	char v131; // [esp+54h] [ebp-44h]
	int v132; // [esp+58h] [ebp-40h]
	int v133; // [esp+64h] [ebp-34h]
	int v134; // [esp+6Ch] [ebp-2Ch]
	int v135; // [esp+70h] [ebp-28h]
	int v136; // [esp+74h] [ebp-24h]
	int v137; // [esp+78h] [ebp-20h]
	int v138; // [esp+7Ch] [ebp-1Ch]
	int v139; // [esp+80h] [ebp-18h]
	short v140; // [esp+84h] [ebp-14h]
	int v141; // [esp+88h] [ebp-10h]
	int v142; // [esp+8Ch] [ebp-Ch]
	int v143; // [esp+90h] [ebp-8h]

	CreatureActive(itemNum);
	if (!v1)
		return;
	if (*(&Objects[33] + 50) & 1)
		LOWORD(animIndex) = Objects[33].animIndex;
	else
		LOWORD(animIndex) = Objects[37].animIndex;
	HIWORD(v2) = HIWORD(items);
	item = &Items[itemNum];
	creature = Items[itemNum].data;
	LOWORD(v2) = item->roomNumber;
	v128 = 0;
	v118 = 0;
	v126 = 0;
	v121 = 0;
	v4 = item->object_number;
	LOWORD(v120) = item->object_number;
	a4 = v2;
	LOWORD(v4) = item->pos.yRot;
	v5 = item->pos.xPos;
	v6 = item->pos.zPos;
	v7 = (v4 >> 3) & 0x1FFE;
	v8 = item->pos.yPos;
	v9 = rcossin_tbl[v7 + 1];
	v119 = 3480 * rcossin_tbl[v7] >> 14;
	v10 = v119 + v5;
	v11 = (3480 * v9 >> 14) + v6;
	v117 = 3480 * v9 >> 14;
	v12 = GetFloor(v10, v8, v11, &a4);
	v13 = GetFloorHeight(v12, v10, v8, v11);
	v14 = v117 + v11;
	v130 = v13;
	LOWORD(v13) = item->roomNumber;
	v15 = v119 + v10;
	a4 = v13;
	v16 = GetFloor(v15, v8, v14, &a4);
	v17 = GetFloorHeight(v16, v15, v8, v14);
	LOWORD(v18) = item->roomNumber;
	v129 = v17;
	a4 = v18;
	v19 = v119 + v15;
	v20 = v117 + v14;
	v21 = GetFloor(v19, v8, v20, &a4);
	v22 = GetFloorHeight(v21, v19, v8, v20);
	v23 = item->box_number;
	v24 = LaraItem->box_number;
	if (v23 == v24 || v8 >= v130 - 384 || (v25 = v129, v8 >= v129 + 256) || v8 <= v129 - 256)
	{
		v25 = v129;
		v119 = 0;
	}
	else
	{
		v119 = 1;
	}
	if (v23 == v24 || v8 >= v130 - 384 || v8 >= v25 - 384 || v8 >= v22 + 256 || (v117 = 1, v8 <= v22 - 256))
		v117 = 0;
	if (item->firedWeapon)
	{
		v134 = SwatGunX;
		v135 = SwatGunY;
		v136 = SwatGunZ;
		GetJointAbsPosition(item, &v134, SwatGunMesh);
		TriggerDynamicLight(v134, v135, v136, 2 * item->firedWeapon + 10, 192, 128, 32);
		item->firedWeapon--;
	}
	v26 = item->_bf15ea;
	v27 = creature;
	if (v26 & 0x3E00)
		GetAITarget(creature);
	else
		creature->enemy = LaraItem;
	CreatureAIInfo(item, &a2);
	v28 = LaraItem;
	if (*(&v27->mood + 2) == LaraItem)
	{
		v29 = v125;
		LOWORD(v133) = v125;
		v132 = v123;
	}
	else
	{
		v30 = LaraItem->pos.zPos - item->pos.zPos;
		v31 = LaraItem->pos.xPos - item->pos.xPos;
		v29 = phd_atan(v30, v31) - item->pos.yRot;
		v132 = v30 * v30 + v31 * v31;
		v28 = LaraItem;
		LOWORD(v133) = v29;
	}
	v32 = animIndex;
	if (item->hitPoints <= 0)
	{
		v33 = item->currentAnimState;
		if (v33 != 8 && v33 != 6)
		{
			if (v29 >= 12288 || v29 <= -12288)
			{
				v35 = animIndex + 16;
				item->currentAnimState = 8;
				item->animNumber = v35;
				item->pos.yRot += v133 + -32768;
			}
			else
			{
				v34 = animIndex + 11;
				item->currentAnimState = 6;
				item->animNumber = v34;
				item->pos.yRot += v133;
			}
			item->frameNumber = Anims[item->animNumber].frameBase;
		}
		goto LABEL_255;
	}
	v36 = *(&v27->mood + 2) != v28;
	GetCreatureMood(item, &a2, v36);
	if (v120 == 69)
	{
		if (item->hitPoints >= Objects[69].hitPoints)
		{
			if (*(&v27->mood + 2) == LaraItem)
				* (&v27->_bfc + 1) = 0;
		}
		else
		{
			*(&v27->_bfc + 1) = 2;
		}
	}
	if (SLOBYTE(Rooms[item->roomNumber].flags) < 0)
	{
		if (item->object_number == 35)
		{
			if (++item->itemFlags[0] > 60 && !(GetRandomControl() & 0xF))
			{
				SoundEffect(88, &item->pos, 0);
				item->itemFlags[0] = 0;
			}
		}
		else
		{
			if (!(GlobalCounter & 7))
				--item->hitPoints;
			*(&v27->_bfc + 1) = 2;
			if (item->hitPoints <= 0)
			{
				item->currentAnimState = 8;
				item->animNumber = v32 + 16;
				item->frameNumber = Anims[(v32 + 16)].frameBase;
			}
		}
	}
	CreatureMood(item, &a2, v36);
	LOWORD(v37) = v27->maximumTurn;
	LOWORD(v38) = CreatureTurn(item, v37);
	v39 = *(&v27->mood + 2);
	v128 = v38;
	*(&v27->mood + 2) = LaraItem;
	v40 = LaraItem;
	if (v132 < 0x400000 && LaraItem->speed > 20 || item->_bf15ea & 0x10)
		goto LABEL_52;
	if (TargetVisible(item, &v131))
	{
		v40 = LaraItem;
	LABEL_52:
		v41 = item->_bf15ea;
		if (!(v41 & 0x2000) && item->object_number != 69 && abs(item->pos.yPos - v40->pos.yPos) < 1280)
		{
			*(&v27->mood + 2) = v40;
			AlertAllGuards(itemNum);
		}
	}
	*(&v27->mood + 2) = v39;
	v42 = &item->pos.xPos;
	v137 = item->pos.xPos;
	v138 = item->pos.yPos - 384;
	v139 = item->pos.zPos;
	v140 = item->roomNumber;
	v141 = LaraItem->pos.xPos;
	v43 = GetBestFrame(LaraItem);
	v142 = LaraItem->pos.yPos + ((*(v43 + 6) + 3 * *(v43 + 4)) >> 2);
	v143 = LaraItem->pos.zPos;
	v45 = !LOS(&v137, &v141) && item->triggerFlags != 10;
	v27->maximumTurn = 0;
	v46 = item->currentAnimState;
	v47 = v46 - 1;
	switch (v46)
	{
	case 1:
		v48 = v133;
		HIWORD(v27[24].LOT.target.x) &= 0xFFF7u;
		v118 = v48;
		v27->flags = 0;
		if (v124)
		{
			v49 = item->_bf15ea;
			if (!(v49 & 0x200))
			{
				LOWORD(v49) = v125 >> 1;
				v121 = v49;
				v126 = *(&v125 + 2);
			}
		}
		v50 = v120;
		if (v120 == 69 && item == Lara.target)
		{
			item->goalAnimState = 39;
			goto LABEL_105;
		}
		v51 = item->requiredAnimState;
		if (v51)
		{
			item->goalAnimState = v51;
			goto LABEL_105;
		}
		v52 = item->_bf15ea;
		if (v52 & 0x200)
		{
			if (v52 & 0x1000)
			{
				v118 = 0;
			}
			else
			{
				LOWORD(v53) = AIGuard(v27);
				v118 = v53;
			}
			v54 = item->_bf15ea;
			if (v54 & 0x800)
			{
				if (--item->triggerFlags < 1)
				{
					BYTE1(v54) &= 0xFDu;
					item->_bf15ea = v54;
				}
			}
			goto LABEL_105;
		}
		if (*(&v27->mood + 2) == LaraItem && (v133 > 20480 || v133 < -20480) && v120 != 69)
		{
			item->goalAnimState = 2;
			goto LABEL_105;
		}
		if (v52 & 0x800)
		{
			item->goalAnimState = 5;
			goto LABEL_105;
		}
		if (v52 & 0x400)
		{
			item->goalAnimState = 7;
			goto LABEL_105;
		}
		if (Targetable(item, &a2) && v50 != 69)
		{
			if (v123 >= 0x1000000 && a2 == HIWORD(a2))
			{
				if ((item->_bf15ea & 0x3E00) == 4096)
					goto LABEL_105;
				goto LABEL_90;
			}
			item->goalAnimState = 4;
		}
		else if (v119 || v117)
		{
			v56 = animIndex;
			v27->maximumTurn = 0;
			v57 = v117 == 0;
			item->animNumber = v56 + 41;
			v58 = Anims[(v56 + 41)].frameBase;
			item->currentAnimState = 26;
			item->frameNumber = v58;
			if (v57)
				item->goalAnimState = 27;
			else
				item->goalAnimState = 28;
			BYTE2(v27[24].LOT.target.x) |= 8u;
		}
		else if (v45)
		{
			item->goalAnimState = 31;
		}
		else if (*(&v27->_bfc + 1))
		{
			if (v123 < &unk_900000 || (v55 = item->_bf15ea, BYTE1(v55) & 0x20))
			{
			LABEL_90:
				item->goalAnimState = 5;
				goto LABEL_105;
			}
			item->goalAnimState = 7;
		}
		else
		{
			item->goalAnimState = 1;
		}
	LABEL_105:
		if (item->triggerFlags == 11)
			item->triggerFlags = 0;
	LABEL_255:
		CreatureJoint(item, 0, v121);
		CreatureJoint(item, 1, v126);
		CreatureJoint(item, 2, v118);
		if (!(v27->_bfc & 8))
			goto LABEL_269;
		v97 = *(&v27->mood + 2);
		if (!v97)
			goto LABEL_269;
		v98 = *(v97 + 40);
		if (v98 != 4)
		{
			if (v98 & 0x10)
			{
				v99 = item->_bf15ea;
				item->goalAnimState = 1;
				item->requiredAnimState = 38;
				item->triggerFlags = 300;
				BYTE1(v99) &= 0xCBu;
			}
			else
			{
				if (v98 & 0x20)
				{
					v100 = item->_bf15ea;
					item->goalAnimState = 1;
					item->requiredAnimState = 36;
					BYTE1(v100) = BYTE1(v100) & 0xC9 | 8;
					item->_bf15ea = v100;
					goto LABEL_268;
				}
				LOWORD(v96) = *(v97 + 24);
				a4 = v96;
				v101 = GetFloor(*(*(&v27->mood + 2) + 64), *(*(&v27->mood + 2) + 68), *(*(&v27->mood + 2) + 72), &a4);
				GetFloorHeight(v101, *(*(&v27->mood + 2) + 64), *(*(&v27->mood + 2) + 68), *(*(&v27->mood + 2) + 72));
				TestTriggers(TriggerIndex, 1, 0);
				item->requiredAnimState = 5;
				if (*(*(&v27->mood + 2) + 40) & 2)
					item->itemFlags[3] = item->pad2[6] - 1;
				if (!(*(*(&v27->mood + 2) + 40) & 8))
					goto LABEL_268;
				v99 = item->_bf15ea;
				item->requiredAnimState = 1;
				item->triggerFlags = 300;
			}
			BYTE1(v99) |= 0xAu;
			item->_bf15ea = v99;
			goto LABEL_268;
		}
		item->goalAnimState = 1;
		item->requiredAnimState = 37;
	LABEL_268:
		++item->itemFlags[3];
		v27->_bfc &= 0xFFF7u;
		*(&v27->mood + 2) = 0;
	LABEL_269:
		v102 = item->currentAnimState;
		if ((v102 >= 20 || v102 == 6 || v102 == 8) && v102 != 30)
		{
			CreatureAnimation(itemNum, v128, 0);
		}
		else
		{
			switch (CreatureVault(itemNum, v128, 2, 256) + 4)
			{
			case 0:
				v113 = animIndex;
				v27->maximumTurn = 0;
				item->animNumber = v113 + 38;
				v114 = Anims[(v113 + 38)].frameBase;
				item->currentAnimState = 23;
				item->frameNumber = v114;
				break;
			case 1:
				v111 = animIndex;
				v27->maximumTurn = 0;
				v111 += 39;
				item->animNumber = v111;
				v112 = Anims[v111].frameBase;
				item->currentAnimState = 24;
				item->frameNumber = v112;
				break;
			case 2:
				v109 = animIndex;
				v27->maximumTurn = 0;
				item->animNumber = v109 + 40;
				v110 = Anims[(v109 + 40)].frameBase;
				item->currentAnimState = 25;
				item->frameNumber = v110;
				break;
			case 6:
				v103 = animIndex;
				v27->maximumTurn = 0;
				item->animNumber = v103 + 35;
				v104 = Anims[(v103 + 35)].frameBase;
				item->currentAnimState = 20;
				item->frameNumber = v104;
				break;
			case 7:
				v105 = animIndex;
				v27->maximumTurn = 0;
				v105 += 36;
				item->animNumber = v105;
				v106 = Anims[v105].frameBase;
				item->currentAnimState = 21;
				item->frameNumber = v106;
				break;
			case 8:
				v107 = animIndex;
				v27->maximumTurn = 0;
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
	case 2:
		v27->flags = 0;
		if ((v125 & 0x8000u) == 0)
			item->pos.yRot -= 364;
		else
			item->pos.yRot += 364;
		if (item->frameNumber == Anims[item->animNumber].frameEnd)
			item->pos.yRot += -32768;
		goto LABEL_255;
	case 3:
	case 35:
		LOWORD(v47) = v133 >> 1;
		v121 = v47;
		v118 = v47;
		if (v124)
			v126 = *(&v125 + 2);
		if (abs(v125) >= 364)
		{
			if ((v125 & 0x8000u) == 0)
				item->pos.yRot += 364;
			else
				item->pos.yRot -= 364;
		}
		else
		{
			item->pos.yRot += v125;
		}
		if (v46 != 35)
			goto LABEL_168;
		if (!v27->flags)
			goto LABEL_283;
		v66 = Anims[item->animNumber].frameBase;
		v67 = item->frameNumber;
		if (v67 < v66 + 10 && (v67 - v66) & 1)
			v27->flags = 0;
	LABEL_168:
		if (!v27->flags)
		{
		LABEL_283:
			v27->flags = 1;
			v57 = item->currentAnimState == 3;
			item->fired_weapon = 2;
			if (v57)
				ShotLara(item, &a2, &SwatGunX, v121, 30 * ((gfCurrentLevel > 0xAu) + 1));
			else
				ShotLara(item, &a2, &SwatGunX, v121, 10);
		}
		goto LABEL_255;
	case 4:
		LOWORD(v47) = v133;
		v27->flags = 0;
		LOWORD(v47) = v47 >> 1;
		v121 = v47;
		v118 = v47;
		if (v124)
			v126 = *(&v125 + 2);
		if (abs(v125) >= 364)
		{
			if ((v125 & 0x8000u) == 0)
				item->pos.yRot += 364;
			else
				item->pos.yRot -= 364;
		}
		else
		{
			item->pos.yRot += v125;
		}
		if (!Targetable(item, &a2))
			goto LABEL_242;
		if (v120 == 37 || v120 == 55)
			item->goalAnimState = 3;
		else
			item->goalAnimState = 35;
		goto LABEL_255;
	case 5:
		HIWORD(v27[24].LOT.target.x) &= 0xFFF7u;
		v27->maximumTurn = 910;
		if (!Targetable(item, &a2)
			|| v123 >= 0x1000000 && a2 == HIWORD(a2)
			|| v120 == 69
			|| (v59 = item->_bf15ea, BYTE1(v59) & 0xC))
		{
			if (v119 || v117)
			{
				v62 = animIndex;
				v27->maximumTurn = 0;
				v57 = v117 == 0;
				item->animNumber = v62 + 41;
				v63 = Anims[(v62 + 41)].frameBase;
				item->currentAnimState = 26;
				item->frameNumber = v63;
				if (v57)
					item->goalAnimState = 27;
				else
					item->goalAnimState = 28;
				BYTE2(v27[24].LOT.target.x) |= 8u;
			}
			else if (v123 >= 0x100000)
			{
				if (!v45 || (v60 = item->_bf15ea, BYTE1(v60) & 0x3E))
				{
					if (v123 > & unk_900000)
					{
						v61 = item->_bf15ea;
						if (!(v61 & 0x800))
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
		goto LABEL_255;
	case 7:
		HIWORD(v27[24].LOT.target.x) &= 0xFFF7u;
		v27->maximumTurn = 1820;
		if (Targetable(item, &a2) && (v123 < 0x1000000 || a2 != HIWORD(a2)) && v120 != 69)
		{
			item->goalAnimState = 4;
		}
		else if (v119 || v117)
		{
			v64 = animIndex;
			v27->maximumTurn = 0;
			v57 = v117 == 0;
			item->animNumber = v64 + 50;
			v65 = Anims[(v64 + 50)].frameBase;
			item->currentAnimState = 26;
			item->frameNumber = v65;
			if (v57)
				item->goalAnimState = 27;
			else
				item->goalAnimState = 28;
			BYTE2(v27[24].LOT.target.x) |= 8u;
		}
		else if (v45)
		{
			item->goalAnimState = 1;
		}
		else if (v123 < &unk_900000)
		{
			item->goalAnimState = 5;
		}
		if (item->triggerFlags == 11)
		{
			BYTE2(v27[24].LOT.target.x) |= 8u;
			v27->maximumTurn = 0;
		}
		goto LABEL_255;
	case 14:
		v68 = item->pos.yPos;
		v118 = v133;
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
			a4 = v69;
			v70 = GetFloor(*v42, item->pos.yPos, item->pos.zPos, &a4);
			GetFloorHeight(v70, *v42, item->pos.yPos, item->pos.zPos);
			TestTriggers(TriggerIndex, 1, 0);
			SoundEffect(340, &item->pos, 0);
		}
		if (abs(v125) >= 364)
		{
			if ((v125 & 0x8000u) == 0)
				item->pos.yRot += 364;
			else
				item->pos.yRot -= 364;
		}
		else
		{
			item->pos.yRot += v125;
		}
		goto LABEL_255;
	case 15:
		LOWORD(v72) = AIGuard(v27);
		v118 = v72;
		if (v27->_bfc & 1)
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
			v77 = Rooms[v76].item_number;
			if (v77 == -1)
				goto LABEL_255;
			while (1)
			{
				v78 = v77;
				v79 = Items[v78].object_number;
				v80 = &Items[v78];
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
			v80->meshBits = -3;
		}
		else if (v75 == Anims[item->animNumber].frameEnd)
		{
			item->pos.yRot -= 0x4000;
		}
		goto LABEL_255;
	case 17:
		v71 = item->_bf15ea;
		v118 = 0;
		if (!(v71 & 0x10) && LaraItem->speed < 40 && !(*(&lara + 69) & 0x10))
			v27->_bfc &= 0xFFFEu;
		if (v27->_bfc & 1)
			item->goalAnimState = 18;
		goto LABEL_255;
	case 19:
		LOWORD(v73) = AIGuard(v27);
		v118 = v73;
		if (v27->_bfc & 1)
			item->goalAnimState = 1;
		goto LABEL_255;
	case 30:
		goto LABEL_176;
	case 31:
		if (item->triggerFlags != 8 || !v45 || item->_bf15ea & 0x10)
			item->goalAnimState = 30;
	LABEL_176:
		HIWORD(v27[24].LOT.target.x) &= 0xFFF7u;
		v57 = v119 == 0;
		v27->maximumTurn = 910;
		if (!v57 || v117 || v123 < 0x100000 || !v45 || item->_bf15ea & 0x10)
			item->goalAnimState = 1;
		goto LABEL_255;
	case 36:
		goto LABEL_253;
	case 37:
		v82 = 0;
		for (i = Rooms[item->roomNumber].item_number; i != -1; i = v82->next_item)
		{
			v82 = &Items[i];
			if (Items[i].object_number == 249)
				break;
		}
		v84 = item->frameNumber;
		v85 = &Anims[item->animNumber];
		creature = v85;
		LOWORD(v85) = HIWORD(v85->aiTarget.floor);
		if (v84 == v85)
		{
			v86 = v82->pos.yRot;
			v82->meshBits = 0x1FFF;
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
				v82->meshBits = 16381;
			}
			else if (v84 == v85->aiTarget.itemFlags)
			{
				v82->meshBits = 278461;
			}
			else if (v84 == &v85->aiTarget.pad_ex_light[16])
			{
				v82->meshBits = 802621;
			}
			else if (v84 == (&v85->aiTarget.draw_room + 1))
			{
				v82->meshBits = 819001;
			}
			else if (v84 == &v85->aiTarget.pad1[30])
			{
				v82->meshBits = 17592121;
			}
			else if (v84 == LOWORD(creature->aiTarget.touch_bits))
			{
				v82->meshBits = 0x1FFF;
				LOWORD(v88) = item->roomNumber;
				a4 = v88;
				v89 = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &a4);
				GetFloorHeight(v89, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				TestTriggers(TriggerIndex, 1, 0);
				item->requiredAnimState = 5;
				*item->pad2 = 0;
			}
		}
		goto LABEL_255;
	case 38:
		if ((v120 != 69 || item != Lara.target)
			&& (GetRandomControl() & 0x7F || (v90 = item->triggerFlags, v90 >= 10) || v90 == 9))
		{
			v91 = item->_bf15ea;
			if (v91 & 0x200)
			{
				LOWORD(v92) = AIGuard(v27);
				v118 = v92;
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
		if (item != Lara.target && !(GetRandomControl() & 0x3F))
		{
			v94 = item->triggerFlags;
			if (v94 == 7 || v94 == 9)
				item->requiredAnimState = 38;
			item->goalAnimState = 1;
		}
	LABEL_253:
		HIWORD(v44) = HIWORD(anims);
		if (item->frameNumber == Anims[item->animNumber].frameBase + 39)
		{
		LABEL_254:
			LOWORD(v44) = item->roomNumber;
			a4 = v44;
			v95 = GetFloor(*v42, item->pos.yPos, item->pos.zPos, &a4);
			GetFloorHeight(v95, *v42, item->pos.yPos, item->pos.zPos);
			TestTriggers(TriggerIndex, 1, 0);
		}
		goto LABEL_255;
	default:
		goto LABEL_255;
	}
}**/