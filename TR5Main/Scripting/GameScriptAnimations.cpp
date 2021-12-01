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
		"crawlExtraExits", &GameScriptAnimations::CrawlExtraExits,
		"crawlExtraVaults", &GameScriptAnimations::CrawlExtraVaults,
		"crawlFlexSubmerged", &GameScriptAnimations::CrawlFlexSubmerged,
		"crawlFlexWaterPullUp", &GameScriptAnimations::CrawlFlexWaterPullUp,
		"crawlStep", &GameScriptAnimations::CrawlStep,
		"crouchRoll", &GameScriptAnimations::CrouchRoll,
		"monkeyRoll", &GameScriptAnimations::MonkeyRoll,
		"monkeyVault", &GameScriptAnimations::MonkeyVault,
		"oscillateHanging", &GameScriptAnimations::OscillateHanging,
		"swandiveRollRun", &GameScriptAnimations::SwandiveRollRun
		);
}