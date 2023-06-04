#include "framework.h"
#include "Game/Lara/lara_flare.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/chaffFX.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/level.h"

using namespace TEN::Math;

constexpr auto FLARE_LIFE_MAX	 = 60.0f * FPS;
constexpr auto FLARE_DEATH_DELAY = 1.0f  * FPS;

void FlareControl(short itemNumber)
{
	auto& flareItem = g_Level.Items[itemNumber];

	if (TestEnvironment(ENV_FLAG_SWAMP, &flareItem))
	{
		KillItem(itemNumber);
		return;
	}

	if (flareItem.Animation.Velocity.y)
	{
		flareItem.Pose.Orientation.x -= ANGLE(5.0f);
		flareItem.Pose.Orientation.z += ANGLE(5.0f);
	}
	else
	{
		flareItem.Pose.Orientation.x = 0;
		flareItem.Pose.Orientation.z = 0;
	}

	auto vel = Vector3i(
		flareItem.Animation.Velocity.z * phd_sin(flareItem.Pose.Orientation.y),
		flareItem.Animation.Velocity.y,
		flareItem.Animation.Velocity.z * phd_cos(flareItem.Pose.Orientation.y));

	auto prevPos = flareItem.Pose.Position;
	flareItem.Pose.Position += Vector3i(vel.x, 0, vel.z);

	if (TestEnvironment(ENV_FLAG_WATER, &flareItem) ||
		TestEnvironment(ENV_FLAG_SWAMP, &flareItem))
	{
		flareItem.Animation.Velocity.y += (5 - flareItem.Animation.Velocity.y) / 2;
		flareItem.Animation.Velocity.z += (5 - flareItem.Animation.Velocity.z) / 2;
	}
	else
	{
		flareItem.Animation.Velocity.y += 6;
	}

	flareItem.Pose.Position.y += flareItem.Animation.Velocity.y;
	DoProjectileDynamics(itemNumber, prevPos.x, prevPos.y, prevPos.z, vel.x, vel.y, vel.z);

	int& life = flareItem.Data;
	life &= 0x7FFF;
	if (life >= FLARE_LIFE_MAX)
	{
		if (flareItem.Animation.Velocity.y == 0.0f &&
			flareItem.Animation.Velocity.z == 0.0f)
		{
			KillItem(itemNumber);
			return;
		}
	}
	else
	{
		life++;
	}

	if (DoFlareLight(flareItem.Pose.Position, life))
	{
		TriggerChaffEffects(flareItem, life);
		life |= 0x8000;
	}
}

void ReadyFlare(ItemInfo& laraItem)
{
	auto& player = *GetLaraInfo(&laraItem);

	player.Control.HandStatus = HandStatus::Free;
	player.LeftArm.Orientation =
	player.RightArm.Orientation = EulerAngles::Zero;
	player.LeftArm.Locked =
	player.RightArm.Locked = false;
	player.TargetEntity = nullptr;
}

void UndrawFlareMeshes(ItemInfo& laraItem)
{
	laraItem.Model.MeshIndex[LM_LHAND] = laraItem.Model.BaseMesh + LM_LHAND;
}

void DrawFlareMeshes(ItemInfo& laraItem)
{
	laraItem.Model.MeshIndex[LM_LHAND] = Objects[ID_FLARE_ANIM].meshIndex + LM_LHAND;
}

