#pragma once

#include <string>
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/TEN/Color/Color.h"
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
	ScriptColor Color = ScriptColor(255, 180, 0);
	float LensflareBrightness = 1.0f;
	bool Sparks = true;
	bool Smoke = true;
	bool Flicker = true;
	int Range = 9;
	int Timeout = 60 * FPS;

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

struct Settings
{
	AnimSettings Animations = {};
	FlareSettings Flare = {};
	HudSettings Hud = {};
	SystemSettings System = {};

	static void Register(sol::table& parent);
};