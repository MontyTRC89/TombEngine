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

BITE_INFO SethaBite1 = { 0,220,50,17 };
BITE_INFO SethaBite2 = { 0,220,50,13 };
BITE_INFO SethaAttack1 = { -16,200,32,13 };
BITE_INFO SethaAttack2 = { 16,200,32,17 };

void InitialiseSetha(short itemNumber)
{
	ItemInfo* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);
	
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = 12;
	item->Animation.ActiveState = 12;
}

void SethaControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ItemInfo* item = &g_Level.Items[itemNumber];
	CreatureInfo* creature = (CreatureInfo*)item->Data;

	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y;
	int z = item->Pose.Position.z;

	int dx = 870 * phd_sin(item->Pose.Orientation.y);
	int dz = 870 * phd_cos(item->Pose.Orientation.y);

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(x, y, z, &roomNumber);
	int ceiling = GetCeiling(floor, x, y, z);

	x += dx;
	z += dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, y, z);

	bool canJump = (y < height1 - 384 || y < height2 - 384)
		&& (y < height3 + 256 && y > height3 - 256 || height3 == NO_HEIGHT);

	x = item->Pose.Position.x - dx;
	z = item->Pose.Position.z - dz;
	roomNumber = item->RoomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height4 = GetFloorHeight(floor, x, y, z);

	AI_INFO info;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
	}
	else
	{
		if (item->AIBits & AMBUSH)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

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
			else if (info.distance < SQUARE(1024) && info.bite)
			{
				item->Animation.TargetState = 8;
				break;
			}
			else if (LaraItem->Pose.Position.y >= item->Pose.Position.y - 1024)
			{
				if (info.distance < SQUARE(2560)
					&& info.ahead
					&& GetRandomControl() & 1
					&& Targetable(item, &info))
				{
					item->ItemFlags[0] = 0;
					item->Animation.TargetState = 11;
					break;
				}
				else if (ceiling != NO_HEIGHT
					&& ceiling < item->Pose.Position.y - 1792
					&& height4 != NO_HEIGHT
					&& height4 > item->Pose.Position.y - 1024
					&& GetRandomControl() & 1)
				{
					item->Pose.Position.y -= 1536;
					if (Targetable(item, &info))
					{
						item->ItemFlags[0] = 0;
						item->Pose.Position.y += 1536;
						item->Animation.TargetState = 12;
					}
					else
					{
						item->Animation.TargetState = 2;
						item->Pose.Position.y += 1536;
					}
					break;
				}
				else
				{
					if (info.distance < SQUARE(3072) && info.angle < 6144 && info.angle > -6144 && info.ahead)
					{
						if (Targetable(item, &info))
						{
							item->Animation.TargetState = 4;
							break;
						}
					}
					else if (info.distance < SQUARE(4096)
						&& info.angle < ANGLE(45)
						&& info.angle > -ANGLE(45)
						&& height4 != NO_HEIGHT
						&& height4 >= item->Pose.Position.y - 256
						&& Targetable(item, &info))
					{
						item->ItemFlags[0] = 0;
						item->Animation.TargetState = 13;
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
			creature->MaxTurn = ANGLE(7);
			if (info.bite 
				&& info.distance < SQUARE(4096) 
				|| canJump 
				|| creature->ReachedGoal)
			{
				item->Animation.TargetState = 1;
			}
			else if (info.distance > SQUARE(3072))
			{
				item->Animation.TargetState = 3;
			}
			break;

		case 3:
			creature->MaxTurn = ANGLE(11);
			if (info.bite 
				&& info.distance < SQUARE(4096)
				|| canJump 
				|| creature->ReachedGoal)
			{
				item->Animation.TargetState = 1;
			}
			else if (info.distance < SQUARE(3072))
			{
				item->Animation.TargetState = 2;
			}
			break;

		case 4:
			if (canJump)
			{
				if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 15
					&& item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				{
					creature->ReachedGoal = true;
					creature->MaxTurn = 0;
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
							LaraItem->HitPoints -= 200;
							LaraItem->HitStatus = true;
							creature->Flags = 1;
							CreatureEffect2(
								item,
								&SethaBite1,
								25,
								-1,
								DoBloodSplat);
						}

						if (item->TouchBits & 0xE0000)
						{
							LaraItem->HitPoints -= 200;
							LaraItem->HitStatus = true;
							creature->Flags = 1;
							CreatureEffect2(
								item,
								&SethaBite2,
								25,
								-1,
								DoBloodSplat);
						}
					}
				}
			}
			
			break;

		case 5:
			creature->ReachedGoal = true;
			creature->MaxTurn = 0;
			break;

		case 7:
			if (item->Animation.AnimNumber == Objects[item->Animation.AnimNumber].animIndex + 17 
				&& item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
			{
				if (GetRandomControl() & 1)
				{
					item->Animation.RequiredState = 10;
				}
			}
		
			break;

		case 8:
			creature->MaxTurn = 0;
			
			if (abs(info.angle) >= ANGLE(3))
			{
				if (info.angle >= 0)
				{
					item->Pose.Orientation.y += ANGLE(3);
				}
				else
				{
					item->Pose.Orientation.y -= ANGLE(3);
				}
			}
			else
			{
				item->Pose.Orientation.y += info.angle;
			}

			if (!creature->Flags)
			{
				if (item->TouchBits)
				{
					if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 15 
						&& item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 26)
					{
						LaraItem->HitPoints -= 250;
						LaraItem->HitStatus = true;
						creature->Flags = 1;
						CreatureEffect2(
							item,
							&SethaBite1,
							25,
							-1,
							DoBloodSplat);
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
			if (item->Animation.ActiveState==15)
				creature->Target.y = LaraItem->Pose.Position.y;
		
			creature->MaxTurn = 0;

			if (abs(info.angle) >= ANGLE(3))
			{
				if (info.angle >= 0)
				{
					item->Pose.Orientation.y += ANGLE(3);
				}
				else
				{
					item->Pose.Orientation.y -= ANGLE(3);
				}
				SethaAttack(itemNumber);
			}
			else
			{
				item->Pose.Orientation.y += info.angle;
				SethaAttack(itemNumber);
			}

			break;

		case 14:
			if (item->Animation.AnimNumber != Objects[item->Animation.AnimNumber].animIndex + 26)
			{
				creature->LOT.Fly = 16;
				item->Animation.Airborne = false;
				creature->MaxTurn = 0;
				creature->Target.y = LaraItem->Pose.Position.y;

				if (abs(info.angle) >= ANGLE(3))
				{
					if (info.angle >= 0)
					{
						item->Pose.Orientation.y += ANGLE(3);
					}
					else
					{
						item->Pose.Orientation.y -= ANGLE(3);
					}
				}
				else
				{
					item->Pose.Orientation.y += info.angle;
				}
			}

			if (LaraItem->Pose.Position.y <= item->Floor - 512)
			{
				if (Targetable(item, &info))
				{
					item->ItemFlags[0] = 0;
					item->Animation.TargetState = 15;
				}
			}
			else
			{
				creature->LOT.Fly = 0;
				item->Animation.Airborne = true;
				if (item->Pose.Position.y - item->Floor > 0)
				{
					item->Animation.TargetState = 1;
				}
			}

			break;

		default:
			break;

		}
	}

	if (item->HitStatus)
	{
		if ((Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun || Lara.Control.Weapon.GunType == LaraWeaponType::Revolver)
			&& info.distance < SQUARE(2048)
			&& !(creature->LOT.IsJumping))
		{
			if (item->Animation.ActiveState != 12)
			{
				if (item->Animation.ActiveState <= 13)
				{
					if (abs(height4 - item->Pose.Position.y) >= 512)
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 11;
						item->Animation.TargetState = 6;
						item->Animation.ActiveState = 6;
					}
					else
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 17;
						item->Animation.TargetState = 7;
						item->Animation.ActiveState = 7;
					}
				}
				else
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 25;
					item->Animation.TargetState = 16;
					item->Animation.ActiveState = 16;
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
	
	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];
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
		spark->transType = TransTypeEnum::COLADD;
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

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = 1;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dR = 0;
		spark->dG = (GetRandomControl() & 0x7F) + 32;
		spark->dB = spark->dG + 64;
		spark->fadeToBlack = 8;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
		spark->transType = TransTypeEnum::COLADD;
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
		{
			spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
		}
		else
		{
			spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
		}
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 0x1F) + 16;
		spark->fxObj = itemNumber;
		spark->nodeNumber = node;
		spark->scalar = 2;
		spark->sSize = spark->size = GetRandomControl() & 0xF + size;
		spark->dSize = spark->size / 16;
	}
}

