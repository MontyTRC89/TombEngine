#include "framework.h"
#include "Objects/TR3/Entity/tr3_scuba.h"

#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO ScubaGunBite = { 17, 164, 44, 18 };


// TODO
enum ScubaDiverState
{

};

// TODO
enum ScubaDiverAnim
{

};

static void ShootHarpoon(ITEM_INFO* item, int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short harpoonItemNumber = CreateItem();
	if (harpoonItemNumber != NO_ITEM)
	{
		auto* harpoonItem = &g_Level.Items[harpoonItemNumber];

		harpoonItem->ObjectNumber = ID_SCUBA_HARPOON;
		harpoonItem->RoomNumber = item->RoomNumber;

		harpoonItem->Position.xPos = x;
		harpoonItem->Position.yPos = y;
		harpoonItem->Position.zPos = z;

		InitialiseItem(harpoonItemNumber);

		harpoonItem->Position.xRot = 0;
		harpoonItem->Position.yRot = yRot;
		harpoonItem->Animation.Velocity = 150;

		AddActiveItem(harpoonItemNumber);
		harpoonItem->Status = ITEM_ACTIVE;
	}
}

void ScubaHarpoonControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->TouchBits)
	{
		DoBloodSplat(item->Position.xPos, item->Position.yPos, item->Position.zPos, (GetRandomControl() & 3) + 4, LaraItem->Position.yRot, LaraItem->RoomNumber);
		KillItem(itemNumber);

		LaraItem->HitPoints -= 50;
		LaraItem->HitStatus = true;
	}
	else
	{
		int ox = item->Position.xPos;
		int oz = item->Position.zPos;

		int velocity = item->Animation.Velocity * phd_cos(item->Position.xRot);
		item->Position.zPos += velocity * phd_cos(item->Position.yRot);
		item->Position.xPos += velocity * phd_sin(item->Position.yRot);
		item->Position.yPos += -item->Animation.Velocity * phd_sin(item->Position.xRot);

		auto probe = GetCollision(item);

		if (item->RoomNumber != probe.RoomNumber)
			ItemNewRoom(itemNumber, probe.RoomNumber);

		item->Floor = GetCollision(item).Position.Floor;
		if (item->Position.yPos >= item->Floor)
			KillItem(itemNumber);
	}
}

void ScubaControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;
	short neck = 0;

	int waterHeight;
	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 9)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 16;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 9;
		}

		CreatureFloat(itemNumber);
		return;
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, TIMID);
		CreatureMood(item, &AI, TIMID);

		GAME_VECTOR start;
		GAME_VECTOR target;
		bool shoot = false;

		if (Lara.Control.WaterStatus == WaterStatus::Dry)
		{
			start.x = item->Position.xPos;
			start.y = item->Position.yPos - CLICK(1);
			start.z = item->Position.zPos;
			start.roomNumber = item->RoomNumber;

			target.x = LaraItem->Position.xPos;
			target.y = LaraItem->Position.yPos - (LARA_HEIGHT - 150);
			target.z = LaraItem->Position.zPos;

			shoot = LOS(&start, &target);
			if (shoot)
			{
				creature->Target.x = LaraItem->Position.xPos;
				creature->Target.y = LaraItem->Position.yPos;
				creature->Target.z = LaraItem->Position.zPos;
			}

			if (AI.angle < -ANGLE(45.0f) || AI.angle > ANGLE(45.0f))
				shoot = false;
		}
		else if (AI.angle > -ANGLE(45.0f) && AI.angle < ANGLE(45.0f))
		{
			start.x = item->Position.xPos;
			start.y = item->Position.yPos;
			start.z = item->Position.zPos;
			start.roomNumber = item->RoomNumber;

			target.x = LaraItem->Position.xPos;
			target.y = LaraItem->Position.yPos;
			target.z = LaraItem->Position.zPos;

			shoot = LOS(&start, &target);
		}
		else
			shoot = false;

		angle = CreatureTurn(item, creature->MaxTurn);
		waterHeight = GetWaterSurface(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber) + WALL_SIZE / 2;

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->MaxTurn = ANGLE(3.0f);
			if (shoot)
				neck = -AI.angle;

			if (creature->Target.y < waterHeight && item->Position.yPos < waterHeight + creature->LOT.Fly)
				item->Animation.TargetState = 2;
			else if (creature->Mood == MoodType::Escape)
				break;
			else if (shoot)
				item->Animation.TargetState = 4;

			break;

		case 4:
			creature->Flags = 0;

			if (shoot)
				neck = -AI.angle;

			if (!shoot || creature->Mood == MoodType::Escape ||
				(creature->Target.y < waterHeight && item->Position.yPos < waterHeight + creature->LOT.Fly))
			{
				item->Animation.TargetState = 1;
			}
			else
				item->Animation.TargetState = 3;

			break;

		case 3:
			if (shoot)
				neck = -AI.angle;

			if (!creature->Flags)
			{
				ShootHarpoon(item, item->Position.xPos, item->Position.yPos, item->Position.zPos, item->Animation.Velocity, item->Position.yRot, item->RoomNumber);
				creature->Flags = 1;
			}

			break;


		case 2:
			creature->MaxTurn = ANGLE(3.0f);

			if (shoot)
				head = AI.angle;

			if (creature->Target.y > waterHeight)
				item->Animation.TargetState = 1;
			else if (creature->Mood == MoodType::Escape)
				break;
			else if (shoot)
				item->Animation.TargetState = 6;

			break;

		case 6:
			creature->Flags = 0;

			if (shoot)
				head = AI.angle;

			if (!shoot || creature->Mood == MoodType::Escape || creature->Target.y > waterHeight)
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 7;

			break;

		case 7:
			if (shoot)
				head = AI.angle;

			if (!creature->Flags)
			{
				ShootHarpoon(item, item->Position.xPos, item->Position.yPos, item->Position.zPos, item->Animation.Velocity, item->Position.yRot, item->RoomNumber);
				creature->Flags = 1;
			}

			break;
			
		}
	}

	CreatureJoint(item, 0, head);
	CreatureJoint(item, 1, neck);
	CreatureAnimation(itemNumber, angle, 0);

	switch (item->Animation.ActiveState)
	{
	case 1:
	case 4:
	case 3:
		CreatureUnderwater(item, CLICK(2));
		break;

	default:
		item->Position.yPos = waterHeight - CLICK(2);
	}
}
