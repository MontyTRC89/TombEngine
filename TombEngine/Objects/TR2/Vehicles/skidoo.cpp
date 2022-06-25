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
#include "Game/effects/simple_particle.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Sound/sound.h"

using namespace TEN::Input;
using namespace TEN::Math::Random;

namespace TEN::Entities::Vehicles
{
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

	#define SKIDOO_IN_ACCELERATE	IN_FORWARD
	#define SKIDOO_IN_BRAKE			IN_BACK
	#define SKIDOO_IN_SLOW			IN_WALK
	#define SKIDOO_IN_FIRE			IN_ACTION
	#define SKIDOO_IN_DISMOUNT		(IN_JUMP | IN_ROLL)
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
		skidoo->MomentumAngle = skidooItem->Pose.Orientation.y;
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

	int GetSkidooMountType(ItemInfo* laraItem, ItemInfo* skidooItem, CollisionInfo* coll)
	{
		auto* lara = (LaraInfo*&)laraItem->Data;

		int mountType = 0;

		if (!(TrInput & IN_ACTION) ||
			lara->Control.HandStatus != HandStatus::Free ||
			laraItem->Animation.Airborne)
		{
			return mountType = 0;
		}

		short deltaAngle = skidooItem->Pose.Orientation.y - laraItem->Pose.Orientation.y;
		if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
			mountType = 1;
		else if (deltaAngle > -ANGLE(135.0f) && deltaAngle < -ANGLE(45.0f))
			mountType = 2;
		else
			mountType = 0;

		auto probe = GetCollision(skidooItem);
		if (probe.Position.Floor < -32000 ||
			!TestBoundsCollide(skidooItem, laraItem, coll->Setup.Radius) ||
			!TestCollision(skidooItem, laraItem))
		{
			mountType = 0;
		}

		return mountType;
	}

