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
    switch (item->triggerFlags)
    {
        case 0:
        case 10:
            item->animNumber = anim;
            item->goalAnimState = STATE_GUARD_STOP;
            break;
        case 1:
            item->goalAnimState = STATE_GUARD_RECOIL;
            item->animNumber = anim + 23;
            break;
        case 2:
            item->goalAnimState = STATE_GUARD_CROUCH;
            item->animNumber = anim + 25;
            // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
            break;
        case 3:
            item->goalAnimState = STATE_GUARD_SITTING;
            item->animNumber = anim + 28;
            item->swapMeshFlags = 9216;
            roomItemNumber = g_Level.Rooms[item->roomNumber].itemNumber;
            if (roomItemNumber != NO_ITEM)
            {
                while (true)
                {
                    item2 = &g_Level.Items[roomItemNumber];
                    if (item2->objectNumber >= ID_ANIMATING1 && item2->objectNumber <= ID_ANIMATING15 && item2->roomNumber == item->roomNumber && item2->triggerFlags == 3)
                        break;
                    roomItemNumber = item2->nextItem;
                    if (roomItemNumber == NO_ITEM)
                    {
                        item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
                        item->currentAnimState = item->goalAnimState;
                        break;
                    }
                }
                item2->meshBits = -5;
            }
            break;
        case 4:
            item->goalAnimState = 17;
			item->swapMeshFlags = 8192;
            item->animNumber = anim + 30;
            break;
        case 5:
            FLOOR_INFO *floor;
            short roomNumber;
            item->animNumber = anim + 26;
            item->goalAnimState = STATE_GUARD_ROPE_DOWN;
            roomNumber = item->roomNumber;
            floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
            GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
            item->pos.yPos = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) - SECTOR(2);
            break;
        case 6:
            item->goalAnimState = STATE_GUARD_WAITING_ON_WALL;
            item->animNumber = anim + 32;
            break;
        case 7:
        case 9:
            item->goalAnimState = STATE_GUARD_USE_COMPUTER;
            item->animNumber = anim + 59;
            item->pos.xPos -= 512 * phd_sin(item->pos.yRot);
            item->pos.zPos -= 512 * phd_cos(item->pos.yRot);
            break;
        case 8:
            item->goalAnimState = STATE_GUARD_HUNTING_IDLE;
            item->animNumber = anim + 46;
            break;
        case 11:
            item->goalAnimState = STATE_GUARD_RUN;
            item->animNumber = anim + 12;
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
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = STATE_SNIPER_STOP;
    item->currentAnimState = STATE_SNIPER_STOP;
	item->pos.yPos += STEP_SIZE * 2;
    item->pos.xPos += 1024 * phd_sin(item->pos.yRot + ANGLE(90));
    item->pos.zPos += 1024 * phd_cos(item->pos.yRot + ANGLE(90));
}

