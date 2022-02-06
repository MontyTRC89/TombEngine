#pragma once

#include "ScriptAssert.h"
#include <string>

namespace sol {
	class state;
}

struct Animations
{
	bool CrawlExtended;				// Extended crawl moveset
	bool CrouchRoll;				// Crouch roll
	bool CrawlspaceSwandive;		// Swandive into crawlspaces
	bool MonkeyTurn180;				// 180 degree turn on monkey swing
	bool MonkeyAutoJump;			// Auto jump to monkey swing when pressing UP + ACTION beneath
	bool OscillateHang;				// Grab thin ledge animation from TR1 and 2
	bool Pose;						// Crossed arms AFK posing

	static void Register(sol::table &);
};
