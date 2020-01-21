#include "../newobjects.h"
#include "../../Game/sphere.h"
#include "../../Game/items.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/effect2.h"
#include "../../Game/Box.h"
#include "../../Game/people.h"
#include "../../Game/debris.h"

#define STATE_ROMAN_STATUE_STOP					1
#define STATE_ROMAN_STATUE_SCREAMING			2
#define STATE_ROMAN_STATUE_ATTACK1				3
#define STATE_ROMAN_STATUE_ATTACK2				4
#define STATE_ROMAN_STATUE_ATTACK3				5
#define STATE_ROMAN_STATUE_ATTACK4				6
#define STATE_ROMAN_STATUE_WALK					7
#define STATE_ROMAN_STATUE_DEATH				11

#define ANIMATION_ROMAN_STATUE_START_JUMP_DOWN	16

struct ROMAN_STATUE_STRUCT
{
	PHD_VECTOR pos;
	int counter;
};

ROMAN_STATUE_STRUCT RomanStatueData;

void InitialiseRomanStatue(short itemNum)
{
    ITEM_INFO* item = &Items[itemNum];
    
	ClearItem(itemNum);
    
    item->animNumber = Objects[ID_ROMAN_GOD].animIndex + ANIMATION_ROMAN_STATUE_START_JUMP_DOWN;
    item->goalAnimState = 13;
    item->currentAnimState = 13;
    item->frameNumber = Anims[item->animNumber].frameBase;
	item->status = ITEM_INACTIVE;
	item->pos.xPos += 486 * SIN(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;
    item->pos.zPos += 486 * COS(item->pos.yRot + ANGLE(90)) >> W2V_SHIFT;

	ZeroMemory(&RomanStatueData, sizeof(ROMAN_STATUE_STRUCT));
}

void ControlRomanStatue(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	
	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	
	int oldSwapMeshFlags = item->swapMeshFlags;

	// At some HP values, roman statues loses a piece
	if (item->hitPoints < 1 && !(item->swapMeshFlags & 0x10000))
	{
		ExplodeItemNode(item, 16, 0, 8);
		item->meshBits |= 0x10000;
		item->swapMeshFlags |= 0x10000;
	}
	else if (item->hitPoints < 75 && !(item->swapMeshFlags & 0x100))
	{
		ExplodeItemNode(item, 8, 0, 8);
		item->meshBits |= 0x100;
		item->swapMeshFlags |= 0x100;
	}
	else if (item->hitPoints < 150 && !(item->swapMeshFlags & 0x400))
	{
		ExplodeItemNode(item, 10, 0, 32);
		ExplodeItemNode(item, 11, 0, 32);
		item->meshBits |= 0x400u;
		item->swapMeshFlags |= 0x400;
	}
	else if (item->hitPoints < 225 && !(item->swapMeshFlags & 0x10))
	{
		ExplodeItemNode(item, 4, 0, 8);
		item->meshBits |= 0x10;
		item->swapMeshFlags |= 0x10;
	}

	// Play hit animation
	if (oldSwapMeshFlags != item->swapMeshFlags)
	{
		item->goalAnimState = 6;
		item->currentAnimState = 6;
		item->animNumber = Objects[ID_ROMAN_GOD].animIndex + 5;
		item->frameNumber = Anims[item->animNumber].frameBase;
	}

/*
	if (item->hitPoints > 0)
	{
		creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);
		
		v23 = 1;
		
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		
		angle = CreatureTurn(item, creature->maximumTurn);
		
		if (info.ahead)
		{
			joint0 = info.angle >> 1;
			joint2 = info.angle >> 1;
			joint1 = info.xAngle;
		}

		creature->maximumTurn = 0;

		PHD_VECTOR pos, pos1, pos2;
		int deltaFrame, deltaFrame2, frameNumber;
		byte color;
		int i;
		int x, y, z;
		ROOM_INFO* r;
		FLOOR_INFO* floor;
		MESH_INFO* mesh;

		switch (item->currentAnimState)
		{
		case STATE_ROMAN_STATUE_STOP:    
			creature->flags = 0;
			
			if (creature->mood == ATTACK_MOOD)
			{
				creature->maximumTurn = ANGLE(2);
			}
			else
			{
				creature->maximumTurn = 0;
				item->goalAnimState = STATE_ROMAN_STATUE_WALK;
			}
			
			joint2 = info.angle;
			
			if (item->aiBits
				|| !(GetRandomControl() & 0x1F)
				&& (info.distance > SQUARE(1024)
					|| creature->mood != ATTACK_MOOD))
			{
				joint2 = AIGuard((CREATURE_INFO*)creature);
			}
			else if (info.angle > 20480 || info.angle < -20480)
			{
				item->goalAnimState = 10;
			}
			else if (info.ahead && info.distance < SQUARE(1024))
			{
				if (info.bite & ((GetRandomControl() & 3) == 0))
				{
					item->goalAnimState = STATE_ROMAN_STATUE_ATTACK1;
				}
				else if (GetRandomControl() & 1)
				{
					item->goalAnimState = STATE_ROMAN_STATUE_ATTACK2;
				}
				else
				{
					item->goalAnimState = STATE_ROMAN_STATUE_ATTACK3;
				}
			}
			else
			{
				if (!item->itemFlags[0])
				{
					item->goalAnimState = STATE_ROMAN_STATUE_SCREAMING;
					item->itemFlags[0] = 5;
					break;
				}
				if (item->triggerFlags == 1)
				{
					if (Targetable(item, &info) && GetRandomControl() & 1)
					{
						item->goalAnimState = 12;
						break;
					}
				}
				if (item->triggerFlags || info.distance >= (signed int)& unk_640000 || !info.bite)
				{
					item->goalAnimState = STATE_ROMAN_STATUE_WALK;
					break;
				}
				item->goalAnimState = STATE_ROMAN_STATUE_ATTACK1;
			}
			
			break;

		case STATE_ROMAN_STATUE_SCREAMING:    
			v84 = 0;

			pos1.x = -32;
			pos1.y = 48;
			pos1.z = 64;
			GetJointAbsPosition(item, &pos1, 14);

			pos2.x = -48;
			pos2.y = 48;
			pos2.z = 490;
			GetJointAbsPosition(item, &pos2, 14);

			pos.x = (pos1.x + pos2.x) >> 1;
			pos.y = (pos1.y + pos2.y) >> 1;
			pos.z = (pos1.z + pos2.z) >> 1;

			deltaFrame = item->frameNumber - Anims[item->animNumber].frameBase;

			if (deltaFrame > 68 && deltaFrame < 130)
			{
				deltaFrame2 = deltaFrame - 68;
				if (deltaFrame2 <= 58)
				{
					if (deltaFrame2 > 16)
						deltaFrame2 = 16;
				}
				else
				{
					deltaFrame2 = 4 * (62 - deltaFrame);
				}
				
				color = deltaFrame2 * ((GetRandomControl() & 0x3F) + 128) >> 4;
				
				if (item->triggerFlags)
					TriggerDynamicLight(pos.x, pos.y, pos.z, 16, 0, color, color >> 1);
				else
					TriggerDynamicLight(pos.x, pos.y, pos.z, 16, 0, color >> 1, color);
				
				for (int i = 0; i < 2; i++)
				{
					x = (GetRandomControl() & 0x7FF) + pos.x - 1024;
					y = (GetRandomControl() & 0x7FF) + pos.y - 1024;
					z = (GetRandomControl() & 0x7FF) + pos.z - 1024;
					
					TriggerRomanStatueScreamingSparks(
						x,
						y,
						z,
						8 * (pos.x - x),
						8 * (pos.y - y),
						8 * (1024 - (GetRandomControl() & 0x7FF)),
						item->triggerFlags);
				}
			}

			if (deltaFrame <= 90 || deltaFrame >= 130)
				break;

			if (item->triggerFlags)
			{
				pos.x = -48;
				pos.y = 48;
				pos.z = GetRandomControl() % 480;
			}
			else
			{
				pos.x = -40;
				pos.y = 64;
				pos.z = GetRandomControl() % 360;
			}

			GetJointAbsPosition(item, &pos, 14);

			v57 = (GetRandomControl() & 0x3F) + 128;
			pos1.x = (GetRandomControl() & 0xFFF) + item->pos.xPos - 2048;
			pos1.y = item->pos.yPos - (GetRandomControl() & 0x3FF) - 4096;
			pos1.z = (GetRandomControl() & 0xFFF) + item->pos.zPos - 2048;
			v58 = dword_51CFA4;

			break;

		case STATE_ROMAN_STATUE_ATTACK1:
		case STATE_ROMAN_STATUE_ATTACK2:
		case STATE_ROMAN_STATUE_ATTACK3:
		case STATE_ROMAN_STATUE_ATTACK4:                                  
			creature->maximumTurn = 0;

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

			if (item->frameNumber > Anims[item->animNumber].frameBase + 10)
			{
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				
				GetJointAbsPosition(item, &pos, 16);

				r = &Rooms[item->roomNumber];
				floor = &XZ_GET_SECTOR(r, pos.x - r->x, pos.z - r->z);

				// If floor is stopped, then try to find static meshes and shatter them, activating heavy triggers below
				if (floor->stopper)
				{
					for (i = 0; i < r->numMeshes; i++)
					{
						mesh = &r->mesh[i];
						
						if (!((mesh->z ^ pos.z) & 0xFFFFFC00) && !((mesh->x ^ pos.x) & 0xFFFFFC00))
						{
							if (mesh->staticNumber >= 50 && mesh->staticNumber <= 59)
							{
								ShatterObject(0, mesh, -64, LaraItem->roomNumber, 0);
								SoundEffect(
									(unsigned __int8)ShatterSounds[CurrentLevel - 5][mesh->staticNumber],
									(PHD_3DPOS*)mesh,
									0);

								mesh->Flags &= ~1;
								floor->stopper = false;
								GetFloorHeight(floor, pos.x, pos.y, pos.z);
								TestTriggers(TriggerIndex, 1, 0);
							}
						}
					}
				}

				if (!creature->flags)
				{
					if (item->touchBits & 0xC000)
					{
						LaraItem->hitPoints -= 200;
						LaraItem->hitStatus = true;
						CreatureEffect2(item, (int)&unk_509CF0, 20, item->pos.yRot, DoBloodSplat);
						SoundEffect(SFX_LARA_THUD, &item->pos, 0);
						creature->flags = 1;
					}
				}

				if (!item->triggerFlags)
				{
					pos1.x = -40;
					pos1.y = 64;
					pos1.z = 360;
					GetJointAbsPosition(item, &pos1, 14);

					*(_DWORD*)& dest.zRot = item->pos.yPos - 64;
					if (item->frameNumber == Anims[item->animNumber].frameBase + 34 && item->currentAnimState == 3)
					{
						v42 = item->itemFlags[0];
						if (v42)
							item->itemFlags[0] = v42 - 1;
						TriggerShockwave((int)& dest.xRot, (int)& unk_A00010, 96, 0x30004080, 0x10000);
						TriggerRomanStatueShockwaveAttackSparks(pos1.x, pos1.y, pos1.z, 0x80004080);
						*(_DWORD*)& dest.zRot -= 64;
						TriggerShockwave((int)& dest.xRot, (int)& unk_600010, 64, 0x30004080, 0x10000);
					}
					v43 = item->frameNumber;
					v44 = &Anims[item->animNumber];
					v45 = v44->frame_end - v43;
					if (v45 >= 16)
					{
						v46 = v43 - v44->frameBase;
						if (v46 > 16)
							v46 = 16;
						TriggerRomanStatueAttackEffect1(a1, v46);
					}
					else
					{
						TriggerRomanStatueAttackEffect1(a1, v45);
					}
				}
			}
			
			break;

		case STATE_ROMAN_STATUE_WALK:                                   // STATE_ROMAN_STATUE_WALK
												  // 
			v30 = info.angle;
			creature->flags = 0;
			joint2 = v30;
			if (creature->mood == 1)
			{
				creature->maximumTurn = 1274;
			}
			else
			{
				creature->maximumTurn = 0;
				if (abs((signed __int16)info.angle) >= 364)
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
			}
			v31 = info.distance;
			if (info.distance < 0x100000)
			{
				item->goalAnimState = 1;
				goto LABEL_166;
			}
			if (info.bite && info.distance < 3211264)
			{
				item->goalAnimState = 9;
				goto LABEL_166;
			}
			if (item->triggerFlags != 1)
				goto LABEL_76;
			if (Targetable((int)item, (int)& info) && !(GetRandomControl() & 3))
				goto LABEL_78;
			v31 = info.distance;
		LABEL_76:
			if (item->triggerFlags)
				goto LABEL_175;
			if (v31 >= (signed int)& unk_640000)
				LABEL_175:
			item->goalAnimState = 7;
			else
				LABEL_78:
			item->goalAnimState = v23;
			goto LABEL_166;
		case 10:                                  // STATE_ROMAN_STATUE_TURN_180
												  // 
			creature->flags = 0;
			creature->maximumTurn = 0;
			if ((info.angle & 0x8000u) == 0)
				item->pos.yRot -= 364;
			else
				item->pos.yRot += 364;
			if (item->frameNumber == Anims[item->animNumber].frame_end)
				item->pos.yRot += -32768;
			goto LABEL_166;
		case 12:
			creature->flags = 0;
			creature->maximumTurn = 0;
			if (RomanStatueData_Counter)
			{
				v64 = --RomanStatueData_Counter * ((GetRandomControl() & 0x3F) + 128) >> 4;
				TriggerDynamic(RomanStatueData_X, RomanStatueData_Y, RomanStatueData_Z, 16, 0, v64, v64 >> 1);
			}
			v65 = item->frameNumber - Anims[item->animNumber].frameBase;
			if (v65 == 34)
			{
				v94 = -48;
				v95 = 48;
				v96 = 1024;
				GetJointAbsPosition((int)item, (int)& v94, 14);
				a1a = -48;
				v98 = 48;
				a3 = 450;
				GetJointAbsPosition((int)item, (int)& a1a, 14);
				phd_GetVectorAngles(v94 - a1a, v95 - v98, v96 - a3, (int)a4);
				LOWORD(v66) = item->room_number;
				v100 = a4[1];
				v101 = a4[0];
				*(_DWORD*)roomNumber = v66;
				GetFloor(a1a, v98, a3, roomNumber);
				RomanStatueAttack((PHD_3DPOS*)& a1a, roomNumber[0], 1);
				v67 = GetRandomControl();
				TriggerRomanStatueShockwaveAttackSparks(
					a1a,
					v98,
					a3,
					(((v67 & 0x3F) + 128) >> 1) | ((((v67 & 0x3F) + 128) | 0x400000) << 8));
				RomanStatueData_Counter = 16;
				RomanStatueData_X = a1a;
				RomanStatueData_Y = v98;
				RomanStatueData_Z = a3;
				v68 = item->itemFlags[0];
				if (v68)
					item->itemFlags[0] = v68 - 1;
			}
			else if (v65 < 10 || v65 > 49)
			{
				goto LABEL_166;
			}
			v69 = v65 - 10;
			if (v69 < 32)
			{
				dest.xPos = -32;
				dest.yPos = 48;
				dest.zPos = 64;
				GetJointAbsPosition((int)item, (int)& dest, 14);
				*(_DWORD*)& dest.xRot = -48;
				*(_DWORD*)& dest.zRot = 48;
				a5 = 490;
				GetJointAbsPosition((int)item, (int)& dest.xRot, 14);
				v70 = 0;
				do
				{
					v71 = GetRandomControl();
					v72 = v69 * ((v71 & 0x3F) + 128) >> 4;
					v73 = v69 * ((v71 & 0x3F) + 128) >> 5;
					*(_DWORD*)roomNumber = v69 * ((v71 & 0x3F) + 128) >> 5;
					if (!v70)
					{
						TriggerDynamic(
							*(int*)& dest.xRot,
							*(int*)& dest.zRot,
							a5,
							8,
							0,
							v69 * ((v71 & 0x3F) + 128) >> 5,
							v69 * ((v71 & 0x3F) + 128) >> 6);
						v73 = *(_DWORD*)roomNumber;
					}
					v74 = dword_51CFA4[v70];
					if (v74 && v69 && v69 != 24)
					{
						if (v69 < 16)
						{
							*(_BYTE*)(v74 + 51) = 56;
							*(_BYTE*)(v74 + 48) = 0;
							*(_BYTE*)(v74 + 49) = v72;
							*(_BYTE*)(v74 + 50) = v73;
						}
						*(_DWORD*)v74 = dest.xPos;
						*(_DWORD*)(v74 + 4) = dest.yPos;
						*(_DWORD*)(v74 + 8) = dest.zPos;
						*(_DWORD*)(v74 + 36) = *(_DWORD*)& dest.xRot;
						*(_DWORD*)(v74 + 40) = *(_DWORD*)& dest.zRot;
						*(_DWORD*)(v74 + 44) = a5;
					}
					else if (v69 >= 16)
					{
						if (v69 == 24)
						{
							v77 = GetRandomControl();
							v78 = (((v77 & 0x3F) + 128) >> 1) | ((((v77 & 0x3F) + 128) | 0x200000) << 8);
							v79 = GetRandomControl();
							TriggerEnergyArc((PHD_VECTOR*)& dest, (PHD_VECTOR*)& dest.xRot, (v79 & 0xF) + 24, v78, 13, 64, 4);
						}
					}
					else
					{
						v75 = v73 | ((v72 | 0x180000) << 8);
						v76 = GetRandomControl();
						dword_51CFA4[v70] = (int)TriggerEnergyArc(
							(PHD_VECTOR*)& dest,
							(PHD_VECTOR*)& dest.xRot,
							(v76 & 7) + 8,
							v75,
							12,
							64,
							4);
					}
					++v70;
				} while (v70 < 4);
			}
			goto LABEL_166;
		default:
			goto LABEL_166;
		}
		while (1)
		{
			v59 = *v58;
			if (*v58 && *(_BYTE*)(v59 + 51))
			{
				*(_DWORD*)(v59 + 36) = dest.xPos;
				*(_DWORD*)(*v58 + 40) = dest.yPos;
				*(_DWORD*)(*v58 + 44) = dest.zPos;
				if (item->triggerFlags)
					TriggerLightningGlow(dest.xPos, dest.yPos, dest.zPos, (v57 >> 1) | ((v57 | 0x100000) << 8));
				else
					TriggerLightningGlow(dest.xPos, dest.yPos, dest.zPos, v57 | (((v57 >> 1) | 0x100000) << 8));
				goto LABEL_142;
			}
			if (!(GlobalCounter & 3) || v84)
				goto LABEL_138;
			if (item->triggerFlags)
				break;
			v60 = GetRandomControl();
			v61 = TriggerEnergyArc(
				(PHD_VECTOR*)& dest.xRot,
				(PHD_VECTOR*)& dest,
				(v60 & 0x3F) + 16,
				v57 | (((v57 >> 1) | 0x180000) << 8),
				15,
				48,
				5);
			v62 = dest.yPos;
			*v58 = (int)v61;
			TriggerLightningGlow(dest.xPos, v62, dest.zPos, v57 | (((v57 >> 1) | 0x100000) << 8));
			v84 = 1;
		LABEL_142:
			++v58;
			if ((signed int)v58 >= (signed int)& RomanStatueData_Counter)
				goto LABEL_166;
		}
		v63 = GetRandomControl();
		*v58 = (int)TriggerEnergyArc(
			(PHD_VECTOR*)& dest.xRot,
			(PHD_VECTOR*)& dest,
			(v63 & 0x3F) + 16,
			(v57 >> 1) | ((v57 | 0x180000) << 8),
			15,
			48,
			5);
		TriggerLightningGlow(dest.xPos, dest.yPos, dest.zPos, (v57 >> 1) | ((v57 | 0x100000) << 8));
	LABEL_138:
		v84 = 1;
		goto LABEL_142;
	}

	item->hitPoints = 0;

	// STATE_ROMAN_STATUE_DEATH
	if (item->currentAnimState == STATE_ROMAN_STATUE_DEATH)
	{
		if (item->frameNumber > Anims[item->animNumber].frameBase + 54 
			&& item->frameNumber < Anims[item->animNumber].frameBase + 74 
			&& item->touchBits)
		{
			LaraItem->hitPoints -= 40;
			LaraItem->hitStatus = true;
		}
		else if (item->frameNumber == Anims[item->animNumber].frameEnd)
		{
			// Activate trigger on death
			short roomNumber = item->itemFlags[2] & 0xFF;
			short floorHeight = item->itemFlags[2] & 0xFF00;
			ROOM_INFO* r = &Rooms[roomNumber];

			int x = r->x + (((item->TOSSPAD >> 8) & 0xFF) << WALL_SHIFT) + 512;
			int y = r->minfloor + floorHeight;
			int z = r->z + ((item->TOSSPAD & 0xFF) << WALL_SHIFT) + 512;

			FLOOR_INFO * floor = GetFloor(x, y, z, &roomNumber);
			GetFloorHeight(floor, x, y, z);
			TestTriggers(TriggerIndex, 1, 0);
		}
	}
	else
	{
		item->animNumber = Objects[ID_ROMAN_GOD].animIndex + 14;
		item->currentAnimState = STATE_ROMAN_STATUE_DEATH;
		item->frameNumber = Anims[item->animNumber].frameBase;
	}
LABEL_166:

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	if (item->swapMeshFlags & 0x400)
	{
		PHD_VECTOR pos;
		pos.x = (GAME_VECTOR*)((v81 & 0x1F) - 16);
		pos.y = 86;
		pos.z = (GetRandomControl() & 0x1F) - 16;
		RomanStatueHitEffect(item, &pos, 10);
	}

	if (item->swapMeshFlags & 0x10)
	{
		PHD_VECTOR pos;
		pos.x = -40;
		pos.y = (GetRandomControl() & 0x7F) + 148;
		pos.z = (GetRandomControl() & 0x3F) - 32;
		RomanStatueHitEffect(item, &pos, 4);
	}

	if (item->swapMeshFlags & 0x100)
	{
		PHD_VECTOR pos;
		pos.x = (GAME_VECTOR*)((v83 & 0x3F) + 54);
		pos.y = -170;
		pos.z = (GetRandomControl() & 0x1F) + 27;
		RomanStatueHitEffect(item, &pos, 8);
	}

	CreatureAnimation(itemNumber, angle, 0);*/
}

void RomanStatueHitEffect(ITEM_INFO* item, PHD_VECTOR* pos, int joint)
{
	GetJointAbsPosition(item, pos, joint);

	if (!(GetRandomControl() & 0x1F))
	{
		short fxNumber = CreateNewEffect(item->roomNumber);
		if (fxNumber != -1)
		{
			FX_INFO* fx = &Effects[fxNumber];

			fx->pos.xPos = pos->x;
			fx->pos.yPos = pos->y;
			fx->pos.zPos = pos->z;
			fx->roomNumber = item->roomNumber;
			fx->pos.zRot = 0;
			fx->pos.xRot = 0;
			fx->pos.yRot = 2 * GetRandomControl();
			fx->speed = 1;
			fx->fallspeed = 0;
			fx->objectNumber = ID_BODY_PART;
			fx->shade = 16912;
			fx->flag2 = 9729;
			fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 2 * (GetRandomControl() & 7);
			fx->counter = 0;
			fx->flag1 = 0;
		}
	}
	
	if (!(GetRandomControl() & 0xF))
	{
		SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];

		spark->on = 1;
		spark->sShade = 0;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 32;
		spark->dShade = (GetRandomControl() & 0xF) + 64;
		spark->transType = COLADD;
		spark->life = spark->sLife= (GetRandomControl() & 3) + 64;
		spark->x = (GetRandomControl() & 0x1F) + pos->x - 16;
		spark->y = (GetRandomControl() & 0x1F) + pos->y - 16;
		spark->z = (GetRandomControl() & 0x1F) + pos->z - 16;
		spark->yVel = 0;
		spark->xVel = (GetRandomControl() & 0x7F) - 64;
		spark->friction = 4;
		spark->flags = SP_ROTATE;
		spark->zVel = (GetRandomControl() & 0x7F) - 64;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 7) + 8;
		spark->mirror = 0;
		spark->sSize = spark->size = (GetRandomControl() & 7) + 8;
		spark->dSize = spark->size * 2;
	}
}

