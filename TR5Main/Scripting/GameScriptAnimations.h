#pragma once

#include "ScriptAssert.h"
#include <string>

namespace sol
{
	class state;
}

struct GameScriptAnimations
{
	bool HasSlideExtended;		// Extended slope sliding functionality.
	bool HasCrawlExtended;		// Extended crawl moveset.
	bool HasCrouchRoll;			// Crouch roll.
	bool HasCrawlspaceDive;		// Dive into crawlspaces.
	bool HasOverhangClimb;		// Overhang functionality.
	bool HasPose;				// Crossed arms AFK posing.
	bool HasMonkeyAutoJump;		// Auto jump to monkey swing when pressing UP + ACTION.

	static void Register(sol::state* lua);
};
