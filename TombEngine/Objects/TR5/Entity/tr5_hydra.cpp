#include "framework.h"
#include "tr5_hydra.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/animation.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO HydraBite = { 0, 0, 0, 11 };

// TODO
enum HydraState
{
	HYDRA_STATE_STOP = 0,
	HYDRA_STATE_BITE_ATTACK1 = 1,
	HYDRA_STATE_AIM = 2,

	HYDRA_STATE_HURT = 4,

	HYDRA_STATE_BITE_ATTACK2 = 7,
	HYDRA_STATE_BITE_ATTACK3 = 8,
	HYDRA_STATE_BITE_ATTACK4 = 9,

	HYDRA_STATE_DEATH = 11
};

// TODO
enum HydraAnim
{

};

void InitialiseHydra(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = 30 * item->TriggerFlags + g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = HYDRA_STATE_STOP;
	item->Animation.ActiveState = HYDRA_STATE_STOP;

	if (item->TriggerFlags == 1)
		item->Pose.Position.z += CLICK(1.5f);

	if (item->TriggerFlags == 2)
		item->Pose.Position.z -= CLICK(1.5f);

	item->Pose.Orientation.y = ANGLE(90.0f);
	item->Pose.Position.x -= CLICK(1);
}

static void HydraBubblesAttack(PHD_3DPOS* pos, short roomNumber, int count)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.Position.x = pos->Position.x;
		fx->pos.Position.y = pos->Position.y - (GetRandomControl() & 0x3F) - 32;
		fx->pos.Position.z = pos->Position.z;
		fx->pos.Orientation.x = pos->Orientation.x;
		fx->pos.Orientation.y = pos->Orientation.y;
		fx->pos.Orientation.z = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 16 * count + 15;
		fx->flag1 = 0;
		fx->objectNumber = ID_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 64;
		fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 8;
	}
}

void TriggerHydraMissileSparks(Vector3Int* pos, short xv, short yv, short zv)
{
	auto* spark = GetFreeParticle();

	spark->on = true;
	spark->sB = 0;
	spark->sR = (GetRandomControl() & 0x3F) - 96;
	spark->sG = spark->sR / 2;
	spark->dB = 0;
	spark->dR = (GetRandomControl() & 0x3F) - 96;
	spark->dG = spark->dR / 2;
	spark->fadeToBlack = 8;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 20;
	spark->x = (GetRandomControl() & 0xF) - 8;
	spark->y = 0;
	spark->z = (GetRandomControl() & 0xF) - 8;
	spark->x += pos->x;
	spark->y += pos->y;
	spark->z += pos->z;
	spark->xVel = xv;
	spark->yVel = yv;
	spark->zVel = zv;
	spark->friction = 68;
	spark->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
	spark->rotAng = GetRandomControl() & 0xFFF;

	if (GetRandomControl() & 1)
		spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
	else
		spark->rotAdd = (GetRandomControl() & 0x1F) + 32;

	spark->gravity = 0;
	spark->maxYvel = 0;
	spark->scalar = 1;
	spark->sSize = spark->size = (GetRandomControl() & 0xF) + 96;
	spark->dSize = spark->size / 4;
}

static void TriggerHydraSparks(short itemNumber, int frame)
{
	auto* spark = GetFreeParticle();
	
	spark->on = 1;
	spark->sB = 0;
	spark->sR = (GetRandomControl() & 0x3F) - 96;
	spark->dR = (GetRandomControl() & 0x3F) - 96;
	spark->dB = 0;

	if (frame < 16)
	{
		spark->sR = frame * spark->sR / 16;
		spark->dR = frame * spark->dR / 16;
	}

	spark->sG = spark->sR / 2;
	spark->dG = spark->dR / 2;
	spark->fadeToBlack = 4;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
	spark->dynamic = -1;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 32;
	spark->x = (GetRandomControl() & 0xF) - 8;
	spark->y = 0;
	spark->z = (GetRandomControl() & 0xF) - 8;
	spark->xVel = (byte)GetRandomControl() - 128;
	spark->yVel = 0;
	spark->zVel = (byte)GetRandomControl() - 128;
	spark->friction = 4;
	spark->flags = 4762;
	spark->fxObj = itemNumber;
	spark->nodeNumber = 5;
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
	spark->maxYvel = 0;
	spark->gravity = -8 - (GetRandomControl() & 7);
	spark->scalar = 0;
	spark->dSize = 4;
	spark->sSize = spark->size = (frame * ((GetRandomControl() & 0xF) + 16)) / 16;
}

void HydraControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (item->HitPoints > 0)
	{
		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		if (item->Animation.ActiveState != 5 &&
			item->Animation.ActiveState != 10 &&
			item->Animation.ActiveState != HYDRA_STATE_DEATH)
		{
			if (abs(AI.angle) >= ANGLE(1.0f))
			{
				if (AI.angle > 0)
					item->Pose.Orientation.y += ANGLE(1.0f);
				else
					item->Pose.Orientation.y -= ANGLE(1.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (item->TriggerFlags == 1)
				tilt = -ANGLE(2.8f);
			else if (item->TriggerFlags == 2)
				tilt = ANGLE(2.8f);
		}

		if (item->Animation.ActiveState != 12)
		{
			joint1 = AI.angle / 2;
			joint3 = AI.angle / 2;
			joint2 = -AI.xAngle;
		}

		joint0 = -joint1;

		int distance, damage, frame;
		short roomNumber;

		switch (item->Animation.ActiveState)
		{
		case HYDRA_STATE_STOP:
			creature->MaxTurn = ANGLE(1.0f);
			creature->Flags = 0;

			if (item->TriggerFlags == 1)
				tilt = -ANGLE(2.8f);
			else if (item->TriggerFlags == 2)
				tilt = ANGLE(2.8f);

			if (AI.distance >= pow(CLICK(7), 2) && GetRandomControl() & 0x1F)
			{
				if (AI.distance >= pow(SECTOR(2), 2) && GetRandomControl() & 0x1F)
				{
					if (!(GetRandomControl() & 0xF))
						item->Animation.TargetState = HYDRA_STATE_AIM;
				}
				else
					item->Animation.TargetState = HYDRA_STATE_BITE_ATTACK1;
			}
			else
				item->Animation.TargetState = 6;
			
			break;

		case HYDRA_STATE_BITE_ATTACK1:
		case HYDRA_STATE_BITE_ATTACK2:
		case HYDRA_STATE_BITE_ATTACK3:
		case HYDRA_STATE_BITE_ATTACK4:
			creature->MaxTurn = 0;

			if (creature->Flags == 0)
			{
				if (item->TouchBits & 0x400)
				{
					DoDamage(creature->Enemy, 120);
					CreatureEffect2(item, &HydraBite, 10, item->Pose.Orientation.y, DoBloodSplat);
					creature->Flags = 1;
				}

				if (item->HitStatus && AI.distance <  pow(CLICK(7), 2))
				{
					distance = sqrt(AI.distance);
					damage = 5 - distance / SECTOR(1);

					if (Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun)
						damage *= 3;

					if (damage > 0)
					{
						item->Animation.TargetState = HYDRA_STATE_HURT;
						DoDamage(item, damage);
						CreatureEffect2(item, &HydraBite, 10 * damage, item->Pose.Orientation.y, DoBloodSplat);
					}
				}
			}

			break;

		case HYDRA_STATE_AIM:
			creature->MaxTurn = 0;

			if (item->HitStatus)
			{
				// TEST: uncomment this for making HYDRA die on first hit event
				/* DoDamage(item, INT_MAX);
				break;*/

				damage = 6 - sqrt(AI.distance) / 1024;

				if (Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun)
					damage *= 3;

				if ((GetRandomControl() & 0xF) < damage && AI.distance < SQUARE(10240) && damage > 0)
				{
					item->Animation.TargetState = 4;
					DoDamage(item, damage);
					CreatureEffect2(item, &HydraBite, 10 * damage, item->Pose.Orientation.y, DoBloodSplat);
				}
			}

			if (item->TriggerFlags == 1)
				tilt = -ANGLE(2.8f);
			else if (item->TriggerFlags == 2)
				tilt = ANGLE(2.8f);

			if (!(GlobalCounter & 3))
			{
				frame = ((g_Level.Anims[item->Animation.AnimNumber].frameBase - item->Animation.FrameNumber) / 8) + 1;
				if (frame > 16)
					frame = 16;

				TriggerHydraSparks(itemNumber, frame);
			}

			break;

		case 3:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				auto pos1 = Vector3Int(0, 1024, 40);
				GetJointAbsPosition(item, &pos1, 10);

				auto pos2 = Vector3Int(0, 144, 40);
				GetJointAbsPosition(item, &pos2, 10);

				auto angles = GetVectorAngles(pos1.x - pos2.x, pos1.y - pos2.y, pos1.z - pos2.z);
				auto pos = PHD_3DPOS(pos1, angles);
				roomNumber = item->RoomNumber;
				GetFloor(pos2.x, pos2.y, pos2.z, &roomNumber);

				// TEST: uncomment this for making HYDRA not firing bubbles
				HydraBubblesAttack(&pos, roomNumber, 1);
			}

			break;

		case 6:
			creature->MaxTurn = ANGLE(1.0f);
			creature->Flags = 0;

			if (item->TriggerFlags == 1)
				tilt = -ANGLE(2.8f);
			else if (item->TriggerFlags == 2)
				tilt = ANGLE(2.8f);

			if (AI.distance >= pow(CLICK(3), 2))
			{
				if (AI.distance >= pow(CLICK(5), 2))
				{
					if (AI.distance >= pow(CLICK(7), 2))
						item->Animation.TargetState = 0;
					else
						item->Animation.TargetState = 9;
				}
				else
					item->Animation.TargetState = 8;
			}
			else
				item->Animation.TargetState = 7;
			
			break;

		default:
			break;
		}
	}
	else
	{
		item->HitPoints = 0;

		if (item->Animation.ActiveState != HYDRA_STATE_DEATH)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 15;
			item->Animation.ActiveState = HYDRA_STATE_DEATH;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		}

		if (!((item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase) & 7))
		{
			if (item->ItemFlags[3] < 12)
			{
				ExplodeItemNode(item, 11 - item->ItemFlags[3], 0, 64);
				SoundEffect(SFX_TR5_SMASH_ROCK2, &item->Pose);
				item->ItemFlags[3]++;
			}
		}
	}

	CreatureTilt(item, tilt);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);

	CreatureAnimation(itemNumber, 0, 0);
}