void UndrawFlare(ItemInfo& laraItem)
{
	auto& player = *GetLaraInfo(&laraItem);

	int flareFrame = player.Flare.Frame;
	int armFrame = player.LeftArm.FrameNumber;

	player.Flare.ControlLeft = true;

	if (laraItem.Animation.TargetState == LS_IDLE &&
		player.Context.Vehicle == NO_ITEM)
	{
		if (laraItem.Animation.AnimNumber == LA_STAND_IDLE)
		{
			laraItem.Animation.AnimNumber = LA_DISCARD_FLARE;
			flareFrame = armFrame + GetAnimData(laraItem).frameBase;
			laraItem.Animation.FrameNumber = flareFrame;
			player.Flare.Frame = flareFrame;
		}

		if (laraItem.Animation.AnimNumber == LA_DISCARD_FLARE)
		{
			player.Flare.ControlLeft = false;

			if (flareFrame >= (GetAnimData(laraItem).frameBase + 31)) // 31 = Last frame.
			{
				player.Control.Weapon.RequestGunType = player.Control.Weapon.LastGunType;
				player.Control.Weapon.GunType = player.Control.Weapon.LastGunType;
				player.Control.HandStatus = HandStatus::Free;

				InitializeNewWeapon(laraItem);

				player.TargetEntity = nullptr;
				player.LeftArm.Locked =
				player.RightArm.Locked = false;
				SetAnimation(laraItem, LA_STAND_IDLE);
				player.Flare.Frame = GetAnimData(laraItem).frameBase;
				return;
			}

			player.Flare.Frame++;
		}
	}
	else if (laraItem.Animation.AnimNumber == LA_DISCARD_FLARE)
	{
		SetAnimation(&laraItem, LA_STAND_IDLE);
	}

	if (armFrame >= 33 && armFrame < 72)
	{
		armFrame = 2;
		DoFlareInHand(laraItem, player.Flare.Life);
	}
	else if (!armFrame)
	{
		armFrame = 1;
		DoFlareInHand(laraItem, player.Flare.Life);
	}
	else if (armFrame >= 72 && armFrame < 95)
	{
		armFrame++;

		if (armFrame == 94)
		{
			armFrame = 1;
			DoFlareInHand(laraItem, player.Flare.Life);
		}
	}
	else if (armFrame >= 1 && armFrame < 33)
	{
		armFrame++;

		if (armFrame == 21)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, true);
			UndrawFlareMeshes(laraItem);
			player.Flare.Life = 0;
		}
		else if (armFrame == 33)
		{
			armFrame = 0;

			player.Control.Weapon.RequestGunType = player.Control.Weapon.LastGunType;
			player.Control.Weapon.GunType = player.Control.Weapon.LastGunType;
			player.Control.HandStatus = HandStatus::Free;

			InitializeNewWeapon(laraItem);

			player.TargetEntity = nullptr;
			player.LeftArm.Locked =
			player.RightArm.Locked = false;
			player.Flare.ControlLeft = false;
			player.Flare.Frame = 0;
		}
		else if (armFrame < 21)
		{
			DoFlareInHand(laraItem, player.Flare.Life);
		}
	}
	else if (armFrame >= 95 && armFrame < 110)
	{
		armFrame++;

		if (armFrame == 110)
		{
			armFrame = 1;
			DoFlareInHand(laraItem, player.Flare.Life);
		}
	}

	player.LeftArm.FrameNumber = armFrame;
	SetFlareArm(laraItem, player.LeftArm.FrameNumber);
}

void DrawFlare(ItemInfo& laraItem)
{
	auto& player = *GetLaraInfo(&laraItem);

	if (laraItem.Animation.ActiveState == LS_PICKUP_FLARE ||
		laraItem.Animation.ActiveState == LS_PICKUP)
	{
		DoFlareInHand(laraItem, player.Flare.Life);
		player.Flare.ControlLeft = false;
		player.LeftArm.FrameNumber = 93;
		SetFlareArm(laraItem, 93);
	}
	else
	{
		int armFrame = player.LeftArm.FrameNumber + 1;
		player.Flare.ControlLeft = true;

		if (armFrame < 33 || armFrame > 94)
		{
			armFrame = 33;
		}
		else if (armFrame == 46)
		{
			DrawFlareMeshes(laraItem);
		}
		else if (armFrame >= 72 && armFrame <= 93)
		{
			if (armFrame == 72)
			{
				player.Flare.Life = 1;
				SoundEffect(
					SFX_TR4_FLARE_IGNITE_DRY,
					&laraItem.Pose,
					TestEnvironment(ENV_FLAG_WATER, &laraItem) ? SoundEnvironment::Water : SoundEnvironment::Land);
			}

			DoFlareInHand(laraItem, player.Flare.Life);
		}
		else
		{
			if (armFrame == 94)
			{
				ReadyFlare(laraItem);
				armFrame = 0;
				DoFlareInHand(laraItem, player.Flare.Life);
			}
		}

		player.LeftArm.FrameNumber = armFrame;
		SetFlareArm(laraItem, armFrame);
	}
}

