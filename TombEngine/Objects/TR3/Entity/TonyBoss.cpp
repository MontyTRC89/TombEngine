#include "framework.h"
#include "Objects/TR3/Entity/TonyBoss.h"

#include "Objects/Effects/Boss.h"
#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/lot.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11Enums.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Boss;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto TONY_TRIGGER_RANGE = SQUARE(SECTOR(4));
	constexpr auto TONY_TURN_RATE_MAX = ANGLE(2.0f);
	constexpr auto TONY_EXPLOSION_NUM_MAX = 60;
	constexpr auto TONY_DAMAGE = 100;
	constexpr auto TONY_EFFECT_COLOR = Vector4(0.8f, 0.4f, 0.0f, 0.5f);

	const BiteInfo TonyLeftHandBite = BiteInfo(Vector3::Zero, 10);
	const BiteInfo TonyRightHandBite = BiteInfo(Vector3::Zero, 13);

	enum class TonyFlameType
	{
		NoFlame = 0,
		RockZAppLeft = 0,
		RockZAppRight,
		ZApp,
		Dropper,
		RockZAppDebris,
		ZAppDebris,
		DropperDebris
	};

	enum TonyState
	{
		TONY_STATE_WAIT,
		TONY_STATE_RISE,
		TONY_STATE_FLY,
		TONY_STATE_SHOOT_RIGHT_HAND,
		TONY_STATE_SHOOT_CEILING,
		TONY_STATE_FLIPMAP, // Cause an shockwave explosion.
		TONY_STATE_DEATH
	};

	enum TonyAnim
	{
		TONY_ANIM_WAIT, // Wait lara.
		TONY_ANIM_RISE,
		TONY_ANIM_FLY, // Real idle state.
		TONY_ANIM_SHOOT_FIRE_RIGHT_HAND,
		TONY_ANIM_SHOOT_CEILING, // with two hand.
		TONY_ANIM_FLIPMAP, // Cause an shockwave explosion.
		TONY_ANIM_DEATH
	};


	struct TonyFlame
	{
		bool on;
		Vector3i pos;
		int fallspeed;
		int speed;
		short yRot;
		short room_number;
		TonyFlameType type;
	};

	void InitialiseTony(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		CheckForRequiredObjects(item); // Lizard is not required.
		item.ItemFlags[1] = 0; // Attack type.
		item.ItemFlags[3] = 0; // Was triggered ?
		item.ItemFlags[5] = 0; // Death count.
		item.ItemFlags[7] = 0; // Explode count.
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
			fx->flag1 = (short)flame.type;
			fx->speed = flame.speed;
			fx->color = Vector4::Zero;
			switch (flame.type)
			{
			case TonyFlameType::ZAppDebris:
				fx->flag2 *= 2;
				break;
			case TonyFlameType::ZApp:
				fx->flag2 = 0;
				break;
			default:
				fx->flag2 = (GetRandomControl() & 3) + 1;
				break;
			}
		}
	}

	static void TriggerTonyFlame(short itemNumber, int hand)
	{
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
		unsigned char size = (GetRandomControl() & 31) + 64;
		sptr->size = sptr->sSize = sptr->dSize = size;
	}

	static void TriggerFireBallFlame(short fxNumber, TonyFlameType type, int xv, int yv, int zv)
	{
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
		unsigned char size = (GetRandomControl() & 31) + 64;
		sptr->size = size;
		sptr->sSize = size;
		sptr->dSize = size / 4;

		switch (type)
		{
		case TonyFlameType::RockZAppLeft:
		case TonyFlameType::RockZAppRight:
			sptr->gravity = (GetRandomControl() & 31) + 16;
			sptr->maxYvel = (GetRandomControl() & 15) + 48;
			sptr->yVel = -sptr->yVel * 16;
			sptr->scalar = 2;
			break;
		case TonyFlameType::RockZAppDebris:
		case TonyFlameType::ZAppDebris:
		case TonyFlameType::DropperDebris:
			sptr->gravity = 0;
			sptr->maxYvel = 0;
			break;
		case TonyFlameType::Dropper:
			sptr->gravity = -(GetRandomControl() & 31) - 16;
			sptr->maxYvel = -(GetRandomControl() & 31) - 64;
			sptr->yVel = sptr->yVel * 16;
			sptr->scalar = 2;
			break;
		case TonyFlameType::ZApp:
			sptr->gravity = sptr->maxYvel = 0;
			sptr->scalar = 2;
			break;
		default:
			sptr->scalar = 1;
			break;
		}
	}

	static void TriggerFireBall(ItemInfo* item, TonyFlameType type, Vector3i* laraPos, short roomNumber, short angle, int zdVelocity)
	{
		TonyFlame flame{};
		flame.type = type;
		switch (type)
		{
		case TonyFlameType::RockZAppLeft:
			flame.on = true;
			flame.pos = GetJointPosition(item, 10);
			flame.fallspeed = -16;
			flame.speed = 0;
			flame.yRot = item->Pose.Orientation.y;
			flame.room_number = roomNumber;
			break;

		case TonyFlameType::RockZAppRight:
			flame.on = true;
			flame.pos = GetJointPosition(item, 13);
			flame.fallspeed = -16;
			flame.speed = 0;
			flame.yRot = item->Pose.Orientation.y;
			flame.room_number = roomNumber;
			break;

		case TonyFlameType::ZApp:
			flame.on = true;
			flame.pos = GetJointPosition(item, 13);
			flame.fallspeed = (GetRandomControl() & 7) + 10;
			flame.speed = 160;
			flame.yRot = item->Pose.Orientation.y;
			flame.room_number = roomNumber;
			break;

		case TonyFlameType::Dropper:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y + 64;
			flame.pos.z = laraPos->z;
			flame.fallspeed = (GetRandomControl() & 3) + 4;
			flame.speed = 0;
			flame.yRot = angle;
			flame.room_number = roomNumber;
			break;

		case TonyFlameType::RockZAppDebris:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y;
			flame.pos.z = laraPos->z;
			flame.fallspeed = (GetRandomControl() & 3) - 2;
			flame.speed = zdVelocity + (GetRandomControl() & 3);
			flame.yRot = GetRandomControl() * 2;
			flame.room_number = roomNumber;
			break;

		case TonyFlameType::ZAppDebris:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y;
			flame.pos.z = laraPos->z;
			flame.fallspeed = -(GetRandomControl() & 15) - 16;
			flame.speed = (GetRandomControl() & 7) + 48;
			angle += (GetRandomControl() & 0x1fff) - 0x9000;
			flame.yRot = angle;
			flame.room_number = roomNumber;
			break;

		case TonyFlameType::DropperDebris:
			flame.on = true;
			flame.pos.x = laraPos->x;
			flame.pos.y = laraPos->y;
			flame.pos.z = laraPos->z;
			flame.fallspeed = -(GetRandomControl() & 31) - 32;
			flame.speed = (GetRandomControl() & 31) + 32;
			flame.yRot = GetRandomControl() * 2;
			flame.room_number = roomNumber;
			break;
		}

		if (flame.on)
			TriggerTonyEffect(flame);
	}

	static TonyFlameType GetDebrisType(TonyFlameType type)
	{
		switch (type)
		{
		case TonyFlameType::ZApp:
			return TonyFlameType::ZAppDebris;
		case TonyFlameType::Dropper:
			return TonyFlameType::DropperDebris;
		}
		return TonyFlameType::RockZAppDebris;
	}

	void ControlTonyFireBall(short fxNumber)
	{
		auto* fx = &EffectList[fxNumber];
		long oldX = fx->pos.Position.x;
		long oldY = fx->pos.Position.y;
		long oldZ = fx->pos.Position.z;
		TonyFlameType type = (TonyFlameType)fx->flag1;
		switch (type)
		{
		case TonyFlameType::RockZAppLeft:
		case TonyFlameType::RockZAppRight:
			fx->fallspeed += (fx->fallspeed / 8) + 1;
			if (fx->fallspeed < -SECTOR(4))
				fx->fallspeed = -SECTOR(4);
			fx->pos.Position.y += fx->fallspeed;

			if (Wibble & 4)
				TriggerFireBallFlame(fxNumber, type, 0, 0, 0);
			break;
		case TonyFlameType::Dropper:
			fx->fallspeed += 2;
			fx->pos.Position.y += fx->fallspeed;

			if (Wibble & 4)
				TriggerFireBallFlame(fxNumber, type, 0, 0, 0);
			break;
		default:
			if (type != TonyFlameType::ZApp)
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
				TriggerFireBallFlame(fxNumber, type, (short)((oldX - fx->pos.Position.x) * 8), (short)((oldY - fx->pos.Position.y) * 8), (short)((oldZ - fx->pos.Position.z) * 4));
			break;
		}

		auto probe = GetCollision(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, fx->roomNumber);

		if (fx->pos.Position.y >= probe.Position.Floor ||
			fx->pos.Position.y < probe.Position.Ceiling)
		{
			Vector3i pos;
			int debrisCount = type == TonyFlameType::ZApp ? 7 : 3;

			switch (type)
			{
			case TonyFlameType::RockZAppLeft:
			case TonyFlameType::RockZAppRight:
				for (int x = 0; x < 2; x++)
					TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, fx->roomNumber);
				probe = GetCollision(LaraItem); // Deal with LaraItem global.
				pos.y = probe.Position.Ceiling + CLICK(1);
				pos.x = LaraItem->Pose.Position.x + (GetRandomControl() & 1023) - CLICK(2);
				pos.z = LaraItem->Pose.Position.z + (GetRandomControl() & 1023) - CLICK(2);
				TriggerExplosionSparks(pos.x, pos.y, pos.z, 3, -2, 0, probe.RoomNumber);
				TriggerFireBall(nullptr, TonyFlameType::Dropper, &pos, probe.RoomNumber, 0, 0); // Falltrough is intended !
			case TonyFlameType::ZApp:
			case TonyFlameType::Dropper:
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, fx->roomNumber);
				pos.x = oldX;
				pos.y = oldY;
				pos.z = oldZ;
				for (int x = 0; x < debrisCount; x++)
					TriggerFireBall(nullptr, GetDebrisType(type), &pos, fx->roomNumber, fx->pos.Orientation.y, 32 + (x * 4));
				break;
			}

			KillEffect(fxNumber);
			return;
		}


		if (TestEnvironment(ENV_FLAG_WATER, probe.RoomNumber))
		{
			KillEffect(fxNumber);
			return;
		}

		if (LaraItem->Effect.Type == EffectType::None)
		{
			if (ItemNearLara(fx->pos.Position, 200))
			{
				LaraItem->HitStatus = true;
				KillEffect(fxNumber);
				DoDamage(LaraItem, 200);
				ItemBurn(LaraItem);
				return;
			}
		}

		if (probe.RoomNumber != fx->roomNumber)
			EffectNewRoom(fxNumber, LaraItem->RoomNumber);

		static short LightIntensityTable[7] = { 16, 0, 14, 9, 7, 7, 7 };
		if (LightIntensityTable[fx->flag1])
		{
			TriggerDynamicLight(
				fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z,
				LightIntensityTable[fx->flag1],
				31 - ((GetRandomControl() / 16) & 3),
				24 - ((GetRandomControl() / 64) & 3),
				GetRandomControl() & 7
			);
		}
	}

	void TonyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		short head = 0;
		short torsoX = 0;
		short torsoY = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != TONY_STATE_DEATH)
				SetAnimation(item, TONY_ANIM_DEATH);

			int frameEnd = g_Level.Anims[object->animIndex + TONY_ANIM_DEATH].frameEnd;
			if (item->Animation.FrameNumber >= frameEnd)
			{
				// Avoid having the object stop working.
				item->Animation.FrameNumber = frameEnd;
				item->MeshBits.ClearAll();

				if (item->ItemFlags[7] < TONY_EXPLOSION_NUM_MAX)
					item->ItemFlags[7]++;

				// Do explosion effect.
				ExplodeBoss(itemNumber, *item, TONY_EXPLOSION_NUM_MAX, TONY_EFFECT_COLOR, false);
				return;
			}
		}
		else
		{
			if (item->ItemFlags[3] != 2) // Shield tony to avoid him to take damage before him is flying.
				item->HitPoints = Objects[item->ObjectNumber].HitPoints;

			AI_INFO ai;
			CreatureAIInfo(item, &ai);

			if (item->ItemFlags[3] == 0)
			{
				if (ai.distance < TONY_TRIGGER_RANGE)
					item->ItemFlags[3] = 1;
				angle = 0;
			}
			else
			{
				creature->Target.x = LaraItem->Pose.Position.x; // TODO: deal with LaraItem global.
				creature->Target.z = LaraItem->Pose.Position.z;
				angle = CreatureTurn(item, creature->MaxTurn);
			}

			if (ai.ahead)
				head = ai.angle;

			switch (item->Animation.ActiveState)
			{
			case TONY_STATE_WAIT:
				creature->MaxTurn = 0;

				if (item->Animation.TargetState != TONY_STATE_RISE && item->ItemFlags[3])
					item->Animation.TargetState = TONY_STATE_RISE;

				break;

			case TONY_STATE_RISE:
				creature->MaxTurn = 0;
				break;

			case TONY_STATE_FLY:
				creature->MaxTurn = TONY_TURN_RATE_MAX;
				if (ai.ahead)
				{
					torsoX = ai.xAngle;
					torsoY = ai.angle;
				}

				if (item->ItemFlags[7] <= 0)
				{
					if (item->Animation.TargetState != TONY_STATE_FLIPMAP && item->ItemFlags[3] != 2)
						item->Animation.TargetState = TONY_STATE_FLIPMAP;

					if (item->Animation.TargetState != TONY_STATE_SHOOT_CEILING && item->ItemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->ItemFlags[1] == 0)
						{
							item->Animation.TargetState = TONY_STATE_SHOOT_CEILING;
							item->ItemFlags[1] = 1;
						}
					}

					if (item->Animation.TargetState != TONY_STATE_SHOOT_RIGHT_HAND && item->Animation.TargetState != TONY_STATE_SHOOT_CEILING &&
						item->ItemFlags[3] == 2)
					{
						if (!(Wibble & 255) && item->ItemFlags[1] == 1)
						{
							item->Animation.TargetState = TONY_STATE_SHOOT_RIGHT_HAND;
							item->ItemFlags[1] = 0;
						}
					}
				}

				break;

			case TONY_STATE_SHOOT_CEILING:
				creature->MaxTurn = 0;
				if (ai.ahead)
				{
					torsoX = ai.xAngle;
					torsoY = ai.angle;
				}

				if (item->Animation.FrameNumber == GetFrameNumber(item, 40))
				{
					TriggerFireBall(item, TonyFlameType::RockZAppLeft, nullptr, item->RoomNumber, 0, 0);
					TriggerFireBall(item, TonyFlameType::RockZAppRight, nullptr, item->RoomNumber, 0, 0);
				}

				break;

			case TONY_STATE_SHOOT_RIGHT_HAND:
				creature->MaxTurn = TONY_TURN_RATE_MAX / 2;
				if (ai.ahead)
				{
					torsoX = ai.xAngle;
					torsoY = ai.angle;
				}

				if (item->Animation.FrameNumber == GetFrameNumber(item, 28))
					TriggerFireBall(item, TonyFlameType::ZApp, nullptr, item->RoomNumber, item->Pose.Orientation.y, 0);

				break;

			case TONY_STATE_FLIPMAP:
				creature->MaxTurn = 0;

				if (item->Animation.FrameNumber == GetFrameNumber(item, 56))
				{
					item->ItemFlags[3] = 2;
					SpawnShockwaveExplosion(*item, TONY_EFFECT_COLOR);
				}

				break;
			}
		}

		if (item->Animation.ActiveState == TONY_STATE_SHOOT_CEILING ||
			item->Animation.ActiveState == TONY_STATE_SHOOT_RIGHT_HAND ||
			item->Animation.ActiveState == TONY_STATE_FLIPMAP)
		{
			int bright = GetCurrentRelativeFrameNumber(item);
			if (bright > 16)
				bright = 16;
			byte r = 31 - ((GetRandomControl() / 16) & 3);
			byte g = 24 - ((GetRandomControl() / 64) & 3);
			byte b = GetRandomControl() & 7;
			r = (r * bright) / 16;
			g = (g * bright) / 16;
			b = (b * bright) / 16;

			auto handPos = GetJointPosition(item, TonyLeftHandBite.meshNum);
			TriggerTonyFlame(itemNumber, 13);
			TriggerDynamicLight(handPos.x, handPos.y, handPos.z, 12, r, g, b);

			if (item->Animation.ActiveState == TONY_STATE_SHOOT_CEILING || item->Animation.ActiveState == TONY_STATE_FLIPMAP)
			{
				handPos = GetJointPosition(item, TonyRightHandBite.meshNum);
				TriggerTonyFlame(itemNumber, 14);
				TriggerDynamicLight(handPos.x, handPos.y, handPos.z, 12, r, g, b);
			}
		}

		CreatureJoint(item, 0, torsoY >> 1);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, torsoY >> 1);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
