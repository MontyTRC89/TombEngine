#include "framework.h"
#include "Objects/TR5/Entity/tr5_twogunlaser.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
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

using namespace TEN::Math::Random;
using namespace TEN::Effects::Spark;



namespace TEN::Entities::Creatures::TR5
{

	TWOGUNINFO twogun[2];
	BiteInfo Guns[2] =
	{
		{ 0, 230, 40, 8 },
		{ 8, 230, 40, 5 }
	};

	const auto LaserguardGun1 = BiteInfo(Vector3(0.0f, 230.0f, 40.0f), 8);
	const auto LaserguardGun2 = BiteInfo(Vector3(8.0f, 230.0f, 40.0f), 5);
	const auto LaserguardHead = BiteInfo(Vector3(0.0f, -200.0f, 0.0f), 2);


	enum LaserguardState
	{
		HYDRA_STATE_IDLE = 0,
		HYDRA_STATE_BITE_ATTACK_1 = 1,
		HYDRA_STATE_AIM = 2,
		HYDRA_STATE_HURT = 4,
		HYDRA_STATE_BITE_ATTACK_2 = 7,
		HYDRA_STATE_BITE_ATTACK_3 = 8,
		HYDRA_STATE_BITE_ATTACK_4 = 9,
		HYDRA_STATE_DEATH = 11
	};

	// TODO
	enum LaserguardAnim
	{
		TWOGUN_ANIM_EMPTY = -1,
		TWOGUN_ANIM_WALK = 0,
		TWOGUN_ANIM_SHOOT_R = 1,
		TWOGUN_ANIM_SHOOT_L = 2,
		TWOGUN_ANIM_DEATH = 3,
		TWOGUN_ANIM_AIM_TO_WALK = 4,
		TWOGUN_ANIM_WALK_AIM_TO_IDLE = 5,
		TWOGUN_ANIM_IDLE = 6,
		TWOGUN_ANIM_IDLE_TO_AIM = 7,
		TWOGUN_ANIM_AIM= 8,
		TWOGUN_ANIM_SHOOT_BOTH = 9,
		TWOGUN_ANIM_AIM_TO_IDLE = 10,
		TWOGUN_ANIM_TURN_180 = 11,
		TWOGUN_ANIM_MISSFIRE = 12,
		TWOGUN_ANIM_GUN_BLOCKAGE = 13,
		TWOGUN_ANIM_FALLSTAIRS = 14,
		TWOGUN_ANIM_FALLLOOP = 15,
		TWOGUN_ANIM_FALLDEATH = 16
	};

	void InitialiseTwogun(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		SetAnimation(item, 6);
	}

