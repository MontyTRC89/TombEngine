#include "framework.h"
#include "Objects/TR5/Entity/tr5_twogunlaser.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/item_fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "control.h"
#include "effects.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "people.h"
#include "Specific/setup.h"
#include "Game/effects/spark.h"
#include "Game/effects/lightning.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Lightning;
using namespace TEN::Math::Random;
using namespace TEN::Effects::Spark;

namespace TEN::Entities::Creatures::TR5
{
	int TWGunMesh[2] = { 8, 5 };

	const Vector3i TWGunPositions[2] =
	{
		Vector3i(0, 230, 40),
		Vector3i(8, 230, 40),
	};

	const auto LaserguardHead = BiteInfo(Vector3(0.0f, -200.0f, 0.0f), 2);

	enum TwogunState
	{
		TWOGUN_STATE_EMPTY = -1,
		TWOGUN_STATE_IDLE = 1,
		TWOGUN_STATE_WALK = 2,
		TWOGUN_STATE_SHOOT_RIGHT = 3,
		TWOGUN_STATE_SHOOT_LEFT = 4,
		TWOGUN_STATE_IDLE_TO_AIM = 5,
		TWOGUN_STATE_SHOOT_BOTH = 6,
		TWOGUN_STATE_DEATH = 7,
		TWOGUN_STATE_TURN_180 = 8,
		TWOGUN_STATE_MISSFIRE = 9,
		TWOGUN_STATE_GUN_BLOCKAGE = 10,
		TWOGUN_STATE_FALLSTART = 11,
		TWOGUN_STATE_FALLLOOP = 12,
		TWOGUN_STATE_FALLDEATH = 13
	};

	enum TwogunAnim
	{
		TWOGUN_ANIM_WALK = 0,
		TWOGUN_ANIM_SHOOT_RIGHT = 1,
		TWOGUN_ANIM_SHOOT_LEFT = 2,
		TWOGUN_ANIM_DEATH = 3,
		TWOGUN_ANIM_AIM_TO_WALK = 4,
		TWOGUN_ANIM_WALK_AIM_TO_IDLE = 5,
		TWOGUN_ANIM_IDLE = 6,
		TWOGUN_ANIM_IDLE_TO_AIM = 7,
		TWOGUN_ANIM_AIM = 8,
		TWOGUN_ANIM_SHOOT_BOTH = 9,
		TWOGUN_ANIM_AIM_TO_IDLE = 10,
		TWOGUN_ANIM_TURN_180 = 11,
		TWOGUN_ANIM_MISSFIRE = 12,
		TWOGUN_ANIM_GUN_BLOCKAGE = 13,
		TWOGUN_ANIM_FALLSTART = 14,
		TWOGUN_ANIM_FALLLOOP = 15,
		TWOGUN_ANIM_FALLDEATH = 16
	};

	void InitialiseTwogun(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		SetAnimation(item, 6);
	}

	void FireTwogunWeapon(short itemNumber, short LeftRight, short plasma)
	{
		auto* item = &g_Level.Items[itemNumber];

		auto pos1 = GetJointPosition(item, TWGunMesh[LeftRight], TWGunPositions[LeftRight]);
		auto pos2 = GetJointPosition(item, TWGunMesh[LeftRight], Vector3i(TWGunPositions[LeftRight].x, TWGunPositions[LeftRight].y + 4096, TWGunPositions[LeftRight].z));
		auto orient = Geometry::GetOrientToPoint(pos1.ToVector3(), pos2.ToVector3());

		if (!plasma)
		{
			TriggerLaserBeam(pos1, pos2, LeftRight);

			item->ItemFlags[LeftRight] = 16;
			TriggerTwogunPlasma(pos1, Pose(pos1.ToVector3(), orient), 16);
			TriggerTwogunPlasma(pos1, Pose(pos1.ToVector3(), orient), 16);
			TriggerTwogunPlasma(pos1, Pose(pos1.ToVector3(), orient), 16);

			Vector3i hitPos;
			MESH_INFO* hitMesh = nullptr;

			GameVector start = GameVector(pos1, 0);
			GameVector end = GameVector(pos2, 0);
			start.RoomNumber = item->RoomNumber;

			if (ObjectOnLOS2(&start, &end, &hitPos, &hitMesh, ID_LARA) == GetLaraInfo(LaraItem)->ItemNumber)
			{
				if (LaraItem->HitPoints < 501)
				{
					ItemCustomBurn(LaraItem, Vector3(0.2f, 0.4f, 1.0f), Vector3(0.2f, 0.3f, 0.8f), 1 * FPS);
					DoDamage(LaraItem, INT_MAX);
				}
				else
					DoDamage(LaraItem, 250);
			}
		}
	
			return;

			TriggerTwogunPlasma(pos1, Pose(pos1.ToVector3(), orient), abs(item->ItemFlags[LeftRight]));
	}
	

