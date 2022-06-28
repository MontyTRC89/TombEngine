#include "framework.h"
#include "tr4_sphinx.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/debris.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"

using std::vector;

namespace TEN::Entities::TR4
{
	BITE_INFO SphinxBiteInfo = { 0, 0, 0, 6 };
	const vector<int> SphinxAttackJoints = { 6 };

	constexpr auto SPHINX_ATTACK_DAMAGE = 200;

	#define SPHINX_WALK_TURN_ANGLE ANGLE(3.0f)
	#define SPHINX_RUN_TURN_ANGLE ANGLE(0.33f)

	enum SphinxState
	{
		SPHINX_STATE_NONE = 0,
		SPHINX_STATE_REST = 1,
		SPHINX_STATE_REST_ALERTED = 2,
		SPHINX_STATE_SLEEP_TO_IDLE = 3,
		SPHINX_STATE_WALK_FORWARD = 4,
		SPHINX_STATE_RUN_FORWARD = 5,
		SPHINX_STATE_WALK_BACK = 6,
		SPHINX_STATE_COLLIDE = 7,
		SPHINX_STATE_SHAKE = 8,
		SPHINX_STATE_IDLE = 9
	};

	enum SphinxAnim
	{
		SPHINX_ANIM_RUN_FORWARD = 0,
		SPHINX_ANIM_REST = 1,
		SPHINX_ANIM_REST_ALERTED = 2,
		SPHINX_ANIM_REST_TO_IDLE = 3,
		SPHINX_ANIM_WALK_FORWARD = 4,
		SPHINX_ANIM_IDLE_TO_WALK_FORWARD = 5,
		SPHINX_ANIM_COLLIDE = 6,
		SPHINX_ANIM_SHAKE = 7,
		SPHINX_ANIM_WALK_FORWARD_TO_RUN_FORWARD = 8,
		SPHINX_ANIM_WALK_BACK = 9,
		SPHINX_ANIM_IDLE = 10,
		SPHINX_ANIM_RUN_FORWARD_TO_IDLE = 11,
		SPHINX_ANIM_WALK_FORWARD_TO_IDLE = 12,
		SPHINX_ANIM_IDLE_TO_RUN_FORWARD = 13,
		SPHINX_ANIM_IDLE_TO_WALK_BACK = 14,
		SPHINX_ANIM_WALK_BACK_TO_IDLE = 15
	};

	void InitialiseSphinx(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SPHINX_ANIM_REST;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = SPHINX_STATE_REST;
		item->Animation.ActiveState = SPHINX_STATE_REST;
	}

	void SphinxControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		int x = item->Pose.Position.x + 614 * phd_sin(item->Pose.Orientation.y);
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z + 614 * phd_cos(item->Pose.Orientation.y);

		auto probe = GetCollision(x, y, z, item->RoomNumber);

		int height1 = probe.Position.Floor;