	void FireTwogunWeapon(ItemInfo* item, short LeftRight, short plasma)
	{
		auto* gun = &Guns[LeftRight];

		//PHD_VECTOR 	pos1, pos2, pos3;

		//PHD_ANGLE		angles[2];

		TWOGUNINFO* tg;
		//sint16* bounds;
		long lp;
		//dx, dy, dz, dx1, dy1, dz1, x, y, z;

		auto pos1 = GetJointPosition(item, gun->meshNum, Vector3i(gun->Position));
		auto pos2 = GetJointPosition(item, gun->meshNum, Vector3i(gun->Position.x, gun->Position.y + 4096, gun->Position.z)); //TODO placement
		auto pos = GetJointPosition(item, gun->meshNum, Vector3i(gun->Position.x, gun->Position.y, gun->Position.z));
		auto orient1 = Geometry::GetOrientToPoint(pos1.ToVector3(), pos.ToVector3());
		auto orient2 = Geometry::GetOrientToPoint(pos2.ToVector3(), pos.ToVector3());

		//phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, &angles[0]);

		if (!plasma)
		{
			tg = &twogun[0];
			for (lp = 0; lp < 4; lp++, tg++)
			{
				if (tg->life == 0 || lp == 3)
					break;
			}

			tg->pos.Position.x = pos1.x;
			tg->pos.Position.y = pos1.y;
			tg->pos.Position.z = pos1.z;
			tg->pos.Position.y = orient1.y;// angles[0];
			tg->pos.Position.x = orient1.x;//angles[1];
			tg->pos.Position.z = 0;
			tg->life = 17;
			tg->spin = (GetRandomControl() & 31) << 11;
			tg->dlength = 4096;
			tg->r = 0;
			tg->b = 255;
			tg->g = 96;
			tg->fadein = 8;

			//TriggerLightningGlow(tg->pos.x_pos, tg->pos.y_pos, tg->pos.z_pos, RGBME(0, (unsigned char)(tg->g >> 1), (unsigned char)(tg->b >> 1)) | ((64 + (GetRandomControl() & 3)) << 24));
			//TriggerLightning(&pos1, &pos2, (GetRandomControl() & 7) + 8, RGBME(0, (unsigned char)tg->g, (unsigned char)tg->b) | (0x16 << 24), LI_THININ | LI_THINOUT, 80, 5);

			item->ItemFlags[LeftRight] = 16;
			//TriggerTwogunPlasma(&pos1, &angles[0], 16);
			//TriggerTwogunPlasma(&pos1, &angles[0], 16);
			//TriggerTwogunPlasma(&pos1, &angles[0], 16);

			// Do some detection.

		/*	if (!lara.burn && SCNoDrawLara == 0)
			{
				bounds = GetBoundsAccurate(LaraItem);
				pos3.x = LaraItem->pos.x_pos + ((bounds[0] + bounds[1]) >> 1);
				pos3.y = LaraItem->pos.y_pos + ((bounds[2] + bounds[3]) >> 1);
				pos3.z = LaraItem->pos.z_pos + ((bounds[4] + bounds[5]) >> 1);

				dx = pos3.x - pos1.x;
				dy = pos3.y - pos1.y;
				dz = pos3.z - pos1.z;
				dx = sqrt((dx * dx) + (dy * dy) + (dz * dz));

				dz = 0;
				if (dx < 4096)
				{
					dx1 = (pos2.x - pos1.x) >> 4;
					dy1 = (pos2.y - pos1.y) >> 4;
					dz1 = (pos2.z - pos1.z) >> 4;
					x = pos1.x;
					y = pos1.y;
					z = pos1.z;
					for (lp = 0; lp < dx; lp += 256)
					{
						if (abs(pos3.x - x) < 320 &&
							abs(pos3.y - y) < 320 &&
							abs(pos3.z - z) < 320)
						{
							dz = 1;
							break;
						}
						x += dx1;
						y += dy1;
						z += dz1;
					}
				}*/

				/*if (dz)
				{
					if (LaraItem->hit_points < 501)
					{
						LaraBurn();
						lara.BurnCount = 48;
						lara.BurnBlue = 1;
						//if (LaraItem->hit_points > 64)
						LaraItem->hit_points = 0;
					}
					else
						LaraItem->hit_points -= 250;
				}*/
		}

			return;
			

			TriggerTwogunPlasma(Pose(pos1, orient1), abs(item->ItemFlags[LeftRight]));
	}

