#pragma once

#include <string>
#include "Game/Lara/lara_struct.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"
#include "Specific/clock.h"

struct AnimSettings
{
	int  PoseTimeout = 20;		// AFK pose timeout.

	bool SlideExtended = false;	// Extended slope sliding functionality (not ready yet).
	bool SprintJump = false;	// Sprint jump.
	bool CrawlspaceDive = true;	// Dive into crawlspaces.
	bool CrawlExtended = true;	// Extended crawl moveset.
	bool CrouchRoll = true;		// Crouch roll.
	bool OverhangClimb = false;	// Overhang functionality.
	bool LedgeJumps = false;	// Jump up or back from a ledge.

	static void Register(sol::table& parent);
};

struct FlareSettings
{
	ScriptColor Color = ScriptColor(128, 64, 0);
	float LensflareBrightness = 0.5f;
	bool Sparks = true;
	bool Smoke = true;
	bool Flicker = true;
	int Range = 9;
	int Timeout = 60 * FPS;

	static void Register(sol::table& parent);
};

struct HairSettings
{
	Vec3 Offset = {};
	std::vector<int> Indices = {};

	static void Register(sol::table& parent);
};

struct HudSettings
{
	bool StatusBars = true;
	bool LoadingBar = true;
	bool Speedometer = true;
	bool PickupNotifier = true;

	static void Register(sol::table& parent);
};

struct SystemSettings
{
	ErrorMode ErrorMode = ErrorMode::Warn;
	bool FastReload = true;

	static void Register(sol::table& parent);
};

struct WeaponSettings
{
	float Accuracy = 0.0f;
	float Distance = BLOCK(8);
	
	int   Interval        = 0;
	int	  WaterLevel      = 0;
	int	  FlashDuration   = 0;
	int	  Damage          = 0;
	int	  AlternateDamage = 0;

	static void Register(sol::table& parent);
};

struct Settings
{
	AnimSettings Animations = {};
	FlareSettings Flare = {};
	HudSettings Hud = {};
	SystemSettings System = {};
	std::array<HairSettings, 3> Hair = {};
	std::array<WeaponSettings, (int)LaraWeaponType::NumWeapons> Weapons = {};

	Settings();
	static void Register(sol::table& parent);
};