void TriggerRomanStatueShockwaveAttackSparks(int x, int y, int z, int color)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->dG = (color >> 16) & 0xFF;
	spark->sG = (color >> 16) & 0xFF;
	spark->colFadeSpeed = 2;
	spark->dR = (color >> 8) & 0xFF;
	spark->sR = (color >> 8) & 0xFF;
	spark->transType = COLADD;
	spark->life = 16;
	spark->sLife = 16;
	spark->x = x;
	spark->on = 1;
	spark->dB = (color >> 24) & 0xFF;
	spark->sB = (color >> 24) & 0xFF;
	spark->fadeToBlack = 4;
	spark->y = y;
	spark->z = z;
	spark->zVel = 0;
	spark->yVel = 0;
	spark->xVel = 0;
	spark->flags = SP_SCALE | SP_DEF;
	spark->scalar = 3;
	spark->maxYvel = 0;
	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 11;
	spark->gravity = 0;
	spark->dSize = spark->sSize = spark->size = (color >> 24) + (GetRandomControl() & 3);
}

void TriggerRomanStatueScreamingSparks(int x, int y, int z, short xv, short yv, short zv, int flags)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	spark->sR = 0;
	spark->sG = 0;
	spark->sB = 0;
	spark->dR = 64;
	if (flags)
	{
		spark->dG = (GetRandomControl() & 0x3F) - 64;
		spark->dB = spark->dG >> 1;
	}
	else
	{
		spark->dB = (GetRandomControl() & 0x3F) - 64;
		spark->dG = spark->dB >> 1;
	}
	spark->y = y;
	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 4;
	spark->life = 16;
	spark->sLife = 16;
	spark->z = z;
	spark->x = x;
	spark->transType = COLADD;
	spark->xVel = xv;
	spark->yVel = yv;
	spark->zVel = zv;
	spark->friction = 34;
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->flags = 0;
}