	void TriggerTwogunPlasma(const Vector3i& posr, const Pose& pos, float life)
	{
		auto* sptr = GetFreeParticle();

		sptr->on = true;

		sptr->sB = (((GetRandomControl() & 127) + 128) * life) /16;
		sptr->sR = sptr->sB - (sptr->sB >> 2);
		sptr->sG = sptr->sR;
		sptr->dR = 0;
		sptr->dB = (((GetRandomControl() & 127) + 32) * life) /16;
		sptr->dG = sptr->dB >> 2;
		sptr->colFadeSpeed = 8 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 8;
		sptr->sLife = sptr->life = (GetRandomControl() & 3) + 24;

		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->x = pos.Position.x;
		sptr->y = pos.Position.y;
		sptr->z = pos.Position.z;

		int size = ((life * 64) * phd_cos(TO_RAD(pos.Orientation.x))) / 5;

		sptr->xVel =  Random::GenerateInt(-128, 128) / 5;
		sptr->yVel = life * 16 / 5;
		sptr->zVel =  Random::GenerateInt(-128, 128) / 5;
		sptr->friction = 0;
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->rotAng = GetRandomControl() & 4095;
		sptr->rotAdd = (GetRandomControl() & 127) - 64;
		sptr->gravity = (GetRandomControl() & 31) + 32;
		sptr->maxYvel = 0;
		sptr->scalar = 1;
		sptr->size = sptr->sSize = size;
		sptr->dSize = 1;
	}

	void TwogunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		short angle, head, xTorso, yTorso, frame, base, roomNumber;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		angle = 0;
		head = 0;
		xTorso = 0;
		yTorso = 0;

