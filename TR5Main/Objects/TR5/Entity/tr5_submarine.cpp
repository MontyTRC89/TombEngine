#include "framework.h"
#include "tr5_submarine.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/people.h"
#include "Game/collision/collide_room.h"
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
	auto* spark = &Sparks[GetFreeSpark()];

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
	auto* spark = &Sparks[GetFreeSpark()];

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
	auto* spark = &Sparks[GetFreeSpark()];

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
		auto* torpedoItem = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR5_UNDERWATERTORPEDO, &torpedoItem->Position, 2);

		torpedoItem->ObjectNumber = ID_TORPEDO;
		torpedoItem->Shade = -15856;

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

		torpedoItem->RoomNumber = item->RoomNumber;
		GetFloor(pos1.x, pos1.y, pos1.z, &torpedoItem->RoomNumber);

		torpedoItem->Position.xPos = pos1.x;
		torpedoItem->Position.yPos = pos1.y;
		torpedoItem->Position.zPos = pos1.z;

		InitialiseItem(itemNumber);

		torpedoItem->Position.xRot = 0;
		torpedoItem->Position.yRot = item->Position.yRot;
		torpedoItem->Position.zRot = 0;
		torpedoItem->Velocity = 0;
		torpedoItem->VerticalVelocity = 0;
		torpedoItem->ItemFlags[0] = -1;

		AddActiveItem(itemNumber);
	}
}

void InitialiseSubmarine(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = 0;
	item->ActiveState = 0;

	if (!item->TriggerFlags)
		item->TriggerFlags = 120;
}

void SubmarineControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = (CreatureInfo*)item->Data;

	if (item->AIBits)
		GetAITarget(creature);
	else
		creature->Enemy = LaraItem;

	AI_INFO AI, laraInfo;
	CreatureAIInfo(item, &AI);

	GetCreatureMood(item, &AI, VIOLENT);
	CreatureMood(item, &AI, VIOLENT);

	short angle = CreatureTurn(item, creature->MaxTurn);

	if (creature->Enemy == LaraItem)
	{
		laraInfo.angle = AI.angle;
		laraInfo.distance = AI.distance;
	}
	else
	{
		int dx = LaraItem->Position.xPos - item->Position.xPos;
		int dz = LaraItem->Position.zPos - item->Position.zPos;

		laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
		laraInfo.distance = pow(dx, 2) + pow(dz, 2);
		laraInfo.ahead = true;
	}

	int tilt = item->ItemFlags[0] + (angle / 2);
	
	if (tilt > 2048)
		tilt = 2048;
	else if (tilt < -2048)
		tilt = -2048;

	item->ItemFlags[0] = tilt;

	if (abs(tilt) >= 64)
	{
		if (tilt > 0)
			item->ItemFlags[0] -= 64;
		else
			item->ItemFlags[0] += 64;
	}
	else
		item->ItemFlags[0] = 0;

	creature->MaxTurn = ANGLE(2.0f);

	short joint = AI.xAngle - ANGLE(45.0f);

	if (creature->Flags < item->TriggerFlags)
		creature->Flags++;

	auto* enemy = creature->Enemy;
	creature->Enemy = LaraItem;

	if (Targetable(item, &laraInfo))
	{
		if (creature->Flags >= item->TriggerFlags &&
			laraInfo.angle > -ANGLE(90.0f) &&
			laraInfo.angle < ANGLE(90.0f))
		{
			SubmarineAttack(item);
			creature->Flags = 0;
		}

		if (laraInfo.distance >= pow(SECTOR(3), 2))
		{
			item->TargetState = 1;
			SoundEffect(SFX_TR5_DIVESUITLOOP, &item->Position, 2);
		}
		else
			item->TargetState = 0;

		if (AI.distance < pow(SECTOR(1), 2))
		{
			creature->MaxTurn = 0;
			if (abs(laraInfo.angle) >= ANGLE(2.0f))
			{
				if (laraInfo.angle >= 0)
					item->Position.yRot += ANGLE(2.0f);
				else
					item->Position.yRot -= ANGLE(2.0f);
			}
			else
				item->Position.yRot += laraInfo.angle;
		}
	}
	else
		item->TargetState = 1;

	creature->Enemy = enemy;

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint);
	CreatureJoint(item, 1, joint);

	if (GlobalCounter & 1)
	{
		PHD_VECTOR pos1 = { 200, 320, 90 };
		GetJointAbsPosition(item, &pos1, 1);

		PHD_VECTOR pos2 = { 200, 1280, 90 };
		GetJointAbsPosition(item, &pos2, 1);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);

		pos1 = { 200, 320, -100 };
		GetJointAbsPosition(item, &pos1, 1);

		pos2 = { 200, 1280, -100 };
		GetJointAbsPosition(item, &pos2, 1);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);
	}
	else
	{
		PHD_VECTOR pos1 = { -200, 320, 90 };
		GetJointAbsPosition(item, &pos1, 2);
		
		PHD_VECTOR pos2 = { -200, 1280, 90 };
		GetJointAbsPosition(item, &pos2, 2);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);
		
		pos1 = { -200, 320, -100 };
		GetJointAbsPosition(item, &pos1, 2);
		
		pos2 = { -200, 1280, -100 };
		GetJointAbsPosition(item, &pos2, 2);

		TriggerTorpedoBubbles(&pos1, &pos2, 0);
	}

	TriggerSubmarineSparks(itemNumber);

	GAME_VECTOR pos1;
	pos1.x = 0;
	pos1.y = -600;
	pos1.z = -40;
	pos1.roomNumber = item->RoomNumber;
	GetJointAbsPosition(item, (PHD_VECTOR*)&pos1, 0);

	GAME_VECTOR pos2;
	pos2.x = 0;
	pos2.y = -15784;
	pos2.z = -40;
	GetJointAbsPosition(item, (PHD_VECTOR*)&pos2, 0);

	if (!LOS((GAME_VECTOR*)&pos1, &pos2))
	{
		int distance = sqrt(pow(pos2.x - pos1.x, 2) + pow(pos2.y - pos1.y, 2) + pow(pos2.z - pos1.z, 2));
		if (distance < SECTOR(16))
		{
			distance = SECTOR(16) - distance;
			byte color = (GetRandomControl() & 0xF) + (distance / 128) + 64;
			TriggerDynamicLight(pos2.x, pos2.y, pos2.z, (GetRandomControl() & 1) + (distance / 2048) + 12, color / 2, color, color / 2);
		}
	}

	if (creature->ReachedGoal)
	{
		if (creature->Enemy)
		{
			if (creature->Flags & 2)
				item->ItemFlags[3] = LOBYTE(creature->Tosspad) - 1;

			item->ItemFlags[3]++;
			creature->ReachedGoal = false;
			creature->Enemy = NULL;
		}
	}

	CreatureAnimation(itemNumber, angle, tilt);
	CreatureUnderwater(item, -14080);
}

void ChaffFlareControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	
	if (item->VerticalVelocity)
	{
		item->Position.xRot += ANGLE(3.0f);
		item->Position.zRot += ANGLE(5.0f);
	}

	int dx = item->Velocity * phd_sin(item->Position.yRot);
	int dz = item->Velocity * phd_cos(item->Position.yRot);

	item->Position.xPos += dx;
	item->Position.zPos += dz;
	
	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
	{
		item->Velocity += (5 - item->Velocity) / 2;
		item->VerticalVelocity += (5 - item->VerticalVelocity) / 2;
	}
	else
		item->VerticalVelocity += GRAVITY;

	item->Position.yPos += item->VerticalVelocity;

	DoProjectileDynamics(itemNumber, item->Position.xPos, item->Position.yPos, item->Position.zPos, dx, item->VerticalVelocity, dz);

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

	if (item->ItemFlags[0] >= 300)
	{
		if (!item->VerticalVelocity && !item->Velocity)
		{
			if (item->ItemFlags[1] <= 90)
				item->ItemFlags[1]++;
			else
				KillItem(itemNumber);
		}
	}
	else
		item->ItemFlags[0]++;
}

