#pragma once

#include "ScriptAssert.h"
#include <string>

namespace sol {
	class state;
}

struct GameScriptAnimations
{
	bool CrouchRoll;				// Crouch roll
	bool MonkeyRoll;				// The 180 degrees roll on monkeybars
	bool CrawlExtra;				// All extra crawl moves
	bool MonkeyVault;				// Vault up to monkeybars when pressing up + action underneath them
	bool SwandiveRollRun;			// The transition from swandive roll to run
	bool OscillateHanging;			// The thin ledge grab animation from TR1 and 2

	static void Register(sol::state* lua);
};