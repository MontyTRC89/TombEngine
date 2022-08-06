#include "framework.h"
#include "ReservedScriptNames.h"
#include "ScriptUtil.h"
#include "Vec3/Vec3.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/explosion.h"
#include "Game/effects/weather.h"
#include "Sound/sound.h"
#include "Specific/configuration.h"
#include "Specific/input.h"

/***
Functions that don't fit in the other modules.
@tentable Misc 
@pragma nostrip
*/

using namespace TEN::Input;
using namespace TEN::Effects::Environment;

namespace Misc 
{
	///Determine if there's a line of sight between two points.
	//
	//i.e. if we run a direct line from one position to another
	//will any geometry get in the way?
	//
	//Note: if you use this with Moveable:GetPosition to test if (for example)
	//two creatures can see one another, you might have to do some extra adjustments.
	//
	//This is because the "position" for most objects refers to its base, i.e., the floor.
	//As a solution, you can increase the y-coordinate of this position to correspond to roughly where the
	//eyes of the creatures would be.
	//@function HasLineOfSight
	//@tparam float room1 ID of the room where the first position is
	//@tparam Vec3 pos1 first position
	//@tparam Vec3 pos2 second position
	//@treturn bool is there a direct line of sight between the two positions?
	//@usage
	//local flamePlinthPos = flamePlinth:GetPosition() + Vec3(0, flamePlinthHeight, 0);
	//print(Misc.HasLineOfSight(enemyHead:GetRoom(), enemyHead:GetPosition(), flamePlinthPos))
	[[nodiscard]] static bool HasLineOfSight(short roomNumber1, Vec3 pos1, Vec3 pos2)
	{
		GameVector vec1, vec2;
		pos1.StoreInGameVector(vec1);
		vec1.roomNumber = roomNumber1;
		pos2.StoreInGameVector(vec2);
		return LOS(&vec1, &vec2);
	}


	static void Vibrate(float strength, sol::optional<float> time)
	{
		Rumble(strength, time.value_or(0.3f), RumbleMode::Both);
	}

	///Do a full-screen fade-to-black. The screen will remain black until a call to FadeIn.
	//@function FadeOut
	//@tparam float speed (default 1.0). Speed in "amount" per second. A value of 1 will make the fade take one second.
	static void FadeOut(TypeOrNil<float> speed)
	{
		SetScreenFadeOut(USE_IF_HAVE(float, speed, 1.0f) / float(FPS));
	}

	///Do a full-screen fade-in from black.
	//@function FadeIn
	//@tparam float speed (default 1.0). Speed in "amount" per second. A value of 1 will make the fade take one second.
	static void FadeIn(TypeOrNil<float> speed)
	{
		SetScreenFadeIn(USE_IF_HAVE(float, speed, 1.0f) / float(FPS));
	}

	///Move black cinematic bars in from the top and bottom of the game window.
	//@function SetCineBars
	//@tparam float height  __(default 30)__ Percentage of the screen to be covered
	//@tparam float speed __(default 30)__ Coverage percent per second
	static void SetCineBars(TypeOrNil<float> height, TypeOrNil<float> speed)
	{
		// divide by 200 so that a percentage of 100 means that each
		// bar takes up half the screen
		float heightProportion = USE_IF_HAVE(float, height, 30) / 200.0f;
		float speedProportion = USE_IF_HAVE(float, speed, 30) / 200.0f;
		SetCinematicBars(heightProportion, speedProportion / float(FPS));
	}

	///Set field of view.
	//@function SetFOV
	//@tparam float angle in degrees (clamped to [10, 170])
	static void SetFOV(float angle)
	{
		AlterFOV(FROM_DEGREES(std::clamp(abs(angle), 10.0f, 170.0f)));
	}

	//Get field of view.
	//@function GetFOV
	//@treturn float current FOV angle in degrees
	static float GetFOV()
	{
		return TO_DEGREES(GetCurrentFOV());
	}
	
	/// Play an audio track
	//@function PlayAudioTrack
	//@tparam string name of track (without file extension) to play
	//@tparam bool loop if true, the track will loop; if false, it won't (default: false)
	static void PlayAudioTrack(std::string const& trackName, TypeOrNil<bool> looped)
	{
		auto mode = USE_IF_HAVE(bool, looped, false) ? SoundTrackType::BGM : SoundTrackType::OneShot;
		PlaySoundTrack(trackName, mode);
	}

	///Set and play an ambient track
	//@function SetAmbientTrack
	//@tparam string name of track (without file extension) to play
	static void SetAmbientTrack(std::string const& trackName)
	{
		PlaySoundTrack(trackName, SoundTrackType::BGM);
	}

	static void PlaySoundEffect(int id, sol::optional<Vec3> p)
	{
		SoundEffect(id, p.has_value() ? &PHD_3DPOS(p.value().x, p.value().y, p.value().z) : nullptr, SoundEnvironment::Always);
	}

	static bool KeyIsHeld(int actionIndex)
	{
		return (TrInput & (1 << actionIndex)) != 0;
	}