void SethaThrowAttack(PHD_3DPOS* pos, short roomNumber, short mesh)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != -1)
	{
		FX_INFO* fx = &EffectList[fxNumber];

		fx->pos.Position.x = pos->Position.x;
		fx->pos.Position.y = pos->Position.y - (GetRandomControl() & 0x3F) - 32;
		fx->pos.Position.z = pos->Position.z;
		fx->pos.Orientation.x = pos->Orientation.x;
		fx->pos.Orientation.y = pos->Orientation.y;
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
	ItemInfo* item = &g_Level.Items[itemNumber];

	item->ItemFlags[0]++;

	Vector3Int pos1;
	pos1.x = SethaAttack1.x;
	pos1.y = SethaAttack1.y;
	pos1.z = SethaAttack1.z;
	GetJointAbsPosition(item, &pos1, SethaAttack1.meshNum);

	Vector3Int pos2;
	pos2.x = SethaAttack2.x;
	pos2.y = SethaAttack2.y;
	pos2.z = SethaAttack2.z;
	GetJointAbsPosition(item, &pos2, SethaAttack2.meshNum);

	int i, size;
	Vector3Int pos;
	short angles[2];
	PHD_3DPOS attackPos;

	switch (item->Animation.ActiveState)
	{
	case 11:
	case 15:
		if (item->ItemFlags[0] < 78 && (GetRandomControl() & 0x1F) < item->ItemFlags[0])
		{
			for (i = 0; i < 2; i++)
			{
				pos.x = (GetRandomControl() & 0x7FF) + pos1.x - 1024;
				pos.y = (GetRandomControl() & 0x7FF) + pos1.y - 1024;
				pos.z = (GetRandomControl() & 0x7FF) + pos1.z - 1024;

				TriggerSethaSparks1(
					pos.x,
					pos.y,
					pos.z,
					8 * (pos1.x - pos.x),
					8 * (pos1.y - pos.y),
					8 * (1024 - (GetRandomControl() & 0x7FF)));

				pos.x = (GetRandomControl() & 0x7FF) + pos2.x - 1024;
				pos.y = (GetRandomControl() & 0x7FF) + pos2.y - 1024;
				pos.z = (GetRandomControl() & 0x7FF) + pos2.z - 1024;

				TriggerSethaSparks1(
					pos.x,
					pos.y,
					pos.z,
					8 * (pos2.x - pos.x),
					8 * (pos2.y - pos.y),
					8 * (1024 - (GetRandomControl() & 0x7FF)));
			}
		}
		
		size = 2 * item->ItemFlags[0];
		if (size > 128)
			size = 128;

		if ((Wibble & 0xF) == 8)
		{
			if (item->ItemFlags[0] < 127)
			{
				TriggerSethaSparks2(itemNumber, 2, size);
			}
		}
		else if (!(Wibble & 0xF) 
			&& item->ItemFlags[0] < 103)
		{
			TriggerSethaSparks2(itemNumber, 3, size);
		}

		if (item->ItemFlags[0] >= 96 && item->ItemFlags[0] <= 99)
		{
			pos.x = SethaAttack1.x;
			pos.y = 2 * SethaAttack1.y;
			pos.z = SethaAttack1.z;
			GetJointAbsPosition(item, &pos, SethaAttack1.meshNum);

			phd_GetVectorAngles(pos.x - pos1.x, pos.y - pos1.y, pos.z - pos1.z, angles);

			attackPos.Position.x = pos1.x;
			attackPos.Position.y = pos1.y;
			attackPos.Position.z = pos1.z;
			attackPos.Orientation.x = angles[1];
			attackPos.Orientation.y = angles[0];

			SethaThrowAttack(&attackPos, item->RoomNumber, 0);
		}
		else if (item->ItemFlags[0] >= 122 && item->ItemFlags[0] <= 125)
		{
			pos.x = SethaAttack2.x;
			pos.y = 2 * SethaAttack2.y;
			pos.z = SethaAttack2.z;
			GetJointAbsPosition(item, &pos, SethaAttack2.meshNum);

			phd_GetVectorAngles(pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z, angles);

			attackPos.Position.x = pos2.x;
			attackPos.Position.y = pos2.y;
			attackPos.Position.z = pos2.z;
			attackPos.Orientation.x = angles[1];
			attackPos.Orientation.y = angles[0];

			SethaThrowAttack(&attackPos, item->RoomNumber, 0);
		}
		
		break;

	case 12:
		size = 4 * item->ItemFlags[0];
		if (size > 160)
			size = 160;

		if ((Wibble & 0xF) == 8)
		{
			if (item->ItemFlags[0] < 132)
			{
				TriggerSethaSparks2(itemNumber, 2, size);
			}
		}
		else if (!(Wibble & 0xF) && item->ItemFlags[0] < 132)
		{
			TriggerSethaSparks2(itemNumber, 3, size);
		}
		
		if (item->ItemFlags[0] >= 60 && item->ItemFlags[0] <= 74
			|| item->ItemFlags[0] >= 112 && item->ItemFlags[0] <= 124)
		{
			if (Wibble & 4)
			{
				pos.x = SethaAttack1.x;
				pos.y = 2 * SethaAttack1.y;
				pos.z = SethaAttack1.z;
				GetJointAbsPosition(item, &pos, SethaAttack1.meshNum);

				phd_GetVectorAngles(pos.x - pos1.x, pos.y - pos1.y, pos.z - pos1.z, angles);

				attackPos.Position.x = pos1.x;
				attackPos.Position.y = pos1.y;
				attackPos.Position.z = pos1.z;
				attackPos.Orientation.x = angles[1];
				attackPos.Orientation.y = angles[0];

				SethaThrowAttack(&attackPos, item->RoomNumber, 0);

				pos.x = SethaAttack2.x;
				pos.y = 2 * SethaAttack2.y;
				pos.z = SethaAttack2.z;
				GetJointAbsPosition(item, &pos, SethaAttack2.meshNum);

				phd_GetVectorAngles(pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z, angles);

				attackPos.Position.x = pos2.x;
				attackPos.Position.y = pos2.y;
				attackPos.Position.z = pos2.z;
				attackPos.Orientation.x = angles[1];
				attackPos.Orientation.y = angles[0];

				SethaThrowAttack(&attackPos, item->RoomNumber, 0);
			}
		}

		break;

	case 13:
		if (item->ItemFlags[0] > 40
			&& item->ItemFlags[0] < 100
			&& (GetRandomControl() & 7) < item->ItemFlags[0] - 40)
		{
			for (i = 0; i < 2; i++)
			{
				pos.x = (GetRandomControl() & 0x7FF) + pos1.x - 1024;
				pos.y = (GetRandomControl() & 0x7FF) + pos1.y - 1024;
				pos.z = (GetRandomControl() & 0x7FF) + pos1.z - 1024;

				TriggerSethaSparks1(
					pos.x,
					pos.y,
					pos.z,
					8 * (pos1.x - pos.x),
					8 * (pos1.y - pos.y),
					8 * (1024 - (GetRandomControl() & 0x7FF)));

				pos.x = (GetRandomControl() & 0x7FF) + pos2.x - 1024;
				pos.y = (GetRandomControl() & 0x7FF) + pos2.y - 1024;
				pos.z = (GetRandomControl() & 0x7FF) + pos2.z - 1024;

				TriggerSethaSparks1(
					pos.x,
					pos.y,
					pos.z,
					8 * (pos2.x - pos.x),
					8 * (pos2.y - pos.y),
					8 * (1024 - (GetRandomControl() & 0x7FF)));
			}
		}
		
		size = 2 * item->ItemFlags[0];
		if (size> 128)
			size = 128;

		if ((Wibble & 0xF) == 8)
		{
			if (item->ItemFlags[0] < 103)
			{
				TriggerSethaSparks2(itemNumber, 2, size);
			}
		}
		else if (!(Wibble & 0xF) && item->ItemFlags[0] < 103)
		{
			TriggerSethaSparks2(itemNumber, 3, size);
		}
		if (item->ItemFlags[0] == 102)
		{
			pos.x = SethaAttack1.x;
			pos.y = 2 * SethaAttack1.y;
			pos.z = SethaAttack1.z;
			GetJointAbsPosition(item, &pos, SethaAttack1.meshNum);

			phd_GetVectorAngles(pos.x - pos1.x, pos.y - pos1.y, pos.z - pos1.z, angles);

			attackPos.Position.x = pos1.x;
			attackPos.Position.y = pos1.y;
			attackPos.Position.z = pos1.z;
			attackPos.Orientation.x = angles[1];
			attackPos.Orientation.y = angles[0];

			SethaThrowAttack(&attackPos, item->RoomNumber, 0);
		}

		break;

	default:
		break;

	}
}