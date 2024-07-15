#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Animations/Animations.h"

/***
New custom animations which Lara can perform.
@tenclass Flow.Animations
@pragma nostrip
*/

void Animations::Register(sol::table& parent)
{
	parent.new_usertype<Animations>("Animations",
		sol::constructors<Animations()>(),
		sol::call_constructor, sol::constructors<Animations()>(),
		"crawlExtended", &Animations::HasCrawlExtended,
		"crouchRoll", &Animations::HasCrouchRoll,
		"crawlspaceSwandive", &Animations::HasCrawlspaceDive,
		"overhangClimb", &Animations::HasOverhangClimb,
		"slideExtended", &Animations::HasSlideExtended,
		"sprintJump", &Animations::HasSprintJump,
		"pose", &Animations::HasPose,
		"ledgeJumps", &Animations::HasLedgeJumps,

		// NOTE: Removed. Keep for now to maintain compatibility. -- Sezz 2024.06.06
		"monkeyAutoJump", & Animations::HasAutoMonkeySwingJump);
}
