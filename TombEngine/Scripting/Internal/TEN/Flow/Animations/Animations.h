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
	bool HasMonkeyAutoJump; // Auto jump to monkey swing when pressing UP + ACTION. TODO: Make this a player setting.
	bool HasCrawlspaceDive; // Dive into crawlspaces.
	bool HasCrawlExtended;	// Extended crawl moveset.
	bool HasCrouchRoll;		// Crouch roll.
	bool HasOverhangClimb;	// Overhang functionality.
	bool HasLedgeJumps;		// Jump up or back from a ledge.

	static void Register(sol::table &);
};
