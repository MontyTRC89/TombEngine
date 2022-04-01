#include "framework.h"
#include "GameScriptAnimations.h"

/***
New animations and functionality which Lara can perform.
@pregameclass Animations
@pragma nostrip
*/

void GameScriptAnimations::Register(sol::state* lua)
{
	lua->new_usertype<GameScriptAnimations>(
		"Animations",
		"hasPose", &GameScriptAnimations::HasPose,
		"hasSlideExtended", &GameScriptAnimations::HasSlideExtended,
		"hasSprintJump", &GameScriptAnimations::HasSprintJump,
		"hasMonkeyAutoJump", &GameScriptAnimations::HasMonkeyAutoJump,
		"hasCrawlspaceDive", &GameScriptAnimations::HasCrawlspaceDive,
		"hasCrawlExtended", &GameScriptAnimations::HasCrawlExtended,
		"hasCrouchRoll", &GameScriptAnimations::HasCrouchRoll,
		"hasOverhangClimb", &GameScriptAnimations::HasOverhangClimb);
}
