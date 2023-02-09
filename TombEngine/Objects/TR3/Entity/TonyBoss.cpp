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
	constexpr auto TONY_TRIGGER_RANGE = SQUARE(BLOCK(4));
	constexpr auto TONY_TURN_RATE_MAX = ANGLE(2.0f);

	constexpr auto TONY_EXPLOSION_COUNT_MAX = 60;
	constexpr auto TONY_EFFECT_COLOR		= Vector4(0.8f, 0.4f, 0.0f, 0.5f);

	const auto TonyLeftHandBite	 = BiteInfo(Vector3::Zero, 10);
	const auto TonyRightHandBite = BiteInfo(Vector3::Zero, 13);

	// I can't set it to the TonyFlame struct since the control of the
	// flame use fxNumber as argument or that FX_INFO have no void* to hold custom data.
	// So i let it there to avoid initialising it every time in the control of the flame.
	const int LightIntensityTable[7] = { 16, 0, 14, 9, 7, 7, 7 }; 

	enum class TonyFlameType
	{
		NoFlame = 0,
		CeilingLeftHand,
		CeilingRightHand,
		CeilingDebris, // It's the fire debris which is called when any of the fire from CeilingLeftHand/CeilingRightHand hit the ceiling, it will create small debris.
		InFront, // Go from it's right hand to lara.
		InFrontDebris, // Will create debris when it hit the wall.
		ShowerFromCeiling, // It's called when the two flame have hit the ceiling, it will start fire rain to lara position.
		ShowerFromCeilingDebris // Same as InFrontDebris but from ShowerFromCeiling fire when it hit the floor.
	};

	enum TonyState
	{
		TONY_STATE_WAIT,
		TONY_STATE_RISE,
		TONY_STATE_FLY,
		TONY_STATE_SHOOT_RIGHT_HAND,
		TONY_STATE_SHOOT_CEILING,
		TONY_STATE_FLIPMAP, // Shockwave explosion.
		TONY_STATE_DEATH
	};

	enum TonyAnim
	{
		TONY_ANIM_WAIT,
		TONY_ANIM_RISE,
		TONY_ANIM_FLY,			 // Real idle state.
		TONY_ANIM_SHOOT_FIRE_RIGHT_HAND,
		TONY_ANIM_SHOOT_CEILING, // Two-handed.
		TONY_ANIM_FLIPMAP,		 // Shockwave explosion.
		TONY_ANIM_DEATH
	};

	struct TonyFlame
	{
		bool on;
		Vector3i Position;
		int VerticalVelocity;
		int speed;
		short yRot;
		short RoomNumber;
		TonyFlameType Type;
	};

	static void TriggerTonyEffect(const TonyFlame& flame)
	{
		int fxNumber = CreateNewEffect(flame.RoomNumber);
		if (fxNumber == NO_ITEM)
			return;

		auto& fx = EffectList[fxNumber];

		fx.pos.Position = flame.Position;
		fx.fallspeed = flame.VerticalVelocity;
		fx.pos.Orientation = EulerAngles(0, flame.yRot, 0);
		fx.objectNumber = ID_TONY_BOSS_FLAME;
		fx.flag1 = (short)flame.Type;
		fx.speed = flame.speed;
		fx.color = Vector4::Zero;

		switch (flame.Type)
		{
		case TonyFlameType::InFrontDebris:
			fx.flag2 *= 2;
			break;

		case TonyFlameType::InFront:
			fx.flag2 = 0;
			break;

		default:
			fx.flag2 = (GetRandomControl() & 3) + 1;
			break;
		}
	}

	static void TriggerTonyFlame(short itemNumber, int hand)
	{
		auto& flame = *GetFreeParticle();

		flame.on = true;
		flame.sR = 255;
		flame.sG = 48 + (GetRandomControl() & 31);
		flame.sB = 48;
		flame.dR = 192 + (GetRandomControl() & 63);
		flame.dG = 128 + (GetRandomControl() & 63);
		flame.dB = 32;
		flame.colFadeSpeed = 12 + (GetRandomControl() & 3);
		flame.fadeToBlack = 8;
		flame.sLife = flame.life = (GetRandomControl() & 7) + 24;
		flame.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		flame.extras = 0;
		flame.dynamic = -1;
		flame.x = ((GetRandomControl() & 15) - 8);
		flame.y = 0;
		flame.z = ((GetRandomControl() & 15) - 8);
		flame.xVel = ((GetRandomControl() & 255) - 128);
		flame.yVel = -(GetRandomControl() & 15) - 16;
		flame.zVel = ((GetRandomControl() & 255) - 128);
		flame.friction = 5;

		if (GetRandomControl() & 1)
		{
			flame.flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
			flame.rotAng = GetRandomControl() & 4095;

			if (Random::TestProbability(1 / 2.0f))
				flame.rotAdd = -(GetRandomControl() & 15) - 16;
			else
				flame.rotAdd = (GetRandomControl() & 15) + 16;
		}
		else
		{
			flame.flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_ITEM | SP_NODEATTACH;
		}

		flame.gravity = -(GetRandomControl() & 31) - 16;
		flame.maxYvel = -(GetRandomControl() & 7) - 16;
		flame.fxObj = itemNumber;
		flame.nodeNumber = hand;
		flame.spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		flame.scalar = 1;

		float size = (GetRandomControl() & 31) + 64;
		flame.size =
		flame.sSize =
		flame.dSize = size;
	}

	static void TriggerFireBallFlame(int fxNumber, TonyFlameType type, int xv, int yv, int zv)
	{
		auto& flame = *GetFreeParticle();

		flame.on = true;
		flame.sR = 255;
		flame.sG = 48 + (GetRandomControl() & 31);
		flame.sB = 48;
		flame.dR = 192 + (GetRandomControl() & 63);
		flame.dG = 128 + (GetRandomControl() & 63);
		flame.dB = 32;
		flame.colFadeSpeed = 12 + (GetRandomControl() & 3);
		flame.fadeToBlack = 8;
		flame.sLife = flame.life = (GetRandomControl() & 7) + 24;
		flame.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		flame.extras = 0;
		flame.dynamic = -1;
		flame.x = ((GetRandomControl() & 15) - 8);
		flame.y = 0;
		flame.z = ((GetRandomControl() & 15) - 8);
		flame.xVel = xv + ((GetRandomControl() & 255) - 128);
		flame.yVel = yv;
		flame.zVel = zv + ((GetRandomControl() & 255) - 128);
		flame.friction = 5;

		if (GetRandomControl() & 1)
		{
			flame.flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_FX;
			flame.rotAng = GetRandomControl() & 4095;

			if (GetRandomControl() & 1)
				flame.rotAdd = -(GetRandomControl() & 15) - 16;
			else
				flame.rotAdd = (GetRandomControl() & 15) + 16;
		}
		else
		{
			flame.flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_FX;
		}

		flame.fxObj = (unsigned char)fxNumber;
		flame.spriteIndex = (unsigned char)Objects[ID_DEFAULT_SPRITES].meshIndex;
		unsigned char size = (GetRandomControl() & 31) + 64;
		flame.size = size;
		flame.sSize = size;
		flame.dSize = size / 4;

		switch (type)
		{
		case TonyFlameType::CeilingLeftHand:
		case TonyFlameType::CeilingRightHand:
			flame.gravity = (GetRandomControl() & 31) + 16;
			flame.maxYvel = (GetRandomControl() & 15) + 48;
			flame.yVel = -flame.yVel * 16;
			flame.scalar = 2;
			break;

		case TonyFlameType::CeilingDebris:
		case TonyFlameType::InFrontDebris:
		case TonyFlameType::ShowerFromCeilingDebris:
			flame.gravity = 0;
			flame.maxYvel = 0;
			break;

		case TonyFlameType::ShowerFromCeiling:
			flame.gravity = -(GetRandomControl() & 31) - 16;
			flame.maxYvel = -(GetRandomControl() & 31) - 64;
			flame.yVel = flame.yVel * 16;
			flame.scalar = 2;
			break;

		case TonyFlameType::InFront:
			flame.gravity = flame.maxYvel = 0;
			flame.scalar = 2;
			break;

		default:
			flame.scalar = 1;
			break;
		}
	}

	static void TriggerFireBall(ItemInfo* item, TonyFlameType type, Vector3i* laraPos, short roomNumber, short angle, int zdVelocity)
	{
		auto flame = TonyFlame();

		flame.Type = type;
		switch (flame.Type)
		{
		case TonyFlameType::CeilingLeftHand:
			flame.on = true;
			flame.Position = GetJointPosition(item, 10);
			flame.VerticalVelocity = -16;
			flame.speed = 0;
			flame.yRot = item->Pose.Orientation.y;
			flame.RoomNumber = roomNumber;
			break;

		case TonyFlameType::CeilingRightHand:
			flame.on = true;
			flame.Position = GetJointPosition(item, 13);
			flame.VerticalVelocity = -16;
			flame.speed = 0;
			flame.yRot = item->Pose.Orientation.y;
			flame.RoomNumber = roomNumber;
			break;

		case TonyFlameType::InFront:
			flame.on = true;
			flame.Position = GetJointPosition(item, 13);
			flame.VerticalVelocity = (GetRandomControl() & 7) + 10;
			flame.speed = 160;
			flame.yRot = item->Pose.Orientation.y;
			flame.RoomNumber = roomNumber;
			break;

		case TonyFlameType::ShowerFromCeiling:
			flame.on = true;
			flame.Position.x = laraPos->x;
			flame.Position.y = laraPos->y + 64;
			flame.Position.z = laraPos->z;
			flame.VerticalVelocity = (GetRandomControl() & 3) + 4;
			flame.speed = 0;
			flame.yRot = angle;
			flame.RoomNumber = roomNumber;
			break;

		case TonyFlameType::CeilingDebris:
			flame.on = true;
			flame.Position.x = laraPos->x;
			flame.Position.y = laraPos->y;
			flame.Position.z = laraPos->z;
			flame.VerticalVelocity = (GetRandomControl() & 3) - 2;
			flame.speed = zdVelocity + (GetRandomControl() & 3);
			flame.yRot = GetRandomControl() * 2;
			flame.RoomNumber = roomNumber;
			break;

		case TonyFlameType::InFrontDebris:
			flame.on = true;
			flame.Position.x = laraPos->x;
			flame.Position.y = laraPos->y;
			flame.Position.z = laraPos->z;
			flame.VerticalVelocity = -(GetRandomControl() & 15) - 16;
			flame.speed = (GetRandomControl() & 7) + 48;
			angle += (GetRandomControl() & 0x1fff) - 0x9000;
			flame.yRot = angle;
			flame.RoomNumber = roomNumber;
			break;

		case TonyFlameType::ShowerFromCeilingDebris:
			flame.on = true;
			flame.Position.x = laraPos->x;
			flame.Position.y = laraPos->y;
			flame.Position.z = laraPos->z;
			flame.VerticalVelocity = -(GetRandomControl() & 31) - 32;
			flame.speed = (GetRandomControl() & 31) + 32;
			flame.yRot = GetRandomControl() * 2;
			flame.RoomNumber = roomNumber;
			break;
		}

		if (flame.on)
			TriggerTonyEffect(flame);
	}

	static TonyFlameType GetDebrisType(TonyFlameType type)
	{
		switch (type)
		{
		case TonyFlameType::InFront:
			return TonyFlameType::InFrontDebris;

		case TonyFlameType::ShowerFromCeiling:
			return TonyFlameType::ShowerFromCeilingDebris;

		default:
			return TonyFlameType::CeilingDebris;
		}
	}

	void InitialiseTony(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		CheckForRequiredObjects(item);

		item.ItemFlags[1] = 0; // Attack type.
		item.ItemFlags[3] = 0; // Was triggered?
		item.ItemFlags[5] = 0; // Death count.
		item.ItemFlags[7] = 0; // Explode count.
	}

	void ControlTonyFireBall(short fxNumber)
	{
		auto& fx = EffectList[fxNumber];

		auto prevPos = fx.pos.Position;
		auto type = (TonyFlameType)fx.flag1;

		switch (type)
		{
		case TonyFlameType::CeilingLeftHand:
		case TonyFlameType::CeilingRightHand:
			fx.fallspeed += (fx.fallspeed / 8) + 1;
			if (fx.fallspeed < -BLOCK(4))
				fx.fallspeed = -BLOCK(4);

			fx.pos.Position.y += fx.fallspeed;

			if (Wibble & 4)
				TriggerFireBallFlame(fxNumber, type, 0, 0, 0);

			break;

		case TonyFlameType::ShowerFromCeiling:
			fx.fallspeed += 2;
			fx.pos.Position.y += fx.fallspeed;

			if (Wibble & 4)
				TriggerFireBallFlame(fxNumber, type, 0, 0, 0);

			break;

		default:
			if (type != TonyFlameType::InFront)
			{
				if (fx.speed > 48)
					fx.speed--;
			}

			fx.fallspeed += fx.flag2;
			if (fx.fallspeed > CLICK(2))
				fx.fallspeed = CLICK(2);

			fx.pos.Position.y += fx.fallspeed / 2;
			fx.pos.Position.z += fx.speed * phd_cos(fx.pos.Orientation.y);
			fx.pos.Position.x += fx.speed * phd_sin(fx.pos.Orientation.y);

			if (Wibble & 4)
			{
				TriggerFireBallFlame(
					fxNumber, type,
					short((prevPos.x - fx.pos.Position.x) * 8), short((prevPos.y - fx.pos.Position.y) * 8), short((prevPos.z - fx.pos.Position.z) * 4));
			}
			
			break;
		}

		auto probe = GetCollision(fx.pos.Position.x, fx.pos.Position.y, fx.pos.Position.z, fx.roomNumber);

		if (fx.pos.Position.y >= probe.Position.Floor ||
			fx.pos.Position.y < probe.Position.Ceiling)
		{
			Vector3i pos;
			int debrisCount = type == TonyFlameType::InFront ? 7 : 3;

			switch (type)
			{
			case TonyFlameType::CeilingLeftHand:
			case TonyFlameType::CeilingRightHand:
				for (int x = 0; x < 2; x++)
					TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -1, 0, fx.roomNumber);

				probe = GetCollision(LaraItem); // TODO: Deal with LaraItem global.
				pos.y = probe.Position.Ceiling + CLICK(1);
				pos.x = LaraItem->Pose.Position.x + (GetRandomControl() & 1023) - CLICK(2);
				pos.z = LaraItem->Pose.Position.z + (GetRandomControl() & 1023) - CLICK(2);

				TriggerExplosionSparks(pos.x, pos.y, pos.z, 3, -2, 0, probe.RoomNumber);
				TriggerFireBall(nullptr, TonyFlameType::ShowerFromCeiling, &pos, probe.RoomNumber, 0, 0); // Fallthrough is intended.

			case TonyFlameType::InFront:
			case TonyFlameType::ShowerFromCeiling:
				TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 0, fx.roomNumber);
				pos = prevPos;

				for (int x = 0; x < debrisCount; x++)
					TriggerFireBall(nullptr, GetDebrisType(type), &pos, fx.roomNumber, fx.pos.Orientation.y, 32 + (x * 4));

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
			if (ItemNearLara(fx.pos.Position, 200))
			{
				LaraItem->HitStatus = true;
				KillEffect(fxNumber);
				DoDamage(LaraItem, 200);
				ItemBurn(LaraItem);
				return;
			}
		}

		if (probe.RoomNumber != fx.roomNumber)
			EffectNewRoom(fxNumber, LaraItem->RoomNumber);

		if (LightIntensityTable[fx.flag1])
		{
			TriggerDynamicLight(
				fx.pos.Position.x, fx.pos.Position.y, fx.pos.Position.z,
				LightIntensityTable[fx.flag1],
				31 - ((GetRandomControl() / 16) & 3),
				24 - ((GetRandomControl() / 64) & 3),
				GetRandomControl() & 7);
		}
	}

	void TonyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short headingAngle = 0;
		short tiltAngle = 0;
		short headAngle = 0;
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

				if (item->ItemFlags[7] < TONY_EXPLOSION_COUNT_MAX)
					item->ItemFlags[7]++;

				// Do explosion effect.
				// TODO: change the explosion color to tony one, which is red, orange.
				ExplodeBoss(itemNumber, *item, TONY_EXPLOSION_COUNT_MAX, TONY_EFFECT_COLOR, Vector4::Zero, Vector4::Zero, false);
				return;
			}
		}
		else
		{
			// Immune to damage until flying and idle.
			if (item->ItemFlags[3] != 2)
				item->HitPoints = Objects[item->ObjectNumber].HitPoints;

			AI_INFO ai;
			CreatureAIInfo(item, &ai);

			if (item->ItemFlags[3] == 0)
			{
				if (ai.distance < TONY_TRIGGER_RANGE)
					item->ItemFlags[3] = 1;

				headingAngle = 0;
			}
			else
			{
				creature->Target.x = LaraItem->Pose.Position.x; // TODO: deal with LaraItem global.
				creature->Target.z = LaraItem->Pose.Position.z;
				headingAngle = CreatureTurn(item, creature->MaxTurn);
			}

			if (ai.ahead)
				headAngle = ai.angle;

			switch (item->Animation.ActiveState)
			{
			case TONY_STATE_WAIT:
				creature->MaxTurn = 0;

				if (item->Animation.TargetState != TONY_STATE_RISE && item->ItemFlags[3] != 0)
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

					if (item->Animation.TargetState != TONY_STATE_SHOOT_RIGHT_HAND &&
						item->Animation.TargetState != TONY_STATE_SHOOT_CEILING &&
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
					TriggerFireBall(item, TonyFlameType::CeilingLeftHand, nullptr, item->RoomNumber, 0, 0);
					TriggerFireBall(item, TonyFlameType::CeilingRightHand, nullptr, item->RoomNumber, 0, 0);
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
					TriggerFireBall(item, TonyFlameType::InFront, nullptr, item->RoomNumber, item->Pose.Orientation.y, 0);

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

			if (item->Animation.ActiveState == TONY_STATE_SHOOT_CEILING ||
				item->Animation.ActiveState == TONY_STATE_FLIPMAP)
			{
				handPos = GetJointPosition(item, TonyRightHandBite.meshNum);
				TriggerTonyFlame(itemNumber, 14);
				TriggerDynamicLight(handPos.x, handPos.y, handPos.z, 12, r, g, b);
			}
		}

		CreatureJoint(item, 0, torsoY >> 1);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, torsoY >> 1);
		CreatureAnimation(itemNumber, headingAngle, 0);
	}
}
