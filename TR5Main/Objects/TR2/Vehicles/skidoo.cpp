#include "framework.h"
#include "Objects/TR2/Vehicles/skidoo.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/particle/SimpleParticle.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Sound/sound.h"

using std::vector;
using namespace TEN::Math::Random;

#define DAMAGE_START	140
#define DAMAGE_LENGTH	14

#define SKIDOO_DISMOUNT_DISTANCE 295

#define SKIDOO_UNDO_TURN			ANGLE(2.0f)
#define SKIDOO_TURN					(ANGLE(0.5f) + SKIDOO_UNDO_TURN)
#define SKIDOO_MAX_TURN				ANGLE(6.0f)
#define SKIDOO_MOMENTUM_TURN		ANGLE(3.0f)
#define SKIDOO_MAX_MOMENTUM_TURN	ANGLE(150.0f)

#define SKIDOO_FAST_VELOCITY	150
#define SKIDOO_MAX_VELOCITY		100
#define SKIDOO_SLOW_VELOCITY	50
#define SKIDOO_MIN_VELOCITY		15

#define SKIDOO_ACCELERATION	10
#define SKIDOO_BRAKE		5
#define SKIDOO_SLOWDOWN		2
#define SKIDOO_REVERSE		-5
#define SKIDOO_MAX_BACK		-30
#define SKIDOO_MAX_KICK		-80

#define SKIDOO_SLIP			100
#define SKIDOO_SLIP_SIDE	50
#define SKIDOO_FRONT		550
#define SKIDOO_SIDE			260
#define SKIDOO_RADIUS		500
#define SKIDOO_SNOW			500

#define SKIDOO_MAX_HEIGHT CLICK(1)
#define SKIDOO_MIN_BOUNCE ((SKIDOO_MAX_VELOCITY / 2) / 256)

#define SKIDOO_IN_FIRE			IN_ACTION
#define SKIDOO_IN_DISMOUNT		(IN_JUMP | IN_ROLL)
#define SKIDOO_IN_SLOW			IN_WALK
#define SKIDOO_IN_ACCELERATE	IN_FORWARD
#define SKIDOO_IN_BRAKE			IN_BACK
#define SKIDOO_IN_LEFT			IN_LEFT
#define SKIDOO_IN_RIGHT			IN_RIGHT

enum SkidooState 
{ 
	SKIDOO_STATE_SIT = 0,
	SKIDOO_STATE_MOUNT = 1,
	SKIDOO_STATE_LEFT = 2,
	SKIDOO_STATE_RIGHT = 3,
	SKIDOO_STATE_FALL = 4,
	SKIDOO_STATE_HIT = 5,
	SKIDOO_STATE_MOUNT_LEFT = 6,
	SKIDOO_STATE_DISMOUNT_LEFT = 7,
	SKIDOO_STATE_IDLE = 8,
	SKIDOO_STATE_DISMOUNT_RIGHT = 9,
	SKIDOO_STATE_JUMP_OFF = 10,
	SKIDOO_STATE_DEATH = 11,
	SKIDOO_STATE_FALLOFF = 12
};

enum SkidooAnim
{
	SKIDOO_ANIM_DRIVE = 0,
	SKIDOO_ANIM_MOUNT_RIGHT = 1,
	SKIDOO_ANIM_TURN_LEFT_START = 2,
	SKIDOO_ANIM_TURN_LEFT_CONTINUE = 3,
	SKIDOO_ANIM_TURN_LEFT_END = 4,
	SKIDOO_ANIM_TURN_RIGHT_START = 5,
	SKIDOO_ANIM_TURN_RIGHT_CONTINUE = 6,
	SKIDOO_ANIM_TURN_RIGHT_END = 7,
	SKIDOO_ANIM_LEAP_START = 8,
	SKIDOO_ANIM_LEAP_END = 9,
	SKIDOO_ANIM_LEAP_CONTINUE = 10,
	SKIDOO_ANIM_HIT_LEFT = 11,
	SKIDOO_ANIM_HIT_RIGHT = 12,
	SKIDOO_ANIM_HIT_FRONT = 13,
	SKIDOO_ANIM_HIT_BACK = 14,
	SKIDOO_ANIM_IDLE = 15,
	SKIDOO_ANIM_DISMOUNT_RIGHT = 16,
	SKIDOO_ANIM_UNK = 17, // TODO
	SKIDOO_ANIM_MOUNT_LEFT = 18,
	SKIDOO_ANIM_DISMOUNT_LEFT = 19,
	SKIDOO_ANIM_FALL_OFF = 20,
	SKIDOO_ANIM_IDLE_DEATH = 21,
	SKIDOO_ANIM_FALL_DEATH = 22
};

void InitialiseSkidoo(short itemNumber)
{
	auto* skidooItem = &g_Level.Items[itemNumber];
	skidooItem->Data = SkidooInfo();
	auto* skidoo = (SkidooInfo*)skidooItem->Data;

	skidoo->TurnRate = 0;
	skidoo->MomentumAngle = skidooItem->Position.yRot;
	skidoo->ExtraRotation = 0;
	skidoo->LeftVerticalVelocity = 0;
	skidoo->RightVerticalVelocity = 0;
	skidoo->Pitch = 0;
	skidoo->FlashTimer = 0;

	if (skidooItem->ObjectNumber == ID_SNOWMOBILE_GUN)
		skidoo->Armed = true;
	else
		skidoo->Armed = false;

	if (skidooItem->Status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNumber);
		skidooItem->Status = ITEM_ACTIVE;
	}
}

