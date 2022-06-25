#include "framework.h"
#include "tr4_knight_templar.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

using std::vector;

namespace TEN::Entities::TR4
{
	BITE_INFO KnightTemplarBite = { 0, 0, 0, 11 };
	const vector<int> KnightTemplarSwordAttackJoints = { 10, 11 };

	constexpr auto KNIGHT_TEMPLAR_SWORD_ATTACK_DAMAGE = 120;

	#define KNIGHT_TEMPLAR_IDLE_TURN_ANGLE ANGLE(2.0f)
	#define KNIGHT_TEMPLAR_WALK_TURN_ANGLE ANGLE(7.0f)

	enum KnightTemplarState
	{
		KTEMPLAR_STATE_NONE = 0,
		KTEMPLAR_STATE_IDLE = 1,
		KTEMPLAR_STATE_WALK_FORWARD = 2,
		KTEMPLAR_STATE_SWORD_ATTACK_1 = 3,
		KTEMPLAR_STATE_SWORD_ATTACK_2 = 4,
		KTEMPLAR_STATE_SWORD_ATTACK_3 = 5,
		KTEMPLAR_STATE_SHIELD = 6,
		KTEMPLAR_STATE_SHIELD_HIT_1 = 7,
		KTEMPLAR_STATE_SHIELD_HIT_2 = 8
	};

	enum KnightTemplarAnim
	{
		KTEMPLAR_ANIM_WALK_FORWARD_LEFT_1 = 0,
		KTEMPLAR_ANIM_WALK_FORWARD_RIGHT_1 = 1,
		KTEMPLAR_ANIM_IDLE = 2,
		KTEMPLAR_ANIM_SWORD_ATTACK_1 = 3,
		KTEMPLAR_ANIM_SWORD_ATTACK_2 = 4,
		KTEMPLAR_ANIM_SWORD_ATTACK_3 = 5,
		KTEMPLAR_ANIM_SHIELD_START = 6,
		KTEMPLAR_ANIM_SHIELD_CONTINUE = 7,
		KTEMPLAR_ANIM_SHIELD_END = 8,
		KTEMPLAR_ANIM_SHIELD_HIT_1 = 9,
		KTEMPLAR_ANIM_SHIELD_HIT_2 = 10,
		KTEMPLAR_ANIM_WALK_FORWARD_LEFT_2 = 11,
		KTEMPLAR_ANIM_WALK_FORWARD_RIGHT_2 = 12
	};

	void InitialiseKnightTemplar(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[ID_KNIGHT_TEMPLAR].animIndex + KTEMPLAR_ANIM_IDLE;
		item->Animation.TargetState = KTEMPLAR_STATE_IDLE;
		item->Animation.ActiveState = KTEMPLAR_STATE_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->MeshBits &= 0xF7FF;
	}

	void KnightTemplarControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		if (item->Animation.AnimNumber == object->animIndex ||
			item->Animation.AnimNumber - object->animIndex == KTEMPLAR_ANIM_WALK_FORWARD_RIGHT_1 ||
			item->Animation.AnimNumber - object->animIndex == KTEMPLAR_ANIM_WALK_FORWARD_LEFT_2 ||
			item->Animation.AnimNumber - object->animIndex == KTEMPLAR_ANIM_WALK_FORWARD_RIGHT_2)
		{
			if (GetRandomControl() & 1)
			{
				auto pos = Vector3Int(0, 48, 448);
				GetJointAbsPosition(item, &pos, 10);

				TriggerMetalSparks(pos.x, pos.y, pos.z, (GetRandomControl() & 0x1FF) - 256, -128 - (GetRandomControl() & 0x7F), (GetRandomControl() & 0x1FF) - 256, 0);
			}
		}

		short tilt = 0;
		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		// Knight is immortal.
		if (item->HitPoints < object->HitPoints)
			item->HitPoints = object->HitPoints;

		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		// TODO: Unused block.
		int a = 0;
		if (creature->Enemy != LaraItem)
			a = phd_atan(item->Pose.Position.z - LaraItem->Pose.Position.z, item->Pose.Position.x - LaraItem->Pose.Position.x);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if (AI.ahead)
		{
			joint0 = AI.angle / 2;
			joint1 = AI.xAngle;
			joint2 = AI.angle / 2;
		}

