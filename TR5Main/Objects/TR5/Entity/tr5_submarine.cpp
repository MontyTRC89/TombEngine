#include "framework.h"
#include "tr5_submarine.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/people.h"
#include "Game/effects/effects.h"
#include "Game/animation.h"
#include "Game/Lara/lara_one_gun.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/collide_item.h"
#include "Game/control/los.h"

static void TriggerSubmarineSparks(short itemNumber)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->sR = -1;
	spark->sG = -1;
	spark->sB = -1;
	spark->colFadeSpeed = 2;
	spark->dG = (GetRandomControl() & 0x1F) - 32;
	spark->life = 2;
	spark->dR = spark->dG / 2;
	spark->dB = spark->dG / 2;
	spark->sLife = 2;
	spark->transType = TransTypeEnum::COLADD;
	spark->fadeToBlack = 0;
	spark->flags = 20650;
	spark->fxObj = itemNumber;
	spark->nodeNumber = 7;
	spark->x = 0;
	spark->z = 0;
	spark->y = 0;
	spark->xVel = 0;
	spark->yVel = 0;
	spark->zVel = 0;
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->scalar = 1;
	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 11;
	spark->dSize = spark->sSize = spark->size = (GetRandomControl() & 7) + 192;
}

static void TriggerTorpedoBubbles(PHD_VECTOR* pos1, PHD_VECTOR* pos2, char factor)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->sR = 32;
	spark->sG = 32;
	spark->sB = 32;
	spark->dR = 80;
	spark->dG = 80;
	spark->dB = 80;
	spark->colFadeSpeed = 2;
	spark->fadeToBlack = 8;
	spark->transType = TransTypeEnum::COLADD;
	spark->life = spark->sLife = (GetRandomControl() & 7) + 16;
	spark->x = pos1->x + (GetRandomControl() & 0x1F);
	spark->y = (GetRandomControl() & 0x1F) + pos1->y - 16;
	spark->z = (GetRandomControl() & 0x1F) + pos1->z - 16;
	spark->xVel = pos2->x + (GetRandomControl() & 0x7F) - pos1->x - 64;
	spark->yVel = pos2->y + (GetRandomControl() & 0x7F) - pos1->y - 64;
	spark->zVel = pos2->z + (GetRandomControl() & 0x7F) - pos1->z - 64;
	spark->friction = 0;
	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 17;
	spark->maxYvel = 0;
	spark->gravity = -4 - (GetRandomControl() & 3);
	spark->scalar = 1;
	spark->flags = SP_ROTATE | SP_DEF | SP_SCALE;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->sSize = spark->size = (GetRandomControl() & 0xF) + 32 >> factor;
	spark->dSize = spark->size * 2;
}

static void TriggerTorpedoSparks2(PHD_VECTOR* pos1, PHD_VECTOR* pos2, char scale)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->sR = 32;
	spark->sG = 32;
	spark->sB = 32;
	spark->dR = -128;
	spark->dG = -128;
	spark->dB = -128;
	spark->colFadeSpeed = 2;
	spark->fadeToBlack = 8;
	spark->transType = TransTypeEnum::COLADD;
	spark->life = spark->sLife = (GetRandomControl() & 7) + 16;
	spark->x = pos1->x + (GetRandomControl() & 0x1F);
	spark->y = (GetRandomControl() & 0x1F) + pos1->y - 16;
	spark->z = (GetRandomControl() & 0x1F) + pos1->z - 16;
	spark->xVel = pos2->x + (GetRandomControl() & 0x7F) - pos1->x - 64;
	spark->yVel = pos2->y + (GetRandomControl() & 0x7F) - pos1->y - 64;
	spark->zVel = pos2->z + (GetRandomControl() & 0x7F) - pos1->z - 64;
	spark->friction = 51;
	spark->gravity = -4 - (GetRandomControl() & 3);
	spark->maxYvel = 0;
	spark->scalar = 2 - scale;
	spark->flags = SP_EXPDEF | SP_ROTATE | SP_SCALE;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->sSize = spark->size = (GetRandomControl() & 0xF) + 32;
	spark->dSize = spark->size * 2;
}

