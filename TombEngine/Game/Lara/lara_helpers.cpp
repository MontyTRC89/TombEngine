#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Game/effects/Bubble.h"
#include "Game/effects/Drip.h"
#include "Game/Lara/Context.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_tests.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR3/Vehicles/big_gun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad_bike.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Control::Volumes;
using namespace TEN::Effects::Bubble;
using namespace TEN::Effects::Drip;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Player;
using namespace TEN::Renderer;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Update AFK pose timer.
	if (lara->Control.Count.Pose < LARA_POSE_TIME && Context::CanStrikeAfkPose(*item, *coll) &&
		!(TrInput & IN_LOOK || IsOpticActionHeld()) &&
		g_GameFlow->HasAFKPose())
	{
		lara->Control.Count.Pose++;
	}
	else
	{
		lara->Control.Count.Pose = 0;
	}

	// Reset running jump timer.
	if (!IsRunJumpCountableState((LaraState)item->Animation.ActiveState))
		lara->Control.Count.Run = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState((LaraState)item->Animation.ActiveState))
		lara->Control.IsRunJumpQueued = false;

	// Reset lean.
	if ((!lara->Control.IsMoving || (lara->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT)))) &&
		(!lara->Control.IsInLowPosition && item->Animation.ActiveState != LS_DEATH)) // HACK: Don't interfere with surface alignment in crouch, crawl, and death states.
	{
		ResetPlayerLean(item, 1 / 6.0f);
	}

	// Reset crawl flex.
	if (!(TrInput & IN_LOOK) && coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM &&	// HACK
		(!item->Animation.Velocity.z || (item->Animation.Velocity.z && !(TrInput & (IN_LEFT | IN_RIGHT)))))
	{
		ResetPlayerFlex(item, 0.1f);
	}

	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate;
	if (!(TrInput & (IN_LEFT | IN_RIGHT)))
		lara->Control.TurnRate = 0;

	lara->Control.IsInLowPosition = false;
	lara->Control.IsMonkeySwinging = false;
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

void HandlePlayerLean(ItemInfo* item, CollisionInfo* coll, short baseRate, short maxAngle)
{
	if (!item->Animation.Velocity.z)
		return;

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	int sign = copysign(1, axisCoeff);
	short maxAngleNormalized = maxAngle * axisCoeff;

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		maxAngleNormalized *= 0.6f;

	item->Pose.Orientation.z += std::min<short>(baseRate, abs(maxAngleNormalized - item->Pose.Orientation.z) / 3) * sign;
}

void HandlePlayerCrawlFlex(ItemInfo& item)
{
	constexpr auto FLEX_RATE_ANGLE = ANGLE(2.25f);
	constexpr auto FLEX_ANGLE_MAX  = ANGLE(50.0f) / 2; // 2 = hardcoded number of bones to flex (head and torso).

	auto& player = GetLaraInfo(item);

	// Check if player is moving.
	if (item.Animation.Velocity.z == 0.0f)
		return;

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	int sign = copysign(1, axisCoeff);
	short maxAngleNormalized = FLEX_ANGLE_MAX * axisCoeff;

	if (abs(player.ExtraTorsoRot.z) < FLEX_ANGLE_MAX)
		player.ExtraTorsoRot.z += std::min<short>(FLEX_RATE_ANGLE, abs(maxAngleNormalized - player.ExtraTorsoRot.z) / 6) * sign;

	if (!IsHeld(In::Look) && item.Animation.ActiveState != LS_CRAWL_BACK)
	{
		player.ExtraHeadRot.z = player.ExtraTorsoRot.z / 2;
		player.ExtraHeadRot.y = player.ExtraHeadRot.z;
	}
}

