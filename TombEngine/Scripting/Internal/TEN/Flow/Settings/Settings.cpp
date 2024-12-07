#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Settings/Settings.h"

#include "Game/effects/Hair.h"
#include "Scripting/Internal/TEN/Flow/Enums/WeaponTypes.h"
#include "Scripting/Internal/ReservedScriptNames.h"

using namespace TEN::Effects::Hair;

/***
Global engine settings, which don't fall into particular category or can't be assigned to a specific object.
Can be accessed using @{Flow.SetSettings} and @{Flow.GetSettings} functions.
@tenclass Flow.Settings
@pragma nostrip
*/

Settings::Settings()
{
	Hair[(int)LaraHairType::Normal]     = { {  -4.0f,  -4.0f, -48.0f }, { 37, 39, 40, 38 } };
	Hair[(int)LaraHairType::YoungLeft]  = { { -48.0f, -48.0f, -50.0f }, { 79, 78, 76, 77 } };
	Hair[(int)LaraHairType::YoungRight] = { {  48.0f, -48.0f, -50.0f }, { 68, 69, 70, 71 } };
	
	// Since we directly bind Weapons array to Lua, and Lua accesses this array by native enum, where 0 is NONE, and 1 is
	// PISTOLS, zero index is omitted due to Lua indexing arrays starting from 1. Therefore we subtract 1 from initializer index.

	Weapons[(int)LaraWeaponType::Pistol          - 1] = { 8.0f,  BLOCK(8),  9,  (int)BLOCK(0.65f), 3, 1,  1,  30, true,  true  };
	Weapons[(int)LaraWeaponType::Revolver        - 1] = { 4.0f,  BLOCK(8),  16, (int)BLOCK(0.65f), 3, 21, 21, 6,  true,  false };
	Weapons[(int)LaraWeaponType::Uzi             - 1] = { 8.0f,  BLOCK(8),  3,  (int)BLOCK(0.65f), 3, 1,  1,  30, true,  true  };
	Weapons[(int)LaraWeaponType::Shotgun         - 1] = { 10.0f, BLOCK(8),  9,  (int)BLOCK(0.50f), 3, 3,  3,  6,  true,  true  };
	Weapons[(int)LaraWeaponType::Crossbow        - 1] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 5,  20, 10, false, false };
	Weapons[(int)LaraWeaponType::HK              - 1] = { 4.0f,  BLOCK(12), 0,  (int)BLOCK(0.50f), 3, 4,  4,  30, true,  true  };
	Weapons[(int)LaraWeaponType::GrenadeLauncher - 1] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 30, 30, 10, true,  false };
	Weapons[(int)LaraWeaponType::RocketLauncher  - 1] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 30, 30, 1,  true,  false };
	Weapons[(int)LaraWeaponType::HarpoonGun      - 1] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 6,  6,  10, false, false };
}

void Settings::Register(sol::table& parent)
{
	AnimSettings::Register(parent);
	FlareSettings::Register(parent);
	HairSettings::Register(parent);
	HudSettings::Register(parent);
	PhysicsSettings::Register(parent);
	SystemSettings::Register(parent);
	WeaponSettings::Register(parent);

	parent.new_usertype<Settings>(ScriptReserved_Settings,
		sol::constructors<Settings()>(),
		ScriptReserved_AnimSettings, &Settings::Animations,
		ScriptReserved_FlareSettings, &Settings::Flare,
		ScriptReserved_HairSettings, &Settings::Hair,
		ScriptReserved_HudSettings, &Settings::Hud,
		ScriptReserved_PhysicsSettings, &Settings::Physics,
		ScriptReserved_SystemSettings, &Settings::System,
		ScriptReserved_WeaponSettings, &Settings::Weapons
	);
}

/*** Animations
@section Animations
These settings determine whether specific moveset is available in game.
*/

void AnimSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<AnimSettings>(ScriptReserved_AnimSettings, sol::constructors<AnimSettings()>(),
		sol::call_constructor, sol::constructors<AnimSettings()>(),

	/*** Extended crawl moveset.
	@tfield bool crawlExtended when enabled, player will be able to traverse across one-click steps in crawlspaces. */
	"crawlExtended", &AnimSettings::CrawlExtended,

	/*** Crouch roll.
	@tfield bool crouchRoll when enabled, player can perform crawlspace roll by pressing sprint key. */
	"crouchRoll", &AnimSettings::CrouchRoll,

	/*** Crawlspace dive.
	@tfield bool crawlspaceSwandive when enabled, player will be able to swandive into crawlspaces. */
	"crawlspaceSwandive", &AnimSettings::CrawlspaceDive,

	/* Overhang climbing.
	@tfield bool overhangClimb enables overhang climbing feature. Currently does not work. */
	"overhangClimb", &AnimSettings::OverhangClimb,

	/* Extended slide mechanics.
	@tfield bool slideExtended if enabled, player will be able to change slide direction with controls. Currently does not work. */
	"slideExtended", &AnimSettings::SlideExtended,

	/*** Sprint jump.
	@tfield bool sprintJump if enabled, player will be able to perform extremely long jump when sprinting. */
	"sprintJump", &AnimSettings::SprintJump,

	/*** Ledge jumps.
	@tfield bool ledgeJumps if this setting is enabled, player will be able to jump upwards while hanging on the ledge. */
	"ledgeJumps", &AnimSettings::LedgeJumps,

	/*** Pose timeout.
	@tfield int poseTimeout if this setting is larger than 0, idle standing pose animation will be performed after given timeout (in seconds). */
	"poseTimeout", &AnimSettings::PoseTimeout);
}

/*** Flare
@section Flare
These settings change appearance and behaviour of a flare.
*/

void FlareSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<FlareSettings>(ScriptReserved_FlareSettings, sol::constructors<FlareSettings()>(),
		sol::call_constructor, sol::constructors<FlareSettings()>(),

	/*** Flare color.
	@tfield Color color flare color. Used for sparks and lensflare coloring as well. */
	"color", &FlareSettings::Color,

	/*** Light range.
	@tfield int range flare light radius or range. Represented in "clicks" equal to 256 world units. */
	"range", &FlareSettings::Range,

	/*** Burn timeout.
	@tfield int timeout flare burn timeout. Flare will stop working after given timeout (specified in seconds).*/
	"timeout", &FlareSettings::Timeout,

	/*** Default flare pickup count.
	@tfield int pickupCount specifies amount of flares that you get when you pick up a box of flares.*/
	"pickupCount", &FlareSettings::PickupCount,

	/*** Lens flare brightness.
	@tfield float lensflareBrightness brightness multiplier. Specifies how bright lens flare is in relation to light (on a range from 0 to 1).*/
	"lensflareBrightness", &FlareSettings::LensflareBrightness,

	/*** Toggle spark effect.
	@tfield bool sparks spark effect. Determines whether flare generates sparks when burning. */
	"sparks", &FlareSettings::Sparks,

	/*** Toggle smoke effect.
	@tfield bool smoke smoke effect. Determines whether flare generates smoke when burning. */
	"smoke", &FlareSettings::Smoke,

	/*** Toggle flicker effect.
	@tfield bool flicker light and lensflare flickering. When turned off, flare light will be constant.*/
	"flicker", &FlareSettings::Flicker);
}

/*** Hair
@section Hair
This is a table of Lara's braid object settings.
Table consists of three entries, with first one representing classic Lara braid, and 2 and 3 representing left and right young Lara braids respectively.
Therefore, if you want to access classic Lara braid settings, use `settings.Hair[1]`, and so on.
*/

void HairSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<HairSettings>(ScriptReserved_HairSettings, sol::constructors<HairSettings()>(),
		sol::call_constructor, sol::constructors<HairSettings()>(),

	/*** Relative braid offset to a headmesh.
	@tfield Vec3 offset specifies how braid is positioned in relation to a headmesh. */
	"offset", &HairSettings::Offset,
	
	/*** Braid connection indices.
	@tfield table indices a list of headmesh's vertex connection indices. Each index corresponds to nearest braid rootmesh vertex. Amount of indices is unlimited. */
	"indices", &HairSettings::Indices);
}

/*** Hud
@section Hud
These settings determine visibility of particular in-game HUD elements.
*/

void HudSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<HudSettings>(ScriptReserved_HudSettings, sol::constructors<HudSettings()>(),
		sol::call_constructor, sol::constructors<HudSettings()>(),

	/*** Toggle in-game status bars visibility.
	@tfield bool statusBars if disabled, all status bars (health, air, stamina) will be hidden.. */
	"statusBars", &HudSettings::StatusBars,

	/*** Toggle loading bar visibility.
	@tfield bool loadingBar if disabled, loading bar will be invisible in game. */
	"loadingBar", &HudSettings::LoadingBar,

	/*** Toggle speedometer visibility.
	@tfield bool speedometer if disabled, speedometer will be invisible in game. */
	"speedometer", &HudSettings::Speedometer,

	/*** Toggle pickup notifier visibility.
	@tfield bool pickupNotifier if disabled, pickup notifier will be invisible in game. */
	"pickupNotifier", &HudSettings::PickupNotifier);
}