int GetSkidooMountType(ITEM_INFO* laraItem, ITEM_INFO* skidooItem, COLL_INFO* coll)
{
	auto* lara = (LaraInfo*&)laraItem->Data;

	int mountType = 0;

	if (!(TrInput & IN_ACTION) ||
		lara->Control.HandStatus != HandStatus::Free ||
		laraItem->Animation.Airborne)
	{
		return mountType = 0;
	}

	short deltaAngle = skidooItem->Position.yRot - laraItem->Position.yRot;
	if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
		mountType = 1;
	else if (deltaAngle > -ANGLE(135.0f) && deltaAngle < -ANGLE(45.0f))
		mountType = 2;
	else
		mountType = 0;

	auto probe = GetCollisionResult(skidooItem);
	if (probe.Position.Floor < -32000 ||
		!TestBoundsCollide(skidooItem, laraItem, coll->Setup.Radius) ||
		!TestCollision(skidooItem, laraItem))
	{
		mountType = 0;
	}

	return mountType;
}

bool TestSkidooDismountOK(ITEM_INFO* skidooItem, int direction)
{
	short angle;
	if (direction == SKIDOO_STATE_DISMOUNT_LEFT)
		angle = skidooItem->Position.yRot + ANGLE(90.0f);
	else
		angle = skidooItem->Position.yRot - ANGLE(90.0f);

	int x = skidooItem->Position.xPos - SKIDOO_DISMOUNT_DISTANCE * phd_sin(angle);
	int y = skidooItem->Position.yPos;
	int z = skidooItem->Position.zPos - SKIDOO_DISMOUNT_DISTANCE * phd_cos(angle);
	auto probe = GetCollisionResult(x, y, z, skidooItem->RoomNumber);

	if ((probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT) ||
		abs(probe.Position.Floor - skidooItem->Position.yPos) > CLICK(2) ||
		((probe.Position.Ceiling - skidooItem->Position.yPos) > -LARA_HEIGHT ||
			(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT))
	{
		return false;
	}

	return true;
}

bool TestSkidooDismount(ITEM_INFO* laraItem, ITEM_INFO* skidooItem)
{
	auto* lara = (LaraInfo*&)laraItem->Data;

	if (lara->Vehicle != NO_ITEM)
	{
		if ((laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_RIGHT || laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT) &&
			laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
		{
			if (laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT)
				laraItem->Position.yRot += ANGLE(90.0f);
			else
				laraItem->Position.yRot -= ANGLE(90.0f);

			SetAnimation(laraItem, LA_STAND_IDLE);
			laraItem->Position.xPos -= SKIDOO_DISMOUNT_DISTANCE * phd_sin(laraItem->Position.yRot);
			laraItem->Position.zPos -= SKIDOO_DISMOUNT_DISTANCE * phd_cos(laraItem->Position.yRot);
			laraItem->Position.xRot = 0;
			laraItem->Position.zRot = 0;
			lara->Vehicle = NO_ITEM;
			lara->Control.HandStatus = HandStatus::Free;
		}
		else if (laraItem->Animation.ActiveState == SKIDOO_STATE_JUMP_OFF &&
			(skidooItem->Position.yPos == skidooItem->Floor || TestLastFrame(laraItem)))
		{
			SetAnimation(laraItem, LA_FREEFALL);

			if (skidooItem->Position.yPos == skidooItem->Floor)
			{
				laraItem->Animation.TargetState = LS_DEATH;
				laraItem->Animation.Velocity = 0;
				laraItem->Animation.VerticalVelocity = DAMAGE_START + DAMAGE_LENGTH;
				SkidooExplode(laraItem, skidooItem);
			}
			else
			{
				laraItem->Animation.TargetState = LS_FREEFALL;
				laraItem->Position.yPos -= 200;
				laraItem->Animation.Velocity = skidooItem->Animation.Velocity;
				laraItem->Animation.VerticalVelocity = skidooItem->Animation.VerticalVelocity;
				SoundEffect(SFX_TR4_LARA_FALL, &laraItem->Position, 0);
			}

			laraItem->Position.xRot = 0;
			laraItem->Position.zRot = 0;
			laraItem->Animation.Airborne = true;
			lara->Control.HandStatus = HandStatus::Free;
			lara->Control.MoveAngle = skidooItem->Position.yRot;
			skidooItem->Flags |= ONESHOT;
			skidooItem->Collidable = false;

			return false;
		}

		return true;
	}
	else
		return true;
}

int GetSkidooCollisionAnim(ITEM_INFO* skidooItem, PHD_VECTOR* moved)
{
	moved->x = skidooItem->Position.xPos - moved->x;
	moved->z = skidooItem->Position.zPos - moved->z;

	if (moved->x || moved->z)
	{
		float s = phd_sin(skidooItem->Position.yRot);
		float c = phd_cos(skidooItem->Position.yRot);

		int side = -moved->z * s + moved->x * c;
		int front = moved->z * c + moved->x * s;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return SKIDOO_ANIM_HIT_BACK;
			else
				return SKIDOO_ANIM_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return SKIDOO_ANIM_HIT_LEFT;
			else
				return SKIDOO_ANIM_HIT_RIGHT;
		}
	}

	return 0;
}

void SkidooCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* lara = (LaraInfo*&)laraItem->Data;
	auto* skidooItem = &g_Level.Items[itemNum];

	if (laraItem->HitPoints < 0 || lara->Vehicle != NO_ITEM)
		return;

	int mountType = GetSkidooMountType(laraItem, skidooItem, coll);
	if (mountType == 0)
	{
		ObjectCollision(itemNum, laraItem, coll);
		return;
	}

	lara->Vehicle = itemNum;

	if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
	{
		CreateFlare(laraItem, ID_FLARE_ITEM, false);
		UndrawFlareMeshes(laraItem);
		lara->Flare.ControlLeft = 0;
		lara->Control.Weapon.RequestGunType = LaraWeaponType::None;
		lara->Control.HandStatus = HandStatus::Free;
	}

	if (mountType == 1)
		laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_RIGHT;
	else if (mountType == 2)
		laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_MOUNT_LEFT;

	laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
	laraItem->Animation.ActiveState = SKIDOO_STATE_MOUNT;
	laraItem->Position.xPos = skidooItem->Position.xPos;
	laraItem->Position.yPos = skidooItem->Position.yPos;
	laraItem->Position.zPos = skidooItem->Position.zPos;
	laraItem->Position.yRot = skidooItem->Position.yRot;
	lara->Control.HandStatus = HandStatus::Busy;
	skidooItem->Collidable = true;
}