		int frameBase = 0;
		int frameNumber = 0;

		switch (item->Animation.ActiveState)
		{
		case KTEMPLAR_STATE_IDLE:
			item->Animation.TargetState = KTEMPLAR_STATE_WALK_FORWARD;
			creature->MaxTurn = KNIGHT_TEMPLAR_IDLE_TURN_ANGLE;
			creature->Flags = 0;

			if (AI.distance > pow(SECTOR(0.67f), 2))
			{
				if (Lara.TargetEntity == item)
					item->Animation.TargetState = KTEMPLAR_STATE_SHIELD;
			}
			else if (GetRandomControl() & 1)
				item->Animation.TargetState = KTEMPLAR_STATE_SWORD_ATTACK_2;
			else if (GetRandomControl() & 1)
				item->Animation.TargetState = KTEMPLAR_STATE_SWORD_ATTACK_1;
			else
				item->Animation.TargetState = KTEMPLAR_STATE_SWORD_ATTACK_3;

			break;

		case KTEMPLAR_STATE_WALK_FORWARD:
			creature->MaxTurn = KNIGHT_TEMPLAR_WALK_TURN_ANGLE;

			if (Lara.TargetEntity == item || AI.distance <= pow(SECTOR(0.67f), 2))
				item->Animation.TargetState = KTEMPLAR_STATE_IDLE;

			break;

		case KTEMPLAR_STATE_SWORD_ATTACK_1:
		case KTEMPLAR_STATE_SWORD_ATTACK_2:
		case KTEMPLAR_STATE_SWORD_ATTACK_3:
			creature->MaxTurn = 0;

			if (abs(AI.angle) >= ANGLE(1.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += ANGLE(1.0f);
				else
					item->Pose.Orientation.y -= ANGLE(1.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			frameNumber = item->Animation.FrameNumber;
			frameBase = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			if (frameNumber > (frameBase + 42) &&
				frameNumber < (frameBase + 51))
			{
				auto pos = Vector3Int();
				GetJointAbsPosition(item, &pos, LM_LINARM);
				
				auto* room = &g_Level.Rooms[item->RoomNumber];
				FloorInfo* currentFloor = &room->floor[(pos.z - room->z) / SECTOR(1) + (pos.z - room->x) / SECTOR(1) * room->zSize];

				if (currentFloor->Stopper)
				{
					for (int i = 0; i < room->mesh.size(); i++)
					{
						auto* mesh = &room->mesh[i];

						if (floor(pos.x) == floor(mesh->pos.Position.x) &&
							floor(pos.z) == floor(mesh->pos.Position.z) &&
							StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
						{
							ShatterObject(nullptr, mesh, -64, LaraItem->RoomNumber, 0);
							SoundEffect(SFX_TR4_SMASH_ROCK, &item->Pose);

							mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
							currentFloor->Stopper = false;

							TestTriggers(pos.x, pos.y, pos.z, item->RoomNumber, true);
						}

						mesh++;
					}
				}

				if (!creature->Flags)
				{
					if (item->TestBits(JointBitType::Touch, KnightTemplarSwordAttackJoints))
					{
						CreatureEffect2(
							item,
							&KnightTemplarBite,
							20,
							-1,
							DoBloodSplat);

						creature->Flags = 1;

						DoDamage(creature->Enemy, KNIGHT_TEMPLAR_SWORD_ATTACK_DAMAGE);
					}
				}
			}

		case KTEMPLAR_STATE_SHIELD:
			creature->MaxTurn = 0;

			if (abs(AI.angle) >= ANGLE(1.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += ANGLE(1.0f);
				else
					item->Pose.Orientation.y -= ANGLE(1.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (item->HitStatus)
			{
				if (GetRandomControl() & 1)
					item->Animation.TargetState = KTEMPLAR_STATE_SHIELD_HIT_1;
				else
					item->Animation.TargetState = KTEMPLAR_STATE_SHIELD_HIT_2;
			}
			else if (AI.distance <= pow(SECTOR(0.67f), 2) || Lara.TargetEntity != item)
				item->Animation.TargetState = KTEMPLAR_STATE_IDLE;
			else
				item->Animation.TargetState = KTEMPLAR_STATE_SHIELD;

			break;

		default:
			break;
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