static void SubmarineAttack(ITEM_INFO* item)
{
	short itemNumber = CreateItem();
	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* torpedoItem = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR5_UNDERWATER_TORPEDO, &torpedoItem->pos, 2);

		torpedoItem->objectNumber = ID_TORPEDO;
		torpedoItem->shade = -15856;

		PHD_VECTOR pos1;
		PHD_VECTOR pos2;

		for (int i = 0; i < 8; i++)
		{
			pos1.x = (GetRandomControl() & 0x7F) - 414;
			pos1.y = -320;
			pos1.z = 352;
			GetJointAbsPosition(item, &pos1, 4);

			pos2.x = (GetRandomControl() & 0x3FF) - 862;
			pos2.y = -320 - (GetRandomControl() & 0x3FF);
			pos2.z = (GetRandomControl() & 0x3FF) - 160;
			GetJointAbsPosition(item, &pos2, 4);

			TriggerTorpedoSparks2(&pos1, &pos2, 0);
		}

		torpedoItem->roomNumber = item->roomNumber;
		GetFloor(pos1.x, pos1.y, pos1.z, &torpedoItem->roomNumber);

		torpedoItem->pos.xPos = pos1.x;
		torpedoItem->pos.yPos = pos1.y;
		torpedoItem->pos.zPos = pos1.z;

		InitialiseItem(itemNumber);

		torpedoItem->pos.xRot = 0;
		torpedoItem->pos.yRot = item->pos.yRot;
		torpedoItem->pos.zRot = 0;
		torpedoItem->Velocity = 0;
		torpedoItem->VerticalVelocity = 0;
		torpedoItem->itemFlags[0] = -1;

		AddActiveItem(itemNumber);
	}
}

void InitialiseSubmarine(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->targetState = 0;
    item->activeState = 0;
    if (!item->triggerFlags)
        item->triggerFlags = 120;
}

void SubmarineControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->aiBits)
		GetAITarget(creature);
	else
		creature->enemy = LaraItem;

	AI_INFO info, laraInfo;
	CreatureAIInfo(item, &info);

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	short angle = CreatureTurn(item, creature->maximumTurn);

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
		laraInfo.ahead = true;
	}

	int tilt = item->itemFlags[0] + (angle / 2);
	
	if (tilt > 2048)
	{
		tilt = 2048;
	}
	else if (tilt < -2048)
	{
		tilt = -2048;
	}

	item->itemFlags[0] = tilt;

	if (abs(tilt) >= 64)
	{
		if (tilt > 0)
			item->itemFlags[0] -= 64;
		else
			item->itemFlags[0] += 64;
	}
	else
	{
		item->itemFlags[0] = 0;
	}

	creature->maximumTurn = ANGLE(2);

	short joint = info.xAngle - ANGLE(45);

	if (creature->flags < item->triggerFlags)
		creature->flags++;

	ITEM_INFO* enemy = creature->enemy;
	creature->enemy = LaraItem;

	if (Targetable(item, &laraInfo))
	{
		if (creature->flags >= item->triggerFlags 
			&& laraInfo.angle > -ANGLE(90) 
			&& laraInfo.angle < ANGLE(90))
		{
			SubmarineAttack(item);
			creature->flags = 0;
		}

		if (laraInfo.distance >= SQUARE(3072))
		{
			item->targetState = 1;
			SoundEffect(SFX_TR5_MINI_SUB_LOOP, &item->pos, 2);
		}
		else
		{
			item->targetState = 0;
		}

		if (info.distance < SQUARE(1024))
		{
			creature->maximumTurn = 0;
			if (abs(laraInfo.angle) >= ANGLE(2))
			{
				if (laraInfo.angle >= 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += laraInfo.angle;
			}
		}
	}
	else
	{
		item->targetState = 1;
	}

	creature->enemy = enemy;

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint);
	CreatureJoint(item, 1, joint);

	if (GlobalCounter & 1)
	{
		PHD_VECTOR pos1;
		pos1.x = 200;
		pos1.y = 320;
		pos1.z = 90;
		GetJointAbsPosition(item, &pos1, 1);

		PHD_VECTOR pos2;
		pos2.x = 200;
		pos2.y = 1280;
		pos2.z = 90;
		GetJointAbsPosition(item, &pos2, 1);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);

		pos1.x = 200;
		pos1.y = 320;
		pos1.z = -100;
		GetJointAbsPosition(item, &pos1, 1);

		pos2.x = 200;
		pos2.y = 1280;
		pos2.z = -100;
		GetJointAbsPosition(item, &pos2, 1);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);
	}
	else
	{
		PHD_VECTOR pos1;
		pos1.x = -200;
		pos1.y = 320;
		pos1.z = 90;
		GetJointAbsPosition(item, &pos1, 2);
		
		PHD_VECTOR pos2;
		pos2.x = -200;
		pos2.y = 1280;
		pos2.z = 90;
		GetJointAbsPosition(item, &pos2, 2);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);
		
		pos1.x = -200;
		pos1.y = 320;
		pos1.z = -100;
		GetJointAbsPosition(item, &pos1, 2);
		
		pos2.x = -200;
		pos2.y = 1280;
		pos2.z = -100;
		GetJointAbsPosition(item, &pos2, 2);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);
	}

	TriggerSubmarineSparks(itemNumber);

	GAME_VECTOR pos1;
	pos1.x = 0;
	pos1.y = -600;
	pos1.z = -40;
	pos1.roomNumber = item->roomNumber;
	GetJointAbsPosition(item, (PHD_VECTOR*)&pos1, 0);

	GAME_VECTOR pos2;
	pos2.x = 0;
	pos2.y = -15784;
	pos2.z = -40;
	GetJointAbsPosition(item, (PHD_VECTOR*)&pos2, 0);

	if (!LOS((GAME_VECTOR*)&pos1, &pos2))
	{
		int distance = sqrt(SQUARE(pos2.x - pos1.x) + SQUARE(pos2.y - pos1.y) + SQUARE(pos2.z - pos1.z));
		if (distance < 16384)
		{
			distance = 16384 - distance;
			byte color = (GetRandomControl() & 0xF) + (distance / 128) + 64;
			TriggerDynamicLight(pos2.x, pos2.y, pos2.z, (GetRandomControl() & 1) + (distance / 2048) + 12, color / 2, color, color / 2);
		}
	}

	if (creature->reachedGoal)
	{
		if (creature->enemy)
		{
			if (creature->flags & 2)
				item->itemFlags[3] = LOBYTE(item->TOSSPAD) - 1;
			item->itemFlags[3]++;
			creature->reachedGoal = false;
			creature->enemy = NULL;
		}
	}

	CreatureAnimation(itemNumber, angle, tilt);
	CreatureUnderwater(item, -14080);
}

void ChaffFlareControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	if (item->VerticalVelocity)
	{
		item->pos.xRot += ANGLE(3);
		item->pos.zRot += ANGLE(5);
	}

	int dx = item->Velocity * phd_sin(item->pos.yRot);
	int dz = item->Velocity * phd_cos(item->pos.yRot);

	item->pos.xPos += dx;
	item->pos.zPos += dz;
	
	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		item->VerticalVelocity += (5 - item->VerticalVelocity) / 2;
		item->Velocity += (5 - item->Velocity) / 2;
	}
	else
	{
		item->VerticalVelocity += GRAVITY;
	}

	item->pos.yPos += item->VerticalVelocity;

	DoProjectileDynamics(itemNumber, item->pos.xPos, item->pos.yPos, item->pos.zPos, dx, item->VerticalVelocity, dz);

	PHD_VECTOR pos1;
	pos1.x = 0;
	pos1.y = 0;
	pos1.z = (GlobalCounter & 1) != 0 ? 48 : -48;
	GetJointAbsPosition(item, &pos1, 0);

	PHD_VECTOR pos2;
	pos2.x = 0;
	pos2.y = 0;
	pos2.z = 8 * ((GlobalCounter & 1) != 0 ? 48 : -48);
	GetJointAbsPosition(item, &pos2, 0);

	TriggerTorpedoBubbles(&pos1, &pos2, 1);

	if (item->itemFlags[0] >= 300)
	{
		if (!item->VerticalVelocity && !item->Velocity)
		{
			if (item->itemFlags[1] <= 90)
			{
				item->itemFlags[1]++;
			}
			else
			{
				KillItem(itemNumber);
			}
		}
	}
	else
	{
		item->itemFlags[0]++;
	}
}