void TriggerRomanStatueAttackEffect1(short itemNum, int factor)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	spark->sR = 0;
	spark->sB = (GetRandomControl() & 0x3F) - 96;
	spark->dB = (GetRandomControl() & 0x3F) - 96;
	spark->dR = 0;
	if (factor < 16)
	{
		spark->sB = factor * spark->sB >> 4;
		spark->dB = factor * spark->dB >> 4;
	}
	spark->sG = spark->sB >> 1;
	spark->dG = spark->dB >> 1;
	spark->fadeToBlack = 4;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->transType = COLADD;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 32;
	spark->y = 0;
	spark->x = (GetRandomControl() & 0x1F) - 16;
	spark->z = (GetRandomControl() & 0x1F) - 16;
	spark->yVel = 0;
	spark->xVel = (byte)GetRandomControl() - 128;
	spark->friction = 4;
	spark->zVel = (byte)GetRandomControl() - 128;
	spark->flags = SP_NODEATTATCH | SP_EXPDEF | SP_ITEM | SP_ROTATE | SP_DEF | SP_SCALE; // 4762;
	spark->fxObj = itemNum;
	spark->nodeNumber = 6;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->maxYvel = 0;
	spark->gravity = -8 - (GetRandomControl() & 7);
	spark->scalar = 2;
	spark->dSize = 4;
	spark->sSize = spark->size = factor * ((GetRandomControl() & 0x1F) + 64) >> 4;
}

void RomanStatueAttack(PHD_3DPOS* pos, short roomNumber, short count)
{
	short fxNum = CreateNewEffect(roomNumber);

	if (fxNum != NO_ITEM)
	{
		FX_INFO* fx = &Effects[fxNum];

		fx->pos.xPos = pos->xPos;
		fx->pos.yPos = pos->yPos;
		fx->pos.zPos = pos->zPos;
		fx->pos.xRot = pos->xRot;
		fx->pos.yRot = pos->yRot;
		fx->pos.zRot = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 16 * count + 15;
		fx->flag1 = 1;
		fx->objectNumber = ID_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 64;
		fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 16;
	}
}