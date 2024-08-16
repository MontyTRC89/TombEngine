#pragma once

#include <string>
#include "Scripting/Internal/ScriptAssert.h"

namespace sol
{
	class state;
}

struct Animations
{
	bool HasPose;			// Crossed arms AFK posing.
	bool HasSlideExtended;	// Extended slope sliding functionality (not ready yet).
	bool HasSprintJump;		// Sprint jump.
	bool HasCrawlspaceDive; // Dive into crawlspaces.
	bool HasCrawlExtended;	// Extended crawl moveset.
	bool HasCrouchRoll;		// Crouch roll.
	bool HasOverhangClimb;	// Overhang functionality.
	bool HasLedgeJumps;		// Jump up or back from a ledge.

	// NOTE: Removed. Keep for now to maintain compatibility. -- Sezz 2024.06.06
	bool HasAutoMonkeySwingJump;

	static void Register(sol::table&);
};
