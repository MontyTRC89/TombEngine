#include "framework.h"
#include "tr5_guard.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/people.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/control/los.h"
#include "Specific/setup.h"
#include "Game/animation.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO SwatGun = { 80, 200, 13, 0 };
BITE_INFO SniperGun = { 0, 480, 110, 13 };
BITE_INFO ArmedBaddy2Gun = { -50, 220, 60, 13 };

#define STATE_GUARD_STOP				1
#define STATE_GUARD_TURN180				2
#define STATE_GUARD_FIRE_SINGLE			3
#define STATE_GUARD_AIM					4
#define STATE_GUARD_WALK				5
#define STATE_GUARD_RUN					7
#define STATE_GUARD_DEATH1				6
#define STATE_GUARD_DEATH2				8
#define STATE_GUARD_RECOIL				11
#define STATE_GUARD_CROUCH				13
#define STATE_GUARD_ROPE_DOWN			14
#define STATE_GUARD_SITTING				15
#define STATE_GUARD_STAND_UP			16
#define STATE_GUARD_WAITING_ON_WALL		19
#define STATE_GUARD_STOP_START_JUMP		26
#define STATE_GUARD_JUMPING_1BLOCK		27
#define STATE_GUARD_JUMPING_2BLOCKS		28
#define STATE_GUARD_LANDING				29
#define STATE_GUARD_HUNTING				30
#define STATE_GUARD_HUNTING_IDLE		31
#define STATE_GUARD_FIRE_FAST			35
#define STATE_GUARD_INSERT_CODE			36
#define STATE_GUARD_START_USE_COMPUTER	37
#define STATE_GUARD_USE_COMPUTER		38
#define STATE_GUARD_SURREND				39

#define ANIMATION_GUARD_DEATH1			11
#define ANIMATION_GUARD_DEATH2			16
#define ANIMATION_GUARD_START_JUMP		41

#define STATE_MAFIA2_STOP					1
#define STATE_MAFIA2_TURN180_UNDRAW_GUNS	2
#define STATE_MAFIA2_FIRE					3
#define STATE_MAFIA2_AIM					4
#define STATE_MAFIA2_WALK					5
#define STATE_MAFIA2_DEATH1					6
#define STATE_MAFIA2_RUN					7
#define STATE_MAFIA2_DEATH2					8
#define STATE_MAFIA2_STOP_START_JUMP		26
#define STATE_MAFIA2_JUMPING_1BLOCK			27
#define STATE_MAFIA2_JUMPING_2BLOCKS		28
#define STATE_MAFIA2_LANDING				29
#define STATE_MAFIA2_TURN180				32
#define STATE_MAFIA2_UNDRAW_GUNS			37

#define STATE_SNIPER_STOP					1
#define STATE_SNIPER_UNHIDE					2
#define STATE_SNIPER_AIM					3
#define STATE_SNIPER_FIRE					4
#define STATE_SNIPER_HIDE					5
#define STATE_SNIPER_DEATH					6

void InitialiseGuard(short itemNum)
{
    ITEM_INFO* item, *item2;
    short anim;
    short roomItemNumber;
    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    anim = Objects[ID_SWAT].animIndex;
    if (!Objects[ID_SWAT].loaded)
        anim = Objects[ID_GUARD1].animIndex;
    switch (item->TriggerFlags)
    {
        case 0:
        case 10:
            item->AnimNumber = anim;
            item->TargetState = STATE_GUARD_STOP;
            break;
        case 1:
            item->TargetState = STATE_GUARD_RECOIL;
            item->AnimNumber = anim + 23;
            break;
        case 2:
            item->TargetState = STATE_GUARD_CROUCH;
            item->AnimNumber = anim + 25;
            // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
            break;
        case 3:
            item->TargetState = STATE_GUARD_SITTING;
            item->AnimNumber = anim + 28;
            item->SwapMeshFlags = 9216;
            roomItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber;
            if (roomItemNumber != NO_ITEM)
            {
                while (true)
                {
                    item2 = &g_Level.Items[roomItemNumber];
                    if (item2->ObjectNumber >= ID_ANIMATING1 && item2->ObjectNumber <= ID_ANIMATING15 && item2->RoomNumber == item->RoomNumber && item2->TriggerFlags == 3)
                        break;
                    roomItemNumber = item2->NextItem;
                    if (roomItemNumber == NO_ITEM)
                    {
                        item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
                        item->ActiveState = item->TargetState;
                        break;
                    }
                }
                item2->MeshBits = -5;
            }
            break;
        case 4:
            item->TargetState = 17;
			item->SwapMeshFlags = 8192;
            item->AnimNumber = anim + 30;
            break;
        case 5:
            FLOOR_INFO *floor;
            short roomNumber;
            item->AnimNumber = anim + 26;
            item->TargetState = STATE_GUARD_ROPE_DOWN;
            roomNumber = item->RoomNumber;
            floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
            GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
            item->Position.yPos = GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) - SECTOR(2);
            break;
        case 6:
            item->TargetState = STATE_GUARD_WAITING_ON_WALL;
            item->AnimNumber = anim + 32;
            break;
        case 7:
        case 9:
            item->TargetState = STATE_GUARD_USE_COMPUTER;
            item->AnimNumber = anim + 59;
            item->Position.xPos -= 512 * phd_sin(item->Position.yRot);
            item->Position.zPos -= 512 * phd_cos(item->Position.yRot);
            break;
        case 8:
            item->TargetState = STATE_GUARD_HUNTING_IDLE;
            item->AnimNumber = anim + 46;
            break;
        case 11:
            item->TargetState = STATE_GUARD_RUN;
            item->AnimNumber = anim + 12;
            break;
        default:
            break;
    }
}