		if (item->Animation.ActiveState == SPHINX_STATE_RUN_FORWARD && probe.Block->Stopper)
		{
			auto* room = &g_Level.Rooms[item->RoomNumber];

			for (int i = 0; i < room->mesh.size(); i++)
			{
				auto* mesh = &room->mesh[i];

				if (((mesh->pos.Position.z / SECTOR(1)) == (z / SECTOR(1))) &&
					((mesh->pos.Position.x / SECTOR(1)) == (x / SECTOR(1))) &&
					StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
				{
					ShatterObject(nullptr, mesh, -64, item->RoomNumber, 0);
					SoundEffect(SFX_TR4_SMASH_ROCK, &item->Pose);

					mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
					probe.Block = false;

					TestTriggers(x, y, z, item->RoomNumber, true);
				}
			}
		}

		x = item->Pose.Position.x - 614 * phd_sin(item->Pose.Orientation.y);
		y = item->Pose.Position.y;
		z = item->Pose.Position.z - 614 * phd_cos(item->Pose.Orientation.y);

		int height2 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		phd_atan(1228, height2 - height1);

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (creature->Enemy != LaraItem)
			phd_atan(LaraItem->Pose.Position.z - item->Pose.Position.z, LaraItem->Pose.Position.x - item->Pose.Position.x);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		short angle = CreatureTurn(item, creature->MaxTurn);

		int dx = abs(item->ItemFlags[2] - (short)item->Pose.Position.x);
		int dz = abs(item->ItemFlags[3] - (short)item->Pose.Position.z);

		switch (item->Animation.ActiveState)
		{
		case SPHINX_STATE_REST:
			creature->MaxTurn = 0;

			if (AI.distance < pow(SECTOR(1), 2) || item->TriggerFlags)
				item->Animation.TargetState = SPHINX_STATE_SLEEP_TO_IDLE;

			if (GetRandomControl() == 0)
				item->Animation.TargetState = SPHINX_STATE_REST_ALERTED;

			break;

		case SPHINX_STATE_REST_ALERTED:
			creature->MaxTurn = 0;

			if (AI.distance < pow(SECTOR(1), 2) || item->TriggerFlags)
				item->Animation.TargetState = SPHINX_STATE_SLEEP_TO_IDLE;

			if (GetRandomControl() == 0)
				item->Animation.TargetState = SPHINX_STATE_REST;

			break;

		case SPHINX_STATE_WALK_FORWARD:
			creature->MaxTurn = SPHINX_WALK_TURN_ANGLE;

			if (AI.distance > pow(SECTOR(1), 2) && abs(AI.angle) <= ANGLE(2.8f) || item->Animation.RequiredState == SPHINX_STATE_RUN_FORWARD)
				item->Animation.TargetState = SPHINX_STATE_RUN_FORWARD;
			else if (AI.distance < pow(SECTOR(2), 2) && item->Animation.TargetState != SPHINX_STATE_RUN_FORWARD)
			{
				if (height2 <= (item->Pose.Position.y + CLICK(1)) &&
					height2 >= (item->Pose.Position.y - CLICK(1)))
				{
					item->Animation.TargetState = SPHINX_STATE_IDLE;
					item->Animation.RequiredState = SPHINX_STATE_WALK_BACK;
				}
			}

			break;

		case SPHINX_STATE_RUN_FORWARD:
			creature->MaxTurn = SPHINX_RUN_TURN_ANGLE;

			if (creature->Flags == 0)
			{
				if (item->TestBits(JointBitType::Touch, SphinxAttackJoints))
				{
					CreatureEffect2(
						item,
						&SphinxBiteInfo,
						20,
						-1,
						DoBloodSplat);
					creature->Flags = 1;

					DoDamage(creature->Enemy, SPHINX_ATTACK_DAMAGE);
				}
			}

			if (dx >= 50 || dz >= 50 ||
				item->Animation.AnimNumber != Objects[item->ObjectNumber].animIndex)
			{
				if (AI.distance > pow(SECTOR(2), 2) && abs(AI.angle) > ANGLE(2.8f))
					item->Animation.TargetState = SPHINX_STATE_IDLE;
			}
			else
			{
				item->Animation.TargetState = SPHINX_STATE_COLLIDE;
				item->Animation.RequiredState = SPHINX_STATE_WALK_BACK;
				creature->MaxTurn = 0;
			}

			break;

		case SPHINX_STATE_WALK_BACK:
			creature->MaxTurn = SPHINX_WALK_TURN_ANGLE;

			if (AI.distance > pow(SECTOR(2), 2) ||
				height2 > (item->Pose.Position.y + CLICK(1)) ||
				height2 < (item->Pose.Position.y - CLICK(1)))
			{
				item->Animation.TargetState = SPHINX_STATE_IDLE;
				item->Animation.RequiredState = SPHINX_STATE_RUN_FORWARD;
			}

			break;

		case SPHINX_STATE_COLLIDE:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				TestTriggers(item, true);

				if (item->TestBits(JointBitType::Touch, SphinxAttackJoints))
				{
					CreatureEffect2(
						item,
						&SphinxBiteInfo,
						50,
						-1,
						DoBloodSplat);

					DoDamage(creature->Enemy, INT_MAX);
				}
			}

			break;

		case SPHINX_STATE_IDLE:
			creature->Flags = 0;

			if (item->Animation.RequiredState == SPHINX_STATE_WALK_BACK)
				item->Animation.TargetState = SPHINX_STATE_WALK_BACK;
			else
				item->Animation.TargetState = SPHINX_STATE_WALK_FORWARD;

			break;

		default:
			break;
		}

		item->ItemFlags[2] = (short)item->Pose.Position.x;
		item->ItemFlags[3] = (short)item->Pose.Position.z;

		CreatureAnimation(itemNumber, angle, 0);
	}
}
