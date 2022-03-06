#include "framework.h"
#include "tr5_roman_statue.h"
#include "Game/items.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/control/box.h"
#include "Game/people.h"
#include "Game/effects/debris.h"
#include "Game/animation.h"
#include "Game/control/control.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/lightning.h"

using namespace TEN::Effects::Lightning;

#define STATE_ROMAN_STATUE_STOP					1
#define STATE_ROMAN_STATUE_SCREAMING			2
#define STATE_ROMAN_STATUE_ATTACK1				3
#define STATE_ROMAN_STATUE_ATTACK2				4
#define STATE_ROMAN_STATUE_ATTACK3				5
#define STATE_ROMAN_STATUE_HIT					6
#define STATE_ROMAN_STATUE_ATTACK4				9
#define STATE_ROMAN_STATUE_WALK					7
#define STATE_ROMAN_STATUE_TURN_180				10
#define STATE_ROMAN_STATUE_DEATH				11
#define STATE_ROMAN_STATUE_ENERGY_ATTACK		12

#define ANIMATION_ROMAN_STATUE_HIT				5
#define ANIMATION_ROMAN_STATUE_DEATH			14
#define ANIMATION_ROMAN_STATUE_START_JUMP_DOWN	16

struct ROMAN_STATUE_STRUCT
{
	PHD_VECTOR pos;
	LIGHTNING_INFO* energyArcs[8];
	int counter;
};

BITE_INFO RomanStatueBite { 0, 0, 0, 15 };
ROMAN_STATUE_STRUCT RomanStatueData;

static void RomanStatueHitEffect(ITEM_INFO* item, PHD_VECTOR* pos, int joint)
{
	GetJointAbsPosition(item, pos, joint);

	if (!(GetRandomControl() & 0x1F))
	{
		short fxNumber = CreateNewEffect(item->RoomNumber);
		if (fxNumber != -1)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.xPos = pos->x;
			fx->pos.yPos = pos->y;
			fx->pos.zPos = pos->z;
			fx->roomNumber = item->RoomNumber;
			fx->pos.zRot = 0;
			fx->pos.xRot = 0;
			fx->pos.yRot = 2 * GetRandomControl();
			fx->speed = 1;
			fx->fallspeed = 0;
			fx->objectNumber = ID_BODY_PART;
			fx->shade = 16912;
			fx->flag2 = 9729;
			fx->frameNumber = Objects[ID_BUBBLES].meshIndex + (GetRandomControl() & 7);
			fx->counter = 0;
			fx->flag1 = 0;
		}
	}

	if (!(GetRandomControl() & 0xF))
	{
		auto* spark = &SmokeSparks[GetFreeSmokeSpark()];

		spark->on = 1;
		spark->sShade = 0;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 32;
		spark->dShade = (GetRandomControl() & 0xF) + 64;
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 64;
		spark->x = (GetRandomControl() & 0x1F) + pos->x - 16;
		spark->y = (GetRandomControl() & 0x1F) + pos->y - 16;
		spark->z = (GetRandomControl() & 0x1F) + pos->z - 16;
		spark->xVel = (GetRandomControl() & 0x7F) - 64;
		spark->yVel = 0;
		spark->zVel = (GetRandomControl() & 0x7F) - 64;
		spark->friction = 4;
		spark->flags = SP_ROTATE;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 7) + 8;
		spark->mirror = 0;
		spark->sSize = spark->size = (GetRandomControl() & 7) + 8;
		spark->dSize = spark->size * 2;
	}
}

static void TriggerRomanStatueShockwaveAttackSparks(int x, int y, int z, byte r, byte g, byte b, byte size)
{
	auto* spark = &Sparks[GetFreeSpark()];

	spark->dG = g;
	spark->sG = g;
	spark->colFadeSpeed = 2;
	spark->dR = r;
	spark->sR = r;
	spark->transType = TransTypeEnum::COLADD;
	spark->life = 16;
	spark->sLife = 16;
	spark->x = x;
	spark->on = 1;
	spark->dB = b;
	spark->sB = b;
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
	spark->dSize = spark->sSize = spark->size = size + (GetRandomControl() & 3);
}