void InitialiseSniper(short itemNum)
{
    ITEM_INFO* item;
    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->AnimNumber = Objects[item->ObjectNumber].animIndex;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = STATE_SNIPER_STOP;
    item->ActiveState = STATE_SNIPER_STOP;
	item->Position.yPos += STEP_SIZE * 2;
    item->Position.xPos += 1024 * phd_sin(item->Position.yRot + ANGLE(90));
    item->Position.zPos += 1024 * phd_cos(item->Position.yRot + ANGLE(90));
}

void InitialiseGuardLaser(short itemNum)
{
    ITEM_INFO* item;
    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->AnimNumber = Objects[item->ObjectNumber].animIndex + 6;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = STATE_GUARD_STOP;
    item->ActiveState = STATE_GUARD_STOP;
}

void ControlGuardLaser(short itemNumber)
{

}

void GuardControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	int animIndex = 0;
	if (Objects[ID_SWAT].loaded)
		animIndex= Objects[ID_SWAT].animIndex;
	else
		animIndex = Objects[ID_GUARD1].animIndex;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	int x = item->Position.xPos;
	int z = item->Position.zPos;
	int dx = 870 * phd_sin(item->Position.yRot);
	int dz = 870 * phd_cos(item->Position.yRot);
	x += dx;
	z += dz;
	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, item->Position.yPos, z);
	x += dx;
	z += dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, item->Position.yPos, z);
	x += dx;
	z += dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, item->Position.yPos, z);
	bool canJump1block;
	if (item->BoxNumber == LaraItem->BoxNumber
		|| item->Position.yPos >= height1 - 384
		|| item->Position.yPos >= height2 + 256
		|| item->Position.yPos <= height2 - 256)
		canJump1block = false;
	else
		canJump1block = true;
	bool canJump2blocks;
	if (item->BoxNumber == LaraItem->BoxNumber 
		|| item->Position.yPos >= height1 - 384 
		|| item->Position.yPos >= height2 - 384
		|| item->Position.yPos >= height3 + 256 
		|| item->Position.yPos <= height3 - 256)
		canJump2blocks = false;
	else
		canJump2blocks = true;
	if (item->FiredWeapon)
	{
		PHD_VECTOR pos;
		pos.x = SwatGun.x;
		pos.y = SwatGun.y;
		pos.z = SwatGun.z;
		GetJointAbsPosition(item, &pos, SwatGun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->FiredWeapon + 10, 192, 128, 32);
		item->FiredWeapon--;
	}
	if (item->AIBits)
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
		int dx = LaraItem->Position.xPos - item->Position.xPos;
		int dz = LaraItem->Position.zPos - item->Position.zPos;
		laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
		laraInfo.distance = SQUARE(dx) + SQUARE(dz);
	}
	
	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != STATE_GUARD_DEATH1 && item->ActiveState != STATE_GUARD_DEATH2)
		{
			if (laraInfo.angle >= 12288 || laraInfo.angle <= -12288)
			{
				item->ActiveState = STATE_GUARD_DEATH2;
				item->AnimNumber = animIndex + ANIMATION_GUARD_DEATH2;
				item->Position.yRot += laraInfo.angle + -ANGLE(180);
			}
			else
			{
				item->ActiveState = STATE_GUARD_DEATH1;
				item->AnimNumber = animIndex + ANIMATION_GUARD_DEATH1;
				item->Position.yRot += laraInfo.angle;
			}
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
	else
	{
		GetCreatureMood(item, &info, creature->enemy != LaraItem);
		if (item->ObjectNumber == ID_SCIENTIST)
		{
			if (item->HitPoints >= Objects[ID_SCIENTIST].HitPoints)
			{
				if (creature->enemy == LaraItem)
					creature->mood = BORED_MOOD;
			}
			else
			{
				creature->mood = ESCAPE_MOOD;
			}
		}
		if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_NO_LENSFLARE) // CHECK
		{
			if (item->ObjectNumber == ID_SWAT_PLUS)
			{
				item->ItemFlags[0]++;
				if (item->ItemFlags[0] > 60 && !(GetRandomControl() & 0xF))
				{
					SoundEffect(SFX_TR5_BIO_BREATHE_OUT, &item->Position, 0);
					item->ItemFlags[0] = 0;
				}
			}
			else
			{
				if (!(GlobalCounter & 7))
					item->HitPoints--;
				creature->mood = ESCAPE_MOOD;
				if (item->HitPoints <= 0)
				{
					item->ActiveState = STATE_GUARD_DEATH2;
					item->AnimNumber = animIndex + ANIMATION_GUARD_DEATH2;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				}
			}
		}
		CreatureMood(item, &info, creature->enemy != LaraItem);
		ITEM_INFO * enemy = creature->enemy;
		angle = CreatureTurn(item, creature->maximumTurn);
		creature->enemy = LaraItem;
		if (laraInfo.distance < 0x400000 && LaraItem->Velocity > 20
			|| item->HitStatus
			|| TargetVisible(item, &laraInfo))
		{
			if (!(item->AIBits & FOLLOW) && item->ObjectNumber != ID_SCIENTIST && abs(item->Position.yPos - LaraItem->Position.yPos) < 1280)
			{
				creature->enemy = LaraItem;
				AlertAllGuards(itemNum);
			}
		}
		creature->enemy = enemy;
		GAME_VECTOR src;
		src.x = item->Position.xPos;
		src.y = item->Position.yPos - 384;
		src.z = item->Position.zPos;
		src.roomNumber = item->RoomNumber;
		ANIM_FRAME* frame = GetBestFrame(LaraItem);
		GAME_VECTOR dest;
		dest.x = LaraItem->Position.xPos;
		dest.y = LaraItem->Position.yPos + ((frame->boundingBox.Y2 + 3 * frame->boundingBox.Y1) / 4);
		dest.z = LaraItem->Position.zPos;
		bool los = !LOS(&src, &dest) && item->TriggerFlags != 10;
		creature->maximumTurn = 0;
		ITEM_INFO* currentItem;
		short currentItemNumber;
		PHD_VECTOR pos1, pos2;
		PHD_VECTOR pos;
		switch (item->ActiveState)
		{
		case STATE_GUARD_STOP:
			creature->LOT.isJumping = false;
			joint2 = laraInfo.angle;
			creature->flags = 0;
			if (info.ahead)
			{
				if (!(item->AIBits & FOLLOW))
				{
					joint0 = info.angle / 2;
					joint1 = info.xAngle;
				}
			}
			if (item->ObjectNumber == ID_SCIENTIST && item == Lara.target)
			{
				item->TargetState = STATE_GUARD_SURREND;
			}
			else if (item->RequiredState)
			{
				item->TargetState = item->RequiredState;
			}
			else if (item->AIBits & GUARD)
			{
				if (item->AIBits & MODIFY)
					joint2 = 0;
				else
					joint2 = AIGuard(creature);
				if (item->AIBits & PATROL1)
				{
					item->TriggerFlags--;
					if (item->TriggerFlags < 1)
						item->AIBits &= ~GUARD;
				}
			}
			else if (creature->enemy == LaraItem && (laraInfo.angle > 20480 || laraInfo.angle < -20480) && item->ObjectNumber != ID_SCIENTIST)
			{
				item->TargetState = STATE_GUARD_TURN180;
			}
			else if (item->AIBits & PATROL1)
			{
				item->TargetState = STATE_GUARD_WALK;
			}
			else if (item->AIBits & AMBUSH)
			{
				item->TargetState = STATE_GUARD_RUN;
			}
			else if (Targetable(item, &info) && item->ObjectNumber != ID_SCIENTIST)
			{
				if (info.distance >= 0x1000000 && info.zoneNumber == info.enemyZone)
				{
					if (!(item->AIBits & MODIFY))
						item->TargetState = STATE_GUARD_WALK;
				}
				else
					item->TargetState = STATE_GUARD_AIM;
			}
			else if (canJump1block || canJump2blocks)
			{
				creature->maximumTurn = 0;
				item->AnimNumber = animIndex + 41;
				item->ActiveState = STATE_GUARD_STOP_START_JUMP;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				if (canJump1block)
					item->TargetState = STATE_GUARD_JUMPING_1BLOCK;
				else
					item->TargetState = STATE_GUARD_JUMPING_2BLOCKS;
				creature->LOT.isJumping = true;
			}
			else if (los)
			{
				item->TargetState = STATE_GUARD_HUNTING_IDLE;
			}
			else if (creature->mood)
			{
				if (info.distance < 0x900000 || item->AIBits & FOLLOW)
				{
					item->TargetState = STATE_GUARD_WALK;
				}
				else
					item->TargetState = STATE_GUARD_RUN;
			}
			else
			{
				item->TargetState = STATE_GUARD_STOP;
			}
			if (item->TriggerFlags == 11)
				item->TriggerFlags = 0;
			break;
		case STATE_GUARD_TURN180:
			creature->flags = 0;
			if (info.angle >= 0)
				item->Position.yRot -= ANGLE(2);
			else
				item->Position.yRot += ANGLE(2);
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
				item->Position.yRot += -ANGLE(180);
			break;
		case STATE_GUARD_FIRE_SINGLE:
		case STATE_GUARD_FIRE_FAST:
			joint0 = laraInfo.angle / 2;
			joint2 = laraInfo.angle / 2;
			if (info.ahead)
				joint1 = info.xAngle;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
					item->Position.yRot += ANGLE(2);
				else
					item->Position.yRot -= ANGLE(2);
			}
			else
			{
				item->Position.yRot += info.angle;
			}
			if (item->ActiveState == STATE_GUARD_FIRE_FAST)
			{
				if (creature->flags)
				{
					if (item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 10
						&& (item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase) & 1)
						creature->flags = 0;
				}
			}
			if (!creature->flags)
			{
				creature->flags = 1;
				item->FiredWeapon = 2;
				if (item->ActiveState == STATE_GUARD_FIRE_SINGLE)
					ShotLara(item, &info, &SwatGun, joint0, 30);
				else
					ShotLara(item, &info, &SwatGun, joint0, 10);
				
				// TODO: just for testing energy arcs
				/*pos1.x = SwatGun.x;
				pos1.y = SwatGun.y;
				pos1.z = SwatGun.z;
				GetJointAbsPosition(item, &pos1, SwatGun.meshNum);
				TriggerEnergyArc(&pos1, (PHD_VECTOR*)& LaraItem->pos, 192, 128, 192, 256, 150, 256, 0, ENERGY_ARC_STRAIGHT_LINE);*/

			}
			break;
		case STATE_GUARD_AIM:
			creature->flags = 0;
			joint0 = laraInfo.angle / 2;
			joint2 = laraInfo.angle / 2;
			if (info.ahead)
				joint1 = info.xAngle;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
					item->Position.yRot += ANGLE(2);
				else
					item->Position.yRot -= ANGLE(2);
			}
			else
			{
				item->Position.yRot += info.angle;
			}
			if (!Targetable(item, &info))
				item->TargetState = STATE_GUARD_STOP;
			else if (item->ObjectNumber == ID_GUARD1 || item->ObjectNumber == ID_GUARD2)
				item->TargetState = STATE_GUARD_FIRE_SINGLE;
			else
				item->TargetState = STATE_GUARD_FIRE_FAST;
			break;
		case STATE_GUARD_WALK:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(5);
			if (!Targetable(item, &info)
				|| info.distance >= 0x1000000 && info.zoneNumber == info.enemyZone
				|| item->ObjectNumber == ID_SCIENTIST
				|| item->AIBits & AMBUSH || item->AIBits & PATROL1) // CHECK
			{
				if (canJump1block || canJump2blocks)
				{
					creature->maximumTurn = 0;
					item->AnimNumber = animIndex + 41;
					item->ActiveState = STATE_GUARD_STOP_START_JUMP;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					if (canJump1block)
						item->TargetState = STATE_GUARD_JUMPING_1BLOCK;
					else
						item->TargetState = STATE_GUARD_JUMPING_2BLOCKS;
					creature->LOT.isJumping = true;
				}
				else if (info.distance >= 0x100000)
				{
					if (!los || item->AIBits)
					{
						if (info.distance > 0x900000)
						{
							if (!(item->InDrawRoom))
								item->TargetState = STATE_GUARD_RUN;
						}
					}
					else
					{
						item->TargetState = STATE_GUARD_STOP;
					}
				}
				else
				{
					item->TargetState = STATE_GUARD_STOP;
				}
			}
			else
			{
				item->TargetState = STATE_GUARD_AIM;
			}
			break;
		case STATE_GUARD_RUN:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(10);
			if (Targetable(item, &info) && (info.distance < 0x1000000 || info.enemyZone == info.zoneNumber) && item->ObjectNumber != ID_SCIENTIST)
			{
				item->TargetState = STATE_GUARD_AIM;
			}
			else if (canJump1block || canJump2blocks)
			{
				creature->maximumTurn = 0;
				item->AnimNumber = animIndex + 50;
				item->ActiveState = STATE_GUARD_STOP_START_JUMP;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				if (canJump1block)
					item->TargetState = STATE_GUARD_JUMPING_1BLOCK;
				else
					item->TargetState = STATE_GUARD_JUMPING_2BLOCKS;
				creature->LOT.isJumping = true;
			}
			else if (los)
			{
				item->TargetState = STATE_GUARD_STOP;
			}
			else if (info.distance < 0x900000)
			{
				item->TargetState = STATE_GUARD_WALK;
			}
			if (item->TriggerFlags == 11)
			{
				creature->LOT.isJumping = true;
				creature->maximumTurn = 0;
			}
			break;
		case STATE_GUARD_ROPE_DOWN:
			joint2 = laraInfo.angle;
			if (item->Position.yPos <= item->Floor - 2048 || item->TriggerFlags != 5)
			{
				if (item->Position.yPos >= item->Floor - 512)
					item->TargetState = STATE_GUARD_AIM;
			}
			else
			{
				item->TriggerFlags = 0;
				TestTriggers(item, true);
				SoundEffect(SFX_TR4_LARA_POLE_LOOP, &item->Position, 0);
			}
			if (abs(info.angle) >= 364)
			{
				if ((info.angle & 0x8000) == 0)
					item->Position.yRot += ANGLE(2);
				else
					item->Position.yRot -= ANGLE(2);
			}
			else
			{
				item->Position.yRot += info.angle;
			}
			break;
		case STATE_GUARD_SITTING:
			joint2 = AIGuard(creature);
			if (creature->alerted)
				item->TargetState = STATE_GUARD_STAND_UP;
			break;
		case STATE_GUARD_STAND_UP:
		case 18:
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
			{
				TestTriggers(item, true);
				break;
			}
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 44)
			{
				item->SwapMeshFlags = 0;
				short currentItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber;
				if (currentItemNumber == NO_ITEM)
					break;
				ITEM_INFO * currentItem;
				while (true)
				{
					currentItem = &g_Level.Items[currentItemNumber];
					if (currentItem->ObjectNumber >= ID_ANIMATING1
						&& currentItem->ObjectNumber <= ID_ANIMATING15
						&& currentItem->RoomNumber == item->RoomNumber)
					{
						if (currentItem->TriggerFlags > 2 && currentItem->TriggerFlags < 5)
							break;
					}
					currentItemNumber = currentItem->NextItem;
					if (currentItemNumber == -1)
						break;
				}
				if (currentItemNumber == NO_ITEM)
					break;
				currentItem->MeshBits = -3;
			}
			else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
			{
				item->Position.yRot -= ANGLE(90);
			}
			break;
		case 17:
			joint2 = 0;
			if (!item->HitStatus && LaraItem->Velocity < 40 && !Lara.hasFired)
				creature->alerted = false;
			if (creature->alerted)
				item->TargetState = 18;
			break;
		case STATE_GUARD_WAITING_ON_WALL:
			joint2 = AIGuard(creature);
			if (creature->alerted)
				item->TargetState = STATE_GUARD_STOP;
			break;
		case STATE_GUARD_HUNTING:
		case STATE_GUARD_HUNTING_IDLE:
			if (item->ActiveState == STATE_GUARD_HUNTING_IDLE)
			{
				if (item->TriggerFlags != 8 || !los || item->HitStatus)
					item->TargetState = STATE_GUARD_HUNTING;
			}
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(5);
			if (canJump1block || canJump2blocks || info.distance < 0x100000 || !los || item->HitStatus)
				item->TargetState = STATE_GUARD_STOP;
			break;
		case STATE_GUARD_INSERT_CODE:
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 39)
			{
				TestTriggers(item, true);
			}
			break;
		case STATE_GUARD_START_USE_COMPUTER:
			currentItem = NULL;
			for (currentItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber; currentItemNumber != NO_ITEM; currentItemNumber = currentItem->NextItem)
			{
				currentItem = &g_Level.Items[currentItemNumber];
				if (item->ObjectNumber == ID_PUZZLE_HOLE8)
					break;
			}
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
			{
				currentItem->MeshBits = 0x1FFF;
				item->Position.xPos = currentItem->Position.xPos - 256;
				item->Position.yRot = currentItem->Position.yRot;
				item->Position.zPos = currentItem->Position.zPos + 128;
				item->SwapMeshFlags = 1024;
			}
			else
			{
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 32)
				{
					currentItem->MeshBits = 16381;
				}
				else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 74)
				{
					currentItem->MeshBits = 278461;
				}
				else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 120)
				{
					currentItem->MeshBits = 802621;
				}
				else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 157)
				{
					currentItem->MeshBits = 819001;
				}
				else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 190)
				{
					currentItem->MeshBits = 17592121;
				}
				else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + g_Level.Anims[item->AnimNumber].frameEnd)
				{
					currentItem->MeshBits = 0x1FFF;
					TestTriggers(item, true);
					item->RequiredState = STATE_GUARD_WALK;
					item->SwapMeshFlags = 0;
				}
			}
			break;
		case STATE_GUARD_USE_COMPUTER:
			if ((item->ObjectNumber != ID_SCIENTIST || item != Lara.target)
				&& (GetRandomControl() & 0x7F || item->TriggerFlags >= 10 || item->TriggerFlags == 9))
			{
				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);
					if (item->AIBits & PATROL1)
					{
						item->TriggerFlags--;
						if (item->TriggerFlags < 1)
						{
							item->AIBits = PATROL1 | MODIFY;
						}
					}
				}
			}
			else
			{
				item->TargetState = STATE_GUARD_STOP;
			}
			break;
		case STATE_GUARD_SURREND:
			if (item != Lara.target && !(GetRandomControl() & 0x3F))
			{
				if (item->TriggerFlags == 7 || item->TriggerFlags == 9)
					item->RequiredState = STATE_GUARD_USE_COMPUTER;
				item->TargetState = STATE_GUARD_STOP;
			}
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 39)
			{
				TestTriggers(item->Position.xPos, item->Position.yPos, item->Position.zPos, enemy->RoomNumber, true);
			}
			break;
		default:
			break;
		}
	}
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	if (creature->reachedGoal && creature->enemy)
	{
		ITEM_INFO* enemy = creature->enemy;
		
		if (enemy->Flags != 4)
		{
			if (enemy->Flags & 0x10)
			{
				item->TargetState = STATE_GUARD_STOP;
				item->RequiredState = STATE_GUARD_USE_COMPUTER;
				item->TriggerFlags = 300;
				item->AIBits = GUARD | PATROL1;
			}
			else
			{
				if (enemy->Flags & 0x20)
				{
					item->TargetState = STATE_GUARD_STOP;
					item->RequiredState = 36;
					item->AIBits = PATROL1 | MODIFY;
				}
				else
				{
					TestTriggers(creature->enemy->Position.xPos, creature->enemy->Position.yPos, creature->enemy->Position.zPos, enemy->RoomNumber, true);

					item->RequiredState = STATE_GUARD_WALK;
					if (creature->enemy->Flags & 2)
						item->ItemFlags[3] = (item->Tosspad & 0xFF) - 1;
					if (creature->enemy->Flags & 8)
					{
						item->RequiredState = STATE_GUARD_STOP;
						item->TriggerFlags = 300;
						item->AIBits |= GUARD | PATROL1;
					}
				}
			}
		}
		else
		{
			item->TargetState = STATE_GUARD_STOP;
			item->RequiredState = 37;
		}
	}
	if ((item->ActiveState >= 20 
		|| item->ActiveState == 6 
		|| item->ActiveState == 8) 
		&& item->ActiveState != 30)
	{
		CreatureAnimation(itemNum, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNum, angle, 2, 256) + 4)
		{
		case 0:
			creature->maximumTurn = 0;
			item->AnimNumber = animIndex + 38;
			item->ActiveState = 23;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 1:
			creature->maximumTurn = 0;
			item->AnimNumber = animIndex + 39;
			item->ActiveState = 24;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 2:
			creature->maximumTurn = 0;
			item->AnimNumber = animIndex + 40;
			item->ActiveState = 25;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 6:
			creature->maximumTurn = 0;
			item->AnimNumber = animIndex + 35;
			item->ActiveState = 20;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 7:
			creature->maximumTurn = 0; 
			item->AnimNumber = animIndex + 36;
			item->ActiveState = 21;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 8:
			creature->maximumTurn = 0;
			item->AnimNumber = animIndex + 37;
			item->ActiveState = 22;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		}
	}
}

void SniperControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short angle = 0;
		short joint0 = 0;
		short joint2 = 0;
		short joint1 = 0;
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
		if (item->FiredWeapon)
		{
			PHD_VECTOR pos;
			pos.x = SniperGun.x;
			pos.y = SniperGun.y;
			pos.z = SniperGun.z;
			GetJointAbsPosition(item, &pos, SniperGun.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->FiredWeapon + 10, 192, 128, 32);
			item->FiredWeapon--;
		}
		if (item->HitPoints > 0)
		{
			if (item->AIBits)
			{
				GetAITarget(creature);
			}
			else if (creature->hurtByLara)
			{
				creature->enemy = LaraItem;
			}
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
			switch (item->ActiveState)
			{
			case STATE_SNIPER_STOP:
				item->MeshBits = 0;
				if (TargetVisible(item, &info))
					item->TargetState = STATE_SNIPER_UNHIDE;
				break;
			case STATE_SNIPER_UNHIDE:
				item->MeshBits = -1;
				break;
			case 3:
				creature->flags = 0;
				if (!TargetVisible(item, &info)
					|| item->HitStatus
					&& GetRandomControl() & 1)
				{
					item->TargetState = STATE_SNIPER_HIDE;
				}
				else if (!(GetRandomControl() & 0x1F))
				{
					item->TargetState = STATE_SNIPER_FIRE;
				}
				break;
			case STATE_SNIPER_FIRE:
				if (!creature->flags)
				{
					ShotLara(item, &info, &SniperGun, joint0, 100);
					creature->flags = 1;
					item->FiredWeapon = 2;
				}
				break;
			default:
				break;
			}
		}
		else
		{
			item->HitPoints = 0;
			if (item->ActiveState != STATE_SNIPER_DEATH)
			{
				item->AnimNumber = Objects[ID_SNIPER].animIndex + 5;
				item->ActiveState = STATE_SNIPER_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}
		CreatureTilt(item, 0);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}
}