void SkidooEntityCollision(ITEM_INFO* laraItem, ITEM_INFO* skidooItem)
{
	vector<short> roomsList;
	roomsList.push_back(skidooItem->RoomNumber);

	auto* room = &g_Level.Rooms[skidooItem->RoomNumber];
	for (int i = 0; i < room->doors.size(); i++)
		roomsList.push_back(room->doors[i].room);

	for (int i = 0; i < roomsList.size(); i++)
	{
		short itemNumber = g_Level.Rooms[roomsList[i]].itemNumber;

		while (itemNumber != NO_ITEM)
		{
			auto* item = &g_Level.Items[itemNumber];

			if (item->Collidable &&
				item->Status != IFLAG_INVISIBLE &&
				item != laraItem && item != skidooItem)
			{
				auto* object = &Objects[item->ObjectNumber];

				if (object->collision && object->intelligent)
				{
					int x = skidooItem->Position.xPos - item->Position.xPos;
					int y = skidooItem->Position.yPos - item->Position.yPos;
					int z = skidooItem->Position.zPos - item->Position.zPos;

					if (x > -2048 && x < 2048 &&
						y > -2048 && y < 2048 &&
						z > -2048 && z < 2048)
					{
						if (item->ObjectNumber == ID_ROLLINGBALL)
						{
							if (TestBoundsCollide(item, laraItem, 100))
							{
								if (laraItem->HitPoints > 0)
								{
									DoLotsOfBlood(laraItem->Position.xPos, laraItem->Position.yPos - CLICK(2), laraItem->Position.zPos, GetRandomControl() & 3, laraItem->Position.yRot, laraItem->RoomNumber, 5);
									item->HitPoints -= 8;
								}
							}
						}
						else
						{
							if (TestBoundsCollide(item, skidooItem, SKIDOO_FRONT))
							{
								DoLotsOfBlood(skidooItem->Position.xPos, skidooItem->Position.yPos, skidooItem->Position.zPos, GetRandomControl() & 3, laraItem->Position.yRot, laraItem->RoomNumber, 3);
								item->HitPoints = 0;
							}
						}
					}
				}
			}

			itemNumber = item->NextItem;
		}
	}
}

void SkidooGuns(ITEM_INFO* laraItem, ITEM_INFO* skidooItem)
{
	auto* lara = (LaraInfo*&)laraItem->Data;
	auto* skidoo = (SkidooInfo*)skidooItem->Data;
	auto* weapon = &Weapons[(int)LaraWeaponType::Snowmobile];

	LaraGetNewTarget(laraItem, weapon);
	AimWeapon(laraItem, weapon, &lara->RightArm);

	if (TrInput & SKIDOO_IN_FIRE && !skidooItem->ItemFlags[0])
	{
		short angles[] =
		{
			lara->RightArm.Rotation.yRot + laraItem->Position.yRot,
			lara->RightArm.Rotation.xRot
		};
		
		if ((int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles) +
			(int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles))
		{
			skidoo->FlashTimer = 2;
			SoundEffect(weapon->SampleNum, &laraItem->Position, 0);
			skidooItem->ItemFlags[0] = 4;
		}
	}

	if (skidooItem->ItemFlags[0])
		skidooItem->ItemFlags[0]--;
}