static void TriggerRomanStatueScreamingSparks(int x, int y, int z, short xv, short yv, short zv, int flags)
{
	auto* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->sR = 0;
	spark->sG = 0;
	spark->sB = 0;
	spark->dR = 64;

	if (flags)
	{
		spark->dG = (GetRandomControl() & 0x3F) - 64;
		spark->dB = spark->dG / 2;
	}
	else
	{
		spark->dB = (GetRandomControl() & 0x3F) - 64;
		spark->dG = spark->dB / 2;
	}

	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 4;
	spark->life = 16;
	spark->sLife = 16;
	spark->x = x;
	spark->y = y;
	spark->z = z;
	spark->xVel = xv;
	spark->yVel = yv;
	spark->zVel = zv;
	spark->transType = TransTypeEnum::COLADD;
	spark->friction = 34;
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->flags = SP_NONE;
}

static void TriggerRomanStatueAttackEffect1(short itemNum, int factor)
{
	auto* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->sR = 0;
	spark->sB = (GetRandomControl() & 0x3F) - 96;
	spark->dB = (GetRandomControl() & 0x3F) - 96;
	spark->dR = 0;

	if (factor < 16)
	{
		spark->sB = (factor * spark->sB) / 16;
		spark->dB = (factor * spark->dB) / 16;
	}

	spark->sG = spark->sB / 2;
	spark->dG = spark->dB / 2;
	spark->fadeToBlack = 4;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->transType = TransTypeEnum::COLADD;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 32;
	spark->y = 0;
	spark->x = (GetRandomControl() & 0x1F) - 16;
	spark->z = (GetRandomControl() & 0x1F) - 16;
	spark->yVel = 0;
	spark->xVel = (byte)GetRandomControl() - 128;
	spark->friction = 4;
	spark->zVel = (byte)GetRandomControl() - 128;
	spark->flags = SP_NODEATTACH | SP_EXPDEF | SP_ITEM | SP_ROTATE | SP_DEF | SP_SCALE; // 4762;
	spark->fxObj = itemNum;
	spark->nodeNumber = 6;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->maxYvel = 0;
	spark->gravity = -8 - (GetRandomControl() & 7);
	spark->scalar = 2;
	spark->dSize = 4;
	spark->sSize = (spark->size = factor * ((GetRandomControl() & 0x1F) + 64)) / 16;
}

static void RomanStatueAttack(PHD_3DPOS* pos, short roomNumber, short count)
{
	short fxNum = CreateNewEffect(roomNumber);

	if (fxNum != NO_ITEM)
	{
		auto* fx = &EffectList[fxNum];

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
		fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 8;
	}
}

void TriggerRomanStatueMissileSparks(PHD_VECTOR* pos, char fxObj)
{
	auto* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->sR = 0;
	spark->sG = (GetRandomControl() & 0x3F) - 96;
	spark->sB = spark->sG / 2;
	spark->dR = 0;
	spark->dG = (GetRandomControl() & 0x3F) - 96;
	spark->dB = spark->dG / 2;
	spark->fadeToBlack = 8;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->transType = TransTypeEnum::COLADD;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 20;
	spark->x = (GetRandomControl() & 0xF) - 8;
	spark->y = (GetRandomControl() & 0xF) - 8;
	spark->z = (GetRandomControl() & 0xF) - 8;
	spark->xVel = (GetRandomControl() & 0x3FF) - 512;
	spark->yVel = (GetRandomControl() & 0x3FF) - 512;
	spark->zVel = (GetRandomControl() & 0x3FF) - 512;
	spark->friction = 68;
	spark->flags = SP_ROTATE | SP_FX | SP_ROTATE | SP_DEF | SP_SCALE;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->gravity = 0;
	spark->maxYvel = 0;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->fxObj = fxObj;
	spark->scalar = 2;
	spark->sSize = spark->size = (GetRandomControl() & 0xF) + 96;
	spark->dSize = spark->size / 4;
}

