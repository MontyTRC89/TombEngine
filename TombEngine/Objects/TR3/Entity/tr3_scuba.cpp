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

namespace TEN::Entities::TR3
{
	BITE_INFO ScubaGunBite = { 17, 164, 44, 18 };

	// TODO
	enum ScubaDiverState
	{

	};

	// TODO
	enum ScubaDiverAnim
	{

	};

	static void ShootHarpoon(ItemInfo* item, int x, int y, int z, short velocity, short yRot, short roomNumber)
	{
		short harpoonItemNumber = CreateItem();
		if (harpoonItemNumber != NO_ITEM)
		{
			auto* harpoonItem = &g_Level.Items[harpoonItemNumber];

			harpoonItem->ObjectNumber = ID_SCUBA_HARPOON;
			harpoonItem->RoomNumber = item->RoomNumber;

			harpoonItem->Pose.Position.x = x;
			harpoonItem->Pose.Position.y = y;
			harpoonItem->Pose.Position.z = z;

			InitialiseItem(harpoonItemNumber);

			harpoonItem->Pose.Orientation.x = 0;
			harpoonItem->Pose.Orientation.y = yRot;
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
			DoBloodSplat(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, (GetRandomControl() & 3) + 4, LaraItem->Pose.Orientation.y, LaraItem->RoomNumber);
			DoDamage(LaraItem, 50);
			KillItem(itemNumber);
		}
		else
		{
			int ox = item->Pose.Position.x;
			int oz = item->Pose.Position.z;

			int velocity = item->Animation.Velocity * phd_cos(item->Pose.Orientation.x);
			item->Pose.Position.z += velocity * phd_cos(item->Pose.Orientation.y);
			item->Pose.Position.x += velocity * phd_sin(item->Pose.Orientation.y);
			item->Pose.Position.y += -item->Animation.Velocity * phd_sin(item->Pose.Orientation.x);

			auto probe = GetCollision(item);

			if (item->RoomNumber != probe.RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			item->Floor = GetCollision(item).Position.Floor;
			if (item->Pose.Position.y >= item->Floor)
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

			GameVector start;
			GameVector target;
			bool shoot = false;

			if (Lara.Control.WaterStatus == WaterStatus::Dry)
			{
				start.x = item->Pose.Position.x;
				start.y = item->Pose.Position.y - CLICK(1);
				start.z = item->Pose.Position.z;
				start.roomNumber = item->RoomNumber;

				target.x = LaraItem->Pose.Position.x;
				target.y = LaraItem->Pose.Position.y - (LARA_HEIGHT - 150);
				target.z = LaraItem->Pose.Position.z;

				shoot = LOS(&start, &target);
				if (shoot)
				{
					creature->Target.x = LaraItem->Pose.Position.x;
					creature->Target.y = LaraItem->Pose.Position.y;
					creature->Target.z = LaraItem->Pose.Position.z;
				}

				if (AI.angle < -ANGLE(45.0f) || AI.angle > ANGLE(45.0f))
					shoot = false;
			}
			else if (AI.angle > -ANGLE(45.0f) && AI.angle < ANGLE(45.0f))
			{
				start.x = item->Pose.Position.x;
				start.y = item->Pose.Position.y;
				start.z = item->Pose.Position.z;
				start.roomNumber = item->RoomNumber;

				target.x = LaraItem->Pose.Position.x;
				target.y = LaraItem->Pose.Position.y;
				target.z = LaraItem->Pose.Position.z;

				shoot = LOS(&start, &target);
			}
			else
				shoot = false;

			angle = CreatureTurn(item, creature->MaxTurn);
			waterHeight = GetWaterSurface(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber) + WALL_SIZE / 2;

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->MaxTurn = ANGLE(3.0f);
				if (shoot)
					neck = -AI.angle;

				if (creature->Target.y < waterHeight && item->Pose.Position.y < waterHeight + creature->LOT.Fly)
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
					(creature->Target.y < waterHeight && item->Pose.Position.y < waterHeight + creature->LOT.Fly))
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
					ShootHarpoon(item, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->Animation.Velocity, item->Pose.Orientation.y, item->RoomNumber);
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
					ShootHarpoon(item, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->Animation.Velocity, item->Pose.Orientation.y, item->RoomNumber);
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
			item->Pose.Position.y = waterHeight - CLICK(2);
		}
	}
}
