#include "framework.h"
#include "tr4_setha.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Game/itemdata/creature_info.h"
#include "Game/animation.h"
#include "Game/misc.h"

namespace TEN::Entities::TR4
{
	BITE_INFO SethaBite1 = { 0, 220, 50, 17 };
	BITE_INFO SethaBite2 = { 0, 220, 50, 13 };
	BITE_INFO SethaAttack1 = { -16, 200, 32, 13 };
	BITE_INFO SethaAttack2 = { 16, 200, 32, 17 };

	void InitialiseSetha(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = 12;
		item->Animation.TargetState = 12;
	}

	void SethaControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;

		int dx = 870 * phd_sin(item->Pose.Orientation.y);
		int dz = 870 * phd_cos(item->Pose.Orientation.y);

		int ceiling = GetCollision(x, y, z, item->RoomNumber).Position.Ceiling;

		x += dx;
		z += dz;
		int height1 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height2 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height3 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		bool canJump = false;
		if ((y < (height1 - CLICK(1.5f)) || y < (height2 - CLICK(1.5f))) &&
			(y < (height3 + CLICK(1)) && y > (height3 - CLICK(1)) || height3 == NO_HEIGHT))
		{
			canJump = true;
		}

		x = item->Pose.Position.x - dx;
		z = item->Pose.Position.z - dz;
		int height4 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		AI_INFO AI;
		short angle = 0;

		if (item->HitPoints <= 0)
			item->HitPoints = 0;
		else
		{
			if (item->AIBits & AMBUSH)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->LOT.IsJumping = false;
				creature->Flags = 0;

				if (item->Animation.RequiredState)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					break;
				}
				else if (AI.distance < pow(SECTOR(1), 2) && AI.bite)
				{
					item->Animation.TargetState = 8;
					break;
				}
				else if (LaraItem->Pose.Position.y >= (item->Pose.Position.y - SECTOR(1)))
				{
					if (AI.distance < pow(SECTOR(2.5f), 2) &&
						AI.ahead &&
						GetRandomControl() & 1 &&
						Targetable(item, &AI))
					{
						item->Animation.TargetState = 11;
						item->ItemFlags[0] = 0;
						break;
					}
					else if (ceiling != NO_HEIGHT &&
						ceiling < (item->Pose.Position.y - SECTOR(1.75f)) &&
						height4 != NO_HEIGHT &&
						height4 > (item->Pose.Position.y - SECTOR(1)) &&
						GetRandomControl() & 1)
					{
						item->Pose.Position.y -= SECTOR(1.5f);
						if (Targetable(item, &AI))
						{
							item->Pose.Position.y += SECTOR(1.5f);
							item->Animation.TargetState = 12;
							item->ItemFlags[0] = 0;
						}
						else
						{
							item->Pose.Position.y += SECTOR(1.5f);
							item->Animation.TargetState = 2;
						}

						break;
					}
					else
					{
						if (AI.distance < pow(SECTOR(3), 2) &&
							AI.angle < SECTOR(6) &&
							AI.angle > -SECTOR(6) &&
							AI.ahead)
						{
							if (Targetable(item, &AI))
							{
								item->Animation.TargetState = 4;
								break;
							}
						}
						else if (AI.distance < pow(SECTOR(4), 2) &&
							AI.angle < ANGLE(45.0f) &&
							AI.angle > -ANGLE(45.0f) &&
							height4 != NO_HEIGHT &&
							height4 >= (item->Pose.Position.y - CLICK(1)) &&
							Targetable(item, &AI))
						{
							item->Animation.TargetState = 13;
							item->ItemFlags[0] = 0;
							break;
						}
						else if (canJump)
						{
							item->Animation.TargetState = 5;
							break;
						}
					}
				}
				else
				{
					if (creature->ReachedGoal)
					{
						item->Animation.TargetState = 14;
						break;
					}
					else
					{
						item->AIBits = AMBUSH;
						creature->HurtByLara = true;
					}
				}

				item->Animation.TargetState = 2;
				break;

			case 2u:
				creature->MaxTurn = ANGLE(7.0f);

				if (AI.bite &&
					AI.distance < pow(SECTOR(4), 2) ||
					canJump ||
					creature->ReachedGoal)
				{
					item->Animation.TargetState = 1;
				}
				else if (AI.distance > pow(SECTOR(3), 2))
					item->Animation.TargetState = 3;
				
				break;

			case 3:
				creature->MaxTurn = ANGLE(11.0f);