void SetFlareArm(ItemInfo& laraItem, int armFrame)
{
	auto& player = *GetLaraInfo(&laraItem);
	int flareAnimNumber = Objects[ID_FLARE_ANIM].animIndex;

	if (armFrame >= 95)
	{
		flareAnimNumber += 4;
	}
	else if (armFrame >= 72)
	{
		flareAnimNumber += 3;
	}
	else if (armFrame >= 33)
	{
		flareAnimNumber += 2;
	}
	else if (armFrame >= 1)
	{
		flareAnimNumber += 1;
	}

	player.LeftArm.AnimNumber = flareAnimNumber;
	player.LeftArm.FrameBase = GetAnimData(flareAnimNumber).FramePtr;
}

void CreateFlare(ItemInfo& laraItem, GAME_OBJECT_ID objectID, bool isThrown)
{
	const auto& lara = *GetLaraInfo(&laraItem);

	auto itemNumber = CreateItem();
	if (itemNumber == NO_ITEM)
		return;

	auto& flareItem = g_Level.Items[itemNumber];

	flareItem.ObjectNumber = objectID;
	flareItem.RoomNumber = laraItem.RoomNumber;

	auto pos = GetJointPosition(&laraItem, LM_LHAND, Vector3i(-16, 32, 42));

	flareItem.Pose.Position = pos;

	int floorHeight = GetCollision(pos.x, pos.y, pos.z, laraItem.RoomNumber).Position.Floor;
	auto hasCollided = GetCollidedObjects(&flareItem, 0, true, CollidedItems, CollidedMeshes, true);
	bool hasLanded = false;

	if (floorHeight < pos.y || hasCollided)
	{
		hasLanded = true;
		flareItem.Pose.Position.x = laraItem.Pose.Position.x + 320 * phd_sin(flareItem.Pose.Orientation.y);
		flareItem.Pose.Position.z = laraItem.Pose.Position.z + 320 * phd_cos(flareItem.Pose.Orientation.y);
		flareItem.Pose.Orientation.y = laraItem.Pose.Orientation.y + ANGLE(180.0f);
		flareItem.RoomNumber = laraItem.RoomNumber;
	}
	else
	{
		if (isThrown)
		{
			flareItem.Pose.Orientation.y = laraItem.Pose.Orientation.y;
		}
		else
		{
			flareItem.Pose.Orientation.y = laraItem.Pose.Orientation.y - ANGLE(45.0f);
		}

		flareItem.RoomNumber = laraItem.RoomNumber;
	}

	InitializeItem(itemNumber);

	flareItem.Pose.Orientation.x = 0;
	flareItem.Pose.Orientation.z = 0;
	flareItem.Model.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

	if (isThrown)
	{
		flareItem.Animation.Velocity.z = laraItem.Animation.Velocity.z + 50;
		flareItem.Animation.Velocity.y = laraItem.Animation.Velocity.y - 50;
	}
	else
	{
		flareItem.Animation.Velocity.z = laraItem.Animation.Velocity.z + 10;
		flareItem.Animation.Velocity.y = laraItem.Animation.Velocity.y + 50;
	}

	if (hasLanded)
		flareItem.Animation.Velocity.z /= 2;

	if (objectID == ID_FLARE_ITEM)
	{
		flareItem.Data = (int)0;
		int& life = flareItem.Data;

		if (DoFlareLight(flareItem.Pose.Position, lara.Flare.Life))
			life = lara.Flare.Life | 0x8000;
		else
			life = lara.Flare.Life & 0x7FFF;
	}
	else
	{
		flareItem.ItemFlags[3] = lara.Torch.IsLit;
	}

	AddActiveItem(itemNumber);
	flareItem.Status = ITEM_ACTIVE;
}

