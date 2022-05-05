#include "frameworkandsol.h"
#include "Flow/Animations/Animations.h"

/***
New custom animations which Lara can perform.
@tenclass Flow.Animations
@pragma nostrip
*/

void Animations::Register(sol::table& parent)
{
	parent.new_usertype<Animations>("Animations",
		"crawlExtended", &Animations::HasCrawlExtended,
		"crouchRoll", &Animations::HasCrouchRoll,
		"crawlspaceSwandive", &Animations::HasCrawlspaceDive,
		"monkeyAutoJump", &Animations::HasMonkeyAutoJump,
		"overhangClimb", &Animations::HasOverhangClimb,
		"slideExtended", &Animations::HasSlideExtended,
		"sprintJump", &Animations::HasSprintJump,
		"pose", &Animations::HasPose
		);
}