void SkidooExplode(ITEM_INFO* laraItem, ITEM_INFO* skidooItem)
{
	LaraInfo*& lara = laraItem->Data;

	if (TestEnvironment(ENV_FLAG_WATER, skidooItem))
		TriggerUnderwaterExplosion(skidooItem, 1);
	else
	{
		TriggerExplosionSparks(skidooItem->Position.xPos, skidooItem->Position.yPos, skidooItem->Position.zPos, 3, -2, 0, skidooItem->RoomNumber);

		for (int i = 0; i < 3; i++)
			TriggerExplosionSparks(skidooItem->Position.xPos, skidooItem->Position.yPos, skidooItem->Position.zPos, 3, -1, 0, skidooItem->RoomNumber);
	}

	PHD_3DPOS pos;
	pos.xPos = skidooItem->Position.xPos,
	pos.yPos = skidooItem->Position.yPos - 128,
	pos.zPos = skidooItem->Position.zPos,
	pos.xRot = 0,
	pos.yRot = skidooItem->Position.yRot,
	pos.zRot = 0;

	TriggerShockwave(&pos, 50, 180, 40, GenerateFloat(160, 200), 60, 60, 64, GenerateFloat(0, 359), 0);
	//ExplodingDeath(lara->Vehicle, -1, 256);

	KillItem(lara->Vehicle);
	skidooItem->Status = ITEM_DEACTIVATED;

	SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);
	SoundEffect(SFX_TR4_EXPLOSION2, 0, 0);

	lara->Vehicle = NO_ITEM;
}

void DoSnowEffect(ITEM_INFO* skidooItem)
{
	auto material = GetCollisionResult(skidooItem).BottomBlock->Material;
	if (material != FLOOR_MATERIAL::Ice && material != FLOOR_MATERIAL::Snow)
		return;

	TEN::Effects::TriggerSnowmobileSnow(skidooItem);
}

bool SkidooControl(ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* lara = (LaraInfo*&)laraItem->Data;
	auto* skidooItem = &g_Level.Items[lara->Vehicle];
	auto* skidoo = (SkidooInfo*)skidooItem->Data;

	PHD_VECTOR frontLeft, frontRight;
	auto collide = SkidooDynamics(laraItem, skidooItem);
	auto heightFrontLeft = TestSkidooHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, &frontLeft);
	auto heightFrontRight = TestSkidooHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, &frontRight);

	auto probe = GetCollisionResult(skidooItem);

	TestTriggers(skidooItem, true);
	TestTriggers(skidooItem, false);

	bool dead = false;
	int drive = 0;

	if (laraItem->HitPoints <= 0)
	{
		TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		dead = true;
	}
	else if (laraItem->Animation.ActiveState == SKIDOO_STATE_JUMP_OFF)
	{
		dead = true;
		collide = 0;
	}

	int height = probe.Position.Floor;
	int pitch = 0;

	if (skidooItem->Flags & ONESHOT)
	{
		drive = 0;
		collide = 0;
	}
	else
	{
		switch (laraItem->Animation.ActiveState)
		{
		case SKIDOO_STATE_MOUNT:
		case SKIDOO_STATE_DISMOUNT_RIGHT:
		case SKIDOO_STATE_DISMOUNT_LEFT:
		case SKIDOO_STATE_JUMP_OFF:
			drive = -1;
			collide = 0;

			break;

		default:
			drive = SkidooUserControl(laraItem, skidooItem, height, &pitch);

			break;
		}
	}

	bool banditSkidoo = skidoo->Armed;
	if (drive > 0)
	{
		skidoo->TrackMesh = ((skidoo->TrackMesh & 3) == 1) ? 2 : 1;

		skidoo->Pitch += (pitch - skidoo->Pitch) / 4;
		SoundEffect(skidoo->Pitch ? SFX_TR2_SKIDOO_MOVING : SFX_TR2_SKIDOO_ACCELERATE, &skidooItem->Position, 0, 0.5f + skidoo->Pitch / (float)SKIDOO_MAX_VELOCITY);
	}
	else
	{
		skidoo->TrackMesh = 0;
		if (!drive)
			SoundEffect(SFX_TR2_SKIDOO_IDLE, &skidooItem->Position, 0);
		skidoo->Pitch = 0;
	}
	skidooItem->Floor = height;

	skidoo->LeftVerticalVelocity = DoSkidooDynamics(heightFrontLeft, skidoo->LeftVerticalVelocity, (int*)&frontLeft.y);
	skidoo->RightVerticalVelocity = DoSkidooDynamics(heightFrontRight, skidoo->RightVerticalVelocity, (int*)&frontRight.y);
	skidooItem->Animation.VerticalVelocity = DoSkidooDynamics(height, skidooItem->Animation.VerticalVelocity, (int*)&skidooItem->Position.yPos);

	height = (frontLeft.y + frontRight.y) / 2;
	short xRot = phd_atan(SKIDOO_FRONT, skidooItem->Position.yPos - height);
	short zRot = phd_atan(SKIDOO_SIDE, height - frontLeft.y);

	skidooItem->Position.xRot += ((xRot - skidooItem->Position.xRot) / 2);
	skidooItem->Position.zRot += ((zRot - skidooItem->Position.zRot) / 2);

	if (skidooItem->Flags & ONESHOT)
	{
		if (probe.RoomNumber != skidooItem->RoomNumber)
		{
			ItemNewRoom(lara->Vehicle, probe.RoomNumber);
			ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}

		AnimateItem(laraItem);

		if (skidooItem->Position.yPos == skidooItem->Floor)
			SkidooExplode(laraItem, skidooItem);

		return 0;
	}

	SkidooAnimation(laraItem, skidooItem, collide, dead);

	if (probe.RoomNumber != skidooItem->RoomNumber)
	{
		ItemNewRoom(lara->Vehicle, probe.RoomNumber);
		ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
	}

	if (laraItem->Animation.ActiveState != SKIDOO_STATE_FALLOFF)
	{
		laraItem->Position.xPos = skidooItem->Position.xPos;
		laraItem->Position.yPos = skidooItem->Position.yPos;
		laraItem->Position.zPos = skidooItem->Position.zPos;
		laraItem->Position.yRot = skidooItem->Position.yRot;

		if (drive >= 0)
		{
			laraItem->Position.xRot = skidooItem->Position.xRot;
			laraItem->Position.zRot = skidooItem->Position.zRot;
		}
		else
			laraItem->Position.xRot = laraItem->Position.zRot = 0;
	}
	else
		laraItem->Position.xRot = laraItem->Position.zRot = 0;

	AnimateItem(laraItem);

	if (!dead && drive >= 0 && banditSkidoo)
		SkidooGuns(laraItem, skidooItem);

	if (!dead)
	{
		skidooItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex);
		skidooItem->Animation.FrameNumber = g_Level.Anims[skidooItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
	}
	else
	{
		skidooItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE].animIndex + SKIDOO_ANIM_IDLE;
		skidooItem->Animation.FrameNumber = g_Level.Anims[skidooItem->Animation.AnimNumber].frameBase;
	}

	if (skidooItem->Animation.Velocity && skidooItem->Floor == skidooItem->Position.yPos)
	{
		DoSnowEffect(skidooItem);

		if (skidooItem->Animation.Velocity < 50)
			DoSnowEffect(skidooItem);
	}

	return TestSkidooDismount(laraItem, skidooItem);
}