	bool TestSkidooDismountOK(ItemInfo* skidooItem, int direction)
	{
		short angle;
		if (direction == SKIDOO_STATE_DISMOUNT_LEFT)
			angle = skidooItem->Pose.Orientation.y + ANGLE(90.0f);
		else
			angle = skidooItem->Pose.Orientation.y - ANGLE(90.0f);

		int x = skidooItem->Pose.Position.x - SKIDOO_DISMOUNT_DISTANCE * phd_sin(angle);
		int y = skidooItem->Pose.Position.y;
		int z = skidooItem->Pose.Position.z - SKIDOO_DISMOUNT_DISTANCE * phd_cos(angle);
		auto probe = GetCollision(x, y, z, skidooItem->RoomNumber);

		if ((probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT) ||
			abs(probe.Position.Floor - skidooItem->Pose.Position.y) > CLICK(2) ||
			((probe.Position.Ceiling - skidooItem->Pose.Position.y) > -LARA_HEIGHT ||
				(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT))
		{
			return false;
		}

		return true;
	}

	bool TestSkidooDismount(ItemInfo* laraItem, ItemInfo* skidooItem)
	{
		auto* lara = (LaraInfo*&)laraItem->Data;

		if (lara->Vehicle != NO_ITEM)
		{
			if ((laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_RIGHT || laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT) &&
				laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
			{
				if (laraItem->Animation.ActiveState == SKIDOO_STATE_DISMOUNT_LEFT)
					laraItem->Pose.Orientation.y += ANGLE(90.0f);
				else
					laraItem->Pose.Orientation.y -= ANGLE(90.0f);

				SetAnimation(laraItem, LA_STAND_IDLE);
				laraItem->Pose.Position.x -= SKIDOO_DISMOUNT_DISTANCE * phd_sin(laraItem->Pose.Orientation.y);
				laraItem->Pose.Position.z -= SKIDOO_DISMOUNT_DISTANCE * phd_cos(laraItem->Pose.Orientation.y);
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				lara->Vehicle = NO_ITEM;
				lara->Control.HandStatus = HandStatus::Free;
			}
			else if (laraItem->Animation.ActiveState == SKIDOO_STATE_JUMP_OFF &&
				(skidooItem->Pose.Position.y == skidooItem->Floor || TestLastFrame(laraItem)))
			{
				SetAnimation(laraItem, LA_FREEFALL);

				if (skidooItem->Pose.Position.y == skidooItem->Floor)
				{
					laraItem->Animation.TargetState = LS_DEATH;
					laraItem->Animation.Velocity = 0;
					laraItem->Animation.VerticalVelocity = DAMAGE_START + DAMAGE_LENGTH;
					ExplodeVehicle(laraItem, skidooItem);
				}
				else
				{
					laraItem->Animation.TargetState = LS_FREEFALL;
					laraItem->Pose.Position.y -= 200;
					laraItem->Animation.Velocity = skidooItem->Animation.Velocity;
					laraItem->Animation.VerticalVelocity = skidooItem->Animation.VerticalVelocity;
					SoundEffect(SFX_TR4_LARA_FALL, &laraItem->Pose);
				}

				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				laraItem->Animation.Airborne = true;
				lara->Control.HandStatus = HandStatus::Free;
				lara->Control.MoveAngle = skidooItem->Pose.Orientation.y;
				skidooItem->Flags |= ONESHOT;
				skidooItem->Collidable = false;

				return false;
			}

			return true;
		}
		else
			return true;
	}

	int GetSkidooCollisionAnim(ItemInfo* skidooItem, Vector3Int* moved)
	{
		moved->x = skidooItem->Pose.Position.x - moved->x;
		moved->z = skidooItem->Pose.Position.z - moved->z;

		if (moved->x || moved->z)
		{
			float s = phd_sin(skidooItem->Pose.Orientation.y);
			float c = phd_cos(skidooItem->Pose.Orientation.y);

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

	void SkidooCollision(short itemNum, ItemInfo* laraItem, CollisionInfo* coll)
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
		laraItem->Pose.Position.x = skidooItem->Pose.Position.x;
		laraItem->Pose.Position.y = skidooItem->Pose.Position.y;
		laraItem->Pose.Position.z = skidooItem->Pose.Position.z;
		laraItem->Pose.Orientation.y = skidooItem->Pose.Orientation.y;
		lara->Control.HandStatus = HandStatus::Busy;
		skidooItem->Collidable = true;
	}

	void SkidooGuns(ItemInfo* laraItem, ItemInfo* skidooItem)
	{
		auto* lara = (LaraInfo*&)laraItem->Data;
		auto* skidoo = (SkidooInfo*)skidooItem->Data;
		auto* weapon = &Weapons[(int)LaraWeaponType::Snowmobile];

		LaraGetNewTarget(laraItem, weapon);
		AimWeapon(laraItem, weapon, &lara->RightArm);

		if (TrInput & SKIDOO_IN_FIRE && !skidooItem->ItemFlags[0])
		{
			auto angles = Vector3Shrt(
				lara->RightArm.Orientation.x,
				lara->RightArm.Orientation.y + laraItem->Pose.Orientation.y,
				0
			);

			if ((int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles) +
				(int)FireWeapon(LaraWeaponType::Pistol, lara->TargetEntity, laraItem, angles))
			{
				skidoo->FlashTimer = 2;
				SoundEffect(weapon->SampleNum, &laraItem->Pose);
				skidooItem->ItemFlags[0] = 4;
			}
		}

		if (skidooItem->ItemFlags[0])
			skidooItem->ItemFlags[0]--;
	}

	void DoSnowEffect(ItemInfo* skidooItem)
	{
		auto material = GetCollision(skidooItem).BottomBlock->Material;
		if (material != FLOOR_MATERIAL::Ice && material != FLOOR_MATERIAL::Snow)
			return;

		TEN::Effects::TriggerSnowmobileSnow(skidooItem);
	}

	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = (LaraInfo*&)laraItem->Data;
		auto* skidooItem = &g_Level.Items[lara->Vehicle];
		auto* skidoo = (SkidooInfo*)skidooItem->Data;

		Vector3Int frontLeft, frontRight;
		auto collide = SkidooDynamics(laraItem, skidooItem);
		auto heightFrontLeft = TestSkidooHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, &frontLeft);
		auto heightFrontRight = TestSkidooHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, &frontRight);

		auto probe = GetCollision(skidooItem);

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

			auto pitch = std::clamp(0.5f + (float)abs(skidoo->Pitch) / (float)SKIDOO_MAX_VELOCITY, 0.6f, 1.4f);
			SoundEffect(skidoo->Pitch ? SFX_TR2_VEHICLE_SNOWMOBILE_MOVING : SFX_TR2_VEHICLE_SNOWMOBILE_ACCELERATE, &skidooItem->Pose, SoundEnvironment::Land, pitch);
		}
		else
		{
			skidoo->TrackMesh = 0;
			if (!drive)
				SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &skidooItem->Pose);
			skidoo->Pitch = 0;
		}
		skidooItem->Floor = height;

		skidoo->LeftVerticalVelocity = DoSkidooDynamics(heightFrontLeft, skidoo->LeftVerticalVelocity, (int*)&frontLeft.y);
		skidoo->RightVerticalVelocity = DoSkidooDynamics(heightFrontRight, skidoo->RightVerticalVelocity, (int*)&frontRight.y);
		skidooItem->Animation.VerticalVelocity = DoSkidooDynamics(height, skidooItem->Animation.VerticalVelocity, (int*)&skidooItem->Pose.Position.y);
		skidooItem->Animation.Velocity = DoVehicleWaterMovement(skidooItem, laraItem, skidooItem->Animation.Velocity, SKIDOO_RADIUS, &skidoo->TurnRate);

		height = (frontLeft.y + frontRight.y) / 2;
		short xRot = phd_atan(SKIDOO_FRONT, skidooItem->Pose.Position.y - height);
		short zRot = phd_atan(SKIDOO_SIDE, height - frontLeft.y);

		skidooItem->Pose.Orientation.x += ((xRot - skidooItem->Pose.Orientation.x) / 2);
		skidooItem->Pose.Orientation.z += ((zRot - skidooItem->Pose.Orientation.z) / 2);

		if (skidooItem->Flags & ONESHOT)
		{
			if (probe.RoomNumber != skidooItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, probe.RoomNumber);
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
			}

			AnimateItem(laraItem);

			if (skidooItem->Pose.Position.y == skidooItem->Floor)
				ExplodeVehicle(laraItem, skidooItem);

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
			laraItem->Pose.Position.x = skidooItem->Pose.Position.x;
			laraItem->Pose.Position.y = skidooItem->Pose.Position.y;
			laraItem->Pose.Position.z = skidooItem->Pose.Position.z;
			laraItem->Pose.Orientation.y = skidooItem->Pose.Orientation.y;

			if (drive >= 0)
			{
				laraItem->Pose.Orientation.x = skidooItem->Pose.Orientation.x;
				laraItem->Pose.Orientation.z = skidooItem->Pose.Orientation.z;
			}
			else
				laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;
		}
		else
			laraItem->Pose.Orientation.x = laraItem->Pose.Orientation.z = 0;

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

		if (skidooItem->Animation.Velocity && skidooItem->Floor == skidooItem->Pose.Position.y)
		{
			DoSnowEffect(skidooItem);

			if (skidooItem->Animation.Velocity < 50)
				DoSnowEffect(skidooItem);
		}

		return TestSkidooDismount(laraItem, skidooItem);
	}

	bool SkidooUserControl(ItemInfo* laraItem, ItemInfo* skidooItem, int height, int* pitch)
	{
		auto* skidoo = (SkidooInfo*)skidooItem->Data;

		bool drive = false;
		int maxVelocity = 0;

		if (skidooItem->Pose.Position.y >= (height - CLICK(1)))
		{
			*pitch = skidooItem->Animation.Velocity + (height - skidooItem->Pose.Position.y);

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

	void SkidooAnimation(ItemInfo* laraItem, ItemInfo* skidooItem, int collide, bool dead)
	{
		auto* skidoo = (SkidooInfo*)skidooItem->Data;

		if (laraItem->Animation.ActiveState != SKIDOO_STATE_FALL &&
			skidooItem->Pose.Position.y != skidooItem->Floor &&
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
					SoundEffect(SFX_TR2_VEHICLE_IMPACT1, &skidooItem->Pose);
				else
					SoundEffect(SFX_TR2_VEHICLE_IMPACT2, &skidooItem->Pose);

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
					SoundEffect(SFX_TR2_VEHICLE_IMPACT3, &skidooItem->Pose);
				}
				else if (skidooItem->Animation.VerticalVelocity > (DAMAGE_START + DAMAGE_LENGTH))
					laraItem->Animation.TargetState = SKIDOO_STATE_JUMP_OFF;

				break;
			}
		}
	}

	int DoSkidooDynamics(int height, int verticalVelocity, int* y)
	{
		if (height > *y)
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

	int TestSkidooHeight(ItemInfo* skidooItem, int zOffset, int xOffset, Vector3Int* pos)
	{
		pos->y = skidooItem->Pose.Position.y - zOffset * phd_sin(skidooItem->Pose.Orientation.x) + xOffset * phd_sin(skidooItem->Pose.Orientation.z);

		float s = phd_sin(skidooItem->Pose.Orientation.y);
		float c = phd_cos(skidooItem->Pose.Orientation.y);

		pos->x = skidooItem->Pose.Position.x + zOffset * s + xOffset * c;
		pos->z = skidooItem->Pose.Position.z + zOffset * c - xOffset * s;

		auto probe = GetCollision(pos->x, pos->y, pos->z, skidooItem->RoomNumber);
		if (probe.Position.Ceiling > pos->y ||
			probe.Position.Ceiling == NO_HEIGHT)
		{
			return NO_HEIGHT;
		}

		return probe.Position.Floor;
	}

	short DoSkidooShift(ItemInfo* skidooItem, Vector3Int* pos, Vector3Int* old)
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
				skidooItem->Pose.Position.z += (old->z - pos->z);
				skidooItem->Pose.Position.x += (old->x - pos->x);
			}
			else if (z > zOld)
			{
				skidooItem->Pose.Position.z -= shiftZ + 1;
				return (pos->x - skidooItem->Pose.Position.x);
			}
			else
			{
				skidooItem->Pose.Position.z += SECTOR(1) - shiftZ;
				return (skidooItem->Pose.Position.x - pos->x);
			}
		}
		else if (z == zOld)
		{
			if (x > xOld)
			{
				skidooItem->Pose.Position.x -= shiftX + 1;
				return (skidooItem->Pose.Position.z - pos->z);
			}
			else
			{
				skidooItem->Pose.Position.x += SECTOR(1) - shiftX;
				return (pos->z - skidooItem->Pose.Position.z);
			}
		}
		else
		{
			x = 0;
			z = 0;

			auto probe = GetCollision(old->x, pos->y, pos->z, skidooItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->z > old->z)
					z = -shiftZ - 1;
				else
					z = SECTOR(1) - shiftZ;
			}

			probe = GetCollision(pos->x, pos->y, old->z, skidooItem->RoomNumber);
			if (probe.Position.Floor < (old->y - CLICK(1)))
			{
				if (pos->x > old->x)
					x = -shiftX - 1;
				else
					x = SECTOR(1) - shiftX;
			}

			if (x && z)
			{
				skidooItem->Pose.Position.z += z;
				skidooItem->Pose.Position.x += x;
				skidooItem->Animation.Velocity -= 50;
			}
			else if (z)
			{
				skidooItem->Pose.Position.z += z;
				skidooItem->Animation.Velocity -= 50;

				if (z > 0)
					return (skidooItem->Pose.Position.x - pos->x);
				else
					return (pos->x - skidooItem->Pose.Position.x);
			}
			else if (x)
			{
				skidooItem->Pose.Position.x += x;
				skidooItem->Animation.Velocity -= 50;

				if (x > 0)
					return (pos->z - skidooItem->Pose.Position.z);
				else
					return (skidooItem->Pose.Position.z - pos->z);
			}
			else
			{
				skidooItem->Pose.Position.z += old->z - pos->z;
				skidooItem->Pose.Position.x += old->x - pos->x;
				skidooItem->Animation.Velocity -= 50;
			}
		}

		return 0;
	}

	int SkidooDynamics(ItemInfo* laraItem, ItemInfo* skidooItem)
	{
		auto* skidoo = (SkidooInfo*)skidooItem->Data;

		Vector3Int frontLeftOld, frontRightOld, backLeftOld, backRightOld;
		auto heightFrontLeftOld = TestSkidooHeight(skidooItem, SKIDOO_FRONT, -SKIDOO_SIDE, &frontLeftOld);
		auto heightFrontRightOld = TestSkidooHeight(skidooItem, SKIDOO_FRONT, SKIDOO_SIDE, &frontRightOld);
		auto heightBackLeftOld = TestSkidooHeight(skidooItem, -SKIDOO_FRONT, -SKIDOO_SIDE, &backLeftOld);
		auto heightBackRightOld = TestSkidooHeight(skidooItem, -SKIDOO_FRONT, SKIDOO_SIDE, &backRightOld);

		Vector3Int old;
		old.x = skidooItem->Pose.Position.x;
		old.y = skidooItem->Pose.Position.y;
		old.z = skidooItem->Pose.Position.z;

		if (backLeftOld.y > heightBackLeftOld)
			backLeftOld.y = heightBackLeftOld;
		if (backRightOld.y > heightBackRightOld)
			backRightOld.y = heightBackRightOld;
		if (frontLeftOld.y > heightFrontLeftOld)
			frontLeftOld.y = heightFrontLeftOld;
		if (frontRightOld.y > heightFrontRightOld)
			frontRightOld.y = heightFrontRightOld;

		short rotation;

		if (skidooItem->Pose.Position.y > (skidooItem->Floor - CLICK(1)))
		{
			if (skidoo->TurnRate < -SKIDOO_UNDO_TURN)
				skidoo->TurnRate += SKIDOO_UNDO_TURN;
			else if (skidoo->TurnRate > SKIDOO_UNDO_TURN)
				skidoo->TurnRate -= SKIDOO_UNDO_TURN;
			else
				skidoo->TurnRate = 0;
			skidooItem->Pose.Orientation.y += skidoo->TurnRate + skidoo->ExtraRotation;

			rotation = skidooItem->Pose.Orientation.y - skidoo->MomentumAngle;
			if (rotation < -SKIDOO_MOMENTUM_TURN)
			{
				if (rotation < -SKIDOO_MAX_MOMENTUM_TURN)
				{
					rotation = -SKIDOO_MAX_MOMENTUM_TURN;
					skidoo->MomentumAngle = skidooItem->Pose.Orientation.y - rotation;
				}
				else
					skidoo->MomentumAngle -= SKIDOO_MOMENTUM_TURN;
			}
			else if (rotation > SKIDOO_MOMENTUM_TURN)
			{
				if (rotation > SKIDOO_MAX_MOMENTUM_TURN)
				{
					rotation = SKIDOO_MAX_MOMENTUM_TURN;
					skidoo->MomentumAngle = skidooItem->Pose.Orientation.y - rotation;
				}
				else
					skidoo->MomentumAngle += SKIDOO_MOMENTUM_TURN;
			}
			else
				skidoo->MomentumAngle = skidooItem->Pose.Orientation.y;
		}
		else
			skidooItem->Pose.Orientation.y += skidoo->TurnRate + skidoo->ExtraRotation;

		skidooItem->Pose.Position.z += skidooItem->Animation.Velocity * phd_cos(skidoo->MomentumAngle);
		skidooItem->Pose.Position.x += skidooItem->Animation.Velocity * phd_sin(skidoo->MomentumAngle);

		int slip = SKIDOO_SLIP * phd_sin(skidooItem->Pose.Orientation.x);
		if (abs(slip) > (SKIDOO_SLIP / 2))
		{
			skidooItem->Pose.Position.z -= slip * phd_cos(skidooItem->Pose.Orientation.y);
			skidooItem->Pose.Position.x -= slip * phd_sin(skidooItem->Pose.Orientation.y);
		}

		slip = SKIDOO_SLIP_SIDE * phd_sin(skidooItem->Pose.Orientation.z);
		if (abs(slip) > (SKIDOO_SLIP_SIDE / 2))
		{
			skidooItem->Pose.Position.z -= slip * phd_sin(skidooItem->Pose.Orientation.y);
			skidooItem->Pose.Position.x += slip * phd_cos(skidooItem->Pose.Orientation.y);
		}

		Vector3Int moved;
		moved.x = skidooItem->Pose.Position.x;
		moved.z = skidooItem->Pose.Position.z;

		if (!(skidooItem->Flags & ONESHOT))
			DoVehicleCollision(skidooItem, SKIDOO_RADIUS);

		Vector3Int frontLeft, frontRight, backRight, backLeft;
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

		auto probe = GetCollision(skidooItem);
		if (probe.Position.Floor < (skidooItem->Pose.Position.y - CLICK(1)))
			DoSkidooShift(skidooItem, (Vector3Int*)&skidooItem->Pose, &old);

		skidoo->ExtraRotation = rotation;

		auto collide = GetSkidooCollisionAnim(skidooItem, &moved);
		if (collide)
		{
			int newVelocity = (skidooItem->Pose.Position.z - old.z) * phd_cos(skidoo->MomentumAngle) + (skidooItem->Pose.Position.x - old.x) * phd_sin(skidoo->MomentumAngle);
			if (skidooItem->Animation.Velocity > (SKIDOO_MAX_VELOCITY + SKIDOO_ACCELERATION) &&
				newVelocity < (skidooItem->Animation.Velocity - 10))
			{
				DoDamage(laraItem, (skidooItem->Animation.Velocity - newVelocity) / 2);
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

	void DrawSkidoo(ItemInfo* skidooItem)
	{

	}
}
