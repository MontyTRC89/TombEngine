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

namespace TEN::Entities::TR4
{
	enum SphinxState
	{
		SPHINX_STATE_NONE,
		SPHINX_STATE_SLEEPING,
		SPHINX_STATE_ALERTED,
		SPHINX_STATE_WAKING_UP,
		SPHINX_STATE_WALK,
		SPHINX_STATE_RUN,
		SPHINX_STATE_WALK_BACK,
		SPHINX_STATE_HIT,
		SPHINX_STATE_SHAKING,
		SPHINX_STATE_IDLE
	};

	// TODO
	enum SphinxAnim
	{

	};

	BITE_INFO SphinxBiteInfo = { 0, 0, 0, 6 };

	void InitialiseSphinx(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = SPHINX_STATE_SLEEPING;
		item->Animation.ActiveState = SPHINX_STATE_SLEEPING;
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

		if (item->Animation.ActiveState == 5 && probe.Block->Stopper)
		{
			auto* room = &g_Level.Rooms[item->RoomNumber];

			for (int i = 0; i < room->mesh.size(); i++)
			{
				auto* mesh = &room->mesh[i];

				if (((mesh->pos.Position.z / 1024) == (z / 1024)) &&
					((mesh->pos.Position.x / 1024) == (x / 1024)) &&
					StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
				{
					ShatterObject(NULL, mesh, -64, item->RoomNumber, 0);
					SoundEffect(SFX_TR4_HIT_ROCK, &item->Pose);

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
		case SPHINX_STATE_SLEEPING:
			creature->MaxTurn = 0;

			if (AI.distance < pow(SECTOR(1), 2) || item->TriggerFlags)
				item->Animation.TargetState = SPHINX_STATE_WAKING_UP;

			if (GetRandomControl() == 0)
				item->Animation.TargetState = SPHINX_STATE_ALERTED;

			break;

		case SPHINX_STATE_ALERTED:
			creature->MaxTurn = 0;

			if (AI.distance < pow(SECTOR(1), 2) || item->TriggerFlags)
				item->Animation.TargetState = SPHINX_STATE_WAKING_UP;

			if (GetRandomControl() == 0)
				item->Animation.TargetState = SPHINX_STATE_SLEEPING;

			break;

		case SPHINX_STATE_WALK:
			creature->MaxTurn = ANGLE(3.0f);

			if (AI.distance > pow(SECTOR(1), 2) && abs(AI.angle) <= ANGLE(2.8f) || item->Animation.RequiredState == SPHINX_STATE_RUN)
				item->Animation.TargetState = SPHINX_STATE_RUN;
			else if (AI.distance < pow(SECTOR(2), 2) && item->Animation.TargetState != SPHINX_STATE_RUN)
			{
				if (height2 <= (item->Pose.Position.y + CLICK(1)) &&
					height2 >= (item->Pose.Position.y - CLICK(1)))
				{
					item->Animation.TargetState = SPHINX_STATE_IDLE;
					item->Animation.RequiredState = SPHINX_STATE_WALK_BACK;
				}
			}

			break;

		case SPHINX_STATE_RUN:
			creature->MaxTurn = 60;

			if (creature->Flags == 0)
			{
				if (item->TouchBits & 0x40)
				{
					CreatureEffect2(
						item,
						&SphinxBiteInfo,
						20,
						-1,
						DoBloodSplat);

					creature->Flags = 1;

					LaraItem->HitPoints -= 200;
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
				item->Animation.TargetState = SPHINX_STATE_HIT;
				item->Animation.RequiredState = SPHINX_STATE_WALK_BACK;
				creature->MaxTurn = 0;
			}

			break;

		case SPHINX_STATE_WALK_BACK:
			creature->MaxTurn = ANGLE(3.0f);

			if (AI.distance > pow(SECTOR(2), 2) ||
				height2 > (item->Pose.Position.y + CLICK(1)) ||
				height2 < (item->Pose.Position.y - CLICK(1)))
			{
				item->Animation.TargetState = SPHINX_STATE_IDLE;
				item->Animation.RequiredState = SPHINX_STATE_RUN;
			}

			break;

		case SPHINX_STATE_HIT:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				TestTriggers(item, true);

				if (item->TouchBits & 0x40)
				{
					CreatureEffect2(
						item,
						&SphinxBiteInfo,
						50,
						-1,
						DoBloodSplat);

					LaraItem->HitPoints = 0;
				}
			}

			break;

		case SPHINX_STATE_IDLE:
			creature->Flags = 0;

			if (item->Animation.RequiredState == SPHINX_STATE_WALK_BACK)
				item->Animation.TargetState = SPHINX_STATE_WALK_BACK;
			else
				item->Animation.TargetState = SPHINX_STATE_WALK;

			break;

		default:
			break;
		}

		item->ItemFlags[2] = (short)item->Pose.Position.x;
		item->ItemFlags[3] = (short)item->Pose.Position.z;

		CreatureAnimation(itemNumber, angle, 0);
	}
}