bool SkidooUserControl(ITEM_INFO* laraItem, ITEM_INFO* skidooItem, int height, int* pitch)
{
	auto* skidoo = (SkidooInfo*)skidooItem->Data;

	bool drive = false;
	int maxVelocity = 0;

	if (skidooItem->Position.yPos >= (height - CLICK(1)))
	{
		*pitch = skidooItem->Animation.Velocity + (height - skidooItem->Position.yPos);

		if (TrInput & IN_LOOK && skidooItem->Animation.Velocity == 0)
			LookUpDown(laraItem);

		if ((TrInput & SKIDOO_IN_LEFT && !(TrInput & SKIDOO_IN_BRAKE)) ||
			(TrInput & SKIDOO_IN_RIGHT && TrInput & SKIDOO_IN_BRAKE))
		{
			skidoo->TurnRate -= SKIDOO_TURN;
			if (skidoo->TurnRate < -SKIDOO_MAX_TURN)
				skidoo->TurnRate = -SKIDOO_MAX_TURN;
		}

		if ((TrInput & SKIDOO_IN_RIGHT && !(TrInput & SKIDOO_IN_BRAKE)) ||
			(TrInput & SKIDOO_IN_LEFT && TrInput & SKIDOO_IN_BRAKE))
		{
			skidoo->TurnRate += SKIDOO_TURN;
			if (skidoo->TurnRate > SKIDOO_MAX_TURN)
				skidoo->TurnRate = SKIDOO_MAX_TURN;
		}

		if (TrInput & SKIDOO_IN_BRAKE)
		{
			if (skidooItem->Animation.Velocity > 0)
				skidooItem->Animation.Velocity -= SKIDOO_BRAKE;
			else
			{
				if (skidooItem->Animation.Velocity > SKIDOO_MAX_BACK)
					skidooItem->Animation.Velocity += SKIDOO_REVERSE;

				drive = true;
			}
		}
		else if (TrInput & SKIDOO_IN_ACCELERATE)
		{
			if (TrInput & SKIDOO_IN_FIRE && !skidoo->Armed)
				maxVelocity = SKIDOO_FAST_VELOCITY;
			else if (TrInput & SKIDOO_IN_SLOW)
				maxVelocity = SKIDOO_SLOW_VELOCITY;
			else
				maxVelocity = SKIDOO_MAX_VELOCITY;

			if (skidooItem->Animation.Velocity < maxVelocity)
				skidooItem->Animation.Velocity += (SKIDOO_ACCELERATION / 2) + (SKIDOO_ACCELERATION * (skidooItem->Animation.Velocity / (2 * maxVelocity)));
			else if (skidooItem->Animation.Velocity > (maxVelocity + SKIDOO_SLOWDOWN))
				skidooItem->Animation.Velocity -= SKIDOO_SLOWDOWN;
			drive = true;
		}
		else if (TrInput & (SKIDOO_IN_LEFT | SKIDOO_IN_RIGHT) &&
			skidooItem->Animation.Velocity >= 0 &&
			skidooItem->Animation.Velocity < SKIDOO_MIN_VELOCITY)
		{
			skidooItem->Animation.Velocity = SKIDOO_MIN_VELOCITY;
			drive = true;
		}
		else if (skidooItem->Animation.Velocity > SKIDOO_SLOWDOWN)
		{
			skidooItem->Animation.Velocity -= SKIDOO_SLOWDOWN;
			if ((GetRandomControl() & 0x7f) < skidooItem->Animation.Velocity)
				drive = true;
		}
		else
			skidooItem->Animation.Velocity = 0;
	}
	else if (TrInput & (SKIDOO_IN_ACCELERATE | SKIDOO_IN_BRAKE))
	{
		*pitch = skidoo->Pitch + 50;
		drive = true;
	}

	return drive;
}

