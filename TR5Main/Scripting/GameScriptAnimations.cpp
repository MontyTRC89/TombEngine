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
		"hasSlideExtended", &GameScriptAnimations::HasSlideExtended,
		"hasCrawlExtended", &GameScriptAnimations::HasCrawlExtended,
		"hasCrouchRoll", &GameScriptAnimations::HasCrouchRoll,
		"hasCrawlspaceSwandive", &GameScriptAnimations::HasCrawlspaceDive,
		"hasOverhangClimb", &GameScriptAnimations::HasOverhangClimb,
		"hasPose", &GameScriptAnimations::HasPose,
		"hasMonkeyAutoJump", &GameScriptAnimations::HasMonkeyAutoJump);
}