	void TriggerTwogunPlasma(const Pose& pos, long life)
	{
		//auto* spark = &twogun[0];
		//auto& spark = *twogun[0];
		//auto& spark = *GetFreeParticle();
		auto* spark = GetFreeParticle();
		//auto* spark = &SmokeSparks[GetFreeSmokeSpark()]
		Vector3 sp = Vector3(pos.Position.x, pos.Position.y, pos.Position.z);
		auto sphere = BoundingSphere(sp, BLOCK(1));
		auto posit = Random::GeneratePointInSphere(sphere);
		auto vel = (pos.Position - posit) * 8;
		//spark = &spark[GetFreeSpark()];

		spark->on = 1;

		spark->sB = (((GetRandomControl() & 127) + 128) * life) >> 4;
		spark->sR = spark->sB - (spark->sB >> 2);
		spark->sG = spark->sR;

		spark->dR = 0;
		spark->dB = (((GetRandomControl() & 127) + 32) * life) >> 4;
		spark->dG = spark->dB >> 2;

		spark->colFadeSpeed = 8 + (GetRandomControl() & 3);
		spark->fadeToBlack = 8;
		spark->sLife = spark->life = (GetRandomControl() & 3) + 24;

		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;



		float ang = TO_RAD(pos.Orientation.y);
		float vAng = -TO_RAD(pos.Orientation.x);
		spark->x = pos.Position.x;
		spark->y = pos.Position.y;
		spark->z = pos.Position.z;
		spark->xVel = (int)round(vel.x);
		spark->yVel = (int)round(vel.y);
		spark->zVel = (int)round(vel.z);
		spark->friction = 0;

		spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		spark->rotAng = GetRandomControl() & 4095;
		spark->rotAdd = (GetRandomControl() & 127) - 64;

		spark->gravity = (GetRandomControl() & 31) + 32;
		spark->maxYvel = 0;
		spark->scalar = 1;
		float size = (GetRandomControl() & 63) + 16;
		spark->size = spark->sSize = size;
		spark->dSize = 1;
	}


	


	void TwogunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;


		//long dx, dz;
		short angle, head, torso_x, torso_y, frame, base, roomNumber;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		//auto& creature = *GetCreatureInfo(item);

		angle = 0;
		head = 0;
		torso_x = 0;
		torso_y = 0;