void SkidooAnimation(ITEM_INFO* laraItem, ITEM_INFO* skidooItem, int collide, bool dead)
{
	auto* skidoo = (SkidooInfo*)skidooItem->Data;

	if (laraItem->Animation.ActiveState != SKIDOO_STATE_FALL &&
		skidooItem->Position.yPos != skidooItem->Floor &&
		skidooItem->Animation.VerticalVelocity > 0 &&
		!dead)
	{
		laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + SKIDOO_ANIM_LEAP_START;
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = SKIDOO_STATE_FALL;
		laraItem->Animation.TargetState = SKIDOO_STATE_FALL;
	}
	else if (laraItem->Animation.ActiveState != SKIDOO_STATE_FALL &&
		collide && !dead)
	{
		if (laraItem->Animation.ActiveState != SKIDOO_STATE_HIT)
		{
			if (collide == SKIDOO_ANIM_HIT_FRONT)
				SoundEffect(SFX_TR2_CLATTER_1, &skidooItem->Position, 0);
			else
				SoundEffect(SFX_TR2_CLATTER_2, &skidooItem->Position, 0);

			laraItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_LARA_ANIMS].animIndex + collide;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.ActiveState = laraItem->Animation.TargetState = SKIDOO_STATE_HIT;
		}
	}
	else
	{
		switch (laraItem->Animation.ActiveState)
		{
		case SKIDOO_STATE_IDLE:

			if (dead)
			{
				laraItem->Animation.TargetState = SKIDOO_STATE_DEATH;
				break;
			}

			laraItem->Animation.TargetState = SKIDOO_STATE_IDLE; 

			if (TrInput & SKIDOO_IN_DISMOUNT)
			{
				if (TrInput & SKIDOO_IN_RIGHT &&
					TestSkidooDismountOK(skidooItem, SKIDOO_STATE_DISMOUNT_RIGHT))
				{
					laraItem->Animation.TargetState = SKIDOO_STATE_DISMOUNT_RIGHT;
					skidooItem->Animation.Velocity = 0;
				}
				else if (TrInput & SKIDOO_IN_LEFT &&
					TestSkidooDismountOK(skidooItem, SKIDOO_STATE_DISMOUNT_LEFT))
				{
					laraItem->Animation.TargetState = SKIDOO_STATE_DISMOUNT_LEFT;
					skidooItem->Animation.Velocity = 0;
				}
			}
			else if (TrInput & SKIDOO_IN_LEFT)
				laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
			else if (TrInput & SKIDOO_IN_RIGHT)
				laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;
			else if (TrInput & (SKIDOO_IN_ACCELERATE | SKIDOO_IN_BRAKE))
				laraItem->Animation.TargetState = SKIDOO_STATE_SIT;

			break;

		case SKIDOO_STATE_SIT:
			if (skidooItem->Animation.Velocity == 0)
				laraItem->Animation.TargetState = SKIDOO_STATE_IDLE;

			if (dead)
				laraItem->Animation.TargetState = SKIDOO_STATE_FALLOFF;
			else if (TrInput & SKIDOO_IN_LEFT)
				laraItem->Animation.TargetState = SKIDOO_STATE_LEFT;
			else if (TrInput & SKIDOO_IN_RIGHT)
				laraItem->Animation.TargetState = SKIDOO_STATE_RIGHT;

			break;

		case SKIDOO_STATE_LEFT:
			if (!(TrInput & SKIDOO_IN_LEFT))
				laraItem->Animation.TargetState = SKIDOO_STATE_SIT;

			break;

		case SKIDOO_STATE_RIGHT:
			if (!(TrInput & SKIDOO_IN_RIGHT))
				laraItem->Animation.TargetState = SKIDOO_STATE_SIT;

			break;

		case SKIDOO_STATE_FALL:
			if (skidooItem->Animation.VerticalVelocity <= 0 ||
				skidoo->LeftVerticalVelocity <= 0 ||
				skidoo->RightVerticalVelocity <= 0)
			{
				laraItem->Animation.TargetState = SKIDOO_STATE_SIT;
				SoundEffect(SFX_TR2_CLATTER_3, &skidooItem->Position, 0);
			}
			else if (skidooItem->Animation.VerticalVelocity > (DAMAGE_START + DAMAGE_LENGTH))
				laraItem->Animation.TargetState = SKIDOO_STATE_JUMP_OFF;

			break;
		}
	}

	if (TestEnvironment(ENV_FLAG_WATER, skidooItem) ||
		TestEnvironment(ENV_FLAG_SWAMP, skidooItem))
	{
		laraItem->Animation.TargetState = SKIDOO_STATE_JUMP_OFF;
		laraItem->HitPoints = 0;
		laraItem->RoomNumber = skidooItem->RoomNumber;
		SkidooExplode(laraItem, skidooItem);
	}
}

int DoSkidooDynamics(int height, int verticalVelocity, int* y)
{
	if (height > * y)
	{
		*y += verticalVelocity;
		if (*y > height - SKIDOO_MIN_BOUNCE)
		{
			*y = height;
			verticalVelocity = 0;
		}
		else
			verticalVelocity += GRAVITY;
	}
	else
	{
		int kick = (height - *y) * 4;
		if (kick < SKIDOO_MAX_KICK)
			kick = SKIDOO_MAX_KICK;

		verticalVelocity += (kick - verticalVelocity) / 8;

		if (*y > height)
			*y = height;
	}

	return verticalVelocity;
}