void HandlePlayerWetnessDrips(ItemInfo& item)
{
	auto& player = *GetLaraInfo(&item);

	if (Wibble & 0xF)
		return;

	int jointIndex = 0;
	for (auto& node : player.Effect.DripNodes)
	{
		auto pos = GetJointPosition(&item, jointIndex).ToVector3();
		int roomNumber = GetRoom(item.Location, pos.x, pos.y, pos.z).roomNumber;
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
			SpawnWetnessDrip(pos, item.RoomNumber);

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
		auto pos = GetJointPosition(&item, jointIndex).ToVector3();
		int roomNumber = GetRoom(item.Location, pos.x, pos.y, pos.z).roomNumber;
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
			SpawnDiveBubbles(pos, roomNumber, count);

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
void EaseOutLaraHeight(ItemInfo* item, int height)
{
	constexpr auto LINEAR_THRESHOLD		= STEPUP_HEIGHT / 2;
	constexpr auto EASING_THRESHOLD_MIN = BLOCK(1.0f / 64);
	constexpr auto LINEAR_RATE			= 50;
	constexpr auto EASING_ALPHA			= 0.35f;

	// Check for wall.
	if (height == NO_HEIGHT)
		return;

	// Handle swamp case.
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && height > 0)
	{
		item->Pose.Position.y += SWAMP_GRAVITY;
		return;
	}

	int easingThreshold = std::max(abs(item->Animation.Velocity.z), EASING_THRESHOLD_MIN);

	// Handle regular case.
	if (abs(height) > LINEAR_THRESHOLD)
	{
		int sign = std::copysign(1, height);
		item->Pose.Position.y += LINEAR_RATE * sign;
	}
	else if (abs(height) > easingThreshold)
	{
		int vPos = item->Pose.Position.y;
		item->Pose.Position.y = (int)round(Lerp(vPos, vPos + height, EASING_ALPHA));
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
		if (Context::CanStepUp(*item, *coll))
		{
			item->Animation.TargetState = LS_STEP_UP;
			if (GetStateDispatch(item, GetAnimData(*item)))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
		else if (Context::CanStepDown(*item, *coll))
		{
			item->Animation.TargetState = LS_STEP_DOWN;
			if (GetStateDispatch(item, GetAnimData(*item)))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
	}

	EaseOutLaraHeight(item, coll->Middle.Floor);
}

void DoLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll)
{
	EaseOutLaraHeight(item, coll->Middle.Ceiling);
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

	if (TrInput & IN_LEFT)
		lara->Control.Tightrope.Balance += ANGLE(1.4f);
	if (TrInput & IN_RIGHT)
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

JumpDirection GetPlayerJumpDirection(const ItemInfo& item, const CollisionInfo& coll)
{
	if (IsHeld(In::Forward) && Context::CanJumpForward(item, coll))
	{
		return JumpDirection::Forward;
	}
	else if (IsHeld(In::Back) && Context::CanJumpBackward(item, coll))
	{
		return JumpDirection::Back;
	}
	else if (IsHeld(In::Left) && Context::CanJumpLeft(item, coll))
	{
		return JumpDirection::Left;
	}
	else if (IsHeld(In::Right) && Context::CanJumpRight(item, coll))
	{
		return JumpDirection::Right;
	}
	else if (Context::CanJumpUp(item, coll))
	{
		return JumpDirection::Up;
	}

	return JumpDirection::None;
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

	//lara->Control.TurnRate.x = ModulateLaraTurnRate(lara->Control.TurnRate.x, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveVertical], invert);
}

void ModulateLaraTurnRateY(ItemInfo* item, short accelRate, short minTurnRate, short maxTurnRate, bool invert)
{
	auto* lara = GetLaraInfo(item);

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	if (item->Animation.IsAirborne)
	{
		int sign = std::copysign(1, axisCoeff);
		axisCoeff = std::min(1.2f, abs(axisCoeff)) * sign;
	}

	lara->Control.TurnRate/*.y*/ = ModulateLaraTurnRate(lara->Control.TurnRate/*.y*/, accelRate, minTurnRate, maxTurnRate, axisCoeff, invert);
}

static short ResetPlayerTurnRate(short turnRate, short decelRate)
{
	int sign = std::copysign(1, turnRate);
	if (abs(turnRate) > decelRate)
		return (turnRate - (decelRate * sign));

	return 0;
}

void ResetPlayerTurnRateX(ItemInfo& item, short decelRate)
{
	auto& player = GetLaraInfo(item);
	player.Control.TurnRate/*.x*/ = ResetPlayerTurnRate(player.Control.TurnRate/*.x*/, decelRate);
}

void ResetPlayerTurnRateY(ItemInfo& item, short decelRate)
{
	auto& player = GetLaraInfo(item);
	player.Control.TurnRate/*.y*/ = ResetPlayerTurnRate(player.Control.TurnRate/*.y*/, decelRate);
}

void ModulateLaraSwimTurnRates(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	/*if (TrInput & (IN_FORWARD | IN_BACK))
		ModulateLaraTurnRateX(item, 0, 0, 0);*/

	if (TrInput & IN_FORWARD)
		item->Pose.Orientation.x -= ANGLE(3.0f);
	else if (TrInput & IN_BACK)
		item->Pose.Orientation.x += ANGLE(3.0f);

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

		// TODO: ModulateLaraLean() doesn't really work here. -- Sezz 2022.06.22
		if (TrInput & IN_LEFT)
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (TrInput & IN_RIGHT)
			item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

void ModulateLaraSubsuitSwimTurnRates(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD && item->Pose.Orientation.x > -ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = -ANGLE(45.0f);
	else if (TrInput & IN_BACK && item->Pose.Orientation.x < ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = ANGLE(45.0f);
	else
		lara->Control.Subsuit.DXRot = 0;

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_SUBSUIT_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

		if (TrInput & IN_LEFT)
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (TrInput & IN_RIGHT)
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

	if (lara->Control.Subsuit.Velocity[0] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[0] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[1] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[1] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[0] != 0 || lara->Control.Subsuit.Velocity[1] != 0)
	{
		auto mul1 = (float)abs(lara->Control.Subsuit.Velocity[0]) / SECTOR(8);
		auto mul2 = (float)abs(lara->Control.Subsuit.Velocity[1]) / SECTOR(8);
		auto vol = ((mul1 + mul2) * 5.0f) + 0.5f;
		SoundEffect(SFX_TR5_VEHICLE_DIVESUIT_ENGINE, &item->Pose, SoundEnvironment::Water, 1.0f + (mul1 + mul2), vol);
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

// TODO: Add a timeout? Imagine a small, sad rain cloud with the properties of a ceiling following Lara overhead.
// RunJumpQueued will never reset, and when the sad cloud flies away after an indefinite amount of time, Lara will jump. -- Sezz 2022.01.22
void SetLaraRunJumpQueue(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = SECTOR(1);
	auto probe = GetCollision(item, item->Pose.Orientation.y, distance, -coll->Setup.Height);

	if ((Context::CanRunJumpForward(*item, *coll) ||													// Area close ahead is permissive...
		(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||		// OR ceiling height far ahead is permissive
		(probe.Position.Floor - y) >= CLICK(0.5f)) &&											// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		lara->Control.IsRunJumpQueued = IsRunJumpQueueableState((LaraState)item->Animation.TargetState);
	}
	else
	{
		lara->Control.IsRunJumpQueued = false;
	}
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

void RumbleLaraHealthCondition(ItemInfo* item)
{
	constexpr auto POWER = 0.2f;
	constexpr auto DELAY = 0.1f;

	const auto& player = GetLaraInfo(*item);

	if (item->HitPoints > LARA_HEALTH_CRITICAL && player.Status.Poison == 0)
		return;

	bool doPulse = ((GlobalCounter & 0x0F) % 0x0F == 1);
	if (doPulse)
		Rumble(POWER, DELAY);
}
