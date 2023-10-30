#include "framework.h"
#include "Scripting/Internal/TEN/View/ViewHandler.h"

#include "Game/camera.h"
#include "Game/effects/weather.h"
#include "Game/Lara/lara.h"
#include "Game/spotcam.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/View/CameraTypes.h"
#include "Specific/clock.h"

using namespace TEN::Effects::Environment;

/***
Functions to manage camera and game view.
@tentable View
@pragma nostrip
*/

namespace View
{
	static void FadeOut(TypeOrNil<float> speed)
	{
		SetScreenFadeOut(USE_IF_HAVE(float, speed, 1.0f) / (float)FPS);
	}

	static void FadeIn(TypeOrNil<float> speed)
	{
		SetScreenFadeIn(USE_IF_HAVE(float, speed, 1.0f) / (float)FPS);
	}

	static bool FadeOutComplete()
	{
		return ScreenFadeCurrent == 0.0f;
	}

	static void SetCineBars(TypeOrNil<float> height, TypeOrNil<float> speed)
	{
		// divide by 200 so that a percentage of 100 means that each
		// bar takes up half the screen
		float heightProportion = USE_IF_HAVE(float, height, 30) / 200.0f;
		float speedProportion = USE_IF_HAVE(float, speed, 30) / 200.0f;
		SetCinematicBars(heightProportion, speedProportion / float(FPS));
	}

	static void SetFOV(float angle)
	{
		AlterFOV(ANGLE(std::clamp(abs(angle), 10.0f, 170.0f)));
	}

	static float GetFOV()
	{
		return TO_DEGREES(GetCurrentFOV());
	}

	static CameraType GetCameraType()
	{
		return Camera.oldType;
	}

	static void ResetObjCamera()
	{
		ObjCamera(LaraItem, 0, LaraItem, 0, false);
	}

	static void PlayFlyBy(short flyby)
	{
		UseSpotCam = true;
		InitializeSpotCam(flyby);
	}

	static void FlashScreen(TypeOrNil<ScriptColor> col, TypeOrNil<float> speed)
	{
		auto color = USE_IF_HAVE(ScriptColor, col, ScriptColor(255, 255, 255));
		Weather.Flash(color.GetR(), color.GetG(), color.GetB(), (USE_IF_HAVE(float, speed, 1.0)) / (float)FPS);
	}

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table tableView{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_View, tableView);

		///Do a full-screen fade-in from black.
		//@function FadeIn
		//@tparam float speed (default 1.0). Speed in "amount" per second. A value of 1 will make the fade take one second.
		tableView.set_function(ScriptReserved_FadeIn, &FadeIn);

		///Do a full-screen fade-to-black. The screen will remain black until a call to FadeIn.
		//@function FadeOut
		//@tparam float speed (default 1.0). Speed in "amount" per second. A value of 1 will make the fade take one second.
		tableView.set_function(ScriptReserved_FadeOut, &FadeOut);

		///Check if fade out is complete and screen is completely black.
		//@treturn bool state of the fade out
		tableView.set_function(ScriptReserved_FadeOutComplete, &FadeOutComplete);

		///Move black cinematic bars in from the top and bottom of the game window.
		//@function SetCineBars
		//@tparam float height  __(default 30)__ Percentage of the screen to be covered
		//@tparam float speed __(default 30)__ Coverage percent per second
		tableView.set_function(ScriptReserved_SetCineBars, &SetCineBars);

		///Set field of view.
		//@function SetFOV
		//@tparam float angle in degrees (clamped to [10, 170])
		tableView.set_function(ScriptReserved_SetFOV, &SetFOV);

		//Get field of view.
		//@function GetFOV
		//@treturn float current FOV angle in degrees
		tableView.set_function(ScriptReserved_GetFOV, &GetFOV);

		///Shows the mode of the game camera.
		//@function GetCameraType
		//@treturn View.CameraType value used by the Main Camera.
		//@usage
		//LevelFuncs.OnControlPhase = function() 
		//	if (View.GetCameraType() == CameraType.Combat) then
		//		--Do your Actions here.
		//	end
		//end
		tableView.set_function(ScriptReserved_GetCameraType, &GetCameraType);

		///Enable FlyBy with specific ID
		//@function PlayFlyBy
		//@tparam short flyby (ID of flyby)
		tableView.set_function(ScriptReserved_PlayFlyBy, &PlayFlyBy);

		/// Reset object camera back to Lara and deactivate object camera.
		//@function ResetObjCamera
		tableView.set_function(ScriptReserved_ResetObjCamera, &ResetObjCamera);


		/// Flash screen.
		//@function FlashScreen
		//@tparam Color color (default Color(255, 255, 255))
		//@tparam float speed (default 1.0). Speed in "amount" per second. Value of 1 will make flash take one second. Clamped to [0.005, 1.0].
		tableView.set_function(ScriptReserved_FlashScreen, &FlashScreen);

		LuaHandler handler{ state };
		handler.MakeReadOnlyTable(tableView, ScriptReserved_CameraType, CAMERA_TYPE);
	}
};
