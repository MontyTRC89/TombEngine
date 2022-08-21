#include "framework.h"
#include "Objects/TR5/Entity/tr5_submarine.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::TR5
{
	void TriggerSubmarineSparks(short itemNumber)
	{
		auto* spark = GetFreeParticle();

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
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
		spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + 11;
		spark->dSize = spark->sSize = spark->size = (GetRandomControl() & 7) + 192;
	}

	void TriggerTorpedoBubbles(Vector3Int* pos1, Vector3Int* pos2, char factor)
	{
		auto* spark = GetFreeParticle();

		spark->on = 1;
		spark->sR = 32;
		spark->sG = 32;
		spark->sB = 32;
		spark->dR = 80;
		spark->dG = 80;
		spark->dB = 80;
		spark->colFadeSpeed = 2;
		spark->fadeToBlack = 8;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 16;
		spark->x = pos1->x + (GetRandomControl() & 0x1F);
		spark->y = (GetRandomControl() & 0x1F) + pos1->y - 16;
		spark->z = (GetRandomControl() & 0x1F) + pos1->z - 16;
		spark->xVel = pos2->x + (GetRandomControl() & 0x7F) - pos1->x - 64;
		spark->yVel = pos2->y + (GetRandomControl() & 0x7F) - pos1->y - 64;
		spark->zVel = pos2->z + (GetRandomControl() & 0x7F) - pos1->z - 64;
		spark->friction = 0;
		spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + 17;
		spark->maxYvel = 0;
		spark->gravity = -4 - (GetRandomControl() & 3);
		spark->scalar = 1;
		spark->flags = SP_ROTATE | SP_DEF | SP_SCALE;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
		spark->sSize = spark->size = (GetRandomControl() & 0xF) + 32 >> factor;
		spark->dSize = spark->size * 2;
	}

	void TriggerTorpedoSparks2(Vector3Int* pos1, Vector3Int* pos2, char scale)
	{
		auto* spark = GetFreeParticle();

		spark->on = 1;
		spark->sR = 32;
		spark->sG = 32;
		spark->sB = 32;
		spark->dR = -128;
		spark->dG = -128;
		spark->dB = -128;
		spark->colFadeSpeed = 2;
		spark->fadeToBlack = 8;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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

	void SubmarineAttack(ItemInfo* item)
	{
		short itemNumber = CreateItem();
		if (itemNumber == NO_ITEM)
			return;

		auto* torpedoItem = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR5_UNDERWATER_TORPEDO, &torpedoItem->Pose, SoundEnvironment::Always);

		torpedoItem->ObjectNumber = ID_TORPEDO;
		torpedoItem->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

		Vector3Int pos1;
		Vector3Int pos2;

		for (int i = 0; i < 8; i++)
		{
			auto pos1 = Vector3Int(
				(GetRandomControl() & 0x7F) - 414,
				-320,
				352
			);
			GetJointAbsPosition(item, &pos1, 4);

			auto pos2 = Vector3Int(
				(GetRandomControl() & 0x3FF) - 862,
				-320 - (GetRandomControl() & 0x3FF),
				(GetRandomControl() & 0x3FF) - 160
			);
			GetJointAbsPosition(item, &pos2, 4);

			TriggerTorpedoSparks2(&pos1, &pos2, 0);
		}

		torpedoItem->RoomNumber = item->RoomNumber;
		GetFloor(pos1.x, pos1.y, pos1.z, &torpedoItem->RoomNumber);

		torpedoItem->Pose.Position = pos1;

		InitialiseItem(itemNumber);

		torpedoItem->Animation.Velocity.y = 0.0f;
		torpedoItem->Animation.Velocity.z = 0.0f;
		torpedoItem->Pose.Orientation.x = 0;
		torpedoItem->Pose.Orientation.y = item->Pose.Orientation.y;
		torpedoItem->Pose.Orientation.z = 0;
		torpedoItem->ItemFlags[0] = -1;

		AddActiveItem(itemNumber);
	}

	void InitialiseSubmarine(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		SetAnimation(item, 0);

		if (!item->TriggerFlags)
			item->TriggerFlags = 120;
	}

	void SubmarineControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO AI, laraAI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, true);
		CreatureMood(item, &AI, true);

		short angle = CreatureTurn(item, creature->MaxTurn);

		if (creature->Enemy == LaraItem)
		{
			laraAI.angle = AI.angle;
			laraAI.distance = AI.distance;
		}
		else
		{
			int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

			laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
			laraAI.distance = pow(dx, 2) + pow(dz, 2);
			laraAI.ahead = true;
		}

		int tilt = item->ItemFlags[0] + (angle / 2);

		if (tilt > ANGLE(11.25f))
			tilt = ANGLE(11.25f);
		else if (tilt < ANGLE(-11.25f))
			tilt = ANGLE(-11.25f);

		item->ItemFlags[0] = tilt;

		if (abs(tilt) >= ANGLE(0.35f))
		{
			if (tilt > 0)
				item->ItemFlags[0] -= ANGLE(0.35f);
			else
				item->ItemFlags[0] += ANGLE(0.35f);
		}
		else
			item->ItemFlags[0] = 0;

		creature->MaxTurn = ANGLE(2.0f);

		short joint = AI.xAngle - ANGLE(45.0f);

		if (creature->Flags < item->TriggerFlags)
			creature->Flags++;

		auto* enemy = creature->Enemy;
		creature->Enemy = LaraItem;

		if (Targetable(item, &laraAI))
		{
			if (creature->Flags >= item->TriggerFlags &&
				laraAI.angle > -ANGLE(90.0f) &&
				laraAI.angle < ANGLE(90.0f))
			{
				SubmarineAttack(item);
				creature->Flags = 0;
			}

			if (laraAI.distance >= pow(SECTOR(3), 2))
			{
				item->Animation.TargetState = 1;
				SoundEffect(SFX_TR5_VEHICLE_DIVESUIT_LOOP, &item->Pose, SoundEnvironment::Always);
			}
			else
				item->Animation.TargetState = 0;

			if (AI.distance < pow(SECTOR(1), 2))
			{
				creature->MaxTurn = 0;
				if (abs(laraAI.angle) >= ANGLE(2.0f))
				{
					if (laraAI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(2.0f);
					else
						item->Pose.Orientation.y -= ANGLE(2.0f);
				}
				else
					item->Pose.Orientation.y += laraAI.angle;
			}
		}
		else
			item->Animation.TargetState = 1;

		creature->Enemy = enemy;

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint);
		CreatureJoint(item, 1, joint);

		if (GlobalCounter & 1)
		{
			auto pos1 = Vector3Int(200, 320, 90);
			GetJointAbsPosition(item, &pos1, 1);

			auto pos2 = Vector3Int(200, 1280, 90);
			GetJointAbsPosition(item, &pos2, 1);

			TriggerTorpedoBubbles(&pos1, &pos2, 0);

			pos1 = Vector3Int(200, 320, -100);
			GetJointAbsPosition(item, &pos1, 1);

			pos2 = Vector3Int(200, 1280, -100);
			GetJointAbsPosition(item, &pos2, 1);

			TriggerTorpedoBubbles(&pos1, &pos2, 0);
		}
		else
		{
			auto pos1 = Vector3Int(-200, 320, 90);
			GetJointAbsPosition(item, &pos1, 2);

			auto pos2 = Vector3Int(-200, 1280, 90);
			GetJointAbsPosition(item, &pos2, 2);

			TriggerTorpedoBubbles(&pos1, &pos2, 0);

			pos1 = Vector3Int(-200, 320, -100);
			GetJointAbsPosition(item, &pos1, 2);

			pos2 = Vector3Int(-200, 1280, -100);
			GetJointAbsPosition(item, &pos2, 2);

			TriggerTorpedoBubbles(&pos1, &pos2, 0);
		}

		TriggerSubmarineSparks(itemNumber);

		GameVector pos1;
		pos1.x = 0;
		pos1.y = -600;
		pos1.z = -40;
		pos1.roomNumber = item->RoomNumber;
		GetJointAbsPosition(item, (Vector3Int*)&pos1, 0);

		GameVector pos2;
		pos2.x = 0;
		pos2.y = -15784;
		pos2.z = -40;
		GetJointAbsPosition(item, (Vector3Int*)&pos2, 0);

		if (!LOS((GameVector*)&pos1, &pos2))
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
				creature->Enemy = nullptr;
			}
		}

		CreatureAnimation(itemNumber, angle, tilt);
		CreatureUnderwater(item, -14080);
	}

	void ChaffFlareControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Animation.Velocity.y)
		{
			item->Pose.Orientation.x += ANGLE(3.0f);
			item->Pose.Orientation.z += ANGLE(5.0f);
		}

		int dx = item->Animation.Velocity.z * phd_sin(item->Pose.Orientation.y);
		int dz = item->Animation.Velocity.z * phd_cos(item->Pose.Orientation.y);

		item->Pose.Position.x += dx;
		item->Pose.Position.z += dz;

		if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
		{
			item->Animation.Velocity.y += (5.0f - item->Animation.Velocity.y) / 2.0f;
			item->Animation.Velocity.z += (5.0f - item->Animation.Velocity.z) / 2.0f;
		}
		else
			item->Animation.Velocity.y += GRAVITY;

		item->Pose.Position.y += item->Animation.Velocity.y;

		DoProjectileDynamics(itemNumber, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, dx, item->Animation.Velocity.y, dz);

		Vector3Int pos1;
		pos1.x = 0;
		pos1.y = 0;
		pos1.z = (GlobalCounter & 1) != 0 ? 48 : -48;
		GetJointAbsPosition(item, &pos1, 0);

		Vector3Int pos2;
		pos2.x = 0;
		pos2.y = 0;
		pos2.z = 8 * ((GlobalCounter & 1) != 0 ? 48 : -48);
		GetJointAbsPosition(item, &pos2, 0);

		TriggerTorpedoBubbles(&pos1, &pos2, 1);

		if (item->ItemFlags[0] >= 300)
		{
			if (!item->Animation.Velocity.y && !item->Animation.Velocity.z)
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
		auto* item = &g_Level.Items[itemNumber];

		SoundEffect(SFX_TR5_VEHICLE_DIVESUIT_HIT, &item->Pose, SoundEnvironment::Always);

		Vector3Int pos;

		if (item->ItemFlags[0] == NO_ITEM)
		{
			bool found = false;
			for (int i = g_Level.NumItems; i < 256; i++)
			{
				auto* searchItem = &g_Level.Items[i];

				if (searchItem->ObjectNumber == ID_CHAFF && searchItem->Active)
				{
					item->ItemFlags[0] = i;
					pos = searchItem->Pose.Position;
					found = true;
					break;
				}
			}

			if (!found)
				pos = LaraItem->Pose.Position;
		}
		else
		{
			auto* chaffItem = &g_Level.Items[item->ItemFlags[0]];

			if (chaffItem->Active && chaffItem->ObjectNumber == ID_CHAFF)
			{
				pos = chaffItem->Pose.Position;
				item->Animation.ActiveState = pos.x / 4;
				item->Animation.TargetState = pos.y / 4;
				item->Animation.RequiredState = pos.z / 4;
			}
			else
			{
				pos.x = 4 * item->Animation.ActiveState;
				pos.y = 4 * item->Animation.TargetState;
				pos.z = 4 * item->Animation.RequiredState;
			}
		}

		auto angles = GetVectorAngles(pos.x - item->Pose.Position.x, pos.y - item->Pose.Position.y, pos.z - item->Pose.Position.z);

		if (item->Animation.Velocity.z >= 48.0f)
		{
			if (item->Animation.Velocity.z < 192.0f)
				item->Animation.Velocity.z++;
		}
		else
			item->Animation.Velocity.z += 4.0f;

		item->ItemFlags[1]++;

		if (item->ItemFlags[1] - 1 < 60)
		{
			short dry = angles.y - item->Pose.Orientation.y;
			if (abs(dry) > ANGLE(180.0f))
				dry = -dry;

			short drx = angles.x - item->Pose.Orientation.x;
			if (abs(drx) > ANGLE(180.0f))
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

			item->Pose.Orientation.y += dry;
			item->Pose.Orientation.x += drx;
		}

		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;

		item->Pose.Orientation.z += 16 * item->Animation.Velocity.z;

		int c = item->Animation.Velocity.z * phd_cos(item->Pose.Orientation.x);

		item->Pose.Position.x += c * phd_sin(item->Pose.Orientation.y);
		item->Pose.Position.y += item->Animation.Velocity.z * phd_sin(-item->Pose.Orientation.x);
		item->Pose.Position.z += c * phd_cos(item->Pose.Orientation.y);

		auto probe = GetCollision(item);

		if (item->Pose.Position.y < probe.Position.Floor &&
			item->Pose.Position.y > probe.Position.Ceiling &&
			TestEnvironment(ENV_FLAG_WATER, probe.RoomNumber))
		{
			if (ItemNearLara(&item->Pose, 200))
			{
				LaraItem->HitStatus = true;
				KillItem(itemNumber);
				TriggerUnderwaterExplosion(item, 1);
				SoundEffect(SFX_TR5_UNDERWATER_EXPLOSION, &item->Pose, SoundEnvironment::Always);
				SoundEffect(SFX_TR5_VEHICLE_DIVESUIT_HIT, &LaraItem->Pose, SoundEnvironment::Always);
				DoDamage(LaraItem, 200);
			}
			else
			{
				//	if (ItemNearLara(&item->pos, 400) && Lara.anxiety < 0xE0)
				//		Lara.anxiety += 32;

				if (probe.RoomNumber != item->RoomNumber)
					ItemNewRoom(itemNumber, probe.RoomNumber);

				Vector3Int pos1 = { 0, 0, -64 };
				GetJointAbsPosition(item, &pos1, 0);

				Vector3Int pos2;
				pos2.x = 0;
				pos2.y = 0;
				pos2.z = -64 << ((GlobalCounter & 1) + 2);
				GetJointAbsPosition(item, &pos2, 0);

				TriggerTorpedoBubbles(&pos1, &pos2, 1);
			}
		}
		else
		{
			item->Pose.Position.x = x;
			item->Pose.Position.y = y;
			item->Pose.Position.z = z;
			TriggerUnderwaterExplosion(item, 1);
			SoundEffect(SFX_TR5_UNDERWATER_EXPLOSION, &item->Pose, SoundEnvironment::Always);
			KillItem(itemNumber);
		}
	}
}
