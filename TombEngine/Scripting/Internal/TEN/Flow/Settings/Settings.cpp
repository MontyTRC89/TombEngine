#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Settings/Settings.h"
#include "Scripting/Internal/TEN/Flow/Enums/WeaponTypes.h"
#include "Scripting/Internal/ReservedScriptNames.h"

/***
Global engine settings, which don't fall into particular category or can't be assigned to a specific object.
Can be accessed using @{Flow.SetSettings} and @{Flow.GetSettings} functions.
@tenclass Flow.Settings
@pragma nostrip
*/

Settings::Settings()
{
	Weapons[(int)LaraWeaponType::Pistol         ] = { 8.0f,  BLOCK(8),  9,  (int)BLOCK(0.65f), 3, 1,  1  };
	Weapons[(int)LaraWeaponType::Revolver       ] = { 4.0f,  BLOCK(8),  16, (int)BLOCK(0.65f), 3, 21, 21 };
	Weapons[(int)LaraWeaponType::Uzi            ] = { 8.0f,  BLOCK(8),  3,  (int)BLOCK(0.65f), 3, 1,  1  };
	Weapons[(int)LaraWeaponType::Shotgun        ] = { 10.0f, BLOCK(8),  9,  (int)BLOCK(0.50f), 3, 3,  3  };
	Weapons[(int)LaraWeaponType::Crossbow       ] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 5,  20 };
	Weapons[(int)LaraWeaponType::HK             ] = { 4.0f,  BLOCK(12), 0,  (int)BLOCK(0.50f), 3, 4,  4  };
	Weapons[(int)LaraWeaponType::GrenadeLauncher] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 20, 30 };
	Weapons[(int)LaraWeaponType::RocketLauncher ] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 30, 30 };
	Weapons[(int)LaraWeaponType::HarpoonGun     ] = { 8.0f,  BLOCK(8),  0,  (int)BLOCK(0.50f), 2, 6,  6  };
}

void Settings::Register(sol::table& parent)
{
	AnimSettings::Register(parent);
	FlareSettings::Register(parent);
	HudSettings::Register(parent);
	SystemSettings::Register(parent);
	WeaponSettings::Register(parent);

	parent.new_usertype<Settings>(ScriptReserved_Settings,
		sol::constructors<Settings()>(),
		ScriptReserved_AnimSettings, &Settings::Animations,
		ScriptReserved_FlareSettings, &Settings::Flare,
		ScriptReserved_HudSettings, &Settings::Hud,
		ScriptReserved_SystemSettings, &Settings::System,
		ScriptReserved_WeaponSettings, &Settings::Weapons
	);
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

	/*** Toggle flicker effect.
	@tfield int timeout flare duration. Flare stops working after specified timeout (specified in seconds).*/
	"timeout", &FlareSettings::Timeout,

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

	/*** Overhang climbing.
	@tfield bool overhangClimb enables overhang climbing feature. Currently does not work. */
	"overhangClimb", &AnimSettings::OverhangClimb,

	/*** Extended slide mechanics.
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

/*** Weapons
@section Weapons
This is a table of weapon settings, with several parameters available for every weapon.
Access particular weapon's settings by using {Flow.WeaponType} as an index into array, e.g. `settings.Weapons[Flow.WeaponType.PISTOL]`.
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

		/*** Recoil timeout.
		@tfield float recoilTimeout specifies a timeout (in frames) of recoil before Lara is able to shoot again. Not applicable for backholster weapons. */
		"recoilTimeout", & WeaponSettings::RecoilTimeout,

		/*** Damage.
		@tfield int damage amount of hit points taken for every hit. For crossbow, specifies damage only for normal crossbow bolts. */
		"damage", &WeaponSettings::Damage,

		/*** Secondary damage.
		@tfield int secondaryDamage applicable only to weapons with alternate ammo, such as crossbow explosive ammo or grenade super ammo. */
		"secondaryDamage", & WeaponSettings::SecondaryDamage,

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