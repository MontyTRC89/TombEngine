#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Game/effects/Bubble.h"
#include "Game/effects/Drip.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_tests.h"
#include "Game/savegame.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR3/Vehicles/big_gun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad_bike.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

using namespace TEN::Control::Volumes;
using namespace TEN::Effects::Bubble;
using namespace TEN::Effects::Drip;
using namespace TEN::Collision::Floordata;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Update AFK pose timer.
	if (lara->Control.Count.Pose < LARA_POSE_TIME && TestLaraPose(item, coll) &&
		!(IsHeld(In::Look) || IsOpticActionHeld()) &&
		g_GameFlow->HasAFKPose())
	{
		lara->Control.Count.Pose++;
	}
	else
	{
		lara->Control.Count.Pose = 0;
	}

	// Reset running jump timer.
	if (!IsRunJumpCountableState(item->Animation.ActiveState))
		lara->Control.Count.Run = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState(item->Animation.ActiveState))
		lara->Control.RunJumpQueued = false;

	// Reset lean.
	if ((!lara->Control.IsMoving || (lara->Control.IsMoving && !(IsHeld(In::Left) || IsHeld(In::Right)))) &&
		(!lara->Control.IsLow && item->Animation.ActiveState != LS_DEATH)) // HACK: Don't interfere with surface alignment in crouch, crawl, and death states.
	{
		ResetPlayerLean(item, 1 / 6.0f);
	}

	// Reset crawl flex.
	if (!IsHeld(In::Look) && coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM && // HACK
		(item->Animation.Velocity.z == 0.0f || (item->Animation.Velocity.z != 0.0f && !(IsHeld(In::Left) || IsHeld(In::Right)))))
	{
		ResetPlayerFlex(item, 0.1f);
	}

	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate;
	if (!(IsHeld(In::Left) || IsHeld(In::Right)))
		lara->Control.TurnRate = 0;

	lara->Control.IsLow = false;
	lara->Control.IsMonkeySwinging = false;
}

void HandlePlayerStatusEffects(ItemInfo& item, WaterStatus waterStatus, PlayerWaterData& water)
{
	auto& player = GetLaraInfo(item);

	// Update health status.
	if (TestEnvironment(ENV_FLAG_DAMAGE, &item) && item.HitPoints > 0)
		item.HitPoints--;

	// Update poison status.
	if (player.Status.Poison)
	{
		if (player.Status.Poison > LARA_POISON_MAX)
			player.Status.Poison = LARA_POISON_MAX;

		if (!(Wibble & 0xFF))
			item.HitPoints -= player.Status.Poison;
	}

	// Update stamina status.
	if (player.Status.Stamina < LARA_STAMINA_MAX && item.Animation.ActiveState != LS_SPRINT)
		player.Status.Stamina++;

	// TODO: Dehardcode values and make cleaner implementation.
	// Handle environmental status effects.
	switch (waterStatus)
	{
	case WaterStatus::Dry:
	case WaterStatus::Wade:
		// TODO: Find best height. -- Sezz 2021.11.10
		if (water.IsSwamp && player.Context.WaterSurfaceDist < -(LARA_HEIGHT + 8))
		{
			if (item.HitPoints >= 0)
			{
				player.Status.Air -= 6;
				if (player.Status.Air < 0)
				{
					player.Status.Air = -1;
					item.HitPoints -= 10;
				}
			}
		}
		else if (player.Status.Air < LARA_AIR_MAX && item.HitPoints >= 0)
		{
			// HACK: Special case for UPV.
			if (player.Context.Vehicle == NO_ITEM)
			{
				player.Status.Air += 10;
				if (player.Status.Air > LARA_AIR_MAX)
					player.Status.Air = LARA_AIR_MAX;
			}
		}

		if (item.HitPoints >= 0)
		{
			if (player.Control.WaterStatus == WaterStatus::Dry)
			{
				// HACK: Special case for UPV.
				if (player.Context.Vehicle != NO_ITEM)
				{
					const auto& vehicleItem = g_Level.Items[player.Context.Vehicle];
					if (vehicleItem.ObjectNumber == ID_UPV)
					{
						auto pointColl = GetCollision(&item, 0, 0, CLICK(1));

						water.IsCold = (water.IsCold || TestEnvironment(ENV_FLAG_COLD, pointColl.RoomNumber));
						if (water.IsCold)
						{
							player.Status.Exposure--;
							if (player.Status.Exposure <= 0)
							{
								player.Status.Exposure = 0;
								item.HitPoints -= 10;
							}
						}
					}
				}
				else
				{
					player.Status.Exposure++;
					if (player.Status.Exposure >= LARA_EXPOSURE_MAX)
						player.Status.Exposure = LARA_EXPOSURE_MAX;
				}
			}
			else
			{
				if (water.IsCold)
				{
					player.Status.Exposure--;
					if (player.Status.Exposure <= 0)
					{
						player.Status.Exposure = 0;
						item.HitPoints -= 10;
					}
				}
				else
				{
					player.Status.Exposure++;
					if (player.Status.Exposure >= LARA_EXPOSURE_MAX)
						player.Status.Exposure = LARA_EXPOSURE_MAX;
				}
			}
		}

		break;

	case WaterStatus::Underwater:
		if (item.HitPoints >= 0)
		{
			const auto& level = *g_GameFlow->GetLevel(CurrentLevel);
			if (level.GetLaraType() != LaraType::Divesuit)
				player.Status.Air--;

			if (player.Status.Air < 0)
			{
				item.HitPoints -= 5;
				player.Status.Air = -1;
			}

			if (water.IsCold)
			{
				player.Status.Exposure -= 2;
				if (player.Status.Exposure <= 0)
				{
					player.Status.Exposure = 0;
					item.HitPoints -= 10;
				}
			}
			else
			{
				player.Status.Exposure++;
				if (player.Status.Exposure >= LARA_EXPOSURE_MAX)
					player.Status.Exposure = LARA_EXPOSURE_MAX;
			}
		}

		break;

	case WaterStatus::TreadWater:
		if (item.HitPoints >= 0)
		{
			player.Status.Air += 10;
			if (player.Status.Air > LARA_AIR_MAX)
				player.Status.Air = LARA_AIR_MAX;

			if (water.IsCold)
			{
				player.Status.Exposure -= 2;
				if (player.Status.Exposure <= 0)
				{
					player.Status.Exposure = 0;
					item.HitPoints -= 10;
				}
			}
		}

		break;

	default:
		break;
	}

	// Update death counter.
	if (item.HitPoints <= 0)
	{
		item.HitPoints = -1;

		if (player.Control.Count.Death == 0)
			StopSoundTracks(true);

		player.Control.Count.Death++;
		if ((item.Flags & IFLAG_INVISIBLE))
		{
			player.Control.Count.Death++;
			return;
		}
	}
}

