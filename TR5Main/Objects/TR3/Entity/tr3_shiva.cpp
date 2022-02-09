#include "framework.h"
#include "Objects/TR3/Entity/tr3_shiva.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO shivaLeftBite = { 0, 0, 920, 13 };
BITE_INFO shivaRightBite = { 0, 0, 920, 22 };

static void TriggerShivaSmoke(long x, long y, long z, long uw)
{
	long size;
	SPARKS* sptr;
	long dx, dz;

	dx = LaraItem->Position.xPos - x;
	dz = LaraItem->Position.zPos - z;

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
		sptr->transType = TransTypeEnum::COLADD;
	else
		sptr->transType = TransTypeEnum::COLADD;

	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x + (GetRandomControl() & 31) - 16;
	sptr->y = y + (GetRandomControl() & 31) - 16;
	sptr->z = z + (GetRandomControl() & 31) - 16;
	sptr->xVel = ((GetRandomControl() & 4095) - 2048) / 4;
	sptr->yVel = (GetRandomControl() & 255) - 128;
	sptr->zVel = ((GetRandomControl() & 4095) - 2048) / 4;

	if (uw)
	{
		sptr->yVel /= 16;
		sptr->y += 32;
		sptr->friction = 4 | (16);
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
	sptr->size = sptr->sSize = size / 4;
	sptr->dSize = size;
	size += (GetRandomControl() & 31) + 32;
	sptr->size = sptr->sSize = size / 8;
	sptr->dSize = size;
}

static void ShivaDamage(ITEM_INFO* item, CREATURE_INFO* shiva, int damage)
{
	if (!(shiva->flags) && (item->TouchBits & 0x2400000))
	{
		LaraItem->HitPoints -= damage;
		LaraItem->HitStatus = true;
		CreatureEffect(item, &shivaRightBite, DoBloodSplat);
		shiva->flags = 1;
		SoundEffect(SFX_TR2_CRUNCH2, &item->Position, 0);
	}

	if (!(shiva->flags) && (item->TouchBits & 0x2400))
	{
		LaraItem->HitPoints -= damage;
		LaraItem->HitStatus = true;
		CreatureEffect(item, &shivaLeftBite, DoBloodSplat);
		shiva->flags = 1;
		SoundEffect(SFX_TR2_CRUNCH2, &item->Position, 0);
	}
}

void InitialiseShiva(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;

	ClearItem(itemNum);

	item = &g_Level.Items[itemNum];
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 14;

	anim = &g_Level.Anims[item->AnimNumber];

	item->FrameNumber = anim->frameBase;
	item->ActiveState = anim->ActiveState;
}

void ShivaControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* shiva;
	short angle, head_x, head_y, torso_x, torso_y, tilt, roomNumber;
	int x, z;
	int random, lara_alive;
	AI_INFO info;
	PHD_VECTOR	pos;
	FLOOR_INFO* floor;
	int effect_mesh = 0;

	item = &g_Level.Items[itemNum];
	shiva = (CREATURE_INFO*)item->Data;
	head_x = head_y = torso_x = torso_y = angle = tilt = 0;
	lara_alive = (LaraItem->HitPoints > 0);
	pos.x = 0;
	pos.y = 0;
	pos.z = 256;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 9)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 22;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 9;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (shiva->mood == ESCAPE_MOOD)
		{
			shiva->target.x = LaraItem->Position.xPos;
			shiva->target.z = LaraItem->Position.zPos;
		}

		angle = CreatureTurn(item, shiva->maximumTurn);

		if (item->ActiveState != 4)
			item->MeshBits = 0xFFFFFFFF;

		switch (item->ActiveState)
		{
		case 4:
			shiva->maximumTurn = 0;

			if (!shiva->flags)
			{
				if (item->MeshBits == 0)
					effect_mesh = 0;
				item->MeshBits = (item->MeshBits * 2) + 1;
				shiva->flags = 1;

				GetJointAbsPosition(item, &pos, effect_mesh++);
				TriggerExplosionSparks(pos.x, pos.y, pos.z, 2, 0, 0, item->RoomNumber);
				TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);

			}
			else
			{
				shiva->flags--;
			}

			if (item->MeshBits == 0x7FFFFFFF)
			{
				item->TargetState = 0;
				effect_mesh = 0;
				shiva->flags = -45;
			}
			break;

		case 0:
			if (info.ahead)
				head_y = info.angle;

			if (shiva->flags < 0)
			{
				shiva->flags++;
				TriggerShivaSmoke(item->Position.xPos + (GetRandomControl() & 0x5FF) - 0x300, pos.y - (GetRandomControl() & 0x5FF), item->Position.zPos + (GetRandomControl() & 0x5FF) - 0x300, 1);
				break;
			}

			if (shiva->flags == 1)
				shiva->flags = 0;

			shiva->maximumTurn = 0;

			if (shiva->mood == ESCAPE_MOOD)
			{
				roomNumber = item->RoomNumber;
				x = item->Position.xPos + WALL_SIZE * phd_sin(item->Position.yRot + 0x8000);
				z = item->Position.zPos + WALL_SIZE * phd_cos(item->Position.yRot + 0x8000);
				floor = GetFloor(x, item->Position.yPos, z, &roomNumber);

				if (!shiva->flags && floor->Box != NO_BOX && !(g_Level.Boxes[floor->Box].flags & BLOCKABLE))
					item->TargetState = 8;
				else
					item->TargetState = 2;
			}
			else if (shiva->mood == BORED_MOOD)
			{
				random = GetRandomControl();
				if (random < 0x400)
					item->TargetState = 1;
			}
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 5 / 4))
			{
				item->TargetState = 5;
				shiva->flags = 0;
			}
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 4 / 3))
			{
				item->TargetState = 7;
				shiva->flags = 0;
			}
			else if (item->HitStatus && info.ahead)
			{
				shiva->flags = 4;
				item->TargetState = 2;
			}
			else
			{
				item->TargetState = 1;
			}
			break;

		case 2:
			if (info.ahead)
				head_y = info.angle;

			shiva->maximumTurn = 0;
			if (item->HitStatus || shiva->mood == ESCAPE_MOOD)
				shiva->flags = 4;

			if ((info.bite && info.distance < SQUARE(WALL_SIZE * 4 / 3)) || (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase && !shiva->flags) || !info.ahead)
			{
				item->TargetState = 0;
				shiva->flags = 0;
			}
			else if (shiva->flags)
			{
				item->TargetState = 2;
			}

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase && shiva->flags > 1)
				shiva->flags -= 2;
			break;

		case 1:
			if (info.ahead)
				head_y = info.angle;

			shiva->maximumTurn = ANGLE(4);

			if (shiva->mood == ESCAPE_MOOD)
			{
				item->TargetState = 0;
			}
			else if (shiva->mood == BORED_MOOD)
			{
				item->TargetState = 0;
			}
			else if (info.bite && info.distance < SQUARE(WALL_SIZE * 4 / 3))
			{
				item->TargetState = 0;
				shiva->flags = 0;
			}
			else if (item->HitStatus)
			{
				shiva->flags = 4;
				item->TargetState = 3;
			}
			break;

		case 3:
			if (info.ahead)
				head_y = info.angle;

			shiva->maximumTurn = ANGLE(4);

			if (item->HitStatus)
				shiva->flags = 4;

			if ((info.bite && info.distance < SQUARE(WALL_SIZE * 5 / 4)) || (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase && !shiva->flags))
			{
				item->TargetState = 1;
				shiva->flags = 0;
			}
			else if (shiva->flags)
			{
				item->TargetState = 3;
			}

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
			{
				shiva->flags = 0;
			}
			break;

		case 8:
			if (info.ahead)
				head_y = info.angle;

			shiva->maximumTurn = ANGLE(4);
			if ((info.ahead && info.distance < SQUARE(WALL_SIZE * 4 / 3)) || (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase && !shiva->flags))
			{
				item->TargetState = 0;
			}
			else if (item->HitStatus)
			{
				shiva->flags = 4;
				item->TargetState = 0;
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
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 10 || item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 21 || item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 33)
			{
				CreatureEffect(item, &shivaRightBite, DoBloodSplat);
				CreatureEffect(item, &shivaLeftBite, DoBloodSplat);
			}
			break;
		}
	}

	if (lara_alive && LaraItem->HitPoints <= 0)
	{
		CreatureKill(item, 18, 6, 2);
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