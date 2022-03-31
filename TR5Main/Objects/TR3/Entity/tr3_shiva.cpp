#include "framework.h"
#include "Objects/TR3/Entity/tr3_shiva.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO ShivaBiteLeft = { 0, 0, 920, 13 };
BITE_INFO ShivaBiteRight = { 0, 0, 920, 22 };

// TODO
enum ShivaState
{

};

// TODO
enum ShivaAnim
{

};

static void TriggerShivaSmoke(long x, long y, long z, long uw)
{
	long dx = LaraItem->Position.xPos - x;
	long dz = LaraItem->Position.zPos - z;

	if (dx < -SECTOR(16) || dx > SECTOR(16) ||
		dz < -SECTOR(16) || dz > SECTOR(16))
	{
		return;
	}

	auto* sptr = &Sparks[GetFreeSpark()];

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
		sptr->friction = 6;

	sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	sptr->rotAng = GetRandomControl() & 4095;

	if (GetRandomControl() & 1)
		sptr->rotAdd = -(GetRandomControl() & 15) - 16;
	else
		sptr->rotAdd = (GetRandomControl() & 15) + 16;

	sptr->scalar = 3;

	if (uw)
		sptr->gravity = sptr->maxYvel = 0;
	else
	{
		sptr->gravity = -(GetRandomControl() & 3) - 3;
		sptr->maxYvel = -(GetRandomControl() & 3) - 4;
	}
	
	long size = (GetRandomControl() & 31) + 128;

	sptr->size = sptr->sSize = size / 4;
	sptr->dSize = size;
	size += (GetRandomControl() & 31) + 32;
	sptr->size = sptr->sSize = size / 8;
	sptr->dSize = size;
}

static void ShivaDamage(ITEM_INFO* item, CreatureInfo* creature, int damage)
{
	if (!(creature->Flags) && item->TouchBits & 0x2400000)
	{
		CreatureEffect(item, &ShivaBiteRight, DoBloodSplat);
		SoundEffect(SFX_TR2_CRUNCH2, &item->Position, 0);
		creature->Flags = 1;

		LaraItem->HitPoints -= damage;
		LaraItem->HitStatus = true;
	}

	if (!(creature->Flags) && item->TouchBits & 0x2400)
	{
		CreatureEffect(item, &ShivaBiteLeft, DoBloodSplat);
		SoundEffect(SFX_TR2_CRUNCH2, &item->Position, 0);
		creature->Flags = 1;

		LaraItem->HitPoints -= damage;
		LaraItem->HitStatus = true;
	}
}

void InitialiseShiva(short itemNumber)
{
	ClearItem(itemNumber);

	auto* item = &g_Level.Items[itemNumber];
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 14;
	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];

	item->Animation.FrameNumber = anim->frameBase;
	item->Animation.ActiveState = anim->ActiveState;
}

void ShivaControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* shiva = GetCreatureInfo(item);

	PHD_VECTOR pos = { 0, 0, 256 };
	int laraAlive = LaraItem->HitPoints > 0;

	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 9)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 22;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 9;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		if (shiva->Mood == MoodType::Escape)
		{
			shiva->Target.x = LaraItem->Position.xPos;
			shiva->Target.z = LaraItem->Position.zPos;
		}

		angle = CreatureTurn(item, shiva->MaxTurn);

		if (item->Animation.ActiveState != 4)
			item->MeshBits = 0xFFFFFFFF;

		int effectMesh = 0;

		switch (item->Animation.ActiveState)
		{
		case 4:
			shiva->MaxTurn = 0;

			if (!shiva->Flags)
			{
				if (item->MeshBits == 0)
					effectMesh = 0;

				item->MeshBits = (item->MeshBits * 2) + 1;
				shiva->Flags = 1;

				GetJointAbsPosition(item, &pos, effectMesh++);
				TriggerExplosionSparks(pos.x, pos.y, pos.z, 2, 0, 0, item->RoomNumber);
				TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);

			}
			else
				shiva->Flags--;

			if (item->MeshBits == 0x7FFFFFFF)
			{
				item->Animation.TargetState = 0;
				shiva->Flags = -45;
				effectMesh = 0;
			}

			break;

		case 0:
			shiva->MaxTurn = 0;

			if (AI.ahead)
				headY = AI.angle;

			if (shiva->Flags < 0)
			{
				shiva->Flags++;
				TriggerShivaSmoke(item->Position.xPos + (GetRandomControl() & 0x5FF) - 0x300, pos.y - (GetRandomControl() & 0x5FF), item->Position.zPos + (GetRandomControl() & 0x5FF) - 0x300, 1);
				break;
			}

			if (shiva->Flags == 1)
				shiva->Flags = 0;

			if (shiva->Mood == MoodType::Escape)
			{
				int x = item->Position.xPos + SECTOR(1) * phd_sin(item->Position.yRot + ANGLE(180.0f));
				int z = item->Position.zPos + SECTOR(1) * phd_cos(item->Position.yRot + ANGLE(180.0f));
				auto box = GetCollision(x, item->Position.yPos, z, item->RoomNumber).BottomBlock->Box;

				if (box != NO_BOX && !(g_Level.Boxes[box].flags & BLOCKABLE) && !shiva->Flags)
					item->Animation.TargetState = 8;
				else
					item->Animation.TargetState = 2;
			}
			else if (shiva->Mood == MoodType::Bored)
			{
				int random = GetRandomControl();
				if (random < 0x400)
					item->Animation.TargetState = 1;
			}
			else if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2))
			{
				item->Animation.TargetState = 5;
				shiva->Flags = 0;
			}
			else if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2))
			{
				item->Animation.TargetState = 7;
				shiva->Flags = 0;
			}
			else if (item->HitStatus && AI.ahead)
			{
				shiva->Flags = 4;
				item->Animation.TargetState = 2;
			}
			else
				item->Animation.TargetState = 1;

			break;

		case 2:
			shiva->MaxTurn = 0;

			if (AI.ahead)
				headY = AI.angle;

			if (item->HitStatus || shiva->Mood == MoodType::Escape)
				shiva->Flags = 4;

			if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2) ||
				(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
					!shiva->Flags) ||
				!AI.ahead)
			{
				item->Animation.TargetState = 0;
				shiva->Flags = 0;
			}
			else if (shiva->Flags)
				item->Animation.TargetState = 2;


			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase && shiva->Flags > 1)
				shiva->Flags -= 2;

			break;

		case 1:
			shiva->MaxTurn = ANGLE(4.0f);

			if (AI.ahead)
				headY = AI.angle;

			if (shiva->Mood == MoodType::Escape)
				item->Animation.TargetState = 0;
			else if (shiva->Mood == MoodType::Bored)
				item->Animation.TargetState = 0;
			else if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2))
			{
				item->Animation.TargetState = 0;
				shiva->Flags = 0;
			}
			else if (item->HitStatus)
			{
				shiva->Flags = 4;
				item->Animation.TargetState = 3;
			}

			break;

		case 3:
			shiva->MaxTurn = ANGLE(4.0f);

			if (AI.ahead)
				headY = AI.angle;

			if (item->HitStatus)
				shiva->Flags = 4;

			if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2) ||
				(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
					!shiva->Flags))
			{
				item->Animation.TargetState = 1;
				shiva->Flags = 0;
			}
			else if (shiva->Flags)
				item->Animation.TargetState = 3;

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				shiva->Flags = 0;
			
			break;

		case 8:
			shiva->MaxTurn = ANGLE(4.0f);

			if (AI.ahead)
				headY = AI.angle;

			if (AI.ahead && AI.distance < pow(SECTOR(4) / 3, 2) ||
				(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
					!shiva->Flags))
			{
				item->Animation.TargetState = 0;
			}
			else if (item->HitStatus)
			{
				shiva->Flags = 4;
				item->Animation.TargetState = 0;
			}
			
			break;

		case 5:
			shiva->MaxTurn = ANGLE(4.0f);

			if (AI.ahead)
			{
				headY = AI.angle;
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			ShivaDamage(item, shiva, 150);
			break;

		case 7:
			shiva->MaxTurn = ANGLE(4.0f);
			headY = AI.angle;
			torsoY = AI.angle;

			if (AI.xAngle > 0)
				torsoX = AI.xAngle;

			ShivaDamage(item, shiva, 180);
			break;

		case 6:
			shiva->MaxTurn = 0;
			headX = 0;
			headY = 0;
			torsoX = 0;
			torsoY = 0;

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 10 ||
				item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 21 ||
				item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 33)
			{
				CreatureEffect(item, &ShivaBiteRight, DoBloodSplat);
				CreatureEffect(item, &ShivaBiteLeft, DoBloodSplat);
			}

			break;
		}
	}

	if (laraAlive && LaraItem->HitPoints <= 0)
	{
		CreatureKill(item, 18, 6, 2);
		return;
	}

	CreatureTilt(item, tilt);
	headY -= torsoY;
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, headY);
	CreatureJoint(item, 3, headX);
	CreatureAnimation(itemNumber, angle, tilt);
}
