#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Settings/Settings.h"
#include "Scripting/Internal/ReservedScriptNames.h"

/***
Global engine settings, which don't fall into particular category or can't be assigned to a specific object.
Can be accessed using @{Flow.SetSettings} and @{Flow.GetSettings} functions.
@tenclass Flow.Settings
@pragma nostrip
*/

void Settings::Register(sol::table& parent)
{
	FlareSettings::Register(parent);
	AnimSettings::Register(parent);
	SystemSettings::Register(parent);

	parent.new_usertype<Settings>(ScriptReserved_Settings,
		sol::constructors<Settings()>(),
		ScriptReserved_FlareSettings, &Settings::Flare,
		ScriptReserved_AnimSettings, &Settings::Animations,
		ScriptReserved_SystemSettings, &Settings::System);
}

/*** Flare
@section Flare
These settings change appearance and behaviour of a flare.
*/

void FlareSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<FlareSettings>(ScriptReserved_FlareSettings, sol::constructors<FlareSettings()>(),
		sol::call_constructor, sol::constructors<FlareSettings()>(),

	/*** Flare color 
	@tfield Color color flare color. Used for sparks and lensflare coloring as well. */
	"color", &FlareSettings::Color,

	/*** Light range
	@tfield int range flare light radius or range. Represented in "clicks" equal to 256 world units. */
	"range", &FlareSettings::Range,

	/*** Toggle flicker effect
	@tfield int timeout flare duration. Flare stops working after specified timeout (specified in seconds).*/
	"timeout", & FlareSettings::Timeout,

	/*** Lens flare brightness
	@tfield float lensflareBrightness brightness multiplier. Specifies how bright lens flare is in relation to light (on a range from 0 to 1).*/
	"lensflareBrightness", & FlareSettings::LensflareBrightness,

	/*** Toggle spark effect
	@tfield bool sparks spark effect. Determines whether flare generates sparks when burning. */
	"sparks", &FlareSettings::Sparks,

	/*** Toggle smoke effect
	@tfield bool smoke smoke effect. Determines whether flare generates smoke when burning. */
	"smoke", &FlareSettings::Smoke,

	/*** Toggle flicker effect
	@tfield bool flicker light and lensflare flickering. When turned off, flare light will be constant.*/
	"flicker", &FlareSettings::Flicker);
}

/*** Animations
@section Animations
These settings determine whether specific moveset is available in game.
*/

void AnimSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<AnimSettings>(ScriptReserved_AnimSettings, sol::constructors<AnimSettings()>(),
		sol::call_constructor, sol::constructors<AnimSettings()>(),

	/*** Extended crawl moveset
	@tfield bool crawlExtended when enabled, player will be able to traverse across one-click steps in crawlspaces. */
	"crawlExtended", &AnimSettings::CrawlExtended,

	/*** Crouch roll
	@tfield bool crouchRoll when enabled, player can perform crawlspace roll by pressing sprint key. */
	"crouchRoll", &AnimSettings::CrouchRoll,

	/*** Crawlspace dive
	@tfield bool crawlspaceSwandive when enabled, player will be able to swandive into crawlspaces. */
	"crawlspaceSwandive", &AnimSettings::CrawlspaceDive,

	/*** Overhang climbing
	@tfield bool overhangClimb enables overhang climbing feature. Currently does not work. */
	"overhangClimb", &AnimSettings::OverhangClimb,

	/*** Extended slide mechanics
	@tfield bool slideExtended if enabled, player will be able to change slide direction with controls. Currently does not work. */
	"slideExtended", &AnimSettings::SlideExtended,

	/*** Sprint jump
	@tfield bool sprintJump if enabled, player will be able to perform extremely long jump when sprinting. */
	"sprintJump", &AnimSettings::SprintJump,

	/*** Ledge jumps
	@tfield bool ledgeJumps if this setting is enabled, player will be able to jump upwards while hanging on the ledge. */
	"ledgeJumps", &AnimSettings::LedgeJumps,

	/*** Pose timeout
	@tfield int poseTimeout if this setting is larger than 0, idle standing pose animation will be performed after given timeout (in seconds). */
	"poseTimeout", & AnimSettings::PoseTimeout);
}

/*** System
@section System
Global system settings that is not directly related to gameplay.
*/

void SystemSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<SystemSettings>(ScriptReserved_SystemSettings, sol::constructors<SystemSettings()>(),
		sol::call_constructor, sol::constructors<SystemSettings()>(),

/*** How should the application respond to script errors? <br>
Must be one of the following:
`ErrorMode.TERMINATE` - print to the log file and return to the title level when any script error is hit.
This is the one you will want to go for if you want to know IMMEDIATELY if something has gone wrong.

`ErrorMode.WARN` - print to the log file and continue running the application when a recoverable script error is hit.
Choose this one if booting to the title level is too much for you.

`ErrorMode.SILENT` - do nothing when a recoverable script error is hit.
Think __very__ carefully before using this setting. These error modes are here to help you to keep your scripts
working properly, but if you opt to ignore errors, you won't be alerted if you've misused a function or passed
an invalid argument.

In all of these modes, an *unrecoverable* error will boot you to the title level. If the title level itself
has an unrecoverable error, the game will close.

@mem errorMode
*/
	"errorMode", &SystemSettings::ErrorMode,

/*** Can the game utilize the fast reload feature? <br>
When set to `true`, the game will attempt to perform fast savegame reloading if current level is the same as
the level loaded from the savegame. It will not work if the level timestamp or checksum has changed
(i.e. level was updated). If set to `false`, this functionality is turned off.

@mem fastReload
*/
	"fastReload", &SystemSettings::FastReload);
}