/*** Physics
@section Physics
Here you will find various settings for game world physics.
*/

void PhysicsSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<PhysicsSettings>(ScriptReserved_PhysicsSettings, sol::constructors<PhysicsSettings()>(),
		sol::call_constructor, sol::constructors<PhysicsSettings()>(),

	/*** Global world gravity.
	@tfield float gravity specifies global gravity. Mostly affects Lara and several other objects. */
	"gravity", &PhysicsSettings::Gravity,

	/*** Swim velocity.
	@tfield float swimVelocity specifies swim velocity for Lara. Affects both surface and underwater. */
	"swimVelocity", &PhysicsSettings::SwimVelocity);
}

/*** Weapons
@section Weapons
This is a table of weapon settings, with several parameters available for every weapon.
Access particular weapon's settings by using @{Flow.WeaponType} as an index for this table, e.g. `settings.Weapons[Flow.WeaponType.PISTOLS]`.
*/

void WeaponSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<WeaponSettings>(ScriptReserved_WeaponSettings, sol::constructors<WeaponSettings()>(),
		sol::call_constructor, sol::constructors<WeaponSettings()>(),

	/*** Shooting accuracy.
	@tfield float accuracy determines accuracy range in angles (smaller angles mean higher accuracy). Not applicable for projectile weapons. */
	"accuracy", &WeaponSettings::Accuracy,

	/*** Targeting distance.
	@tfield float targetingDistance specifies maximum targeting distance in world units (1 block = 1024 world units) for a given weapon. */
	"distance", &WeaponSettings::Distance,

	/*** Shooting interval.
	@tfield float interval specifies an interval (in frames), after which Lara is able to shoot again. Not applicable for backholster weapons. */
	"interval", &WeaponSettings::Interval,

	/*** Damage.
	@tfield int damage amount of hit points taken for every hit. */
	"damage", &WeaponSettings::Damage,

	/*** Alternate damage.
	@tfield int alternateDamage for Revolver and HK, specifies damage in lasersight mode. For crossbow, specifies damage for explosive ammo. */
	"alternateDamage", &WeaponSettings::AlternateDamage,

	/*** Default ammo pickup count.
	@tfield int pickupCount amount of ammo which is given with every ammo pickup for this weapon. */
	"pickupCount", &WeaponSettings::PickupCount,

	/*** Gun smoke.
	@tfield bool smoke if set to true, indicates that weapon emits gun smoke. Not applicable for crossbow and harpoon gun. */
	"smoke", &WeaponSettings::Smoke,

	/*** Gun shell.
	@tfield bool shell if set to true, indicates that weapon emits gun shell. Not applicable for projectile weapons. */
	"shell", &WeaponSettings::Shell,

	/*** Gunflash duration.
	@tfield int flashDuration specifies the duration of a gunflash effect. Not applicable for weapons without gunflash. */
	"flashDuration", &WeaponSettings::FlashDuration,

	/*** Water level.
	@tfield int waterLevel specifies water depth, at which Lara will put weapons back into holsters, indicating it's not possible to use it in water. */
	"waterLevel", &WeaponSettings::WaterLevel);
}

/*** System
@section System
Global system settings that is not directly related to gameplay.
*/

void SystemSettings::Register(sol::table& parent)
{
	parent.create().new_usertype<SystemSettings>(ScriptReserved_SystemSettings, sol::constructors<SystemSettings()>(),
		sol::call_constructor, sol::constructors<SystemSettings()>(),

	/*** How should the application respond to script errors?
	@tfield Flow.ErrorMode errorMode error mode to use. */
	"errorMode", &SystemSettings::ErrorMode,

/*** Can the game utilize the fast reload feature? <br>
When set to `true`, the game will attempt to perform fast savegame reloading if current level is the same as
the level loaded from the savegame. It will not work if the level timestamp or checksum has changed
(i.e. level was updated). If set to `false`, this functionality is turned off.

@tfield bool fastReload toggle fast reload on or off.
*/
	"fastReload", &SystemSettings::FastReload);
}