static void UsePlayerMedipack(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	// Can't use medipack; return early.
	if (item.HitPoints <= 0 ||
		(item.HitPoints >= LARA_HEALTH_MAX && player.Status.Poison == 0))
	{
		return;
	}

	bool hasUsedMedipack = false;

	if (IsClicked(In::SmallMedipack) &&
		player.Inventory.TotalSmallMedipacks != 0)
	{
		hasUsedMedipack = true;

		item.HitPoints += LARA_HEALTH_MAX / 2;
		if (item.HitPoints > LARA_HEALTH_MAX)
			item.HitPoints = LARA_HEALTH_MAX;

		if (player.Inventory.TotalSmallMedipacks != -1)
			player.Inventory.TotalSmallMedipacks--;
	}
	else if (IsClicked(In::LargeMedipack) &&
		player.Inventory.TotalLargeMedipacks != 0)
	{
		hasUsedMedipack = true;
		item.HitPoints = LARA_HEALTH_MAX;

		if (player.Inventory.TotalLargeMedipacks != -1)
			player.Inventory.TotalLargeMedipacks--;
	}

	if (hasUsedMedipack)
	{
		player.Status.Poison = 0;
		Statistics.Game.HealthUsed++;
		SoundEffect(SFX_TR4_MENU_MEDI, nullptr, SoundEnvironment::Always);
	}
}

static std::optional<LaraWeaponType> GetPlayerScrolledWeaponType(const ItemInfo& item, LaraWeaponType currentWeaponType, bool getPrev)
{
	static const auto SCROLL_WEAPON_TYPES = std::vector<LaraWeaponType>
	{
		LaraWeaponType::Pistol,
		LaraWeaponType::Shotgun,
		LaraWeaponType::Uzi,
		LaraWeaponType::Revolver,
		LaraWeaponType::GrenadeLauncher,
		LaraWeaponType::Crossbow,
		LaraWeaponType::HarpoonGun,
		LaraWeaponType::HK,
		LaraWeaponType::RocketLauncher
	};

	auto getNextIndex = [getPrev](unsigned int index)
	{
		return (index + (getPrev ? ((unsigned int)SCROLL_WEAPON_TYPES.size() - 1) : 1)) % (unsigned int)SCROLL_WEAPON_TYPES.size();
	};

	auto& player = GetLaraInfo(item);

	// Get vector index for current weapon type.
	auto currentIndex = std::optional<unsigned int>(std::nullopt);
	for (int i = 0; i < SCROLL_WEAPON_TYPES.size(); i++)
	{
		if (SCROLL_WEAPON_TYPES[i] == currentWeaponType)
		{
			currentIndex = i;
			break;
		}
	}

	// Invalid current weapon type; return nullopt.
	if (!currentIndex.has_value())
		return std::nullopt;

	// Get next valid weapon type in sequence.
	unsigned int nextIndex = getNextIndex(*currentIndex);
	while (nextIndex != *currentIndex)
	{
		auto nextWeaponType = SCROLL_WEAPON_TYPES[nextIndex];
		if (player.Weapons[(int)nextWeaponType].Present)
			return nextWeaponType;

		nextIndex = getNextIndex(nextIndex);
	}

	// No valid weapon type; return nullopt.
	return std::nullopt;
}