		if (item->ItemFlags[0] || item->ItemFlags[1])
		{

			auto pos1 = GetJointPosition(item, Guns->meshNum, Vector3i(Guns->Position));

			for (int i = 0; i < 2; i++)
			{
				if (item->ItemFlags[i])
				{
					auto pos = GetJointPosition(item, Guns->meshNum, Vector3i(Guns->Position.x, Guns->Position.y * 2, Guns->Position.z));
					auto orient = Geometry::GetOrientToPoint(pos1.ToVector3(), pos.ToVector3());
					//SethProjectileAttack(Pose(pos1, orient), item->RoomNumber, 0);
					//FireTwogunWeapon(item, i, 1);

					if (item->ItemFlags[i] <= 0)
						item->ItemFlags[i]++;
					else
					{
						TriggerDynamicLight(pos.x, pos.y, pos.z, item->ItemFlags[i] + 8, 0, (item->ItemFlags[i] << 4) >> 2, item->ItemFlags[i] << 4);
						item->ItemFlags[i]--;
					}
				}
			}
		}

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7 && item->TriggerFlags != 1)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 7;
			}
			else if (item->TriggerFlags == 1)
			{
				switch (item->Animation.ActiveState)
				{
				case 11:
					frame = item->Animation.FrameNumber;
					base = g_Level.Anims[item->Animation.AnimNumber].frameBase;

					if (frame == base + 48 || frame == base + 15)
					{
						roomNumber = item->RoomNumber;
						//GetHeight(GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &room_number),
							//item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
						TestTriggers(item, true, 0);
					}

					break;

				case 12:
					item->Animation.IsAirborne = true;

					if (item->Pose.Position.y >= item->Floor)
					{
						item->Pose.Position.y = item->Floor;
						item->Animation.IsAirborne = false;
						item->Animation.Velocity.y = 0.0f;
						item->Animation.TargetState = 13;
						CreatureEffect2(item, LaserguardHead, 10, item->Pose.Orientation.y, DoBloodSplat);
					}

					break;

				case 13:
					item->Pose.Position.y = item->Floor;
					break;

				default:
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 14;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = 11;
					creature->LOT.IsJumping = true;
					auto* room = &g_Level.Rooms[item->RoomNumber];

					item->Pose.Position.x = room->x + ((creature->Tosspad & 0xFFFFFF00) << 2) + 512;
					item->Pose.Position.y = room->minfloor + (item->ItemFlags[2] & 0xFFFFFF00);
					item->Pose.Position.z = ((creature->Tosspad & 0xFF) << 10) + room->z + 512;
					item->Pose.Orientation.y = creature->Tosspad & 57344;
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
			case 1:
				creature->Flags = 0;
				creature->MaxTurn = 0;

				if (!(item->AIBits & 1))
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
					torso_y = laraAI.angle >> 1;
					torso_x = AI.xAngle >> 1;
				}

				if (item->AIBits & 1)
					head = AIGuard(creature);
				else
				{
					if (laraAI.angle > 20480 || laraAI.angle < -20480)
						item->Animation.TargetState = 8;
					else if (!Targetable(item, &AI))
					{
						if (item->TriggerFlags != 1)
							item->Animation.TargetState = 2;
					}
					else if (AI.distance < 0x900000 || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = 5;
					else if (item->AIBits != 8)
					{
						if (item->TriggerFlags != 1)
							item->Animation.TargetState = 2;
					}
				}

				break;

			case 2:
				creature->MaxTurn = 910;

				if (Targetable(item, &AI) && laraAI.angle < 6144 && laraAI.angle > -6144)
				{
					if (item->Animation.FrameNumber >= g_Level.Anims[item->Animation.AnimNumber].frameBase + 29)
						item->Animation.TargetState = 3;
					else
						item->Animation.TargetState = 4;
				}
				else
				{
					if (laraAI.angle > 20480 || laraAI.angle < -20480)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 2;
				}

				break;

			case 3:
			case 4:
				head = laraAI.angle;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				{
					if (item->Animation.ActiveState == 4)
						FireTwogunWeapon(item, 0, 0);
					else
						FireTwogunWeapon(item, 1, 0);
				}

				break;

			case 5:
				creature->Flags = 0;
				creature->MaxTurn = 0;
				head = laraAI.angle >> 1;
				torso_y = laraAI.angle >> 1;
				torso_x = AI.xAngle >> 1;

				if (abs(AI.angle) < 364)
					item->Pose.Orientation.y += AI.angle;
				else if (AI.angle >= 0)
					item->Pose.Orientation.y += 364;
				else
					item->Pose.Orientation.y -= 364;

				if (item->TriggerFlags == 2)
					item->Animation.TargetState = 9;
				else if (item->TriggerFlags == 3)
					item->Animation.TargetState = 10;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = 6;
				else
					item->Animation.TargetState = 1;

				break;

			case 6:
				head = laraAI.angle >> 1;
				torso_y = laraAI.angle >> 1;
				torso_x = AI.xAngle;

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
					FireTwogunWeapon(item, 0, 0);
					FireTwogunWeapon(item, 1, 0);
				}

				break;

			case 8:
				creature->Flags = 0;
				creature->MaxTurn = 0;

				if (AI.angle < 0)
					item->Pose.Orientation.y += 364;
				else
					item->Pose.Orientation.y -= 364;

				if (TestLastFrame(item))
					item->Pose.Orientation.y += 32768;

				break;

			case 9:
				torso_x = AI.xAngle >> 1;
				head = laraAI.angle >> 1;
				torso_y = laraAI.angle >> 1;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 18)
				{
					FireTwogunWeapon(item, 0, 0);
					item->ItemFlags[1] = -16;
					item->TriggerFlags = 3;
				}

				break;

			case 10:
				item->TriggerFlags = 0;
				break;
			}
		}

		CreatureJoint(item, 0, torso_y);
		CreatureJoint(item, 1, torso_x);
		CreatureJoint(item, 2, head);
		CreatureAnimation(itemNumber, angle, 0);
	}

	//void inject_twogun(bool replace)
	//{
	//	INJECT(0x0048E3C0, ControlZipController, replace);
	//	INJECT(0x0048CD40, InitialiseTwogun, replace);
	//	INJECT(0x0048CDD0, TwogunControl, replace);
	//}

}