void InitialiseRomanStatue(short itemNum)
{
	auto* item = &g_Level.Items[itemNum];
    
	ClearItem(itemNum);
    
    item->AnimNumber = Objects[item->ObjectNumber].animIndex + ANIMATION_ROMAN_STATUE_START_JUMP_DOWN;
    item->TargetState = 13;
    item->ActiveState = 13;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->Status = ITEM_NOT_ACTIVE;
	item->Position.xPos += 486 * phd_sin(item->Position.yRot + ANGLE(90.0f));
    item->Position.zPos += 486 * phd_cos(item->Position.yRot + ANGLE(90.0f));

	ZeroMemory(&RomanStatueData, sizeof(ROMAN_STATUE_STRUCT));
}

void RomanStatueControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	
	auto* item = &g_Level.Items[itemNumber];
	auto* creature = (CREATURE_INFO*)item->Data;
	
	int oldSwapMeshFlags = item->SwapMeshFlags;

	// At some HP values, roman statues loses a piece
	if (item->HitPoints < 1 && !(item->SwapMeshFlags & 0x10000))
	{
		ExplodeItemNode(item, 16, 0, 8);
		item->MeshBits |= 0x10000;
		item->SwapMeshFlags |= 0x10000;
	}
	else if (item->HitPoints < 75 && !(item->SwapMeshFlags & 0x100))
	{
		ExplodeItemNode(item, 8, 0, 8);
		item->MeshBits |= 0x100;
		item->SwapMeshFlags |= 0x100;
	}
	else if (item->HitPoints < 150 && !(item->SwapMeshFlags & 0x400))
	{
		ExplodeItemNode(item, 10, 0, 32);
		ExplodeItemNode(item, 11, 0, 32);
		item->MeshBits |= 0x400u;
		item->SwapMeshFlags |= 0x400;
	}
	else if (item->HitPoints < 225 && !(item->SwapMeshFlags & 0x10))
	{
		ExplodeItemNode(item, 4, 0, 8);
		item->MeshBits |= 0x10;
		item->SwapMeshFlags |= 0x10;
	}

	// Play hit animation
	if (oldSwapMeshFlags != item->SwapMeshFlags)
	{
		item->TargetState = STATE_ROMAN_STATUE_HIT;
		item->ActiveState = STATE_ROMAN_STATUE_HIT;
		item->AnimNumber = Objects[item->ObjectNumber].animIndex + ANIMATION_ROMAN_STATUE_HIT;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	}

	if (item->HitPoints > 0)
	{
		creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);
		
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		
		angle = CreatureTurn(item, creature->maximumTurn);
		
		if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			joint1 = info.xAngle;
		}

		creature->maximumTurn = 0;

		PHD_VECTOR pos, pos1, pos2;
		int deltaFrame, deltaFrame2, frameNumber;
		byte color;
		int i;
		int x, y, z;
		ROOM_INFO* room;
		FLOOR_INFO* floor;
		MESH_INFO* mesh;
		bool unk = false;
		short angles[2];
		short roomNumber;
		PHD_3DPOS attackPos;
		byte r, g, b;
		LIGHTNING_INFO* arc;
		short random;

		switch (item->ActiveState)
		{
		case STATE_ROMAN_STATUE_STOP:    
			creature->flags = 0;
			
			if (creature->mood == MoodType::Attack)
				creature->maximumTurn = ANGLE(2);
			else
			{
				creature->maximumTurn = 0;
				item->TargetState = STATE_ROMAN_STATUE_WALK;
			}
			
			joint2 = info.angle;
			
			if (item->AIBits ||
				!(GetRandomControl() & 0x1F) &&
				(info.distance > pow(SECTOR(1), 2) ||
					creature->mood != MoodType::Attack))
			{
				joint2 = AIGuard((CREATURE_INFO*)creature);
			}
			else if (info.angle > 20480 || info.angle < -20480)
				item->TargetState = STATE_ROMAN_STATUE_TURN_180;
			else if (info.ahead && info.distance < pow(SECTOR(1), 2))
			{
				if (info.bite & ((GetRandomControl() & 3) == 0))
					item->TargetState = STATE_ROMAN_STATUE_ATTACK1;
				else if (GetRandomControl() & 1)
					item->TargetState = STATE_ROMAN_STATUE_ATTACK2;
				else
					item->TargetState = STATE_ROMAN_STATUE_ATTACK3;
			}
			else
			{
				if (!item->ItemFlags[0])
				{
					item->TargetState = STATE_ROMAN_STATUE_SCREAMING;
					item->ItemFlags[0] = 5;
					break;
				}

				if (item->TriggerFlags == 1)
				{
					if (Targetable(item, &info) && GetRandomControl() & 1)
					{
						item->TargetState = STATE_ROMAN_STATUE_ENERGY_ATTACK;
						break;
					}
				}

				if (item->TriggerFlags || info.distance >= pow(SECTOR(2.5f), 2) || !info.bite)
				{
					item->TargetState = STATE_ROMAN_STATUE_WALK;
					break;
				}

				item->TargetState = STATE_ROMAN_STATUE_ATTACK1;
			}
			
			break;

		case STATE_ROMAN_STATUE_SCREAMING:    
			unk = false;

			pos1.x = -32;
			pos1.y = 48;
			pos1.z = 64;
			GetJointAbsPosition(item, &pos1, 14);

			pos2.x = -48;
			pos2.y = 48;
			pos2.z = 490;
			GetJointAbsPosition(item, &pos2, 14);

			pos.x = (pos1.x + pos2.x) / 2;
			pos.y = (pos1.y + pos2.y) / 2;
			pos.z = (pos1.z + pos2.z) / 2;

			deltaFrame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			if (deltaFrame > 68 && deltaFrame < 130)
			{
				deltaFrame2 = deltaFrame - 68;
				if (deltaFrame2 <= 58)
				{
					if (deltaFrame2 > 16)
						deltaFrame2 = 16;
				}
				else
					deltaFrame2 = 4 * (62 - deltaFrame);
				
				color = (deltaFrame2 * ((GetRandomControl() & 0x3F) + 128)) / 16;
				
				if (item->TriggerFlags)
					TriggerDynamicLight(pos.x, pos.y, pos.z, 16, 0, color, color / 2);
				else
					TriggerDynamicLight(pos.x, pos.y, pos.z, 16, 0, color / 2, color);
				
				for (int i = 0; i < 2; i++)
				{
					random = GetRandomControl();

					x = (GetRandomControl() & 0x7FF) + pos.x - 1024;
					y = (GetRandomControl() & 0x7FF) + pos.y - 1024;
					z = (random & 0x7FF) + pos.z - 1024;
					
					TriggerRomanStatueScreamingSparks(
						x,
						y,
						z,
						8 * (pos.x - x),
						8 * (pos.y - y),
						8 * (1024 - (random & 0x7FF)),
						item->TriggerFlags);
				}
			}

			if (deltaFrame <= 90 || deltaFrame >= 130)
				break;

			if (item->TriggerFlags)
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

			color = (GetRandomControl() & 0x3F) + 128;
			
			pos1.x = (GetRandomControl() & 0xFFF) + item->Position.xPos - 2048;
			pos1.y = item->Position.yPos - (GetRandomControl() & 0x3FF) - 4096;
			pos1.z = (GetRandomControl() & 0xFFF) + item->Position.zPos - 2048;
			
			for (i = 0; i < 8; i++)
			{
				arc = RomanStatueData.energyArcs[i];

				if (arc && arc->life)
				{
					arc->pos4.x = pos2.x;
					arc->pos4.y = pos2.y;
					arc->pos4.z = pos2.z;

					if (item->TriggerFlags)
						TriggerLightningGlow(pos1.x, pos1.y, pos1.z, 16, 0, color, color / 2);
					else
						TriggerLightningGlow(pos1.x, pos1.y, pos1.z, 16, 0, color / 2, color);

					continue;
				}

				if (!(GlobalCounter & 3) || unk)
				{
					unk = 1;
					continue;
				}

				if (item->TriggerFlags)
				{
					/*RomanStatueData.energyArcs[i] = TriggerEnergyArc(
						(PHD_VECTOR*)& dest.xRot,
						(PHD_VECTOR*)& dest,
						(GetRandomControl() & 0x3F) + 16,
						(color >> 1) | ((color | 0x180000) << 8),
						15,
						48,
						5);*/

					TriggerLightningGlow(pos.x, pos.y, pos.z, 16, 0, color, color / 2);
					unk = 1;
					continue;
				}

				/*RomanStatueData.energyArcs[i] = TriggerEnergyArc(
					&pos2,
					&pos1,
					(GetRandomControl() & 0x3F) + 16,
					color | (((color >> 1) | 0x180000) << 8),
					15,
					48,
					5);*/

				TriggerLightningGlow(pos.x, pos.y, pos.z, 16, 0, color / 2, color);
				unk = 1;
			}

			break;

		case STATE_ROMAN_STATUE_ATTACK1:
		case STATE_ROMAN_STATUE_ATTACK2:
		case STATE_ROMAN_STATUE_ATTACK3:
		case STATE_ROMAN_STATUE_ATTACK4:                                  
			creature->maximumTurn = 0;

			if (abs(info.angle) >= ANGLE(2.0f))
			{
				if (info.angle >= 0)
					item->Position.yRot += ANGLE(2.0f);
				else
					item->Position.yRot -= ANGLE(2.0f);
			}
			else
				item->Position.yRot += info.angle;

			if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 10)
			{
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				
				GetJointAbsPosition(item, &pos, 16);

				room = &g_Level.Rooms[item->RoomNumber];
				floor = GetSector(room, pos.x - room->x, pos.z - room->z);

				// If floor is stopped, then try to find static meshes and shatter them, activating heavy triggers below
				if (floor->Stopper)
				{
					for (i = 0; i < room->mesh.size(); i++)
					{
						mesh = &room->mesh[i];
						
						if (!((mesh->pos.zPos ^ pos.z) & 0xFFFFFC00) && !((mesh->pos.xPos ^ pos.x) & 0xFFFFFC00))
						{
							if (StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
							{
								ShatterObject(0, mesh, -64, LaraItem->RoomNumber, 0);
								SoundEffect(GetShatterSound(mesh->staticNumber), (PHD_3DPOS*)mesh, 0);

								mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
								floor->Stopper = false;

								TestTriggers(pos.x, pos.y, pos.z, item->RoomNumber, true);
							}
						}
					}
				}

				if (!creature->flags)
				{
					if (item->TouchBits & 0xC000)
					{
						LaraItem->HitPoints -= 200;
						LaraItem->HitStatus = true;
						CreatureEffect2(item, &RomanStatueBite, 20, item->Position.yRot, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Position, 0);
						creature->flags = 1;
					}
				}

				if (!item->TriggerFlags)
				{
					pos1.x = -40;
					pos1.y = 64;
					pos1.z = 360;
					
					GetJointAbsPosition(item, &pos1, 14);

					pos1.y = item->Position.yPos - 64;
					
					if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 34 && item->ActiveState == 3)
					{
						if (item->ItemFlags[0])
							item->ItemFlags[0]--;
						
						TriggerShockwave((PHD_3DPOS*)&pos1, 16, 160, 96, 0, 64, 128, 48, 0, 1);
						TriggerRomanStatueShockwaveAttackSparks(pos1.x, pos1.y, pos1.z, 128, 64, 0, 128);
						pos1.y -= 64;
						TriggerShockwave((PHD_3DPOS*)&pos1, 16, 160, 64, 0, 64, 128, 48, 0, 1);
					}

					deltaFrame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
					deltaFrame2 = g_Level.Anims[item->AnimNumber].frameEnd - item->FrameNumber;
					
					if (deltaFrame2 >= 16)
					{
						if (deltaFrame > 16)
							deltaFrame = 16;
						TriggerRomanStatueAttackEffect1(itemNumber, deltaFrame);
					}
					else
						TriggerRomanStatueAttackEffect1(itemNumber, deltaFrame2);
				}
			}
			
			break;

		case STATE_ROMAN_STATUE_WALK:
			creature->flags = 0;
			joint2 = info.angle;

			if (creature->mood == MoodType::Attack)
			{
				creature->maximumTurn = ANGLE(7);
			}
			else
			{
				creature->maximumTurn = 0;
				if (abs(info.angle) >= ANGLE(2))
				{
					if (info.angle > 0)
						item->Position.yRot += ANGLE(2);
					else
						item->Position.yRot -= ANGLE(2);
				}
				else
					item->Position.yRot += info.angle;
			}

			if (info.distance < SQUARE(1024))
			{
				item->TargetState = STATE_ROMAN_STATUE_STOP;
				break;
			}

			if (info.bite && info.distance < SQUARE(1792))
			{
				item->TargetState = 9;
				break;
			}

			if (item->TriggerFlags == 1)
			{
				if (Targetable(item, &info) && !(GetRandomControl() & 3))
				{
					item->TargetState = STATE_ROMAN_STATUE_STOP;
					break;
				}
			}

			if (item->TriggerFlags || info.distance >= SQUARE(2560))
				item->TargetState = STATE_ROMAN_STATUE_WALK;
			else
				item->TargetState = STATE_ROMAN_STATUE_STOP;

			break;

		case STATE_ROMAN_STATUE_TURN_180: 
			creature->flags = 0;
			creature->maximumTurn = 0;

			if (info.angle > 0)
				item->Position.yRot -= ANGLE(2);
			else
				item->Position.yRot += ANGLE(2);

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
				item->Position.yRot += -ANGLE(180);
		
			break;

		case STATE_ROMAN_STATUE_ENERGY_ATTACK:
			creature->flags = 0;
			creature->maximumTurn = 0;
			
			if (RomanStatueData.counter)
			{
				RomanStatueData.counter--;
				color = (RomanStatueData.counter * ((GetRandomControl() & 0x3F) + 128)) / 16;
				TriggerDynamicLight(RomanStatueData.pos.x, RomanStatueData.pos.y, RomanStatueData.pos.z, 16, 0, color, color / 2);
			}
			
			deltaFrame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			if (deltaFrame == 34)
			{
				pos1.x = -48;
				pos1.y = 48;
				pos1.z = 1024;
				GetJointAbsPosition(item, &pos1, 14);

				pos2.x = -48;
				pos2.y = 48;
				pos2.z = 450;
				GetJointAbsPosition(item, &pos2, 14);

				phd_GetVectorAngles(pos1.x - pos2.x, pos1.y - pos2.y, pos1.z - pos2.z, angles);

				attackPos.xPos = pos2.x;
				attackPos.yPos = pos2.y;
				attackPos.zPos = pos2.z;
				attackPos.xRot = angles[1];
				attackPos.yRot = angles[0];
				attackPos.zRot = 0;

				roomNumber = item->RoomNumber;
				GetFloor(pos2.x, pos2.y, pos2.z, &roomNumber);

				RomanStatueAttack(&attackPos, roomNumber, 1);

				TriggerRomanStatueShockwaveAttackSparks(
					attackPos.xPos,
					attackPos.yPos,
					attackPos.zPos,
					0, 
					(((GetRandomControl() & 0x3F) + 128) / 2),
					(((GetRandomControl() & 0x3F) + 128)),
					64);
				
				RomanStatueData.counter = 16;	
				RomanStatueData.pos.x = attackPos.xPos;
				RomanStatueData.pos.y = attackPos.yPos;
				RomanStatueData.pos.z = attackPos.zPos;
				
				if (item->ItemFlags[0])
					item->ItemFlags[0]--;
			}
			else if (deltaFrame < 10 || deltaFrame > 49)
			{
				break;
			}
			
			deltaFrame -= 10;
			if (deltaFrame < 32)
			{
				pos1.x = -32;
				pos1.y = 48;
				pos1.z = 64;
				GetJointAbsPosition(item, &pos1, 14);
				
				pos2.x = -48;
				pos2.y = 48;
				pos2.z = 490;
				GetJointAbsPosition(item, &pos2, 14);
				
				for (i = 0; i < 4; i++)
				{
					r = (deltaFrame * ((GetRandomControl() & 0x3F) + 128)) / 32;
					g = (deltaFrame * ((GetRandomControl() & 0x3F) + 128)) / 16;
					b = (deltaFrame * ((GetRandomControl() & 0x3F) + 128)) / 32;
					
					if (i == 0)
					{
						TriggerDynamicLight(
							pos2.x,
							pos2.y,
							pos2.z,
							8,
							0,
							(deltaFrame * ((GetRandomControl() & 0x3F) + 128)) / 32,
							(deltaFrame * ((GetRandomControl() & 0x3F) + 128)) / 64);
					}

					arc = RomanStatueData.energyArcs[i];

					if (arc && deltaFrame && deltaFrame != 24)
					{
						if (deltaFrame < 16)
						{
							arc->life = 56;
							arc->r = 0;
							arc->g = g;
							arc->b = b;
						}

						arc->pos1.x = pos1.x;
						arc->pos1.y = pos1.y;
						arc->pos1.z = pos1.z;
						arc->pos4.x = pos2.x;
						arc->pos4.y = pos2.y;
						arc->pos4.z = pos2.z;
					}
					else if (deltaFrame >= 16)
					{
						if (deltaFrame == 24)
						{
							/*TriggerEnergyArc(&pos1, &pos2, 0, ((GetRandomControl() & 0x3F) + 128),
								(((GetRandomControl() & 0x3F) + 128) / 2), 256, 32, 32, ENERGY_ARC_NO_RANDOMIZE,
								ENERGY_ARC_STRAIGHT_LINE);*/
						}
					}
					else
					{
						/*TriggerEnergyArc(&pos1, &pos2, 0, g, b, 256, 24, 32, ENERGY_ARC_NO_RANDOMIZE,
							ENERGY_ARC_STRAIGHT_LINE);*/

						/*RomanStatueData.energyArcs[i] = TriggerEnergyArc(
							&pos1,
							&pos2,
							(GetRandomControl() & 7) + 8,
							cb | ((cg | 0x180000) << 8),
							12,
							64,
							4);*/
					}
				}
			}

			break;

		default:
			break;

		}
	}
	else
	{
		item->HitPoints = 0;

		if (item->ActiveState == STATE_ROMAN_STATUE_DEATH)
		{
			if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 54
				&& item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 74
				&& item->TouchBits)
			{
				LaraItem->HitPoints -= 40;
				LaraItem->HitStatus = true;
			}
			else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
			{
				// Activate trigger on death
				short roomNumber = item->ItemFlags[2] & 0xFF;
				short floorHeight = item->ItemFlags[2] & 0xFF00;
				ROOM_INFO* r = &g_Level.Rooms[roomNumber];

				int x = r->x + (item->Tosspad / 256 & 0xFF) * SECTOR(1) + 512;
				int y = r->minfloor + floorHeight;
				int z = r->z + (item->Tosspad & 0xFF) * SECTOR(1) + 512;

				TestTriggers(x, y, z, roomNumber, true);
			}
		}
		else
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + ANIMATION_ROMAN_STATUE_DEATH;
			item->ActiveState = STATE_ROMAN_STATUE_DEATH;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	if (item->SwapMeshFlags & 0x400)
	{
		PHD_VECTOR pos;
		pos.x = (GetRandomControl() & 0x1F) - 16;
		pos.y = 86;
		pos.z = (GetRandomControl() & 0x1F) - 16;
		RomanStatueHitEffect(item, &pos, 10);
	}

	if (item->SwapMeshFlags & 0x10)
	{
		PHD_VECTOR pos;
		pos.x = -40;
		pos.y = (GetRandomControl() & 0x7F) + 148;
		pos.z = (GetRandomControl() & 0x3F) - 32;
		RomanStatueHitEffect(item, &pos, 4);
	}

	if (item->SwapMeshFlags & 0x100)
	{
		PHD_VECTOR pos;
		pos.x = (GetRandomControl() & 0x3F) + 54;
		pos.y = -170;
		pos.z = (GetRandomControl() & 0x1F) + 27;
		RomanStatueHitEffect(item, &pos, 8);
	}

	CreatureAnimation(itemNumber, angle, 0);
}