void HandlePlayerQuickActions(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	// Handle medipacks.
	if (IsClicked(In::SmallMedipack) || IsClicked(In::LargeMedipack))
		UsePlayerMedipack(item);

	// Handle weapon scroll request.
	if (IsClicked(In::PreviousWeapon) || IsClicked(In::NextWeapon))
	{
		bool getPrev = IsClicked(In::PreviousWeapon);
		auto weaponType = GetPlayerScrolledWeaponType(item, player.Control.Weapon.GunType, getPrev);

		if (weaponType.has_value())
			player.Control.Weapon.RequestGunType = *weaponType;
	}

	// Handle weapon requests.
	if (IsClicked(In::Weapon1) && player.Weapons[(int)LaraWeaponType::Pistol].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::Pistol;

	if (IsClicked(In::Weapon2) && player.Weapons[(int)LaraWeaponType::Shotgun].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::Shotgun;

	if (IsClicked(In::Weapon3) && player.Weapons[(int)LaraWeaponType::Uzi].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::Uzi;

	if (IsClicked(In::Weapon4) && player.Weapons[(int)LaraWeaponType::Revolver].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::Revolver;

	if (IsClicked(In::Weapon5) && player.Weapons[(int)LaraWeaponType::GrenadeLauncher].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::GrenadeLauncher;

	if (IsClicked(In::Weapon6) && player.Weapons[(int)LaraWeaponType::Crossbow].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::Crossbow;

	if (IsClicked(In::Weapon7) && player.Weapons[(int)LaraWeaponType::HarpoonGun].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::HarpoonGun;

	if (IsClicked(In::Weapon8) && player.Weapons[(int)LaraWeaponType::HK].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::HK;

	if (IsClicked(In::Weapon9) && player.Weapons[(int)LaraWeaponType::RocketLauncher].Present)
		player.Control.Weapon.RequestGunType = LaraWeaponType::RocketLauncher;

	// TODO: 10th possible weapon, probably grapple gun.
	/*if (IsClicked(In::Weapon10) && player.Weapons[(int)LaraWeaponType::].Present)
	player.Control.Weapon.RequestGunType = LaraWeaponType::;*/
}

bool CanPlayerLookAround(const ItemInfo& item)
{
	const auto& player = GetLaraInfo(item);

	// 1) Check if drawn weapon has lasersight.
	if (player.Control.HandStatus == HandStatus::WeaponReady &&
		player.Weapons[(int)player.Control.Weapon.GunType].HasLasersight)
	{
		return true;
	}

	// 2) Test for switchable target.
	if (player.Control.HandStatus == HandStatus::WeaponReady &&
		player.TargetEntity != nullptr)
	{
		unsigned int targetableCount = 0;
		for (const auto* targetPtr : player.TargetList)
		{
			if (targetPtr != nullptr)
				targetableCount++;

			if (targetableCount > 1)
				return false;
		}
	}

	return true;
}

static void ClearPlayerLookAroundActions(const ItemInfo& item)
{
	const auto& player = GetLaraInfo(item);

	switch (player.Control.Look.Mode)
	{
	default:
	case LookMode::None:
		break;

	case LookMode::Vertical:
		ClearAction(In::Forward);
		ClearAction(In::Back);
		break;

	case LookMode::Horizontal:
		ClearAction(In::Left);
		ClearAction(In::Right);
		break;

	case LookMode::Free:
		ClearAction(In::Forward);
		ClearAction(In::Back);
		ClearAction(In::Left);
		ClearAction(In::Right);
		break;
	}
}

static void SetPlayerOptics(ItemInfo* item)
{
	constexpr auto OPTIC_RANGE_DEFAULT = ANGLE(0.7f);

	auto& player = GetLaraInfo(*item);

	bool breakOptics = true;

	// Standing; can use optics.
	if (item->Animation.ActiveState == LS_IDLE || item->Animation.AnimNumber == LA_STAND_IDLE)
		breakOptics = false;

	// Crouching; can use optics.
	if ((player.Control.IsLow || !IsHeld(In::Crouch)) &&
		(item->Animation.TargetState == LS_CROUCH_IDLE || item->Animation.AnimNumber == LA_CROUCH_IDLE))
	{
		breakOptics = false;
	}

	// If lasersight, and Look is not pressed, exit optics.
	if (player.Control.Look.IsUsingLasersight && !IsHeld(In::Look))
		breakOptics = true;

	// If lasersight and weapon is holstered, exit optics.
	if (player.Control.Look.IsUsingLasersight && IsHeld(In::Draw))
		breakOptics = true;

	// Engage lasersight if available.
	if (!player.Control.Look.IsUsingLasersight && !breakOptics && IsHeld(In::Look))
	{
		if (player.Control.HandStatus == HandStatus::WeaponReady &&
			((player.Control.Weapon.GunType == LaraWeaponType::HK && player.Weapons[(int)LaraWeaponType::HK].HasLasersight) ||
				(player.Control.Weapon.GunType == LaraWeaponType::Revolver && player.Weapons[(int)LaraWeaponType::Revolver].HasLasersight) ||
				(player.Control.Weapon.GunType == LaraWeaponType::Crossbow && player.Weapons[(int)LaraWeaponType::Crossbow].HasLasersight)))
		{
			player.Control.Look.OpticRange = OPTIC_RANGE_DEFAULT;
			player.Control.Look.IsUsingBinoculars = true;
			player.Control.Look.IsUsingLasersight = true;
			player.Inventory.IsBusy = true;
			BinocularOldCamera = Camera.oldType;
			return;
		}
	}

	if (!breakOptics)
		return;

	// Noth using optics; return early.
	if (!player.Control.Look.IsUsingBinoculars && !player.Control.Look.IsUsingLasersight)
		return;

	player.Control.Look.OpticRange = 0;
	player.Control.Look.IsUsingBinoculars = false;
	player.Control.Look.IsUsingLasersight = false;
	player.Inventory.IsBusy = false;

	Camera.type = BinocularOldCamera;
	Camera.bounce = 0;
	AlterFOV(LastFOV);
}

static short NormalizeLookAroundTurnRate(short turnRate, short opticRange)
{
	constexpr auto ZOOM_LEVEL_MAX = ANGLE(10.0f);
	constexpr auto ZOOM_LEVEL_REF = ANGLE(17.0f);

	if (opticRange == 0)
		return turnRate;

	return short(turnRate * (ZOOM_LEVEL_MAX - opticRange) / ZOOM_LEVEL_REF);
};

void HandlePlayerLookAround(ItemInfo& item, bool invertXAxis)
{
	constexpr auto OPTIC_RANGE_MAX	= ANGLE(8.5f);
	constexpr auto OPTIC_RANGE_MIN	= ANGLE(0.7f);
	constexpr auto OPTIC_RANGE_RATE = ANGLE(0.35f);
	constexpr auto TURN_RATE_MAX	= ANGLE(4.0f);
	constexpr auto TURN_RATE_ACCEL	= ANGLE(0.75f);

	auto& player = GetLaraInfo(item);

	if (!CanPlayerLookAround(item))
		return;

	// Set optics.
	Camera.type = CameraType::Look;
	SetPlayerOptics(LaraItem);

	bool isSlow = IsHeld(In::Walk);

	// Zoom optics.
	if (player.Control.Look.IsUsingBinoculars || player.Control.Look.IsUsingLasersight)
	{
		short rangeRate = isSlow ? (OPTIC_RANGE_RATE / 2) : OPTIC_RANGE_RATE;

		// NOTE: Zooming allowed with either StepLeft/StepRight or Walk/Sprint.
		if ((IsHeld(In::StepLeft) && !IsHeld(In::StepRight)) ||
			(IsHeld(In::Walk) && !IsHeld(In::Sprint)))
		{
			player.Control.Look.OpticRange -= rangeRate;
			if (player.Control.Look.OpticRange < OPTIC_RANGE_MIN)
			{
				player.Control.Look.OpticRange = OPTIC_RANGE_MIN;
			}
			else
			{
				SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 0.9f);
			}
		}
		else if ((IsHeld(In::StepRight) && !IsHeld(In::StepLeft)) ||
			(IsHeld(In::Sprint) && !IsHeld(In::Walk)))
		{
			player.Control.Look.OpticRange += rangeRate;
			if (player.Control.Look.OpticRange > OPTIC_RANGE_MAX)
			{
				player.Control.Look.OpticRange = OPTIC_RANGE_MAX;
			}
			else
			{
				SoundEffect(SFX_TR4_BINOCULARS_ZOOM, nullptr, SoundEnvironment::Land, 1.0f);
			}
		}
	}

	auto axisCoeff = Vector2::Zero;

	// Determine X axis coefficient.
	if ((IsHeld(In::Forward) || IsHeld(In::Back)) &&
		(player.Control.Look.Mode == LookMode::Free || player.Control.Look.Mode == LookMode::Vertical))
	{
		axisCoeff.x = AxisMap[(int)InputAxis::Move].y;
	}

	// Determine Y axis coefficient.
	if ((IsHeld(In::Left) || IsHeld(In::Right)) &&
		(player.Control.Look.Mode == LookMode::Free || player.Control.Look.Mode == LookMode::Horizontal))
	{
		axisCoeff.y = AxisMap[(int)InputAxis::Move].x;
	}

	// Determine turn rate base values.
	short turnRateMax = isSlow ? (TURN_RATE_MAX / 2) : TURN_RATE_MAX;
	short turnRateAccel = isSlow ? (TURN_RATE_ACCEL / 2) : TURN_RATE_ACCEL;

	// Normalize turn rate base values.
	turnRateMax = NormalizeLookAroundTurnRate(turnRateMax, player.Control.Look.OpticRange);
	turnRateAccel = NormalizeLookAroundTurnRate(turnRateAccel, player.Control.Look.OpticRange);

	// Modulate turn rates.
	player.Control.Look.TurnRate = EulerAngles(
		ModulateLaraTurnRate(player.Control.Look.TurnRate.x, turnRateAccel, 0, turnRateMax, axisCoeff.x, invertXAxis),
		ModulateLaraTurnRate(player.Control.Look.TurnRate.y, turnRateAccel, 0, turnRateMax, axisCoeff.y, false),
		0);

	// Apply turn rates.
	player.Control.Look.Orientation += player.Control.Look.TurnRate;
	player.Control.Look.Orientation = EulerAngles(
		std::clamp(player.Control.Look.Orientation.x, LOOKCAM_ORIENT_CONSTRAINT.first.x, LOOKCAM_ORIENT_CONSTRAINT.second.x),
		std::clamp(player.Control.Look.Orientation.y, LOOKCAM_ORIENT_CONSTRAINT.first.y, LOOKCAM_ORIENT_CONSTRAINT.second.y),
		0);

	// Visually adapt head and torso orientations.
	player.ExtraHeadRot = player.Control.Look.Orientation / 2;
	if (player.Control.HandStatus != HandStatus::Busy &&
		!player.LeftArm.Locked && !player.RightArm.Locked &&
		player.Context.Vehicle == NO_ITEM)
	{
		player.ExtraTorsoRot = player.ExtraHeadRot;
	}

	ClearPlayerLookAroundActions(item);
}

bool HandleLaraVehicle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Context.Vehicle == NO_ITEM)
		return false;

	if (!g_Level.Items[lara->Context.Vehicle].Active)
	{
		lara->Context.Vehicle = NO_ITEM;
		item->Animation.IsAirborne = true;
		SetAnimation(item, LA_FALL_START);
		return false;
	}

	TestVolumes(lara->Context.Vehicle);

	switch (g_Level.Items[lara->Context.Vehicle].ObjectNumber)
	{
	case ID_QUAD:
		QuadBikeControl(item, coll);
		break;

	case ID_JEEP:
		JeepControl(item);
		break;

	case ID_MOTORBIKE:
		MotorbikeControl(item, coll);
		break;

	case ID_KAYAK:
		KayakControl(item);
		break;

	case ID_SNOWMOBILE:
		SkidooControl(item, coll);
		break;

	case ID_UPV:
		UPVControl(item, coll);
		break;

	case ID_MINECART:
		MinecartControl(item);
		break;

	case ID_BIGGUN:
		BigGunControl(item, coll);
		break;

	// Boats are processed like normal items in loop.
	default:
		HandleWeapon(*item);
	}

	return true;
}

void HandlePlayerWetnessDrips(ItemInfo& item)
{
	auto& player = *GetLaraInfo(&item);

	if (Wibble & 0xF)
		return;

	int jointIndex = 0;
	for (auto& node : player.Effect.DripNodes)
	{
		auto pos = GetJointPosition(&item, jointIndex);
		int roomNumber = GetRoom(item.Location, pos).roomNumber;
		jointIndex++;

		// Node underwater; set max wetness value.
		if (TestEnvironment(ENV_FLAG_WATER, roomNumber))
		{
			node = PLAYER_DRIP_NODE_MAX;
			continue;
		}

		// Node inactive; continue.
		if (node <= 0.0f)
			continue;

		// Spawn drip.
		float chance = (node / PLAYER_DRIP_NODE_MAX) / 2;
		if (Random::TestProbability(chance))
		{
			SpawnWetnessDrip(pos.ToVector3(), item.RoomNumber);

			node -= 1.0f;
			if (node <= 0.0f)
				node = 0.0f;
		}
	}
}

void HandlePlayerDiveBubbles(ItemInfo& item)
{
	constexpr auto BUBBLE_COUNT_MULT = 6;

	auto& player = *GetLaraInfo(&item);

	int jointIndex = 0;
	for (auto& node : player.Effect.BubbleNodes)
	{
		auto pos = GetJointPosition(&item, jointIndex);
		int roomNumber = GetRoom(item.Location, pos).roomNumber;
		jointIndex++;

		// Node inactive; continue.
		if (node <= 0.0f)
			continue;

		// Node above water; continue.
		if (!TestEnvironment(ENV_FLAG_WATER, roomNumber))
			continue;

		// Spawn bubbles.
		float chance = node / PLAYER_BUBBLE_NODE_MAX;
		if (Random::TestProbability(chance))
		{
			unsigned int count = (int)round(node * BUBBLE_COUNT_MULT);
			SpawnDiveBubbles(pos.ToVector3(), roomNumber, count);

			node -= 1.0f;
			if (node <= 0.0f)
				node = 0.0f;
		}
	}
}

void HandlePlayerAirBubbles(ItemInfo* item)
{
	constexpr auto BUBBLE_COUNT_MAX = 3;

	SoundEffect(SFX_TR4_LARA_BUBBLES, &item->Pose, SoundEnvironment::Water);

	const auto& level = *g_GameFlow->GetLevel(CurrentLevel);

	auto pos = (level.GetLaraType() == LaraType::Divesuit) ?
		GetJointPosition(item, LM_TORSO, Vector3i(0, -192, -160)).ToVector3() :
		GetJointPosition(item, LM_HEAD, Vector3i(0, -4, -64)).ToVector3();

	unsigned int bubbleCount = Random::GenerateInt(0, BUBBLE_COUNT_MAX);
	for (int i = 0; i < bubbleCount; i++)
		SpawnBubble(pos, item->RoomNumber);
}

// TODO: This approach may cause undesirable artefacts where a platform rapidly ascends/descends or the player gets pushed.
// Potential solutions:
// 1. Consider floor tilt when translating objects.
// 2. Object parenting. -- Sezz 2022.10.28
void EasePlayerVerticalPosition(ItemInfo* item, int height)
{
	constexpr auto LINEAR_RATE_MIN = 50.0f;

	// Check for wall.
	if (height == NO_HEIGHT)
		return;

	// Handle swamp case.
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && height > 0)
	{
		item->Pose.Position.y += SWAMP_GRAVITY;
		return;
	}

	// Handle regular case.
	float linearRate = std::max(LINEAR_RATE_MIN, abs(item->Animation.Velocity.z));
	if (abs(height) > linearRate)
	{
		int sign = std::copysign(1, height);
		item->Pose.Position.y += linearRate * sign;
	}
	else
	{
		item->Pose.Position.y += height;
	}
}

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ItemInfo* item, CollisionInfo* coll)
{
	if (!TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		if (TestLaraStepUp(item, coll))
		{
			item->Animation.TargetState = LS_STEP_UP;
			if (GetStateDispatch(item, GetAnimData(*item)))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
		else if (TestLaraStepDown(item, coll))
		{
			item->Animation.TargetState = LS_STEP_DOWN;
			if (GetStateDispatch(item, GetAnimData(*item)))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
	}

	EasePlayerVerticalPosition(item, coll->Middle.Floor);
}

void DoLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll)
{
	EasePlayerVerticalPosition(item, coll->Middle.Ceiling);
}

void DoLaraCrawlToHangSnap(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.ForwardAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	GetCollisionInfo(coll, item);

	SnapItemToLedge(item, coll);
	LaraResetGravityStatus(item, coll);

	// Bridges behave differently.
	if (coll->Middle.Bridge < 0)
	{
		TranslateItem(item, item->Pose.Orientation.y, -LARA_RADIUS_CRAWL);
		item->Pose.Orientation.y += ANGLE(180.0f);
	}
}

void DoLaraTightropeBalance(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);
	const int factor = ((lara->Control.Tightrope.TimeOnTightrope >> 7) & 0xFF) * 128;

	if (IsHeld(In::Left))
		lara->Control.Tightrope.Balance += ANGLE(1.4f);
	if (IsHeld(In::Right))
		lara->Control.Tightrope.Balance -= ANGLE(1.4f);

	if (lara->Control.Tightrope.Balance < 0)
	{
		lara->Control.Tightrope.Balance -= factor;
		if (lara->Control.Tightrope.Balance <= -ANGLE(45.0f))
			lara->Control.Tightrope.Balance = -ANGLE(45.0f);

	}
	else if (lara->Control.Tightrope.Balance > 0)
	{
		lara->Control.Tightrope.Balance += factor;
		if (lara->Control.Tightrope.Balance >= ANGLE(45.0f))
			lara->Control.Tightrope.Balance = ANGLE(45.0f);
	}
	else
		lara->Control.Tightrope.Balance = GetRandomControl() & 1 ? -1 : 1;
}

void DoLaraTightropeLean(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Pose.Orientation.z = lara->Control.Tightrope.Balance / 4;
	lara->ExtraTorsoRot.z = -lara->Control.Tightrope.Balance;
}

void DoLaraTightropeBalanceRegen(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Tightrope.TimeOnTightrope <= 32)
		lara->Control.Tightrope.TimeOnTightrope = 0;
	else
		lara->Control.Tightrope.TimeOnTightrope -= 32;

	if (lara->Control.Tightrope.Balance > 0)
	{
		if (lara->Control.Tightrope.Balance <= ANGLE(0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance -= ANGLE(0.75f);
	}

	if (lara->Control.Tightrope.Balance < 0)
	{
		if (lara->Control.Tightrope.Balance >= -ANGLE(0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance += ANGLE(0.75f);
	}
}

void DoLaraFallDamage(ItemInfo* item)
{
	constexpr auto RUMBLE_POWER_COEFF = 0.7f;
	constexpr auto RUMBLE_DELAY		  = 0.3f;

	if (item->Animation.Velocity.y >= LARA_DAMAGE_VELOCITY)
	{
		if (item->Animation.Velocity.y >= LARA_DEATH_VELOCITY)
		{
			item->HitPoints = 0;
		}
		else
		{
			float base = item->Animation.Velocity.y - (LARA_DAMAGE_VELOCITY - 1.0f);
			item->HitPoints -= LARA_HEALTH_MAX * (SQUARE(base) / 196.0f);
		}

		float rumblePower = (item->Animation.Velocity.y / LARA_DEATH_VELOCITY) * RUMBLE_POWER_COEFF;
		Rumble(rumblePower, RUMBLE_DELAY);
	}
}

LaraInfo& GetLaraInfo(ItemInfo& item)
{
	if (item.ObjectNumber == ID_LARA)
	{
		auto* player = (LaraInfo*&)item.Data;
		return *player;
	}

	TENLog(std::string("Attempted to fetch LaraInfo data from entity with object ID ") + std::to_string(item.ObjectNumber), LogLevel::Warning);

	auto& firstLaraItem = *FindItem(ID_LARA);
	auto* player = (LaraInfo*&)firstLaraItem.Data;
	return *player;
}

const LaraInfo& GetLaraInfo(const ItemInfo& item)
{
	if (item.ObjectNumber == ID_LARA)
	{
		const auto* player = (LaraInfo*&)item.Data;
		return *player;
	}

	TENLog(std::string("Attempted to fetch LaraInfo data from entity with object ID ") + std::to_string(item.ObjectNumber), LogLevel::Warning);

	const auto& firstPlayerItem = *FindItem(ID_LARA);
	const auto* player = (LaraInfo*&)firstPlayerItem.Data;
	return *player;
}

LaraInfo*& GetLaraInfo(ItemInfo* item)
{
	if (item->ObjectNumber == ID_LARA)
		return (LaraInfo*&)item->Data;

	TENLog(std::string("Attempted to fetch LaraInfo data from entity with object ID ") + std::to_string(item->ObjectNumber), LogLevel::Warning);

	auto& firstPlayerItem = *FindItem(ID_LARA);
	return (LaraInfo*&)firstPlayerItem.Data;
}

PlayerWaterData GetPlayerWaterData(ItemInfo& item)
{
	bool isWater = TestEnvironment(ENV_FLAG_WATER, &item);
	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, &item);
	bool isCold = TestEnvironment(ENV_FLAG_COLD, &item);

	int waterDepth = GetWaterDepth(&item);
	int waterHeight = GetWaterHeight(&item);

	auto pointColl = GetCollision(item);
	int heightFromWater = (waterHeight == NO_HEIGHT) ? NO_HEIGHT : (std::min(item.Pose.Position.y, pointColl.Position.Floor) - waterHeight);

	return PlayerWaterData
	{
		isWater, isSwamp, isCold,
		waterDepth, waterHeight, heightFromWater
	};
}

short GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll)
{
	short headingAngle = coll->Setup.ForwardAngle;
	auto probe = GetCollision(item);

	// Ground is flat.
	if (probe.FloorTilt == Vector2::Zero)
		return headingAngle;

	// Get either:
	// a) the surface aspect angle (extended slides), or
	// b) the derived nearest cardinal direction from it (original slides).
	headingAngle = Geometry::GetSurfaceAspectAngle(GetSurfaceNormal(probe.FloorTilt, true));
	if (g_GameFlow->HasSlideExtended())
		return headingAngle;
	else
		return (GetQuadrant(headingAngle) * ANGLE(90.0f));
}

short ModulateLaraTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert)
{
	axisCoeff *= invert ? -1 : 1;
	int sign = std::copysign(1, axisCoeff);

	short minTurnRateNorm = minTurnRate * abs(axisCoeff);
	short maxTurnRateNorm = maxTurnRate * abs(axisCoeff);

	short newTurnRate = (turnRate + (accelRate * sign)) * sign;
	newTurnRate = std::clamp(newTurnRate, minTurnRateNorm, maxTurnRateNorm);
	return (newTurnRate * sign);
}

// TODO: Make these two functions methods of LaraInfo someday. -- Sezz 2022.06.26
void ModulateLaraTurnRateX(ItemInfo* item, short accelRate, short minTurnRate, short maxTurnRate, bool invert)
{
	auto* lara = GetLaraInfo(item);

	//lara->Control.TurnRate.x = ModulateLaraTurnRate(lara->Control.TurnRate.x, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::Move].y, invert);
}

void ModulateLaraTurnRateY(ItemInfo* item, short accelRate, short minTurnRate, short maxTurnRate, bool invert)
{
	auto* lara = GetLaraInfo(item);

	float axisCoeff = AxisMap[(int)InputAxis::Move].x;
	if (item->Animation.IsAirborne)
	{
		int sign = std::copysign(1, axisCoeff);
		axisCoeff = std::min(1.2f, abs(axisCoeff)) * sign;
	}

	lara->Control.TurnRate/*.y*/ = ModulateLaraTurnRate(lara->Control.TurnRate/*.y*/, accelRate, minTurnRate, maxTurnRate, axisCoeff, invert);
}

void ModulateLaraSwimTurnRates(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	/*if ((IsHeld(In::Forward) || IsHeld(In::Back)))
	ModulateLaraTurnRateX(item, 0, 0, 0);*/

	if (IsHeld(In::Forward))
		item->Pose.Orientation.x -= ANGLE(3.0f);
	else if (IsHeld(In::Back))
		item->Pose.Orientation.x += ANGLE(3.0f);

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

		// TODO: ModulateLaraLean() doesn't really work here. -- Sezz 2022.06.22
		if (IsHeld(In::Left))
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (IsHeld(In::Right))
			item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

void ModulateLaraSubsuitSwimTurnRates(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (IsHeld(In::Forward) && item->Pose.Orientation.x > -ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = -ANGLE(45.0f);
	else if (IsHeld(In::Back) && item->Pose.Orientation.x < ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = ANGLE(45.0f);
	else
		lara->Control.Subsuit.DXRot = 0;

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_SUBSUIT_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

		if (IsHeld(In::Left))
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (IsHeld(In::Right))
			item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

// TODO: Simplify this function. Some members of SubsuitControlData may be unnecessary. -- Sezz 2022.06.22
void UpdateLaraSubsuitAngles(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Subsuit.VerticalVelocity != 0)
	{
		item->Pose.Position.y += lara->Control.Subsuit.VerticalVelocity / 4;
		lara->Control.Subsuit.VerticalVelocity = ceil((15 / 16) * lara->Control.Subsuit.VerticalVelocity - 1);
	}

	lara->Control.Subsuit.Velocity[0] = -4 * item->Animation.Velocity.y;
	lara->Control.Subsuit.Velocity[1] = -4 * item->Animation.Velocity.y;

	if (lara->Control.Subsuit.XRot >= lara->Control.Subsuit.DXRot)
	{
		if (lara->Control.Subsuit.XRot > lara->Control.Subsuit.DXRot)
		{
			if (lara->Control.Subsuit.XRot > 0 && lara->Control.Subsuit.DXRot < 0)
				lara->Control.Subsuit.XRot = ceil(0.75 * lara->Control.Subsuit.XRot);

			lara->Control.Subsuit.XRot -= ANGLE(2.0f);
			if (lara->Control.Subsuit.XRot < lara->Control.Subsuit.DXRot)
				lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
		}
	}
	else
	{
		if (lara->Control.Subsuit.XRot < 0 && lara->Control.Subsuit.DXRot > 0)
			lara->Control.Subsuit.XRot = ceil(0.75 * lara->Control.Subsuit.XRot);

		lara->Control.Subsuit.XRot += ANGLE(2.0f);
		if (lara->Control.Subsuit.XRot > lara->Control.Subsuit.DXRot)
			lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
	}

	if (lara->Control.Subsuit.DXRot != 0)
	{
		short rotation = lara->Control.Subsuit.DXRot >> 3;
		if (rotation < -ANGLE(2.0f))
			rotation = -ANGLE(2.0f);
		else if (rotation > ANGLE(2.0f))
			rotation = ANGLE(2.0f);

		item->Pose.Orientation.x += rotation;
	}

	lara->Control.Subsuit.Velocity[0] += abs(lara->Control.Subsuit.XRot >> 3);
	lara->Control.Subsuit.Velocity[1] += abs(lara->Control.Subsuit.XRot >> 3);

	if (lara->Control.TurnRate > 0)
		lara->Control.Subsuit.Velocity[0] += 2 * abs(lara->Control.TurnRate);
	else if (lara->Control.TurnRate < 0)
		lara->Control.Subsuit.Velocity[1] += 2 * abs(lara->Control.TurnRate);

	if (lara->Control.Subsuit.Velocity[0] > BLOCK(1.5f))
		lara->Control.Subsuit.Velocity[0] = BLOCK(1.5f);

	if (lara->Control.Subsuit.Velocity[1] > BLOCK(1.5f))
		lara->Control.Subsuit.Velocity[1] = BLOCK(1.5f);

	if (lara->Control.Subsuit.Velocity[0] != 0 || lara->Control.Subsuit.Velocity[1] != 0)
	{
		auto mul1 = (float)abs(lara->Control.Subsuit.Velocity[0]) / BLOCK(8);
		auto mul2 = (float)abs(lara->Control.Subsuit.Velocity[1]) / BLOCK(8);
		auto vol = ((mul1 + mul2) * 5.0f) + 0.5f;
		SoundEffect(SFX_TR5_VEHICLE_DIVESUIT_ENGINE, &item->Pose, SoundEnvironment::Water, 1.0f + (mul1 + mul2), vol);
	}
}

void ModulateLaraLean(ItemInfo* item, CollisionInfo* coll, short baseRate, short maxAngle)
{
	if (!item->Animation.Velocity.z)
		return;

	float axisCoeff = AxisMap[(int)InputAxis::Move].x;
	int sign = copysign(1, axisCoeff);
	short maxAngleNormalized = maxAngle * axisCoeff;

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		maxAngleNormalized *= 0.6f;

	item->Pose.Orientation.z += std::min<short>(baseRate, abs(maxAngleNormalized - item->Pose.Orientation.z) / 3) * sign;
}

void ModulateLaraCrawlFlex(ItemInfo* item, short baseRate, short maxAngle)
{
	auto* lara = GetLaraInfo(item);

	if (!item->Animation.Velocity.z)
		return;

	float axisCoeff = AxisMap[(int)InputAxis::Move].x;
	int sign = copysign(1, axisCoeff);
	short maxAngleNormalized = maxAngle * axisCoeff;

	if (abs(lara->ExtraTorsoRot.z) < LARA_CRAWL_FLEX_MAX)
		lara->ExtraTorsoRot.z += std::min<short>(baseRate, abs(maxAngleNormalized - lara->ExtraTorsoRot.z) / 6) * sign;

	if (!IsHeld(In::Look) &&
		item->Animation.ActiveState != LS_CRAWL_BACK)
	{
		lara->ExtraHeadRot.z = lara->ExtraTorsoRot.z / 2;
		lara->ExtraHeadRot.y = lara->ExtraHeadRot.z;
	}
}

// TODO: Unused; I will pick this back up later. -- Sezz 2022.06.22
void ModulateLaraSlideVelocity(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	constexpr int minVelocity = 50;
	constexpr int maxVelocity = LARA_TERMINAL_VELOCITY;

	if (g_GameFlow->HasSlideExtended())
	{
		auto probe = GetCollision(item);
		short minSlideAngle = ANGLE(33.75f);
		//short steepness = Geometry::GetSurfaceSlopeAngle(probe.FloorTilt);
		//short direction = Geometry::GetSurfaceAspectAngle(probe.FloorTilt);

		float velocityMultiplier = 1 / (float)ANGLE(33.75f);
		//int slideVelocity = std::min<int>(minVelocity + 10 * (steepness * velocityMultiplier), LARA_TERMINAL_VELOCITY);
		//short deltaAngle = abs((short)(direction - item->Pose.Orientation.y));

		//g_Renderer.PrintDebugMessage("%d", slideVelocity);

		//lara->ExtraVelocity.x += slideVelocity;
		//lara->ExtraVelocity.y += slideVelocity * phd_sin(steepness);
	}
	//else
	//lara->ExtraVelocity.x += minVelocity;
}

void AlignLaraToSurface(ItemInfo* item, float alpha)
{
	// Determine relative orientation adhering to floor normal.
	auto floorNormal = GetSurfaceNormal(GetCollision(item).FloorTilt, true);
	auto orient = Geometry::GetRelOrientToNormal(item->Pose.Orientation.y, floorNormal);

	// Apply extra rotation according to alpha.
	auto extraRot = orient - item->Pose.Orientation;
	item->Pose.Orientation += extraRot * alpha;
}

void SetLaraJumpDirection(ItemInfo* item, CollisionInfo* coll) 
{
	auto* lara = GetLaraInfo(item);

	if (IsHeld(In::Forward) &&
		TestLaraJumpForward(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Forward;
	}
	else if (IsHeld(In::Back) &&
		TestLaraJumpBack(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Back;
	}
	else if (IsHeld(In::Left) &&
		TestLaraJumpLeft(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Left;
	}
	else if (IsHeld(In::Right) &&
		TestLaraJumpRight(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Right;
	}
	else if (TestLaraJumpUp(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		lara->Control.JumpDirection = JumpDirection::Up;
	else
		lara->Control.JumpDirection = JumpDirection::None;
}

// TODO: Add a timeout? Imagine a small, sad rain cloud with the properties of a ceiling following Lara overhead.
// RunJumpQueued will never reset, and when the sad cloud flies away after an indefinite amount of time, Lara will jump. -- Sezz 2022.01.22
void SetLaraRunJumpQueue(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = BLOCK(1);
	auto probe = GetCollision(item, item->Pose.Orientation.y, distance, -coll->Setup.Height);

	if ((TestLaraRunJumpForward(item, coll) ||													// Area close ahead is permissive...
		(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||		// OR ceiling height far ahead is permissive
		(probe.Position.Floor - y) >= CLICK(0.5f)) &&											// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		lara->Control.RunJumpQueued = IsRunJumpQueueableState((LaraState)item->Animation.TargetState);
	}
	else
		lara->Control.RunJumpQueued = false;
}

void SetLaraVault(ItemInfo* item, CollisionInfo* coll, VaultTestResult vaultResult)
{
	auto* lara = GetLaraInfo(item);

	lara->Context.ProjectedFloorHeight = vaultResult.Height;
	lara->Control.HandStatus = vaultResult.SetBusyHands ? HandStatus::Busy : lara->Control.HandStatus;
	lara->Control.TurnRate = 0;

	if (vaultResult.SnapToLedge)
	{
		SnapItemToLedge(item, coll, 0.2f, false);
		lara->Context.TargetOrientation = EulerAngles(0, coll->NearestLedgeAngle, 0);
	}
	else
	{
		lara->Context.TargetOrientation = EulerAngles(0, item->Pose.Orientation.y, 0);
	}

	if (vaultResult.SetJumpVelocity)
	{
		int height = lara->Context.ProjectedFloorHeight - item->Pose.Position.y;
		if (height > -CLICK(3.5f))
			height = -CLICK(3.5f);
		else if (height < -CLICK(7.5f))
			height = -CLICK(7.5f);

		// TODO: Find a better formula for this that won't require the above block.
		lara->Context.CalcJumpVelocity = -3 - sqrt(-9600 - 12 * (height));
	}
}

void SetLaraLand(ItemInfo* item, CollisionInfo* coll)
{
	// Avoid clearing forward velocity when hitting the ground running.
	if (item->Animation.TargetState != LS_RUN_FORWARD)
		item->Animation.Velocity.z = 0.0f;

	//item->IsAirborne = false; // TODO: Removing this avoids an unusual landing bug. I hope to find a proper solution later. -- Sezz 2022.02.18
	item->Animation.Velocity.y = 0.0f;
	LaraSnapToHeight(item, coll);
}

void SetLaraFallAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_START);
	item->Animation.IsAirborne = true;
	item->Animation.Velocity.y = 0.0f;
}

void SetLaraFallBackAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->Animation.IsAirborne = true;
	item->Animation.Velocity.y = 0.0f;
}

void SetLaraMonkeyFallAnimation(ItemInfo* item)
{
	// HACK: Disallow release during 180 turn action.
	if (item->Animation.ActiveState == LS_MONKEY_TURN_180)
		return;

	SetAnimation(item, LA_MONKEY_TO_FREEFALL);
	SetLaraMonkeyRelease(item);
}

void SetLaraMonkeyRelease(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = true;
	item->Animation.Velocity.y = 1.0f;
	item->Animation.Velocity.z = 2.0f;
	lara->Control.TurnRate = 0;
	lara->Control.HandStatus = HandStatus::Free;
}

// temp
void SetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
		return;

	static short oldAngle = 1;

	if (abs(coll->FloorTilt.x) <= 2 && abs(coll->FloorTilt.y) <= 2)
		return;

	short angle = ANGLE(0.0f);
	if (coll->FloorTilt.x > 2)
		angle = -ANGLE(90.0f);
	else if (coll->FloorTilt.x < -2)
		angle = ANGLE(90.0f);

	if (coll->FloorTilt.y > 2 && coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = ANGLE(180.0f);
	else if (coll->FloorTilt.y < -2 && -coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = ANGLE(0.0f);

	short delta = angle - item->Pose.Orientation.y;

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->Pose.Orientation.y = angle + ANGLE(180.0f);
	}
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
		item->Pose.Orientation.y = angle;
	}

	LaraSnapToHeight(item, coll);
	lara->Control.MoveAngle = angle;
	lara->Control.TurnRate = 0;
	oldAngle = angle;
}

// TODO: Do it later.
void newSetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	short headinAngle = GetLaraSlideDirection(item, coll);
	short deltaAngle = headinAngle - item->Pose.Orientation.y;

	if (!g_GameFlow->HasSlideExtended())
		item->Pose.Orientation.y = headinAngle;

	// Snap to height upon slide entrance.
	if (item->Animation.ActiveState != LS_SLIDE_FORWARD &&
		item->Animation.ActiveState != LS_SLIDE_BACK)
	{
		LaraSnapToHeight(item, coll);
	}

	// Slide forward.
	if (abs(deltaAngle) <= ANGLE(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && abs(deltaAngle) <= ANGLE(180.0f))
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
	}
	// Slide backward.
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && abs((short)(deltaAngle - ANGLE(180.0f))) <= -ANGLE(180.0f))
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
	}
}

void SetLaraCornerAnimation(ItemInfo* item, CollisionInfo* coll, bool flip)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);
		item->Animation.IsAirborne = true;
		item->Animation.Velocity.z = 2;
		item->Animation.Velocity.y = 1;
		item->Pose.Position.y += CLICK(1);
		item->Pose.Orientation.y += lara->Context.NextCornerPos.Orientation.y / 2;
		lara->Control.HandStatus = HandStatus::Free;
		return;
	}

	if (flip)
	{
		if (lara->Control.IsClimbingLadder)
			SetAnimation(item, LA_LADDER_IDLE);
		else
			SetAnimation(item, LA_HANG_IDLE);

		item->Pose.Position = lara->Context.NextCornerPos.Position;
		item->Pose.Orientation.y = lara->Context.NextCornerPos.Orientation.y;
		coll->Setup.PrevPosition = lara->Context.NextCornerPos.Position;
	}
}

void SetLaraSwimDiveAnimation(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_ONWATER_DIVE);
	item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;
	item->Animation.Velocity.y = LARA_SWIM_VELOCITY_MAX * 0.4f;
	item->Pose.Orientation.x = -ANGLE(45.0f);
	lara->Control.WaterStatus = WaterStatus::Underwater;
}

void SetLaraVehicle(ItemInfo* item, ItemInfo* vehicle)
{
	auto* lara = GetLaraInfo(item);

	if (vehicle == nullptr)
	{
		if (lara->Context.Vehicle != NO_ITEM)
			g_Level.Items[lara->Context.Vehicle].Active = false;

		lara->Context.Vehicle = NO_ITEM;
	}
	else
	{
		g_Level.Items[vehicle->Index].Active = true;
		lara->Context.Vehicle = vehicle->Index;
	}
}

void ResetPlayerLean(ItemInfo* item, float alpha, bool resetRoll, bool resetPitch)
{
	if (resetRoll)
		item->Pose.Orientation.Lerp(EulerAngles(item->Pose.Orientation.x, item->Pose.Orientation.y, 0), alpha);

	if (resetPitch)
		item->Pose.Orientation.Lerp(EulerAngles(0, item->Pose.Orientation.y, item->Pose.Orientation.z), alpha);
}

void ResetPlayerFlex(ItemInfo* item, float alpha)
{
	auto& player = GetLaraInfo(*item);

	player.ExtraHeadRot.Lerp(EulerAngles::Zero, alpha);
	player.ExtraTorsoRot.Lerp(EulerAngles::Zero, alpha);
}

void ResetPlayerLookAround(ItemInfo& item, float alpha)
{
	auto& player = GetLaraInfo(item);

	player.Control.Look.Orientation = EulerAngles::Zero;

	if (Camera.type != CameraType::Look)
	{
		player.ExtraHeadRot.Lerp(EulerAngles::Zero, alpha);

		if (player.Control.HandStatus != HandStatus::Busy &&
			!player.LeftArm.Locked && !player.RightArm.Locked &&
			player.Context.Vehicle == NO_ITEM)
		{
			player.ExtraTorsoRot = player.ExtraHeadRot;
		}
		else
		{
			if (!player.ExtraHeadRot.x)
				player.ExtraTorsoRot.x = 0;

			if (!player.ExtraHeadRot.y)
				player.ExtraTorsoRot.y = 0;

			if (!player.ExtraHeadRot.z)
				player.ExtraTorsoRot.z = 0;
		}
	}
}

void RumbleLaraHealthCondition(ItemInfo* item)
{
	constexpr auto POWER = 0.2f;
	constexpr auto DELAY = 0.1f;

	const auto& player = GetLaraInfo(*item);

	if (item->HitPoints > LARA_HEALTH_CRITICAL && player.Status.Poison == 0)
		return;

	if (item->HitPoints == 0)
		return;

	bool doPulse = ((GlobalCounter & 0x0F) % 0x0F == 1);
	if (doPulse)
		Rumble(POWER, DELAY);
}