		if (item->ItemFlags[0] || item->ItemFlags[1])
		{
			for (int i = 0; i < 2; i++)
			{
				if (item->ItemFlags[i])
				{
					FireTwogunWeapon(itemNumber, i, 1);

					if (item->ItemFlags[i] > 0)
					{
						auto pos1 = GetJointPosition(item, TWGunMesh[i], TWGunPositions[i]);

						short blue = item->ItemFlags[i] << 4;
						short green = blue >> 2;
						short red = 0;
						TriggerDynamicLight(pos1.x, pos1.y, pos1.z, item->ItemFlags[i] + 8, red, green, blue);
						item->ItemFlags[i]--;
					}
					else
						item->ItemFlags[i]++;
				}
			}
		}

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != TWOGUN_STATE_DEATH && item->TriggerFlags != 1)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + TWOGUN_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = TWOGUN_STATE_DEATH;
			}
			else if (item->TriggerFlags == 1)
			{
				switch (item->Animation.ActiveState)
				{
				case TWOGUN_STATE_FALLSTART:
					frame = item->Animation.FrameNumber;
					base = g_Level.Anims[item->Animation.AnimNumber].frameBase;

					if (frame == base + 48 || frame == base + 15)
					{
						roomNumber = item->RoomNumber;
						GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber),
						
						TestTriggers(item, true, 0);
					}

					break;

				case TWOGUN_STATE_FALLLOOP:
					item->Animation.IsAirborne = true;

					if (item->Pose.Position.y >= item->Floor)
					{
						item->Pose.Position.y = item->Floor;
						item->Animation.IsAirborne = false;
						item->Animation.Velocity.y = 0.0f;
						item->Animation.TargetState = TWOGUN_STATE_FALLDEATH;
						CreatureEffect2(item, LaserguardHead, 10, item->Pose.Orientation.y, DoBloodSplat);
					}

					break;

				case TWOGUN_STATE_FALLDEATH:
					item->Pose.Position.y = item->Floor;

					break;

				default:
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + TWOGUN_ANIM_FALLSTART;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = TWOGUN_STATE_FALLSTART;
					creature->LOT.IsJumping = true;
					auto* room = &g_Level.Rooms[item->RoomNumber];

					break;
				}
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			CreatureAIInfo(item, &AI);
	
				AI_INFO laraAI;
				if (creature->Enemy->IsLara())
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
				}
				GetCreatureMood(item, &AI, creature->Enemy != LaraItem);
				CreatureMood(item, &AI, creature->Enemy != LaraItem);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (((laraAI.distance < pow(SECTOR(2), 2) && 
				(laraAI.angle < 0x4000 && laraAI.angle > -16384 || 
					LaraItem->Animation.Velocity.z > 20)) ||
				item->HitStatus || 
				TargetVisible(item, &laraAI)) && 
				abs(item->Pose.Position.y - LaraItem->Pose.Position.y) < 1536)
			{
				creature->Enemy = LaraItem;
				AlertAllGuards(itemNumber);
			}

			switch (item->Animation.ActiveState)
			{
			case TWOGUN_STATE_IDLE:
				creature->Flags = 0;
				creature->MaxTurn = 0;

				if (!(item->AIBits & GUARD))
				{
					if (abs(AI.angle) < 364)
						item->Pose.Orientation.y += AI.angle;
					else
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += 364;
						else
							item->Pose.Orientation.y -= 364;
					}

					head = laraAI.angle >> 1;
					yTorso = laraAI.angle >> 1;
					xTorso = AI.xAngle >> 1;
				}

				if (item->AIBits & GUARD)
					head = AIGuard(creature);
				else
				{
					if (laraAI.angle > 20480 || laraAI.angle < -20480)
						item->Animation.TargetState = TWOGUN_STATE_TURN_180;
					else if (!Targetable(item, &AI))
					{
						if (item->TriggerFlags != 1)
							item->Animation.TargetState = TWOGUN_STATE_WALK;
					}
					else if (AI.distance < 0x900000 || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = TWOGUN_STATE_IDLE_TO_AIM;
					else if (item->AIBits != MODIFY)
					{
						if (item->TriggerFlags != 1)
							item->Animation.TargetState = TWOGUN_STATE_WALK;
					}
				}

				break;

			case TWOGUN_STATE_WALK:
				creature->MaxTurn = 910;

				if (Targetable(item, &AI) && laraAI.angle < 6144 && laraAI.angle > -6144)
				{
					if (item->Animation.FrameNumber >= g_Level.Anims[item->Animation.AnimNumber].frameBase + 29)
						item->Animation.TargetState = TWOGUN_STATE_SHOOT_RIGHT;
					else
						item->Animation.TargetState = TWOGUN_STATE_SHOOT_LEFT;
				}
				else
				{
					if (laraAI.angle > 20480 || laraAI.angle < -20480)
						item->Animation.TargetState = TWOGUN_STATE_IDLE;
					else
						item->Animation.TargetState = TWOGUN_STATE_WALK;
				}

				break;

			case TWOGUN_STATE_SHOOT_RIGHT:
			case TWOGUN_STATE_SHOOT_LEFT:
				head = laraAI.angle;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				{
					if (item->Animation.ActiveState == TWOGUN_STATE_SHOOT_LEFT)
						FireTwogunWeapon(itemNumber, 0, 0);
					else
						FireTwogunWeapon(itemNumber, 1, 0);
				}

				break;

			case TWOGUN_STATE_IDLE_TO_AIM:
				creature->Flags = 0;
				creature->MaxTurn = 0;
				head = laraAI.angle >> 1;
				yTorso = laraAI.angle >> 1;
				xTorso = AI.xAngle >> 1;

				if (abs(AI.angle) < 364)
					item->Pose.Orientation.y += AI.angle;
				else if (AI.angle >= 0)
					item->Pose.Orientation.y += 364;
				else
					item->Pose.Orientation.y -= 364;

				if (item->TriggerFlags == 2)
					item->Animation.TargetState = TWOGUN_STATE_MISSFIRE;
				else if (item->TriggerFlags == 3)
					item->Animation.TargetState = TWOGUN_STATE_GUN_BLOCKAGE;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = TWOGUN_STATE_SHOOT_BOTH;
				else
					item->Animation.TargetState = TWOGUN_STATE_IDLE;

				break;

			case TWOGUN_STATE_SHOOT_BOTH:
				head = laraAI.angle >> 1;
				yTorso = laraAI.angle >> 1;
				xTorso = AI.xAngle;

				if (abs(AI.angle) < 364)
					item->Pose.Orientation.y += AI.angle;
				else
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += 364;
					else
						item->Pose.Orientation.y -= 364;
				}

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 17)
				{
					FireTwogunWeapon(itemNumber, 0, 0);
					FireTwogunWeapon(itemNumber, 1, 0);
				}

				break;

			case TWOGUN_STATE_TURN_180:
				creature->Flags = 0;
				creature->MaxTurn = 0;

				if (AI.angle < 0)
					item->Pose.Orientation.y += 364;
				else
					item->Pose.Orientation.y -= 364;

				if (TestLastFrame(item))
					item->Pose.Orientation.y += 32768;

				break;

			case TWOGUN_STATE_MISSFIRE:
				xTorso = AI.xAngle >> 1;
				head = laraAI.angle >> 1;
				yTorso = laraAI.angle >> 1;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 18)
				{
					FireTwogunWeapon(itemNumber, 0, 0);
					item->ItemFlags[1] = -16;
					item->TriggerFlags = 3;
				}

				break;

			case TWOGUN_STATE_GUN_BLOCKAGE:
				item->TriggerFlags = 0;
				break;
			}
		}

		CreatureJoint(item, 0, yTorso);
		CreatureJoint(item, 1, xTorso);
		CreatureJoint(item, 2, head);
		CreatureAnimation(itemNumber, angle, 0);
	}

}
