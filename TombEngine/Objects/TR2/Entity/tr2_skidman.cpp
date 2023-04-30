#include "framework.h"
#include "Objects/TR2/Entity/tr2_skidman.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/smoke.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Setup.h"
#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Effects::Smoke;

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto SMAN_WAIT_RANGE = SQUARE(BLOCK(4));

	const auto SkidooBiteLeft  = CreatureBiteInfo(Vector3i(240, -80, 540), 0);
	const auto SkidooBiteRight = CreatureBiteInfo(Vector3i(-240, -80, 540), 0);
	const auto SkidooBiteSmokeLeft = CreatureBiteInfo(Vector3i(240, -80, 450), 0);
	const auto SkidooBiteSmokeRight = CreatureBiteInfo(Vector3i(-240, -80, 450), 0);

	constexpr auto SMAN_MIN_TURN = ANGLE(2.0f);
	constexpr auto SMAN_TARGET_ANGLE = ANGLE(30.0f);

	enum SnowmobileManState
	{
		// No state 0.
		SMAN_STATE_WAIT = 1,
		SMAN_STATE_MOVING = 2,
		SMAN_STATE_START_LEFT = 3,
		SMAN_STATE_START_RIGHT = 4,
		SMAN_STATE_LEFT = 5,
		SMAN_STATE_RIGHT = 6,
		SMAN_STATE_DEATH = 7
	};

	// TODO
	enum SkidooManAnim
	{
		SMAN_ANIM_DEATH = 10
	};

	static void CreateSnowmobileGun(ItemInfo* driver)
	{
		short snowmobileNumber = CreateItem();
		if (snowmobileNumber != NO_ITEM)
		{
			auto* snowmobileGunItem = &g_Level.Items[snowmobileNumber];
			snowmobileGunItem->Pose.Position = driver->Pose.Position;
			snowmobileGunItem->Pose.Orientation.y = driver->Pose.Orientation.y;
			snowmobileGunItem->RoomNumber = driver->RoomNumber;
			snowmobileGunItem->ObjectNumber = ID_SNOWMOBILE_GUN;
			snowmobileGunItem->Model.Color = driver->Model.Color;
			snowmobileGunItem->Flags = IFLAG_ACTIVATION_MASK;
			InitializeItem(snowmobileNumber); g_Level.NumItems++;
			driver->Data = snowmobileNumber; // Register the snowmobile gun for the driver to control it.
		}
		else
		{
			TENLog("Failed to create the ID_SNOWMOBILE_GUN from ID_SNOWMOBILE_DRIVER.", LogLevel::Warning);
		}
	}

	void InitializeSkidooMan(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		if (item->Flags & IFLAG_REVERSE)
			item->Status &= ~ITEM_INVISIBLE;
		else
			item->Status = ITEM_INVISIBLE;
	}

	void SkidooManCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
			return;

		if (!TestCollision(item, laraItem))
			return;

		if (coll->Setup.EnableObjectPush)
		{
			if (item->Animation.Velocity.z > 0.0f)
				ItemPushItem(item, laraItem, coll, coll->Setup.EnableSpasm, 0);
			else
				ItemPushItem(item, laraItem, coll, false, 0);
		}

		if (Lara.Context.Vehicle == NO_ITEM && item->Animation.Velocity.z > 0.0f)
			DoDamage(laraItem, 100);
	}

	void SkidooManControl(short riderItemNumber)
	{
		auto* rider = &g_Level.Items[riderItemNumber];
		if (!rider->Data)
		{
			// Create the snowmobile.
			CreateSnowmobileGun(rider);
			if (!rider->Data)
				TENLog("Rider data does not contain the skidoo itemNumber !", LogLevel::Error);
			return;
		}

		short itemNumber = (short)rider->Data;
		auto* skidoo = &g_Level.Items[itemNumber];

		if (!skidoo->Data)
		{
			EnableEntityAI(itemNumber, true);
			skidoo->Status = ITEM_ACTIVE;
		}

		auto* creatureInfo = GetCreatureInfo(skidoo);
		short angle = 0;

		if (creatureInfo->MuzzleFlash[0].Delay != 0)
			creatureInfo->MuzzleFlash[0].Delay--;
		if (creatureInfo->MuzzleFlash[1].Delay != 0)
			creatureInfo->MuzzleFlash[1].Delay--;

		AI_INFO AI;
		if (skidoo->HitPoints <= 0)
		{
			if (rider->Animation.ActiveState != SMAN_STATE_DEATH)
			{
				rider->Pose.Position = skidoo->Pose.Position;
				rider->Pose.Orientation.y = skidoo->Pose.Orientation.y;
				rider->RoomNumber = skidoo->RoomNumber;

				rider->Animation.AnimNumber = Objects[ID_SNOWMOBILE_DRIVER].animIndex + SMAN_ANIM_DEATH;
				rider->Animation.FrameNumber = g_Level.Anims[rider->Animation.AnimNumber].frameBase;
				rider->Animation.ActiveState = SMAN_STATE_DEATH;

				if (Lara.TargetEntity == skidoo)
					Lara.TargetEntity = nullptr;
			}
			else
				AnimateItem(rider);

			if (skidoo->Animation.ActiveState == SMAN_STATE_MOVING || skidoo->Animation.ActiveState == SMAN_STATE_WAIT)
				skidoo->Animation.TargetState = SMAN_STATE_WAIT;
			else
				skidoo->Animation.TargetState = SMAN_STATE_MOVING;
		}
		else
		{
			CreatureAIInfo(skidoo, &AI);

			GetCreatureMood(skidoo, &AI, true);
			CreatureMood(skidoo, &AI, true);

			angle = CreatureTurn(skidoo, ANGLE(3.0f));

			switch (skidoo->Animation.ActiveState)
			{
			case SMAN_STATE_WAIT:
				if (creatureInfo->Mood == MoodType::Bored)
					break;
				else if (abs(AI.angle) < SMAN_TARGET_ANGLE && AI.distance < SMAN_WAIT_RANGE)
					break;

				skidoo->Animation.TargetState = SMAN_STATE_MOVING;
				break;

			case SMAN_STATE_MOVING:
				if (creatureInfo->Mood == MoodType::Bored)
					skidoo->Animation.TargetState = SMAN_STATE_WAIT;
				else if (abs(AI.angle) < SMAN_TARGET_ANGLE && AI.distance < SMAN_WAIT_RANGE)
					skidoo->Animation.TargetState = SMAN_STATE_WAIT;
				else if (angle < -SMAN_MIN_TURN)
					skidoo->Animation.TargetState = SMAN_STATE_START_LEFT;
				else if (angle > SMAN_MIN_TURN)
					skidoo->Animation.TargetState = SMAN_STATE_START_RIGHT;

				break;

			case SMAN_STATE_START_LEFT:
			case SMAN_STATE_LEFT:
				if (angle < -SMAN_MIN_TURN)
					skidoo->Animation.TargetState = SMAN_STATE_LEFT;
				else
					skidoo->Animation.TargetState = SMAN_STATE_MOVING;

				break;

			case SMAN_STATE_START_RIGHT:
			case SMAN_STATE_RIGHT:
				if (angle < -SMAN_MIN_TURN)
					skidoo->Animation.TargetState = SMAN_STATE_LEFT;
				else
					skidoo->Animation.TargetState = SMAN_STATE_MOVING;

				break;
			}
		}

		if (rider->Animation.ActiveState != SMAN_STATE_DEATH)
		{
			if (creatureInfo->Flags == 0 && abs(AI.angle) < SMAN_TARGET_ANGLE && creatureInfo->Enemy->HitPoints > 0)
			{
				int damage = (creatureInfo->Enemy->IsLara() && GetLaraInfo(creatureInfo->Enemy)->Context.Vehicle != NO_ITEM) ? 10 : 50;
				
				ShotLara(skidoo, &AI, SkidooBiteLeft, 0, damage);
				auto jointPos = GetJointPosition(skidoo, SkidooBiteSmokeLeft);
				SpawnGunSmokeParticles(jointPos.ToVector3(), Vector3::Zero, skidoo->RoomNumber, 1, LaraWeaponType::Snowmobile, 16);
				creatureInfo->MuzzleFlash[0].Bite = SkidooBiteLeft;
				creatureInfo->MuzzleFlash[0].Delay = 1;
				creatureInfo->MuzzleFlash[0].SwitchToMuzzle2 = true;
				creatureInfo->MuzzleFlash[0].ApplyXRotation = false;
				creatureInfo->MuzzleFlash[0].UseSmoke = false;

				ShotLara(skidoo, &AI, SkidooBiteRight, 0, damage);
				jointPos = GetJointPosition(skidoo, SkidooBiteSmokeRight);
				SpawnGunSmokeParticles(jointPos.ToVector3(), Vector3::Zero, skidoo->RoomNumber, 1, LaraWeaponType::Snowmobile, 16);
				creatureInfo->MuzzleFlash[1].Bite = SkidooBiteRight;
				creatureInfo->MuzzleFlash[1].Delay = 1;
				creatureInfo->MuzzleFlash[1].SwitchToMuzzle2 = true;
				creatureInfo->MuzzleFlash[1].ApplyXRotation = false;
				creatureInfo->MuzzleFlash[1].UseSmoke = false;

				creatureInfo->Flags = 4;
			}

			if (creatureInfo->Flags != 0)
			{
				SoundEffect(SFX_TR4_BADDY_UZI, &skidoo->Pose);
				creatureInfo->Flags--;
			}
		}

		if (skidoo->Animation.ActiveState == SMAN_STATE_WAIT)
		{
			SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &skidoo->Pose);
			creatureInfo->JointRotation[0] = 0;
		}
		else
		{
			creatureInfo->JointRotation[0] = (creatureInfo->JointRotation[0] == 1) ? 2 : 1;
			DoSnowEffect(skidoo);
			SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &skidoo->Pose, SoundEnvironment::Land, 0.5f + skidoo->Animation.Velocity.z / 100.0f); // SKIDOO_MAX_VELOCITY.  TODO: Check actual sound!
		}

		CreatureAnimation(itemNumber, angle, 0);

		if (rider->Animation.ActiveState != SMAN_STATE_DEATH)
		{
			rider->Pose.Position = skidoo->Pose.Position;
			rider->Pose.Orientation.y = skidoo->Pose.Orientation.y;

			if (skidoo->RoomNumber != rider->RoomNumber)
				ItemNewRoom(riderItemNumber, skidoo->RoomNumber);

			rider->Animation.AnimNumber = skidoo->Animation.AnimNumber + (Objects[ID_SNOWMOBILE_DRIVER].animIndex - Objects[ID_SNOWMOBILE_GUN].animIndex);
			rider->Animation.FrameNumber = skidoo->Animation.FrameNumber + (g_Level.Anims[rider->Animation.AnimNumber].frameBase - g_Level.Anims[skidoo->Animation.AnimNumber].frameBase);
		}
		else if (rider->Status == ITEM_DEACTIVATED &&
			skidoo->Animation.Velocity.z == 0 &&
			skidoo->Animation.Velocity.y == 0)
		{
			RemoveActiveItem(riderItemNumber);
			rider->Collidable = false;
			rider->HitPoints = NOT_TARGETABLE;
			rider->Flags |= IFLAG_INVISIBLE;

			DisableEntityAI(itemNumber);
			skidoo->ObjectNumber = ID_SNOWMOBILE;
			skidoo->Status = ITEM_DEACTIVATED;

			InitializeSkidoo(itemNumber);
			if (skidoo->Data.is<SkidooInfo>())
			{
				auto* skidooData = (SkidooInfo*)skidoo->Data;
				skidooData->Armed = true;
			}
		}
	}
}
