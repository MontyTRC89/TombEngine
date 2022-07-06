#include "framework.h"
#include "Objects/TR3/Entity/tr3_tony.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/lot.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/lara_fx.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Objects/TR3/boss.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11Enums.h"

using namespace TEN::Effects::Lara;

namespace TEN::Entities::TR3
{
	static BOSS_STRUCT BossData;

	constexpr auto TONY_TRIGGER_RANGE = SECTOR(16);

	#define TONY_HITS 100
	#define TONY_TURN ANGLE(2.0f)

	enum TonyFlameType
	{
		T_NOFLAME = 0,
		T_ROCKZAPPL = 0,
		T_ROCKZAPPR,
		T_ZAPP,
		T_DROPPER,
		T_ROCKZAPPDEBRIS,
		T_ZAPPDEBRIS,
		T_DROPPERDEBRIS
	};

	struct TonyFlame
	{
		bool on;
		Vector3Int pos;
		int fallspeed;
		int speed;
		short yRot;
		short room_number;
		TonyFlameType type;
	};

	enum TonyState
	{
		TONY_STATE_WAIT,
		TONY_STATE_RISE,
		TONY_STATE_FLOAT,
		TONY_STATE_ZAPP,
		TONY_STATE_ROCKZAPP,
		TONY_STATE_BIGBOOM,
		TONY_STATE_DEATH
	};

	// TODO
	enum TonyAnim
	{

	};

	void InitialiseTony(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->ItemFlags[3] = 0;
		BossData.ExplodeCount = 0;
		BossData.RingCount = 0;
		BossData.DroppedIcon = false;
		BossData.DrawExplode = false;
		BossData.Dead = false;
	}