void TorpedoControl(short itemNumber)
{
	auto*  item = &g_Level.Items[itemNumber];

	SoundEffect(SFX_TR5_DIVESUITHIT, &item->Position, 2);

	PHD_VECTOR pos;

	if (item->ItemFlags[0] == NO_ITEM)
	{
		bool found = false;
		for (int i = g_Level.NumItems; i < 256; i++)
		{
			auto* searchItem = &g_Level.Items[i];

			if (searchItem->ObjectNumber == ID_CHAFF && searchItem->Active)
			{
				item->ItemFlags[0] = i;
				pos.x = searchItem->Position.xPos;
				pos.y = searchItem->Position.yPos;
				pos.z = searchItem->Position.zPos;
				found = true;
				break;
			}
		}

		if (!found)
		{
			pos.x = LaraItem->Position.xPos;
			pos.y = LaraItem->Position.yPos;
			pos.z = LaraItem->Position.zPos;
		}
	}
	else
	{
		auto* chaffItem = &g_Level.Items[item->ItemFlags[0]];

		if (chaffItem->Active && chaffItem->ObjectNumber == ID_CHAFF)
		{
			pos.x = chaffItem->Position.xPos;
			pos.y = chaffItem->Position.yPos;
			pos.z = chaffItem->Position.zPos;
			item->ActiveState = pos.x / 4;
			item->TargetState = pos.y / 4;
			item->RequiredState = pos.z / 4;
		}
		else
		{
			pos.x = 4 * item->ActiveState;
			pos.y = 4 * item->TargetState;
			pos.z = 4 * item->RequiredState;
		}
	}

	short angles[2];
	phd_GetVectorAngles(pos.x - item->Position.xPos, pos.y - item->Position.yPos, pos.z - item->Position.zPos, angles);

	if (item->Velocity >= 48)
	{
		if (item->Velocity < 192)
			item->Velocity++;
	}
	else
		item->Velocity += 4;

	item->ItemFlags[1]++;

	if (item->ItemFlags[1] - 1 < 60)
	{
		short dry = angles[0] - item->Position.yRot;
		if (abs(dry) > 0x8000)
			dry = -dry;

		short drx = angles[1] - item->Position.xRot;
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
			drx = 512;

		if (dry <= 512)
		{
			if (dry < -512)
				dry = -512;
		}
		else
			dry = 512;

		item->Position.yRot += dry;
		item->Position.xRot += drx;
	}

	int x = item->Position.xPos;
	int y = item->Position.yPos;
	int z = item->Position.zPos;

	item->Position.zRot += 16 * item->Velocity;

	int c = item->Velocity * phd_cos(item->Position.xRot);

	item->Position.xPos += c * phd_sin(item->Position.yRot);
	item->Position.yPos += item->Velocity * phd_sin(-item->Position.xRot);
	item->Position.zPos += c * phd_cos(item->Position.yRot);

	auto probe = GetCollisionResult(item);

	if (item->Position.yPos < probe.Position.Floor &&
		item->Position.yPos > probe.Position.Ceiling &&
		TestEnvironment(ENV_FLAG_WATER, probe.RoomNumber))
	{
		if (ItemNearLara(&item->Position, 200))
		{
			LaraItem->HitStatus = true;
			KillItem(itemNumber);
			TriggerUnderwaterExplosion(item, 1);
			SoundEffect(SFX_TR5_UNDERWATEREXPLOSION, &item->Position, 2);
			SoundEffect(SFX_TR5_DIVESUITHIT, &LaraItem->Position, 2); 
			LaraItem->HitPoints -= 200;
		//	if (Lara.anxiety >= 0x7F)
		//		Lara.anxiety--;
		//	else
		//		Lara.anxiety -= 128;
		}
		else
		{
		//	if (ItemNearLara(&item->pos, 400) && Lara.anxiety < 0xE0)
		//		Lara.anxiety += 32;

			if (probe.RoomNumber != item->RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			PHD_VECTOR pos1 = { 0, 0, -64 };
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
		item->Position.xPos = x;
		item->Position.yPos = y;
		item->Position.zPos = z;
		TriggerUnderwaterExplosion(item, 1);
		SoundEffect(SFX_TR5_UNDERWATEREXPLOSION, &item->Position, 2);
		KillItem(itemNumber);
	}
}