void DoFlareInHand(ItemInfo& laraItem, int flareLife)
{
	auto& lara = *GetLaraInfo(&laraItem);

	auto pos = GetJointPosition(&laraItem, LM_LHAND, Vector3i(11, 32, 41));

	if (DoFlareLight(pos, flareLife))
		TriggerChaffEffects(BinocularOn ? 0 : flareLife);

	if (lara.Flare.Life >= FLARE_LIFE_MAX - (FLARE_DEATH_DELAY / 2))
	{
		// Prevent player from intercepting reach/jump states with flare throws.
		if (laraItem.Animation.IsAirborne ||
			laraItem.Animation.TargetState == LS_JUMP_PREPARE ||
			laraItem.Animation.TargetState == LS_JUMP_FORWARD)
		{
			return;
		}

		if (lara.Control.HandStatus == HandStatus::Free)
			lara.Control.HandStatus = HandStatus::WeaponUndraw;
	}
	else if (lara.Flare.Life != 0)
	{
		lara.Flare.Life++;
	}
}

bool DoFlareLight(const Vector3i& pos, int flareLife)
{
	constexpr auto START_DELAY				 = 0.25f * FPS;
	constexpr auto END_DELAY				 = 3.0f  * FPS;
	constexpr auto INTENSITY_MAX			 = 1.0f;
	constexpr auto INTENSITY_MIN			 = 0.9f;
	constexpr auto CHAFF_SPAWN_CHANCE		 = 4 / 10.0f;
	constexpr auto CHAFF_SPAWN_ENDING_CHANCE = CHAFF_SPAWN_CHANCE / 2;
	constexpr auto CHAFF_SPAWN_DYING_CHANCE	 = CHAFF_SPAWN_CHANCE / 4;
	constexpr auto LIGHT_RADIUS				 = 9.0f;
	constexpr auto LIGHT_SPHERE_RADIUS		 = BLOCK(1 / 16.0f);
	constexpr auto LIGHT_POS_OFFSET			 = Vector3(0.0f, -BLOCK(1 / 8.0f), 0.0f);
	constexpr auto LIGHT_COLOR				 = Vector3(0.9f, 0.5f, 0.3f);

	if (flareLife >= FLARE_LIFE_MAX || flareLife == 0)
		return false;

	// Determine flare progress.
	bool isStarting = (flareLife <= START_DELAY);
	bool isEnding   = (flareLife >  (FLARE_LIFE_MAX - END_DELAY));
	bool isDying    = (flareLife >  (FLARE_LIFE_MAX - FLARE_DEATH_DELAY));

	bool spawnChaff = false;
	float mult = 1.0f;

	// Define light multiplier and chaff spawn status.
	if (isStarting)
	{
		mult -= 0.5f * (1.0f - ((float)flareLife / START_DELAY));
	}
	else if (isDying)
	{
		mult = (FLARE_LIFE_MAX - (float)flareLife) / FLARE_DEATH_DELAY;
		spawnChaff = Random::TestProbability(CHAFF_SPAWN_DYING_CHANCE);
	}
	else if (isEnding)
	{
		mult = Random::GenerateFloat(0.8f, 1.0f);
		spawnChaff = Random::TestProbability(CHAFF_SPAWN_ENDING_CHANCE);
	}
	else
	{
		spawnChaff = Random::TestProbability(CHAFF_SPAWN_CHANCE);
	}

	// Determine light position.
	auto sphere = BoundingSphere(pos.ToVector3() + LIGHT_POS_OFFSET, LIGHT_SPHERE_RADIUS);
	auto lightPos = Random::GeneratePointInSphere(sphere);

	// Calculate color.
	float intensity = Random::GenerateFloat(INTENSITY_MIN, INTENSITY_MAX);
	float falloff = intensity * mult * LIGHT_RADIUS;
	auto color = (LIGHT_COLOR * intensity * std::clamp(mult, 0.0f, 1.0f)) * UCHAR_MAX;

	TriggerDynamicLight(lightPos.x, lightPos.y, lightPos.z, (int)falloff, color.x, color.y, color.z);

	// Return chaff spawn status.
	return ((isDying || isEnding) ? spawnChaff : true);
}
