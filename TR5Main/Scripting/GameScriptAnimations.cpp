#include "framework.h"
#include "GameScriptAnimations.h"

/***
New custom animations which Lara can perform.
@pregameclass Animations
@pragma nostrip
*/

void GameScriptAnimations::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptAnimations>("Animations",
		"crawlExtra", &GameScriptAnimations::CrawlExtra,
		"crouchRoll", &GameScriptAnimations::CrouchRoll,
		"monkeyRoll", &GameScriptAnimations::MonkeyRoll,
		"monkeyVault", &GameScriptAnimations::MonkeyVault,
		"oscillateHanging", &GameScriptAnimations::OscillateHanging,
		"swandiveRollRun", &GameScriptAnimations::SwandiveRollRun
		);
}