	static bool KeyIsHit(int actionIndex)
	{
		return (DbInput & (1 << actionIndex)) != 0;
	}

	static void KeyPush(int actionIndex)
	{
		TrInput |= (1 << actionIndex);
	}

	static void KeyClear(int actionIndex)
	{
		TrInput &= ~(1 << actionIndex);
	}

	///Calculate the distance between two positions.
	//@function CalculateDistance
	//@tparam Vec3 posA first position
	//@tparam Vec3 posB second position
	//@treturn int the direct distance from one position to the other
	static int CalculateDistance(Vec3 const& pos1, Vec3 const& pos2)
	{
		auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z));
		return static_cast<int>(round(result));
	}

	///Calculate the horizontal distance between two positions.
	//@function CalculateHorizontalDistance
	//@tparam Vec3 posA first position
	//@tparam Vec3 posB second position
	//@treturn int the direct distance on the XZ plane from one position to the other
	static int CalculateHorizontalDistance(Vec3 const& pos1, Vec3 const& pos2)
	{
		auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.z - pos2.z));
		return static_cast<int>(round(result));
	}

	///Translate a pair of percentages to screen-space pixel coordinates.
	//To be used with @{Strings.DisplayString:SetPosition} and @{Strings.DisplayString}.
	//@function PercentToScreen
	//@tparam float x percent value to translate to x-coordinate
	//@tparam float y percent value to translate to y-coordinate
	//@treturn int x coordinate in pixels
	//@treturn int y coordinate in pixels
	static std::tuple<int, int> PercentToScreen(double x, double y)
	{
		auto fWidth = static_cast<double>(g_Configuration.Width);
		auto fHeight = static_cast<double>(g_Configuration.Height);
		int resX = static_cast<int>(std::round(fWidth / 100.0 * x));
		int resY = static_cast<int>(std::round(fHeight / 100.0 * y));
		//todo this still assumes a resolution of 800/600. account for this somehow
		return std::make_tuple(resX, resY);
	}

	///Translate a pair of coordinates to percentages of window dimensions.
	//To be used with @{Strings.DisplayString:GetPosition}.
	//@function ScreenToPercent
	//@tparam int x pixel value to translate to a percentage of the window width
	//@tparam int y pixel value to translate to a percentage of the window height
	//@treturn float x coordinate as percentage
	//@treturn float y coordinate as percentage
	static std::tuple<double, double> ScreenToPercent(int x, int y)
	{
		auto fWidth = static_cast<double>(g_Configuration.Width);
		auto fHeight = static_cast<double>(g_Configuration.Height);
		double resX = x / fWidth * 100.0;
		double resY = y / fHeight * 100.0;
		return std::make_tuple(resX, resY);
	}


	void Register(sol::state * state, sol::table & parent) {
		sol::table table_misc{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Misc, table_misc);

		///Vibrate gamepad, if possible.
		//@function Vibrate
		//@tparam float strength
		//@tparam float time (in seconds, default: 0.3)
		table_misc.set_function(ScriptReserved_Vibrate, &Vibrate);

		table_misc.set_function(ScriptReserved_FadeIn, &FadeIn);
		table_misc.set_function(ScriptReserved_FadeOut, &FadeOut);

		table_misc.set_function(ScriptReserved_SetCineBars, &SetCineBars);

		table_misc.set_function(ScriptReserved_SetFOV, &SetFOV);
		table_misc.set_function(ScriptReserved_GetFOV, &GetFOV);
		table_misc.set_function(ScriptReserved_SetAmbientTrack, &SetAmbientTrack);

		table_misc.set_function(ScriptReserved_PlayAudioTrack, &PlayAudioTrack);

		/// Play sound effect
		//@function PlaySound
		//@tparam int sound ID to play
		//@tparam Vec3 position
		table_misc.set_function(ScriptReserved_PlaySound, &PlaySoundEffect);

		/// Check if particular action key is held
		//@function KeyIsHeld
		//@tparam int action mapping index to check
		table_misc.set_function(ScriptReserved_KeyIsHeld, &KeyIsHeld);

		/// Check if particular action key was hit (once)
		//@function KeyIsHit
		//@tparam int action mapping index to check
		table_misc.set_function(ScriptReserved_KeyIsHit, &KeyIsHit);

		/// Emulate pushing of a certain action key
		//@function KeyPush
		//@tparam int action mapping index to push
		table_misc.set_function(ScriptReserved_KeyPush, &KeyPush);

		/// Clears particular input from action key
		//@function KeyClear
		//@tparam int action mapping index to clear
		table_misc.set_function(ScriptReserved_KeyClear, &KeyClear);
		table_misc.set_function(ScriptReserved_CalculateDistance, &CalculateDistance);

		table_misc.set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);

		table_misc.set_function(ScriptReserved_PercentToScreen, &PercentToScreen);
		table_misc.set_function(ScriptReserved_HasLineOfSight, &HasLineOfSight);

		table_misc.set_function(ScriptReserved_ScreenToPercent, &ScreenToPercent);
	}
}