void InitialiseMafia2(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	ClearItem(itemNum);
	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = STATE_GUARD_STOP;
	item->ActiveState = STATE_GUARD_STOP;
	item->SwapMeshFlags = 9216;
}

void Mafia2Control(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;
	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	// Can baddy jump? Check for a distance of 1 and 2 sectors
	int x = item->Position.xPos;
	int y = item->Position.yPos;
	int z = item->Position.zPos;
	int dx = 870 * phd_sin(item->Position.yRot);
	int dz = 870 * phd_cos(item->Position.yRot);
	x += dx;
	z += dz;
	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);
	x += dx;
	z += dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);
	x += dx;
	z += dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, y, z);
	int height = 0;
	bool canJump1sector = true;
	if (item->BoxNumber == LaraItem->BoxNumber
		|| y >= height1 - 384
		|| y >= height2 + 256
		|| y <= height2 - 256)
	{
		height = height2;
		canJump1sector = false;
	}
	bool canJump2sectors = true;
	if (item->BoxNumber == LaraItem->BoxNumber
		|| y >= height1 - 384
		|| y >= height - 384
		|| y >= height3 + 256
		|| y <= height3 - 256)
	{
		canJump2sectors = false;
	}
	if (item->FiredWeapon)
	{
		PHD_VECTOR pos;
		pos.x = ArmedBaddy2Gun.x;
		pos.y = ArmedBaddy2Gun.y;
		pos.z = ArmedBaddy2Gun.z;
		GetJointAbsPosition(item, &pos, ArmedBaddy2Gun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->FiredWeapon + 8, 24, 16, 4);
		item->FiredWeapon--;
	}
	AI_INFO info;
	AI_INFO laraInfo;
	ZeroMemory(&info, sizeof(AI_INFO));
	if (item->HitPoints > 0)
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;
		CreatureAIInfo(item, &info);
		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}
		GetCreatureMood(item, &info, creature->enemy != LaraItem);
		CreatureMood(item, &info, creature->enemy != LaraItem);
		angle = CreatureTurn(item, creature->maximumTurn);
		creature->enemy = LaraItem;
		if (laraInfo.distance < SQUARE(2048) && LaraItem->Velocity > 20 || item->HitStatus || TargetVisible(item, &laraInfo))
		{
			if (!(item->AIBits & FOLLOW))
			{
				creature->enemy = LaraItem;
				AlertAllGuards(itemNum);
			}
		}
		switch (item->ActiveState)
		{
		case STATE_MAFIA2_STOP:
			creature->LOT.isJumping = false;
			joint2 = laraInfo.angle;
			creature->flags = 0;
			creature->maximumTurn = 0;
			if (info.ahead && !(item->AIBits & GUARD))
			{
				joint0 = info.angle / 2;
				joint1 = info.xAngle;
			}
			if (item->AIBits & GUARD)
			{
				joint2 = AIGuard(creature);
				break;
			}
			if (laraInfo.angle <= 20480 && laraInfo.angle >= -20480)
			{
				if (item->SwapMeshFlags == 9216)
				{
					item->TargetState = STATE_MAFIA2_UNDRAW_GUNS;
					break;
				}
			}
			else if (item->SwapMeshFlags == 9216)
			{
				item->TargetState = STATE_MAFIA2_TURN180;
				break;
			}
			if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone)
				{
					item->TargetState = STATE_MAFIA2_AIM;
				}
				else if (!(item->AIBits & MODIFY))
				{
					item->TargetState = STATE_MAFIA2_WALK;
				}
			}
			else
			{
				if (item->AIBits & PATROL1)
				{
					item->TargetState = STATE_MAFIA2_WALK;
				}
				else
				{
					if (canJump1sector || canJump2sectors)
					{
						creature->maximumTurn = 0;
						item->AnimNumber = Objects[item->ObjectNumber].animIndex + 41;
						item->ActiveState = STATE_MAFIA2_STOP_START_JUMP;
						item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
						if (canJump2sectors)
							item->TargetState = STATE_MAFIA2_JUMPING_2BLOCKS;
						else
							item->TargetState = STATE_MAFIA2_JUMPING_1BLOCK;
						creature->LOT.isJumping = true;
						break;
					}
					if (creature->mood)
					{
						if (info.distance >= SQUARE(3072))
							item->TargetState = STATE_MAFIA2_WALK;
					}
					else
					{
						item->TargetState = STATE_MAFIA2_STOP;
					}
				}
			}
			break;
		case STATE_MAFIA2_TURN180_UNDRAW_GUNS:
		case STATE_MAFIA2_TURN180:
			creature->maximumTurn = 0;
			if (info.angle >= 0)
				item->Position.yRot -= ANGLE(2);
			else
				item->Position.yRot += ANGLE(2);
			if (item->FrameNumber != g_Level.Anims[item->AnimNumber].frameBase + 16 
				|| item->SwapMeshFlags != 9216)
			{
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
					item->Position.yRot += -ANGLE(180);
			}
			else
			{
				item->SwapMeshFlags = 128;
			}
			break;
		case STATE_MAFIA2_FIRE:
			joint0 = laraInfo.angle / 2;
			joint2 = laraInfo.angle / 2;
			if (info.ahead)
				joint1 = info.xAngle;
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
					item->Position.yRot += ANGLE(2);
				else
					item->Position.yRot -= ANGLE(2);
			}
			else
			{
				item->Position.yRot += info.angle;
			}
			if (!creature->flags)
			{
				ShotLara(item, &info, &ArmedBaddy2Gun, laraInfo.angle / 2, 35);
				creature->flags = 1;
				item->FiredWeapon = 2;
			}
			break;
		case STATE_MAFIA2_AIM:
			joint0 = laraInfo.angle / 2;
			joint2 = laraInfo.angle / 2;
			creature->flags = 0;
			creature->maximumTurn = 0;
			if (info.ahead)
				joint1 = info.xAngle;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
					item->Position.yRot += ANGLE(2);
				else
					item->Position.yRot -= ANGLE(2);
			}
			else
			{
				item->Position.yRot += info.angle;
			}
			if (Targetable(item, &info))
			{
				item->TargetState = STATE_MAFIA2_FIRE;
			}
			else if (laraInfo.angle > 20480 || laraInfo.angle < -20480)
			{
				item->TargetState = 32;
			}
			else
			{
				item->TargetState = STATE_MAFIA2_STOP;
			}
			break;
		case STATE_MAFIA2_WALK:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(5);
			if (Targetable(item, &info) && (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone))
			{
				item->TargetState = STATE_MAFIA2_AIM;
			}
			else
			{
				if (canJump1sector || canJump2sectors)
				{
					creature->maximumTurn = 0;
					creature->maximumTurn = 0;
					item->AnimNumber = Objects[item->ObjectNumber].animIndex + 41;
					item->ActiveState = STATE_MAFIA2_STOP_START_JUMP;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					if (canJump2sectors)
						item->TargetState = STATE_MAFIA2_JUMPING_2BLOCKS;
					else
						item->TargetState = STATE_MAFIA2_JUMPING_1BLOCK;
					creature->LOT.isJumping = true;
					break;
				}
				if (info.distance >= SQUARE(1024))
				{
					if (info.distance > SQUARE(3072))
						item->TargetState = STATE_MAFIA2_RUN;
				}
				else
				{
					item->TargetState = STATE_MAFIA2_STOP;
				}
			}
			break;
		case STATE_MAFIA2_RUN:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(10);
			if (Targetable(item, &info) && (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone))
			{
				item->TargetState = STATE_MAFIA2_AIM;
			}
			else if (canJump1sector || canJump2sectors)
			{
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 50;
				item->ActiveState = STATE_MAFIA2_STOP_START_JUMP;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				if (canJump2sectors)
					item->TargetState = STATE_MAFIA2_JUMPING_2BLOCKS;
				else
					item->TargetState = STATE_MAFIA2_JUMPING_1BLOCK;
				creature->LOT.isJumping = true;
			}
			else if (info.distance < SQUARE(3072))
			{
				item->TargetState = STATE_MAFIA2_WALK;
			}
			break;
		case STATE_MAFIA2_UNDRAW_GUNS:
			creature->maximumTurn = 0;
			if (info.angle >= 0)
				item->Position.yRot += ANGLE(2);
			else
				item->Position.yRot -= ANGLE(2);
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 16 
				&& item->SwapMeshFlags == 9216)
				item->SwapMeshFlags = 128;
			break;
		default:
			break;
		}
	}
	else
	{
		if (item->ActiveState != STATE_MAFIA2_DEATH2 && item->ActiveState != STATE_MAFIA2_DEATH1)
		{
			if (info.angle >= 12288 || info.angle <= -12288)
			{
				item->ActiveState = STATE_MAFIA2_DEATH2;
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 16;
				item->Position.yRot += info.angle - ANGLE(180);
			}
			else
			{
				item->ActiveState = STATE_MAFIA2_DEATH1;
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 11;
				item->Position.yRot += info.angle;
			}
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	if (item->ActiveState >= 20 || item->ActiveState == 6 || item->ActiveState == 8)
	{
		CreatureAnimation(itemNum, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNum, angle, 2, 256) + 4)
		{
		case 0:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 38;
			item->ActiveState = 23;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 1:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 39;
			item->ActiveState = 24;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 2:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 40;
			item->ActiveState = 25;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 6:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 35;
			item->ActiveState = 20;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 7:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 36;
			item->ActiveState = 21;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		case 8:
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 37;
			item->ActiveState = 22;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			break;
		default:
			return;
		}
	}
}