				if (AI.bite &&
					AI.distance < pow(SECTOR(4), 2) ||
					canJump ||
					creature->ReachedGoal)
				{
					item->Animation.TargetState = 1;
				}
				else if (AI.distance < pow(SECTOR(3), 2))
					item->Animation.TargetState = 2;
				
				break;

			case 4:
				if (canJump)
				{
					if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 15 &&
						item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
					{
						creature->MaxTurn = 0;
						creature->ReachedGoal = true;
					}
				}

				if (!creature->Flags)
				{
					if (item->TouchBits)
					{
						if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 16)
						{
							if (item->TouchBits & 0xE000)
							{
								DoDamage(creature->Enemy, 200);
								CreatureEffect2(
									item,
									&SethaBite1,
									25,
									-1,
									DoBloodSplat);
								creature->Flags = 1;
							}

							if (item->TouchBits & 0xE0000)
							{
								DoDamage(creature->Enemy, 200);
								CreatureEffect2(
									item,
									&SethaBite2,
									25,
									-1,
									DoBloodSplat);
								creature->Flags = 1;
							}
						}
					}
				}

				break;

			case 5:
				creature->MaxTurn = 0;
				creature->ReachedGoal = true;
				break;

			case 7:
				if (item->Animation.AnimNumber == Objects[item->Animation.AnimNumber].animIndex + 17 &&
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				{
					if (GetRandomControl() & 1)
						item->Animation.RequiredState = 10;
				}

				break;

			case 8:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= ANGLE(3.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(3.0f);
					else
						item->Pose.Orientation.y -= ANGLE(3.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (!creature->Flags)
				{
					if (item->TouchBits)
					{
						if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 15 &&
							item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 26)
						{
							DoDamage(creature->Enemy, 250);
							CreatureEffect2(
								item,
								&SethaBite1,
								25,
								-1,
								DoBloodSplat);
							creature->Flags = 1;
						}
					}
				}

				if (LaraItem->HitPoints < 0)
				{
					CreatureKill(item, 14, 9, 443);
					creature->MaxTurn = 0;
					return;
				}

				break;

			case 11:
			case 12:
			case 13:
			case 15:
				creature->MaxTurn = 0;

				if (item->Animation.ActiveState == 15)
					creature->Target.y = LaraItem->Pose.Position.y;

				if (abs(AI.angle) >= ANGLE(3.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(3.0f);
					else
						item->Pose.Orientation.y -= ANGLE(3.0f);
					
					SethaAttack(itemNumber);
				}
				else
				{
					item->Pose.Orientation.y += AI.angle;
					SethaAttack(itemNumber);
				}

				break;

			case 14:
				if (item->Animation.AnimNumber != Objects[item->Animation.AnimNumber].animIndex + 26)
				{
					item->Animation.IsAirborne = false;
					creature->MaxTurn = 0;
					creature->Target.y = LaraItem->Pose.Position.y;
					creature->LOT.Fly = 16;

					if (abs(AI.angle) >= ANGLE(3.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(3.0f);
						else
							item->Pose.Orientation.y -= ANGLE(3.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}

				if (LaraItem->Pose.Position.y <= (item->Floor - SECTOR(0.5f)))
				{
					if (Targetable(item, &AI))
					{
						item->ItemFlags[0] = 0;
						item->Animation.TargetState = 15;
					}
				}
				else
				{
					item->Animation.IsAirborne = true;
					creature->LOT.Fly = 0;

					if ((item->Pose.Position.y - item->Floor) > 0)
						item->Animation.TargetState = 1;
				}

				break;

			default:
				break;
			}
		}

		if (item->HitStatus)
		{
			if ((Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun || Lara.Control.Weapon.GunType == LaraWeaponType::Revolver) &&
				AI.distance < pow(SECTOR(2), 2) &&
				!(creature->LOT.IsJumping))
			{
				if (item->Animation.ActiveState != 12)
				{
					if (item->Animation.ActiveState <= 13)
					{
						if (abs(height4 - item->Pose.Position.y) >= SECTOR(0.5f))
						{
							item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 11;
							item->Animation.ActiveState = 6;
							item->Animation.TargetState = 6;
						}
						else
						{
							item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 17;
							item->Animation.ActiveState = 7;
							item->Animation.TargetState = 7;
						}
					}
					else
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 25;
						item->Animation.ActiveState = 16;
						item->Animation.TargetState = 16;
					}

					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				}
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}

	void TriggerSethaSparks1(int x, int y, int z, short xv, short yv, short zv)
	{
		int dx = LaraItem->Pose.Position.x - x;
		int dz = LaraItem->Pose.Position.z - z;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = GetFreeParticle();

			spark->on = 1;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dR = 64;
			spark->dG = (GetRandomControl() & 0x7F) + 64;
			spark->dB = spark->dG + 32;
			spark->life = 16;
			spark->sLife = 16;
			spark->colFadeSpeed = 4;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->fadeToBlack = 4;
			spark->x = x;
			spark->y = y;
			spark->z = z;
			spark->xVel = xv;
			spark->yVel = yv;
			spark->zVel = zv;
			spark->friction = 34;
			spark->scalar = 1;
			spark->sSize = spark->size = (GetRandomControl() & 3) + 4;
			spark->maxYvel = 0;
			spark->gravity = 0;
			spark->dSize = (GetRandomControl() & 1) + 1;
			spark->flags = 0;
		}
	}

	void TriggerSethaSparks2(short itemNumber, char node, int size)
	{
		int dx = LaraItem->Pose.Position.x - g_Level.Items[itemNumber].Pose.Position.x;
		int dz = LaraItem->Pose.Position.z - g_Level.Items[itemNumber].Pose.Position.z;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = GetFreeParticle();

			spark->on = 1;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dR = 0;
			spark->dG = (GetRandomControl() & 0x7F) + 32;
			spark->dB = spark->dG + 64;
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->life = spark->sLife = (GetRandomControl() & 7) + 20;
			spark->x = (GetRandomControl() & 0xF) - 8;
			spark->y = 0;
			spark->z = (GetRandomControl() & 0xF) - 8;
			spark->xVel = GetRandomControl() - 128;
			spark->yVel = 0;
			spark->zVel = GetRandomControl() - 128;
			spark->friction = 5;
			spark->flags = SP_NODEATTACH | SP_EXPDEF | SP_ITEM | SP_ROTATE | SP_SCALE | SP_DEF;
			spark->rotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			else
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;

			spark->maxYvel = 0;
			spark->gravity = (GetRandomControl() & 0x1F) + 16;
			spark->fxObj = itemNumber;
			spark->nodeNumber = node;
			spark->scalar = 2;
			spark->sSize = spark->size = GetRandomControl() & 0xF + size;
			spark->dSize = spark->size / 16;
		}
	}

	void SethaThrowAttack(PHD_3DPOS* pose, short roomNumber, short mesh)
	{
		short fxNumber = CreateNewEffect(roomNumber);
		if (fxNumber != -1)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.Position.x = pose->Position.x;
			fx->pos.Position.y = pose->Position.y - (GetRandomControl() & 0x3F) - 32;
			fx->pos.Position.z = pose->Position.z;
			fx->pos.Orientation.x = pose->Orientation.x;
			fx->pos.Orientation.y = pose->Orientation.y;
			fx->pos.Orientation.z = 0;
			fx->roomNumber = roomNumber;
			fx->counter = 2 * GetRandomControl() + -ANGLE(180);
			fx->flag1 = mesh;
			fx->objectNumber = ID_BODY_PART;
			fx->speed = (GetRandomControl() & 0x1F) - (mesh != 1 ? 0 : 64) + 96;
			fx->frameNumber = Objects[ID_BODY_PART].meshIndex + mesh;
		}
	}

	void SethaAttack(int itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->ItemFlags[0]++;

		auto pos1 = Vector3Int(SethaAttack1.x, SethaAttack1.y, SethaAttack1.z);
		GetJointAbsPosition(item, &pos1, SethaAttack1.meshNum);

		auto pos2 = Vector3Int(SethaAttack2.x, SethaAttack2.y, SethaAttack2.z);
		GetJointAbsPosition(item, &pos2, SethaAttack2.meshNum);

		int size;

		switch (item->Animation.ActiveState)
		{
		case 11:
		case 15:
			if (item->ItemFlags[0] < 78 && (GetRandomControl() & 0x1F) < item->ItemFlags[0])
			{
				for (int i = 0; i < 2; i++)
				{
					auto pos = Vector3Int(
						(GetRandomControl() & 0x7FF) + pos1.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x,
						pos.y,
						pos.z,
						(pos1.x - pos.x),
						(pos1.y - pos.y),
						(SECTOR(1) - (GetRandomControl() & 0x7FF)));

					pos = Vector3Int(
						(GetRandomControl() & 0x7FF) + pos2.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x,
						pos.y,
						pos.z,
						(pos2.x - pos.x) * 8,
						(pos2.y - pos.y) * 8,
						(SECTOR(1) - (GetRandomControl() & 0x7FF)) * 8);
				}
			}

			size = item->ItemFlags[0] * 2;
			if (size > 128)
				size = 128;

			if ((Wibble & 0xF) == 8)
			{
				if (item->ItemFlags[0] < 127)
					TriggerSethaSparks2(itemNumber, 2, size);
			}
			else if (!(Wibble & 0xF) && item->ItemFlags[0] < 103)
				TriggerSethaSparks2(itemNumber, 3, size);

			if (item->ItemFlags[0] >= 96 && item->ItemFlags[0] <= 99)
			{
				auto pos = Vector3Int(SethaAttack1.x, SethaAttack1.y * 2, SethaAttack1.z);
				GetJointAbsPosition(item, &pos, SethaAttack1.meshNum);

				auto angles = GetVectorAngles(pos.x - pos1.x, pos.y - pos1.y, pos.z - pos1.z);
				auto attackPose = PHD_3DPOS(pos1, angles);
				SethaThrowAttack(&attackPose, item->RoomNumber, 0);
			}
			else if (item->ItemFlags[0] >= 122 && item->ItemFlags[0] <= 125)
			{
				auto pos = Vector3Int(SethaAttack2.x, SethaAttack2.y * 2, SethaAttack2.z);
				GetJointAbsPosition(item, &pos, SethaAttack2.meshNum);

				auto angles = GetVectorAngles(pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z);
				auto attackPose = PHD_3DPOS(pos2, angles);
				SethaThrowAttack(&attackPose, item->RoomNumber, 0);
			}

			break;

		case 12:
			size = item->ItemFlags[0] * 4;
			if (size > 160)
				size = 160;

			if ((Wibble & 0xF) == 8)
			{
				if (item->ItemFlags[0] < 132)
					TriggerSethaSparks2(itemNumber, 2, size);
			}
			else if (!(Wibble & 0xF) && item->ItemFlags[0] < 132)
				TriggerSethaSparks2(itemNumber, 3, size);

			if (item->ItemFlags[0] >= 60 && item->ItemFlags[0] <= 74 ||
				item->ItemFlags[0] >= 112 && item->ItemFlags[0] <= 124)
			{
				if (Wibble & 4)
				{
					auto pos = Vector3Int(SethaAttack1.x, SethaAttack1.y * 2, SethaAttack1.z);
					GetJointAbsPosition(item, &pos, SethaAttack1.meshNum);

					auto angles = GetVectorAngles(pos.x - pos1.x, pos.y - pos1.y, pos.z - pos1.z);
					auto attackPose = PHD_3DPOS(pos1, angles);
					SethaThrowAttack(&attackPose, item->RoomNumber, 0);

					pos = Vector3Int(SethaAttack2.x, SethaAttack2.y * 2, SethaAttack2.z);
					GetJointAbsPosition(item, &pos, SethaAttack2.meshNum);

					angles = GetVectorAngles(pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z);
					attackPose = PHD_3DPOS(pos2, angles);
					SethaThrowAttack(&attackPose, item->RoomNumber, 0);
				}
			}

			break;

		case 13:
			if (item->ItemFlags[0] > 40 &&
				item->ItemFlags[0] < 100 &&
				(GetRandomControl() & 7) < item->ItemFlags[0] - 40)
			{
				for (int i = 0; i < 2; i++)
				{
					auto pos = Vector3Int(
						(GetRandomControl() & 0x7FF) + pos1.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x,
						pos.y,
						pos.z,
						(pos1.x - pos.x),
						(pos1.y - pos.y),
						(SECTOR(1) - (GetRandomControl() & 0x7FF)));

					pos = Vector3Int(
						(GetRandomControl() & 0x7FF) + pos2.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x,
						pos.y,
						pos.z,
						(pos2.x - pos.x),
						(pos2.y - pos.y),
						(SECTOR(1) - (GetRandomControl() & 0x7FF)));
				}
			}

			size = item->ItemFlags[0] * 2;
			if (size > 128)
				size = 128;

			if ((Wibble & 0xF) == 8)
			{
				if (item->ItemFlags[0] < 103)
					TriggerSethaSparks2(itemNumber, 2, size);
			}
			else if (!(Wibble & 0xF) && item->ItemFlags[0] < 103)
				TriggerSethaSparks2(itemNumber, 3, size);

			if (item->ItemFlags[0] == 102)
			{
				auto pos = Vector3Int(SethaAttack1.x, SethaAttack1.y * 2, SethaAttack1.z);
				GetJointAbsPosition(item, &pos, SethaAttack1.meshNum);

				auto angles = GetVectorAngles(pos.x - pos1.x, pos.y - pos1.y, pos.z - pos1.z);
				auto attackPose = PHD_3DPOS(pos1, angles);
				SethaThrowAttack(&attackPose, item->RoomNumber, 0);
			}

			break;

		default:
			break;
		}
	}
}
