#include "frameworkandsol.h"
#include "Flow/Animations/Animations.h"

/***
New custom animations which Lara can perform.
@tenclass Flow.Animations
@pragma nostrip
*/

void Animations::Register(sol::table & parent)
{
	parent.new_usertype<Animations>("Animations",
		"crawlExtended", &Animations::CrawlExtended,
		"crouchRoll", &Animations::CrouchRoll,
		"crawlspaceSwandive", &Animations::CrawlspaceSwandive,
		"monkeyTurn180", &Animations::MonkeyTurn180,
		"monkeyAutoJump", &Animations::MonkeyAutoJump,
		"oscillateHang", &Animations::OscillateHang,
		"pose", &Animations::Pose
		);