void TorpedoControl(short itemNumber)
{
	ITEM_INFO*  item = &g_Level.Items[itemNumber];

	SoundEffect(SFX_TR5_SWIMSUIT_METAL_CLASH, &item->pos, 2);

	PHD_VECTOR pos;

	if (item->itemFlags[0] == NO_ITEM)
	{
		bool found = false;
		for (int i = g_Level.NumItems; i < 256; i++)
		{
			ITEM_INFO* searchItem = &g_Level.Items[i];

			if (searchItem->objectNumber == ID_CHAFF && searchItem->active)
			{
				item->itemFlags[0] = i;
				pos.x = searchItem->pos.xPos;
				pos.y = searchItem->pos.yPos;
				pos.z = searchItem->pos.zPos;
				found = true;
				break;
			}
		}

		if (!found)
		{
			pos.x = LaraItem->pos.xPos;
			pos.y = LaraItem->pos.yPos;
			pos.z = LaraItem->pos.zPos;
		}
	}
	else
	{
		ITEM_INFO* chaffItem = &g_Level.Items[item->itemFlags[0]];

		if (chaffItem->active && chaffItem->objectNumber == ID_CHAFF)
		{
			pos.x = chaffItem->pos.xPos;
			pos.y = chaffItem->pos.yPos;
			pos.z = chaffItem->pos.zPos;
			item->activeState = pos.x / 4;
			item->targetState = pos.y / 4;
			item->requiredState = pos.z / 4;
		}
		else
		{
			pos.x = 4 * item->activeState;
			pos.y = 4 * item->targetState;
			pos.z = 4 * item->requiredState;
		}
	}

	short angles[2];
	phd_GetVectorAngles(pos.x - item->pos.xPos, pos.y - item->pos.yPos, pos.z - item->pos.zPos, angles);

	if (item->Velocity >= 48)
	{
		if (item->Velocity < 192)
			item->Velocity++;
	}
	else
	{
		item->Velocity += 4;
	}

	item->itemFlags[1]++;

	if (item->itemFlags[1] - 1 < 60)
	{
		short dry = angles[0] - item->pos.yRot;
		if (abs(dry) > 0x8000)
			dry = -dry;

		short drx = angles[1] - item->pos.xRot;
		if (abs(drx) > 0x8000)
			drx = -drx;

		drx /= 8;
		dry /= 8;

		if (drx <= 512)
		{
			if (drx < -512)
				drx = -512;
		}
		else
		{
			drx = 512;
		}

		if (dry <= 512)
		{
			if (dry < -512)
				dry = -512;
		}
		else
		{
			dry = 512;
		}

		item->pos.yRot += dry;
		item->pos.xRot += drx;
	}

	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	item->pos.zRot += 16 * item->Velocity;

	int c = item->Velocity * phd_cos(item->pos.xRot);

	item->pos.xPos += c * phd_sin(item->pos.yRot);
	item->pos.yPos += item->Velocity * phd_sin(-item->pos.xRot);
	item->pos.zPos += c * phd_cos(item->pos.yRot);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int floorHeight = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int ceilingHeight = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (item->pos.yPos < floorHeight && item->pos.yPos > ceilingHeight && g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
	{
		if (ItemNearLara(&item->pos, 200))
		{
			LaraItem->hitStatus = true;
			KillItem(itemNumber);
			TriggerUnderwaterExplosion(item, 1);
			SoundEffect(SFX_TR5_UNDERWATER_EXPLOSION, &item->pos, 2);
			SoundEffect(SFX_TR5_LARA_UNDERWATER_HIT, &LaraItem->pos, 2);
			LaraItem->hitPoints -= 200;
		//	if (Lara.anxiety >= 0x7F)
		//		Lara.anxiety--;
		//	else
		//		Lara.anxiety -= 128;
		}
		else
		{
		//	if (ItemNearLara(&item->pos, 400) && Lara.anxiety < 0xE0)
		//		Lara.anxiety += 32;

			if (roomNumber!= item->roomNumber)
				ItemNewRoom(itemNumber, roomNumber);

			PHD_VECTOR pos1;
			pos1.x = 0;
			pos1.y = 0;
			pos1.z = -64;
			GetJointAbsPosition(item, &pos1, 0);

			PHD_VECTOR pos2;
			pos2.x = 0;
			pos2.y = 0;
			pos2.z = -64 << ((GlobalCounter & 1) + 2);
			GetJointAbsPosition(item, &pos2, 0);

			TriggerTorpedoBubbles(&pos1, &pos2, 1);
		}
	}
	else
	{
		item->pos.xPos = x;
		item->pos.yPos = y;
		item->pos.zPos = z;
		TriggerUnderwaterExplosion(item, 1);
		SoundEffect(SFX_TR5_UNDERWATER_EXPLOSION, &item->pos, 2);
		KillItem(itemNumber);
	}
}