	static void TriggerTonyEffect(const TonyFlame flame)
	{
		short fxNumber = CreateNewEffect(flame.room_number);
		if (fxNumber != -1)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.Position.x = flame.pos.x;
			fx->pos.Position.y = flame.pos.y;
			fx->pos.Position.z = flame.pos.z;
			fx->fallspeed = flame.fallspeed;
			fx->pos.Orientation.x = 0;
			fx->pos.Orientation.y = flame.yRot;
			fx->pos.Orientation.z = 0;
			fx->objectNumber = ID_TONY_BOSS_FLAME;
			fx->speed = flame.speed;
			fx->shade = 0;
			fx->flag1 = flame.type;
			fx->flag2 = (GetRandomControl() & 3) + 1;

			switch (flame.type)
			{
			case T_ZAPPDEBRIS:
				fx->flag2 *= 2;
				break;

			case T_ZAPP:
				fx->flag2 = 0;
				break;
			}
		}
	}

	static void TriggerTonyFlame(short itemNumber, int hand)
	{
		auto* item = &g_Level.Items[itemNumber];

		int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
		int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
		if (dx < -TONY_TRIGGER_RANGE || dx > TONY_TRIGGER_RANGE ||
			dz < -TONY_TRIGGER_RANGE || dz > TONY_TRIGGER_RANGE)
		{
			return;
		}

		auto* sptr = GetFreeParticle();

		sptr->on = true;
		sptr->sR = 255;
		sptr->sG = 48 + (GetRandomControl() & 31);
		sptr->sB = 48;
		sptr->dR = 192 + (GetRandomControl() & 63);
		sptr->dG = 128 + (GetRandomControl() & 63);
		sptr->dB = 32;
		sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 8;
		sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;
		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->extras = NULL;
		sptr->dynamic = -1;
		sptr->x = ((GetRandomControl() & 15) - 8);
		sptr->y = 0;
		sptr->z = ((GetRandomControl() & 15) - 8);
		sptr->xVel = ((GetRandomControl() & 255) - 128);
		sptr->yVel = -(GetRandomControl() & 15) - 16;
		sptr->zVel = ((GetRandomControl() & 255) - 128);
		sptr->friction = 5;

		if (GetRandomControl() & 1)
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
			sptr->rotAng = GetRandomControl() & 4095;
			if (GetRandomControl() & 1)
				sptr->rotAdd = -(GetRandomControl() & 15) - 16;
			else
				sptr->rotAdd = (GetRandomControl() & 15) + 16;
		}
		else
			sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;

		sptr->gravity = -(GetRandomControl() & 31) - 16;
		sptr->maxYvel = -(GetRandomControl() & 7) - 16;
		sptr->fxObj = itemNumber;
		sptr->nodeNumber = hand;
		sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		sptr->scalar = 1;
		unsigned char size = (GetRandomControl() & 31) + 32;
		sptr->size = size;
		sptr->sSize = size;
		sptr->dSize = size / 4;
	}

	static void TriggerFireBallFlame(short fxNumber, long type, long xv, long yv, long zv)
	{
		int dx = LaraItem->Pose.Position.x - EffectList[fxNumber].pos.Position.x;
		int dz = LaraItem->Pose.Position.z - EffectList[fxNumber].pos.Position.z;
		if (dx < -TONY_TRIGGER_RANGE || dx > TONY_TRIGGER_RANGE ||
			dz < -TONY_TRIGGER_RANGE || dz > TONY_TRIGGER_RANGE)
		{
			return;
		}

		auto* sptr = GetFreeParticle();

		sptr->on = true;
		sptr->sR = 255;
		sptr->sG = 48 + (GetRandomControl() & 31);
		sptr->sB = 48;
		sptr->dR = 192 + (GetRandomControl() & 63);
		sptr->dG = 128 + (GetRandomControl() & 63);
		sptr->dB = 32;
		sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 8;
		sptr->sLife = sptr->life = (GetRandomControl() & 7) + 24;
		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->extras = 0;
		sptr->dynamic = -1;
		sptr->x = ((GetRandomControl() & 15) - 8);
		sptr->y = 0;
		sptr->z = ((GetRandomControl() & 15) - 8);
		sptr->xVel = xv + ((GetRandomControl() & 255) - 128);
		sptr->yVel = yv;
		sptr->zVel = zv + ((GetRandomControl() & 255) - 128);
		sptr->friction = 5;

		if (GetRandomControl() & 1)
		{
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
			sptr->rotAng = GetRandomControl() & 4095;
			if (GetRandomControl() & 1)
				sptr->rotAdd = -(GetRandomControl() & 15) - 16;
			else
				sptr->rotAdd = (GetRandomControl() & 15) + 16;
		}
		else
			sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;

		sptr->fxObj = (unsigned char)fxNumber;
		sptr->spriteIndex = (unsigned char)Objects[ID_DEFAULT_SPRITES].meshIndex;
		sptr->scalar = 1;
		unsigned char size = (GetRandomControl() & 31) + 64;
		sptr->size = size;
		sptr->sSize = size;
		sptr->dSize = size / 4;

		if (type == T_ROCKZAPPL || type == T_ROCKZAPPR)
		{
			sptr->gravity = (GetRandomControl() & 31) + 16;
			sptr->maxYvel = (GetRandomControl() & 15) + 48;
			sptr->yVel = -sptr->yVel * 16;
			sptr->scalar = 2;
		}
		else if (type == T_ROCKZAPPDEBRIS || type == T_ZAPPDEBRIS || type == T_DROPPERDEBRIS)
		{
			sptr->gravity = 0;
			sptr->maxYvel = 0;
		}
		else if (type == T_DROPPER)
		{
			sptr->gravity = -(GetRandomControl() & 31) - 16;
			sptr->maxYvel = -(GetRandomControl() & 31) - 64;
			sptr->yVel = sptr->yVel * 16;
			sptr->scalar = 2;
		}
		else if (type == T_ZAPP)
		{
			sptr->gravity = sptr->maxYvel = 0;
			sptr->scalar = 2;
		}
	}

	static void TriggerFireBall(ItemInfo* item, TonyFlameType type, Vector3Int* laraPos, short roomNumber, short angle, int zdVelocity)
	{
		TonyFlame flame;
		memset(&flame, 0, sizeof(TonyFlame));

		switch (type)
		{
		case T_ROCKZAPPL:
			flame.on = true;

			flame.pos.x = 0;
			flame.pos.y = 0;
			flame.pos.z = 0;
			GetJointAbsPosition(item, &flame.pos, 10);

			flame.fallspeed = -16;
			flame.speed = 0;
			flame.yRot = item->Pose.Orientation.y;
			flame.room_number = roomNumber;
			flame.type = T_ROCKZAPPL;
			break;

		case T_ROCKZAPPR:
			flame.on = true;

			flame.pos.x = 0;
			flame.pos.y = 0;
			flame.pos.z = 0;
			GetJointAbsPosition(item, &flame.pos, 13);

			flame.fallspeed = -16;
			flame.speed = 0;
			flame.yRot = item->Pose.Orientation.y;
			flame.room_number = roomNumber;
			flame.type = T_ROCKZAPPR;
			break;

		case T_ZAPP:
			flame.on = true;

			flame.pos.x = 0;
			flame.pos.y = 0;
			flame.pos.z = 0;
			GetJointAbsPosition(item, &flame.pos, 13);

			flame.fallspeed = (GetRandomControl() & 7) + 10;
			flame.speed = 160;
			flame.yRot = item->Pose.Orientation.y;
			flame.room_number = roomNumber;
			flame.type = T_ZAPP;
			break;

		case T_DROPPER:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y + 64;
			flame.pos.z = laraPos->z;
			flame.fallspeed = (GetRandomControl() & 3) + 4;
			flame.speed = 0;
			flame.yRot = angle;
			flame.room_number = roomNumber;
			flame.type = T_DROPPER;
			break;

		case T_ROCKZAPPDEBRIS:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y;
			flame.pos.z = laraPos->z;
			flame.fallspeed = (GetRandomControl() & 3) - 2;
			flame.speed = zdVelocity + (GetRandomControl() & 3);
			flame.yRot = GetRandomControl() * 2;
			flame.room_number = roomNumber;
			flame.type = T_ROCKZAPPDEBRIS;
			break;

		case T_ZAPPDEBRIS:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y;
			flame.pos.z = laraPos->z;
			flame.fallspeed = -(GetRandomControl() & 15) - 16;
			flame.speed = (GetRandomControl() & 7) + 48;
			angle += (GetRandomControl() & 0x1fff) - 0x9000;
			flame.yRot = angle;
			flame.room_number = roomNumber;
			flame.type = T_ZAPPDEBRIS;
			break;

		case T_DROPPERDEBRIS:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y;
			flame.pos.z = laraPos->z;
			flame.fallspeed = -(GetRandomControl() & 31) - 32;
			flame.speed = (GetRandomControl() & 31) + 32;
			flame.yRot = GetRandomControl() * 2;
			flame.room_number = roomNumber;
			flame.type = T_DROPPERDEBRIS;
			break;
		}

		if (flame.on)
			TriggerTonyEffect(flame);
	}

	void ControlTonyFireBall(short fxNumber)
	{
		auto* fx = &EffectList[fxNumber];
		long oldX = fx->pos.Position.x;
		long oldY = fx->pos.Position.y;
		long oldZ = fx->pos.Position.z;

		if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR)
		{
			fx->fallspeed += (fx->fallspeed / 8) + 1;
			if (fx->fallspeed < -SECTOR(4))
				fx->fallspeed = -SECTOR(4);

			fx->pos.Position.y += fx->fallspeed;

			if (Wibble & 4)
				TriggerFireBallFlame(fxNumber, (TonyFlameType)fx->flag1, 0, 0, 0);
		}
		else if (fx->flag1 == T_DROPPER)
		{
			fx->fallspeed += 2;
			fx->pos.Position.y += fx->fallspeed;

			if (Wibble & 4)
				TriggerFireBallFlame(fxNumber, (TonyFlameType)fx->flag1, 0, 0, 0);
		}
		else
		{
			if (fx->flag1 != T_ZAPP)
			{
				if (fx->speed > 48)
					fx->speed--;
			}

			fx->fallspeed += fx->flag2;
			if (fx->fallspeed > CLICK(2))
				fx->fallspeed = CLICK(2);

			fx->pos.Position.y += fx->fallspeed / 2;
			fx->pos.Position.z += fx->speed * phd_cos(fx->pos.Orientation.y);
			fx->pos.Position.x += fx->speed * phd_sin(fx->pos.Orientation.y);

			if (Wibble & 4)
				TriggerFireBallFlame(fxNumber, (TonyFlameType)fx->flag1, (short)((oldX - fx->pos.Position.x) * 8), (short)((oldY - fx->pos.Position.y) * 8), (short)((oldZ - fx->pos.Position.z) * 4));
		}

		auto probe = GetCollision(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, fx->roomNumber);

		if (fx->pos.Position.y >= probe.Position.Floor ||
			fx->pos.Position.y < probe.Position.Ceiling)
		{
			if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR || fx->flag1 == T_ZAPP || fx->flag1 == T_DROPPER)
			{
				Vector3Int pos;

				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, fx->roomNumber);
				if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR)
				{
					for (int x = 0; x < 2; x++)
						TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, fx->roomNumber);
				}

				pos.x = oldX;
				pos.y = oldY;
				pos.z = oldZ;

				int j;
				TonyFlameType type;
				if (fx->flag1 == T_ZAPP)
					j = 7;
				else
					j = 3;

				if (fx->flag1 == T_ZAPP)
					type = T_ZAPPDEBRIS;
				else if (fx->flag1 == T_DROPPER)
					type = T_DROPPERDEBRIS;
				else
					type = T_ROCKZAPPDEBRIS;

				for (int x = 0; x < j; x++)
					TriggerFireBall(NULL, type, &pos, fx->roomNumber, fx->pos.Orientation.y, 32 + (x * 4));

				if (fx->flag1 == T_ROCKZAPPL || fx->flag1 == T_ROCKZAPPR)
				{
					probe = GetCollision(LaraItem);

					pos.y = probe.Position.Ceiling + CLICK(1);
					pos.x = LaraItem->Pose.Position.x + (GetRandomControl() & 1023) - CLICK(2);
					pos.z = LaraItem->Pose.Position.z + (GetRandomControl() & 1023) - CLICK(2);

					TriggerExplosionSparks(pos.x, pos.y, pos.z, 3, -2, 0, probe.RoomNumber);
					TriggerFireBall(NULL, T_DROPPER, &pos, probe.RoomNumber, 0, 0);
				}
			}

			KillEffect(fxNumber);
			return;
		}


		if (TestEnvironment(ENV_FLAG_WATER, probe.RoomNumber))
		{
			KillEffect(fxNumber);
			return;
		}

		if (!Lara.Burn)
		{
			if (ItemNearLara(&fx->pos, 200))
			{
				LaraItem->HitStatus = true;
				KillEffect(fxNumber);
				DoDamage(LaraItem, 200);
				LaraBurn(LaraItem);
				return;
			}
		}

		if (probe.RoomNumber != fx->roomNumber)
			EffectNewRoom(fxNumber, LaraItem->RoomNumber);

		unsigned char radtab[7] = { 16, 0, 14, 9, 7, 7, 7 };
		if (radtab[fx->flag1])
		{
			int random = GetRandomControl();
			BYTE r3 = 31 - ((random / 16) & 3);
			BYTE g3 = 24 - ((random / 64) & 3);
			BYTE b3 = random & 7;
			TriggerDynamicLight(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, radtab[fx->flag1], r3, g3, b3);
		}
	}

	static void TonyBossDie(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->Collidable = false;
		item->HitPoints = NOT_TARGETABLE;

		KillItem(itemNumber);
		DisableEntityAI(itemNumber);

		item->Flags |= IFLAG_INVISIBLE;
	}

	static bool TonyIsDying()
	{
		return	BossData.ExplodeCount == 01 ||
			BossData.ExplodeCount == 15 ||
			BossData.ExplodeCount == 25 ||
			BossData.ExplodeCount == 35 ||
			BossData.ExplodeCount == 45 ||
			BossData.ExplodeCount == 55;
	}

	static void ExplodeTonyBoss(ItemInfo* item)
	{
		if (item->HitPoints <= 0 && TonyIsDying())
		{
			int x, y, z;
			x = item->Pose.Position.x + (GetRandomDraw() & 0x3FF) - 512;
			y = item->Pose.Position.y - (GetRandomDraw() & 0x3FF) - 256;
			z = item->Pose.Position.z + (GetRandomDraw() & 0x3FF) - 512;
			BossData.DrawExplode = true;

			TriggerExplosionSparks(x, y, z, 3, -2, 0, item->RoomNumber);
			for (int i = 0; i < 2; i++)
				TriggerExplosionSparks(x, y, z, 3, -1, 0, item->RoomNumber);
		}

		if (BossData.DrawExplode)
		{
			BossData.DrawExplode = false;
		}
	}

	void TonyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		short head = 0;
		short torsoX = 0;
		short torsoY = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 6)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 6;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 6;
			}

			if ((item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase) > 110)
			{
				item->MeshBits = 0;
				if (!BossData.DroppedIcon)
					BossData.DroppedIcon = true;
			}

			if (BossData.ExplodeCount < 256)
				BossData.ExplodeCount++;

			if (BossData.ExplodeCount <= 128 || BossData.RingCount != 6)
				ExplodeTonyBoss(item);
			else
			{
				TonyBossDie(itemNumber);
				BossData.Dead = true;
			}
		}
		else
		{
			if (item->ItemFlags[3] != 2)
				item->HitPoints = TONY_HITS;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (!item->ItemFlags[3])
			{
				int dx = item->Pose.Position.x - LaraItem->Pose.Position.x;
				int dz = item->Pose.Position.z - LaraItem->Pose.Position.z;
				if ((pow(dx, 2) + pow(dz, 2)) < pow(SECTOR(5), 2))
					item->ItemFlags[3] = 1;

				angle = 0;
			}
			else
			{
				creature->Target.x = LaraItem->Pose.Position.x;
				creature->Target.z = LaraItem->Pose.Position.z;
				angle = CreatureTurn(item, creature->MaxTurn);
			}

			if (AI.ahead)
				head = AI.angle;

			switch (item->Animation.ActiveState)
			{
			case TONY_STATE_WAIT:
				creature->MaxTurn = 0;

				if (item->Animation.TargetState != TONY_STATE_RISE && item->ItemFlags[3])
					item->Animation.TargetState = TONY_STATE_RISE;

				break;

			case TONY_STATE_RISE:
				if ((item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase) > 16)
					creature->MaxTurn = TONY_TURN;
				else
					creature->MaxTurn = 0;

				break;

			case TONY_STATE_FLOAT:
				creature->MaxTurn = TONY_TURN;
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (!BossData.ExplodeCount)
				{
					if (item->Animation.TargetState != TONY_STATE_BIGBOOM && item->ItemFlags[3] != 2)
					{
						item->Animation.TargetState = TONY_STATE_BIGBOOM;
						creature->MaxTurn = 0;
					}

					if (item->Animation.TargetState != TONY_STATE_ROCKZAPP && item->ItemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->ItemFlags[0] == 0)
						{
							item->Animation.TargetState = TONY_STATE_ROCKZAPP;
							item->ItemFlags[0] = 1;
						}
					}

					if (item->Animation.TargetState != TONY_STATE_ZAPP && item->Animation.TargetState != TONY_STATE_ROCKZAPP &&
						item->ItemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->ItemFlags[0] == 1)
						{
							item->Animation.TargetState = TONY_STATE_ZAPP;
							item->ItemFlags[0] = 0;
						}
					}
				}

				break;

			case TONY_STATE_ROCKZAPP:
				creature->MaxTurn = 0;
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == 40)
				{
					TriggerFireBall(item, T_ROCKZAPPL, NULL, item->RoomNumber, 0, 0);
					TriggerFireBall(item, T_ROCKZAPPR, NULL, item->RoomNumber, 0, 0);
				}

				break;

			case TONY_STATE_ZAPP:
				creature->MaxTurn = TONY_TURN / 2;
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if ((item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase) == 28)
					TriggerFireBall(item, T_ZAPP, NULL, item->RoomNumber, item->Pose.Orientation.y, 0);

				break;

			case TONY_STATE_BIGBOOM:
				creature->MaxTurn = 0;

				if ((item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase) == 56)
				{
					item->ItemFlags[3] = 2;
					BossData.DrawExplode = true;
				}

				break;

			default:
				break;
			}
		}

		if (item->Animation.ActiveState == TONY_STATE_ROCKZAPP ||
			item->Animation.ActiveState == TONY_STATE_ZAPP ||
			item->Animation.ActiveState == TONY_STATE_BIGBOOM)
		{
			int bright = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
			if (bright > 16)
			{
				bright = g_Level.Anims[item->Animation.AnimNumber].frameEnd - item->Animation.FrameNumber;
				if (bright > 16)
					bright = 16;
			}

			int random = GetRandomControl();
			byte r = 31 - ((random / 16) & 3);
			byte g = 24 - ((random / 64) & 3);
			byte b = random & 7;
			r = (r * bright) / 16;
			g = (g * bright) / 16;
			b = (b * bright) / 16;

			Vector3Int pos1 = { 0, 0, 0 };
			GetJointAbsPosition(item, &pos1, 10);

			TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 12, r, g, b);
			TriggerTonyFlame(itemNumber, 14);

			if (item->Animation.ActiveState == TONY_STATE_ROCKZAPP || item->Animation.ActiveState == TONY_STATE_BIGBOOM)
			{
				pos1.x = pos1.y = pos1.z = 0;
				GetJointAbsPosition(item, &pos1, 13);
				TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 12, r, g, b);
				TriggerTonyFlame(itemNumber, 13);
			}
		}

		if (BossData.ExplodeCount && item->HitPoints > 0)
		{
			ExplodeTonyBoss(item);
			BossData.ExplodeCount++;

			if (BossData.ExplodeCount > 64)
			{
				BossData.ExplodeCount = 0;
				BossData.RingCount = 0;
			}
		}

		CreatureJoint(item, 0, torsoY >> 1);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, torsoY >> 1);
		CreatureAnimation(itemNumber, angle, 0);
	}

	void S_DrawTonyBoss(ItemInfo* item)
	{
		DrawAnimatingItem(item);
	}
}