int TestSkidooHeight(ITEM_INFO* skidooItem, int zOffset, int xOffset, PHD_VECTOR* pos)
{
	pos->y = skidooItem->Position.yPos - zOffset * phd_sin(skidooItem->Position.xRot) + xOffset * phd_sin(skidooItem->Position.zRot);

	float s = phd_sin(skidooItem->Position.yRot);
	float c = phd_cos(skidooItem->Position.yRot);

	pos->x = skidooItem->Position.xPos + zOffset * s + xOffset * c;
	pos->z = skidooItem->Position.zPos + zOffset * c - xOffset * s;
	
	auto probe = GetCollisionResult(pos->x, pos->y, pos->z, skidooItem->RoomNumber);
	if (probe.Position.Ceiling > pos->y ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return NO_HEIGHT;
	}

	return probe.Position.Floor;
}

short DoSkidooShift(ITEM_INFO* skidooItem, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int	x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);
	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);
	int shiftX = pos->x & (SECTOR(1) - 1);
	int shiftZ = pos->z & (SECTOR(1) - 1);

	if (x == xOld)
	{
		if (z == zOld)
		{
			skidooItem->Position.zPos += (old->z - pos->z);
			skidooItem->Position.xPos += (old->x - pos->x);
		}
		else if (z > zOld)
		{
			skidooItem->Position.zPos -= shiftZ + 1;
			return (pos->x - skidooItem->Position.xPos);
		}
		else
		{
			skidooItem->Position.zPos += SECTOR(1) - shiftZ;
			return (skidooItem->Position.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			skidooItem->Position.xPos -= shiftX + 1;
			return (skidooItem->Position.zPos - pos->z);
		}
		else
		{
			skidooItem->Position.xPos += SECTOR(1) - shiftX;
			return (pos->z - skidooItem->Position.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;

		auto probe = GetCollisionResult(old->x, pos->y, pos->z, skidooItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = SECTOR(1) - shiftZ;
		}

		probe = GetCollisionResult(pos->x, pos->y, old->z, skidooItem->RoomNumber);
		if (probe.Position.Floor < (old->y - CLICK(1)))
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = SECTOR(1) - shiftX;
		}

		if (x && z)
		{
			skidooItem->Position.zPos += z;
			skidooItem->Position.xPos += x;
			skidooItem->Animation.Velocity -= 50;
		}
		else if (z)
		{
			skidooItem->Position.zPos += z;
			skidooItem->Animation.Velocity -= 50;

			if (z > 0)
				return (skidooItem->Position.xPos - pos->x);
			else
				return (pos->x - skidooItem->Position.xPos);
		}
		else if (x)
		{
			skidooItem->Position.xPos += x;
			skidooItem->Animation.Velocity -= 50;

			if (x > 0)
				return (pos->z - skidooItem->Position.zPos);
			else
				return (skidooItem->Position.zPos - pos->z);
		}
		else
		{
			skidooItem->Position.zPos += old->z - pos->z;
			skidooItem->Position.xPos += old->x - pos->x;
			skidooItem->Animation.Velocity -= 50;
		}
	}

	return 0;
}

int SkidooDynamics(ITEM_INFO* laraItem, ITEM_INFO* skidooItem)
{
	auto* skidoo = (SkidooInfo*)skidooItem->Data;

	PHD_VECTOR frontLeftOld, frontRightOld, backLeftOld, backRightOld;
	auto heightFrontLeftOld = TestSkidooHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, &frontLeftOld);
	auto heightFrontRightOld = TestSkidooHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, &frontRightOld);
	auto heightBackLeftOld = TestSkidooHeight(skidooItem, -SKIDOO_FRONT, -SKIDOO_SIDE, &backLeftOld);
	auto heightBackRightOld = TestSkidooHeight(skidooItem, -SKIDOO_FRONT, SKIDOO_SIDE, &backRightOld);

	PHD_VECTOR old;
	old.x = skidooItem->Position.xPos;
	old.y = skidooItem->Position.yPos;
	old.z = skidooItem->Position.zPos;

	if (backLeftOld.y > heightBackLeftOld)
		backLeftOld.y = heightBackLeftOld;
	if (backRightOld.y > heightBackRightOld)
		backRightOld.y = heightBackRightOld;
	if (frontLeftOld.y > heightFrontLeftOld)
		frontLeftOld.y = heightFrontLeftOld;
	if (frontRightOld.y > heightFrontRightOld)
		frontRightOld.y = heightFrontRightOld;

	short rotation;

	if (skidooItem->Position.yPos > (skidooItem->Floor - CLICK(1)))
	{
		if (skidoo->TurnRate < -SKIDOO_UNDO_TURN)
			skidoo->TurnRate += SKIDOO_UNDO_TURN;
		else if (skidoo->TurnRate > SKIDOO_UNDO_TURN)
			skidoo->TurnRate -= SKIDOO_UNDO_TURN;
		else
			skidoo->TurnRate = 0;
		skidooItem->Position.yRot += skidoo->TurnRate + skidoo->ExtraRotation;

		rotation = skidooItem->Position.yRot - skidoo->MomentumAngle;
		if (rotation < -SKIDOO_MOMENTUM_TURN)
		{
			if (rotation < -SKIDOO_MAX_MOMENTUM_TURN)
			{
				rotation = -SKIDOO_MAX_MOMENTUM_TURN;
				skidoo->MomentumAngle = skidooItem->Position.yRot - rotation;
			}
			else
				skidoo->MomentumAngle -= SKIDOO_MOMENTUM_TURN;
		}
		else if (rotation > SKIDOO_MOMENTUM_TURN)
		{
			if (rotation > SKIDOO_MAX_MOMENTUM_TURN)
			{
				rotation = SKIDOO_MAX_MOMENTUM_TURN;
				skidoo->MomentumAngle = skidooItem->Position.yRot - rotation;
			}
			else
				skidoo->MomentumAngle += SKIDOO_MOMENTUM_TURN;
		}
		else
			skidoo->MomentumAngle = skidooItem->Position.yRot;
	}
	else
		skidooItem->Position.yRot += skidoo->TurnRate + skidoo->ExtraRotation;

	skidooItem->Position.zPos += skidooItem->Animation.Velocity * phd_cos(skidoo->MomentumAngle);
	skidooItem->Position.xPos += skidooItem->Animation.Velocity * phd_sin(skidoo->MomentumAngle);

	int slip = SKIDOO_SLIP * phd_sin(skidooItem->Position.xRot);
	if (abs(slip) > (SKIDOO_SLIP / 2))
	{
		skidooItem->Position.zPos -= slip * phd_cos(skidooItem->Position.yRot);
		skidooItem->Position.xPos -= slip * phd_sin(skidooItem->Position.yRot);
	}

	slip = SKIDOO_SLIP_SIDE * phd_sin(skidooItem->Position.zRot);
	if (abs(slip) > (SKIDOO_SLIP_SIDE / 2))
	{
		skidooItem->Position.zPos -= slip * phd_sin(skidooItem->Position.yRot);
		skidooItem->Position.xPos += slip * phd_cos(skidooItem->Position.yRot);
	}

	PHD_VECTOR moved;
	moved.x = skidooItem->Position.xPos;
	moved.z = skidooItem->Position.zPos;

	if (!(skidooItem->Flags & ONESHOT))
		SkidooEntityCollision(laraItem, skidooItem);

	PHD_VECTOR frontLeft, frontRight, backRight, backLeft;
	rotation = 0;
	auto heightBackLeft = TestSkidooHeight(skidooItem, -SKIDOO_FRONT, -SKIDOO_SIDE, &backLeft);
	if (heightBackLeft < (backLeftOld.y - CLICK(1)))
		rotation = DoSkidooShift(skidooItem, &backLeft, &backLeftOld);

	auto heightBackRight = TestSkidooHeight(skidooItem, -SKIDOO_FRONT, SKIDOO_SIDE, &backRight);
	if (heightBackRight < (backRightOld.y - CLICK(1)))
		rotation += DoSkidooShift(skidooItem, &backRight, &backRightOld);

	auto heightFrontLeft = TestSkidooHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, &frontLeft);
	if (heightFrontLeft < (frontLeftOld.y - CLICK(1)))
		rotation += DoSkidooShift(skidooItem, &frontLeft, &frontLeftOld);

	auto heightFrontRight = TestSkidooHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, &frontRight);
	if (heightFrontRight < (frontRightOld.y - CLICK(1)))
		rotation += DoSkidooShift(skidooItem, &frontRight, &frontRightOld);

	auto probe = GetCollisionResult(skidooItem);
	if (probe.Position.Floor < (skidooItem->Position.yPos - CLICK(1)))
		DoSkidooShift(skidooItem, (PHD_VECTOR*)&skidooItem->Position, &old);

	skidoo->ExtraRotation = rotation;

	auto collide = GetSkidooCollisionAnim(skidooItem, &moved);
	if (collide)
	{
		int newVelocity = (skidooItem->Position.zPos - old.z) * phd_cos(skidoo->MomentumAngle) + (skidooItem->Position.xPos - old.x) * phd_sin(skidoo->MomentumAngle);
		if (skidooItem->Animation.Velocity > (SKIDOO_MAX_VELOCITY + SKIDOO_ACCELERATION) &&
			newVelocity < (skidooItem->Animation.Velocity - 10))
		{
			laraItem->HitPoints -= (skidooItem->Animation.Velocity - newVelocity) / 2;
			laraItem->HitStatus = true;
		}

		if (skidooItem->Animation.Velocity > 0 && newVelocity < skidooItem->Animation.Velocity)
			skidooItem->Animation.Velocity = (newVelocity < 0) ? 0 : newVelocity;
		else if (skidooItem->Animation.Velocity < 0 && newVelocity > skidooItem->Animation.Velocity)
			skidooItem->Animation.Velocity = (newVelocity > 0) ? 0 : newVelocity;

		if (skidooItem->Animation.Velocity < SKIDOO_MAX_BACK)
			skidooItem->Animation.Velocity = SKIDOO_MAX_BACK;
	}

	return collide;
}

void DrawSkidoo(ITEM_INFO* skidooItem)
{

}