void InitialiseGuardLaser(short itemNum)
{
    ITEM_INFO* item;
    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex + 6;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = STATE_GUARD_STOP;
    item->currentAnimState = STATE_GUARD_STOP;
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
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	int x = item->pos.xPos;
	int z = item->pos.zPos;
	int dx = 870 * phd_sin(item->pos.yRot);
	int dz = 870 * phd_cos(item->pos.yRot);
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
		pos.x = SwatGun.x;
		pos.y = SwatGun.y;
		pos.z = SwatGun.z;
		GetJointAbsPosition(item, &pos, SwatGun.meshNum);
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
		laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;
		laraInfo.distance = SQUARE(dx) + SQUARE(dz);
	}
	
	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != STATE_GUARD_DEATH1 && item->currentAnimState != STATE_GUARD_DEATH2)
		{
			if (laraInfo.angle >= 12288 || laraInfo.angle <= -12288)
			{
				item->currentAnimState = STATE_GUARD_DEATH2;
				item->animNumber = animIndex + ANIMATION_GUARD_DEATH2;
				item->pos.yRot += laraInfo.angle + -ANGLE(180);
			}
			else
			{
				item->currentAnimState = STATE_GUARD_DEATH1;
				item->animNumber = animIndex + ANIMATION_GUARD_DEATH1;
				item->pos.yRot += laraInfo.angle;
			}
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
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
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_NO_LENSFLARE) // CHECK
		{
			if (item->objectNumber == ID_SWAT_PLUS)
			{
				item->itemFlags[0]++;
				if (item->itemFlags[0] > 60 && !(GetRandomControl() & 0xF))
				{
					SoundEffect(SFX_TR5_BIO_BREATHE_OUT, &item->pos, 0);
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
					item->currentAnimState = STATE_GUARD_DEATH2;
					item->animNumber = animIndex + ANIMATION_GUARD_DEATH2;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				}
			}
		}
		CreatureMood(item, &info, creature->enemy != LaraItem);
		ITEM_INFO * enemy = creature->enemy;
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
		src.roomNumber = item->roomNumber;
		ANIM_FRAME* frame = GetBestFrame(LaraItem);
		GAME_VECTOR dest;
		dest.x = LaraItem->pos.xPos;
		dest.y = LaraItem->pos.yPos + ((frame->boundingBox.Y2 + 3 * frame->boundingBox.Y1) / 4);
		dest.z = LaraItem->pos.zPos;
		bool los = !LOS(&src, &dest) && item->triggerFlags != 10;
		creature->maximumTurn = 0;
		ITEM_INFO* currentItem;
		short currentItemNumber;
		PHD_VECTOR pos1, pos2;
		PHD_VECTOR pos;
		switch (item->currentAnimState)
		{
		case STATE_GUARD_STOP:
			creature->LOT.isJumping = false;
			joint2 = laraInfo.angle;
			creature->flags = 0;
			if (info.ahead)
			{
				if (!(item->aiBits & FOLLOW))
				{
					joint0 = info.angle / 2;
					joint1 = info.xAngle;
				}
			}
			if (item->objectNumber == ID_SCIENTIST && item == Lara.target)
			{
				item->goalAnimState = STATE_GUARD_SURREND;
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
				item->goalAnimState = STATE_GUARD_TURN180;
			}
			else if (item->aiBits & PATROL1)
			{
				item->goalAnimState = STATE_GUARD_WALK;
			}
			else if (item->aiBits & AMBUSH)
			{
				item->goalAnimState = STATE_GUARD_RUN;
			}
			else if (Targetable(item, &info) && item->objectNumber != ID_SCIENTIST)
			{
				if (info.distance >= 0x1000000 && info.zoneNumber == info.enemyZone)
				{
					if (!(item->aiBits & MODIFY))
						item->goalAnimState = STATE_GUARD_WALK;
				}
				else
					item->goalAnimState = STATE_GUARD_AIM;
			}
			else if (canJump1block || canJump2blocks)
			{
				creature->maximumTurn = 0;
				item->animNumber = animIndex + 41;
				item->currentAnimState = STATE_GUARD_STOP_START_JUMP;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				if (canJump1block)
					item->goalAnimState = STATE_GUARD_JUMPING_1BLOCK;
				else
					item->goalAnimState = STATE_GUARD_JUMPING_2BLOCKS;
				creature->LOT.isJumping = true;
			}
			else if (los)
			{
				item->goalAnimState = STATE_GUARD_HUNTING_IDLE;
			}
			else if (creature->mood)
			{
				if (info.distance < 0x900000 || item->aiBits & FOLLOW)
				{
					item->goalAnimState = STATE_GUARD_WALK;
				}
				else
					item->goalAnimState = STATE_GUARD_RUN;
			}
			else
			{
				item->goalAnimState = STATE_GUARD_STOP;
			}
			if (item->triggerFlags == 11)
				item->triggerFlags = 0;
			break;
		case STATE_GUARD_TURN180:
			creature->flags = 0;
			if (info.angle >= 0)
				item->pos.yRot -= ANGLE(2);
			else
				item->pos.yRot += ANGLE(2);
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
				item->pos.yRot += -ANGLE(180);
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
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (item->currentAnimState == STATE_GUARD_FIRE_FAST)
			{
				if (creature->flags)
				{
					if (item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 10
						&& (item->frameNumber - g_Level.Anims[item->animNumber].frameBase) & 1)
						creature->flags = 0;
				}
			}
			if (!creature->flags)
			{
				creature->flags = 1;
				item->firedWeapon = 2;
				if (item->currentAnimState == STATE_GUARD_FIRE_SINGLE)
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
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (!Targetable(item, &info))
				item->goalAnimState = STATE_GUARD_STOP;
			else if (item->objectNumber == ID_GUARD1 || item->objectNumber == ID_GUARD2)
				item->goalAnimState = STATE_GUARD_FIRE_SINGLE;
			else
				item->goalAnimState = STATE_GUARD_FIRE_FAST;
			break;
		case STATE_GUARD_WALK:
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
					item->currentAnimState = STATE_GUARD_STOP_START_JUMP;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					if (canJump1block)
						item->goalAnimState = STATE_GUARD_JUMPING_1BLOCK;
					else
						item->goalAnimState = STATE_GUARD_JUMPING_2BLOCKS;
					creature->LOT.isJumping = true;
				}
				else if (info.distance >= 0x100000)
				{
					if (!los || item->aiBits)
					{
						if (info.distance > 0x900000)
						{
							if (!(item->inDrawRoom))
								item->goalAnimState = STATE_GUARD_RUN;
						}
					}
					else
					{
						item->goalAnimState = STATE_GUARD_STOP;
					}
				}
				else
				{
					item->goalAnimState = STATE_GUARD_STOP;
				}
			}
			else
			{
				item->goalAnimState = STATE_GUARD_AIM;
			}
			break;
		case STATE_GUARD_RUN:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(10);
			if (Targetable(item, &info) && (info.distance < 0x1000000 || info.enemyZone == info.zoneNumber) && item->objectNumber != ID_SCIENTIST)
			{
				item->goalAnimState = STATE_GUARD_AIM;
			}
			else if (canJump1block || canJump2blocks)
			{
				creature->maximumTurn = 0;
				item->animNumber = animIndex + 50;
				item->currentAnimState = STATE_GUARD_STOP_START_JUMP;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				if (canJump1block)
					item->goalAnimState = STATE_GUARD_JUMPING_1BLOCK;
				else
					item->goalAnimState = STATE_GUARD_JUMPING_2BLOCKS;
				creature->LOT.isJumping = true;
			}
			else if (los)
			{
				item->goalAnimState = STATE_GUARD_STOP;
			}
			else if (info.distance < 0x900000)
			{
				item->goalAnimState = STATE_GUARD_WALK;
			}
			if (item->triggerFlags == 11)
			{
				creature->LOT.isJumping = true;
				creature->maximumTurn = 0;
			}
			break;
		case STATE_GUARD_ROPE_DOWN:
			joint2 = laraInfo.angle;
			if (item->pos.yPos <= item->floor - 2048 || item->triggerFlags != 5)
			{
				if (item->pos.yPos >= item->floor - 512)
					item->goalAnimState = STATE_GUARD_AIM;
			}
			else
			{
				item->triggerFlags = 0;
				TestTriggers(item, true);
				SoundEffect(SFX_TR4_LARA_POLE_LOOP, &item->pos, 0);
			}
			if (abs(info.angle) >= 364)
			{
				if ((info.angle & 0x8000) == 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			break;
		case STATE_GUARD_SITTING:
			joint2 = AIGuard(creature);
			if (creature->alerted)
				item->goalAnimState = STATE_GUARD_STAND_UP;
			break;
		case STATE_GUARD_STAND_UP:
		case 18:
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
			{
				TestTriggers(item, true);
				break;
			}
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 44)
			{
				item->swapMeshFlags = 0;
				short currentItemNumber = g_Level.Rooms[item->roomNumber].itemNumber;
				if (currentItemNumber == NO_ITEM)
					break;
				ITEM_INFO * currentItem;
				while (true)
				{
					currentItem = &g_Level.Items[currentItemNumber];
					if (currentItem->objectNumber >= ID_ANIMATING1
						&& currentItem->objectNumber <= ID_ANIMATING15
						&& currentItem->roomNumber == item->roomNumber)
					{
						if (currentItem->triggerFlags > 2 && currentItem->triggerFlags < 5)
							break;
					}
					currentItemNumber = currentItem->nextItem;
					if (currentItemNumber == -1)
						break;
				}
				if (currentItemNumber == NO_ITEM)
					break;
				currentItem->meshBits = -3;
			}
			else if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
			{
				item->pos.yRot -= ANGLE(90);
			}
			break;
		case 17:
			joint2 = 0;
			if (!item->hitStatus && LaraItem->speed < 40 && !Lara.hasFired)
				creature->alerted = false;
			if (creature->alerted)
				item->goalAnimState = 18;
			break;
		case STATE_GUARD_WAITING_ON_WALL:
			joint2 = AIGuard(creature);
			if (creature->alerted)
				item->goalAnimState = STATE_GUARD_STOP;
			break;
		case STATE_GUARD_HUNTING:
		case STATE_GUARD_HUNTING_IDLE:
			if (item->currentAnimState == STATE_GUARD_HUNTING_IDLE)
			{
				if (item->triggerFlags != 8 || !los || item->hitStatus)
					item->goalAnimState = STATE_GUARD_HUNTING;
			}
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(5);
			if (canJump1block || canJump2blocks || info.distance < 0x100000 || !los || item->hitStatus)
				item->goalAnimState = STATE_GUARD_STOP;
			break;
		case STATE_GUARD_INSERT_CODE:
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 39)
			{
				TestTriggers(item, true);
			}
			break;
		case STATE_GUARD_START_USE_COMPUTER:
			currentItem = NULL;
			for (currentItemNumber = g_Level.Rooms[item->roomNumber].itemNumber; currentItemNumber != NO_ITEM; currentItemNumber = currentItem->nextItem)
			{
				currentItem = &g_Level.Items[currentItemNumber];
				if (item->objectNumber == ID_PUZZLE_HOLE8)
					break;
			}
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
			{
				currentItem->meshBits = 0x1FFF;
				item->pos.xPos = currentItem->pos.xPos - 256;
				item->pos.yRot = currentItem->pos.yRot;
				item->pos.zPos = currentItem->pos.zPos + 128;
				item->swapMeshFlags = 1024;
			}
			else
			{
				if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 32)
				{
					currentItem->meshBits = 16381;
				}
				else if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 74)
				{
					currentItem->meshBits = 278461;
				}
				else if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 120)
				{
					currentItem->meshBits = 802621;
				}
				else if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 157)
				{
					currentItem->meshBits = 819001;
				}
				else if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 190)
				{
					currentItem->meshBits = 17592121;
				}
				else if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + g_Level.Anims[item->animNumber].frameEnd)
				{
					currentItem->meshBits = 0x1FFF;
					TestTriggers(item, true);
					item->requiredAnimState = STATE_GUARD_WALK;
					item->swapMeshFlags = 0;
				}
			}
			break;
		case STATE_GUARD_USE_COMPUTER:
			if ((item->objectNumber != ID_SCIENTIST || item != Lara.target)
				&& (GetRandomControl() & 0x7F || item->triggerFlags >= 10 || item->triggerFlags == 9))
			{
				if (item->aiBits & GUARD)
				{
					joint2 = AIGuard(creature);
					if (item->aiBits & PATROL1)
					{
						item->triggerFlags--;
						if (item->triggerFlags < 1)
						{
							item->aiBits = PATROL1 | MODIFY;
						}
					}
				}
			}
			else
			{
				item->goalAnimState = STATE_GUARD_STOP;
			}
			break;
		case STATE_GUARD_SURREND:
			if (item != Lara.target && !(GetRandomControl() & 0x3F))
			{
				if (item->triggerFlags == 7 || item->triggerFlags == 9)
					item->requiredAnimState = STATE_GUARD_USE_COMPUTER;
				item->goalAnimState = STATE_GUARD_STOP;
			}
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 39)
			{
				TestTriggers(item->pos.xPos, item->pos.yPos, item->pos.zPos, enemy->roomNumber, true);
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
		
		if (enemy->flags != 4)
		{
			if (enemy->flags & 0x10)
			{
				item->goalAnimState = STATE_GUARD_STOP;
				item->requiredAnimState = STATE_GUARD_USE_COMPUTER;
				item->triggerFlags = 300;
				item->aiBits = GUARD | PATROL1;
			}
			else
			{
				if (enemy->flags & 0x20)
				{
					item->goalAnimState = STATE_GUARD_STOP;
					item->requiredAnimState = 36;
					item->aiBits = PATROL1 | MODIFY;
				}
				else
				{
					TestTriggers(creature->enemy->pos.xPos, creature->enemy->pos.yPos, creature->enemy->pos.zPos, enemy->roomNumber, true);

					item->requiredAnimState = STATE_GUARD_WALK;
					if (creature->enemy->flags & 2)
						item->itemFlags[3] = (item->TOSSPAD & 0xFF) - 1;
					if (creature->enemy->flags & 8)
					{
						item->requiredAnimState = STATE_GUARD_STOP;
						item->triggerFlags = 300;
						item->aiBits |= GUARD | PATROL1;
					}
				}
			}
		}
		else
		{
			item->goalAnimState = STATE_GUARD_STOP;
			item->requiredAnimState = 37;
		}
	}
	if ((item->currentAnimState >= 20 
		|| item->currentAnimState == 6 
		|| item->currentAnimState == 8) 
		&& item->currentAnimState != 30)
	{
		CreatureAnimation(itemNum, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNum, angle, 2, 256) + 4)
		{
		case 0:
			creature->maximumTurn = 0;
			item->animNumber = animIndex + 38;
			item->currentAnimState = 23;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 1:
			creature->maximumTurn = 0;
			item->animNumber = animIndex + 39;
			item->currentAnimState = 24;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = animIndex + 40;
			item->currentAnimState = 25;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 6:
			creature->maximumTurn = 0;
			item->animNumber = animIndex + 35;
			item->currentAnimState = 20;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 7:
			creature->maximumTurn = 0; 
			item->animNumber = animIndex + 36;
			item->currentAnimState = 21;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 8:
			creature->maximumTurn = 0;
			item->animNumber = animIndex + 37;
			item->currentAnimState = 22;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
		if (item->firedWeapon)
		{
			PHD_VECTOR pos;
			pos.x = SniperGun.x;
			pos.y = SniperGun.y;
			pos.z = SniperGun.z;
			GetJointAbsPosition(item, &pos, SniperGun.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->firedWeapon + 10, 192, 128, 32);
			item->firedWeapon--;
		}
		if (item->hitPoints > 0)
		{
			if (item->aiBits)
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
			switch (item->currentAnimState)
			{
			case STATE_SNIPER_STOP:
				item->meshBits = 0;
				if (TargetVisible(item, &info))
					item->goalAnimState = STATE_SNIPER_UNHIDE;
				break;
			case STATE_SNIPER_UNHIDE:
				item->meshBits = -1;
				break;
			case 3:
				creature->flags = 0;
				if (!TargetVisible(item, &info)
					|| item->hitStatus
					&& GetRandomControl() & 1)
				{
					item->goalAnimState = STATE_SNIPER_HIDE;
				}
				else if (!(GetRandomControl() & 0x1F))
				{
					item->goalAnimState = STATE_SNIPER_FIRE;
				}
				break;
			case STATE_SNIPER_FIRE:
				if (!creature->flags)
				{
					ShotLara(item, &info, &SniperGun, joint0, 100);
					creature->flags = 1;
					item->firedWeapon = 2;
				}
				break;
			default:
				break;
			}
		}
		else
		{
			item->hitPoints = 0;
			if (item->currentAnimState != STATE_SNIPER_DEATH)
			{
				item->animNumber = Objects[ID_SNIPER].animIndex + 5;
				item->currentAnimState = STATE_SNIPER_DEATH;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = STATE_GUARD_STOP;
	item->currentAnimState = STATE_GUARD_STOP;
	item->swapMeshFlags = 9216;
}

void Mafia2Control(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;
	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	// Can baddy jump? Check for a distance of 1 and 2 sectors
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;
	int dx = 870 * phd_sin(item->pos.yRot);
	int dz = 870 * phd_cos(item->pos.yRot);
	x += dx;
	z += dz;
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);
	x += dx;
	z += dz;
	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);
	x += dx;
	z += dz;
	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, y, z);
	int height = 0;
	bool canJump1sector = true;
	if (item->boxNumber == LaraItem->boxNumber
		|| y >= height1 - 384
		|| y >= height2 + 256
		|| y <= height2 - 256)
	{
		height = height2;
		canJump1sector = false;
	}
	bool canJump2sectors = true;
	if (item->boxNumber == LaraItem->boxNumber
		|| y >= height1 - 384
		|| y >= height - 384
		|| y >= height3 + 256
		|| y <= height3 - 256)
	{
		canJump2sectors = false;
	}
	if (item->firedWeapon)
	{
		PHD_VECTOR pos;
		pos.x = ArmedBaddy2Gun.x;
		pos.y = ArmedBaddy2Gun.y;
		pos.z = ArmedBaddy2Gun.z;
		GetJointAbsPosition(item, &pos, ArmedBaddy2Gun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->firedWeapon + 8, 24, 16, 4);
		item->firedWeapon--;
	}
	AI_INFO info;
	AI_INFO laraInfo;
	ZeroMemory(&info, sizeof(AI_INFO));
	if (item->hitPoints > 0)
	{
		if (item->aiBits)
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
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}
		GetCreatureMood(item, &info, creature->enemy != LaraItem);
		CreatureMood(item, &info, creature->enemy != LaraItem);
		angle = CreatureTurn(item, creature->maximumTurn);
		creature->enemy = LaraItem;
		if (laraInfo.distance < SQUARE(2048) && LaraItem->speed > 20 || item->hitStatus || TargetVisible(item, &laraInfo))
		{
			if (!(item->aiBits & FOLLOW))
			{
				creature->enemy = LaraItem;
				AlertAllGuards(itemNum);
			}
		}
		switch (item->currentAnimState)
		{
		case STATE_MAFIA2_STOP:
			creature->LOT.isJumping = false;
			joint2 = laraInfo.angle;
			creature->flags = 0;
			creature->maximumTurn = 0;
			if (info.ahead && !(item->aiBits & GUARD))
			{
				joint0 = info.angle / 2;
				joint1 = info.xAngle;
			}
			if (item->aiBits & GUARD)
			{
				joint2 = AIGuard(creature);
				break;
			}
			if (laraInfo.angle <= 20480 && laraInfo.angle >= -20480)
			{
				if (item->swapMeshFlags == 9216)
				{
					item->goalAnimState = STATE_MAFIA2_UNDRAW_GUNS;
					break;
				}
			}
			else if (item->swapMeshFlags == 9216)
			{
				item->goalAnimState = STATE_MAFIA2_TURN180;
				break;
			}
			if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone)
				{
					item->goalAnimState = STATE_MAFIA2_AIM;
				}
				else if (!(item->aiBits & MODIFY))
				{
					item->goalAnimState = STATE_MAFIA2_WALK;
				}
			}
			else
			{
				if (item->aiBits & PATROL1)
				{
					item->goalAnimState = STATE_MAFIA2_WALK;
				}
				else
				{
					if (canJump1sector || canJump2sectors)
					{
						creature->maximumTurn = 0;
						item->animNumber = Objects[item->objectNumber].animIndex + 41;
						item->currentAnimState = STATE_MAFIA2_STOP_START_JUMP;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						if (canJump2sectors)
							item->goalAnimState = STATE_MAFIA2_JUMPING_2BLOCKS;
						else
							item->goalAnimState = STATE_MAFIA2_JUMPING_1BLOCK;
						creature->LOT.isJumping = true;
						break;
					}
					if (creature->mood)
					{
						if (info.distance >= SQUARE(3072))
							item->goalAnimState = STATE_MAFIA2_WALK;
					}
					else
					{
						item->goalAnimState = STATE_MAFIA2_STOP;
					}
				}
			}
			break;
		case STATE_MAFIA2_TURN180_UNDRAW_GUNS:
		case STATE_MAFIA2_TURN180:
			creature->maximumTurn = 0;
			if (info.angle >= 0)
				item->pos.yRot -= ANGLE(2);
			else
				item->pos.yRot += ANGLE(2);
			if (item->frameNumber != g_Level.Anims[item->animNumber].frameBase + 16 
				|| item->swapMeshFlags != 9216)
			{
				if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
					item->pos.yRot += -ANGLE(180);
			}
			else
			{
				item->swapMeshFlags = 128;
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
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (!creature->flags)
			{
				ShotLara(item, &info, &ArmedBaddy2Gun, laraInfo.angle / 2, 35);
				creature->flags = 1;
				item->firedWeapon = 2;
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
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (Targetable(item, &info))
			{
				item->goalAnimState = STATE_MAFIA2_FIRE;
			}
			else if (laraInfo.angle > 20480 || laraInfo.angle < -20480)
			{
				item->goalAnimState = 32;
			}
			else
			{
				item->goalAnimState = STATE_MAFIA2_STOP;
			}
			break;
		case STATE_MAFIA2_WALK:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(5);
			if (Targetable(item, &info) && (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone))
			{
				item->goalAnimState = STATE_MAFIA2_AIM;
			}
			else
			{
				if (canJump1sector || canJump2sectors)
				{
					creature->maximumTurn = 0;
					creature->maximumTurn = 0;
					item->animNumber = Objects[item->objectNumber].animIndex + 41;
					item->currentAnimState = STATE_MAFIA2_STOP_START_JUMP;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					if (canJump2sectors)
						item->goalAnimState = STATE_MAFIA2_JUMPING_2BLOCKS;
					else
						item->goalAnimState = STATE_MAFIA2_JUMPING_1BLOCK;
					creature->LOT.isJumping = true;
					break;
				}
				if (info.distance >= SQUARE(1024))
				{
					if (info.distance > SQUARE(3072))
						item->goalAnimState = STATE_MAFIA2_RUN;
				}
				else
				{
					item->goalAnimState = STATE_MAFIA2_STOP;
				}
			}
			break;
		case STATE_MAFIA2_RUN:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(10);
			if (Targetable(item, &info) && (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone))
			{
				item->goalAnimState = STATE_MAFIA2_AIM;
			}
			else if (canJump1sector || canJump2sectors)
			{
				creature->maximumTurn = 0;
				item->animNumber = Objects[item->objectNumber].animIndex + 50;
				item->currentAnimState = STATE_MAFIA2_STOP_START_JUMP;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				if (canJump2sectors)
					item->goalAnimState = STATE_MAFIA2_JUMPING_2BLOCKS;
				else
					item->goalAnimState = STATE_MAFIA2_JUMPING_1BLOCK;
				creature->LOT.isJumping = true;
			}
			else if (info.distance < SQUARE(3072))
			{
				item->goalAnimState = STATE_MAFIA2_WALK;
			}
			break;
		case STATE_MAFIA2_UNDRAW_GUNS:
			creature->maximumTurn = 0;
			if (info.angle >= 0)
				item->pos.yRot += ANGLE(2);
			else
				item->pos.yRot -= ANGLE(2);
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 16 
				&& item->swapMeshFlags == 9216)
				item->swapMeshFlags = 128;
			break;
		default:
			break;
		}
	}
	else
	{
		if (item->currentAnimState != STATE_MAFIA2_DEATH2 && item->currentAnimState != STATE_MAFIA2_DEATH1)
		{
			if (info.angle >= 12288 || info.angle <= -12288)
			{
				item->currentAnimState = STATE_MAFIA2_DEATH2;
				item->animNumber = Objects[item->objectNumber].animIndex + 16;
				item->pos.yRot += info.angle - ANGLE(180);
			}
			else
			{
				item->currentAnimState = STATE_MAFIA2_DEATH1;
				item->animNumber = Objects[item->objectNumber].animIndex + 11;
				item->pos.yRot += info.angle;
			}
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	if (item->currentAnimState >= 20 || item->currentAnimState == 6 || item->currentAnimState == 8)
	{
		CreatureAnimation(itemNum, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNum, angle, 2, 256) + 4)
		{
		case 0:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 38;
			item->currentAnimState = 23;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 1:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 39;
			item->currentAnimState = 24;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 40;
			item->currentAnimState = 25;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 6:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 35;
			item->currentAnimState = 20;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 7:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 36;
			item->currentAnimState = 21;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		case 8:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 37;
			item->currentAnimState = 22;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;
		default:
			return;
		}
	}
}