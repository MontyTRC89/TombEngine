#include "framework.h"
#include "Scripting/Internal/TEN/Misc/Miscellaneous.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/explosion.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/Lara/lara.h"
#include "Game/room.h"
#include "Game/spotcam.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Misc/ActionIDs.h"
#include "Scripting/Internal/TEN/Misc/CameraTypes.h"
#include "Scripting/Internal/TEN/Misc/LevelLog.h"
#include "Scripting/Internal/TEN/Misc/SoundTrackTypes.h"
#include "Scripting/Internal/TEN/Vec2/Vec2.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/configuration.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

/***
Functions that don't fit in the other modules.
@tentable Misc 
@pragma nostrip
*/

using namespace TEN::Effects::Environment;
using namespace TEN::Input;
using namespace TEN::Renderer;

namespace Misc 
{
	/// Get the display position of the cursor in percent.
	// @function GetCursorDisplayPosition()
	// @treturn Vec2 Cursor display position in percent.
	static Vec2 GetCursorDisplayPosition()
	{
		// NOTE: Conversion from internal 800x600 to more intuitive 100x100 display space resolution is required.
		// In a future refactor, everything will use 100x100 natively. -- Sezz 2023.10.20

		auto cursorPos = TEN::Input::GetCursorDisplayPosition();
		cursorPos = Vector2(cursorPos.x / SCREEN_SPACE_RES.x, cursorPos.y / SCREEN_SPACE_RES.y) * 100;
		return Vec2(cursorPos);
	}

	/*** Determine if there is a clear line of sight between two positions.
	NOTE: Limited to room geometry. Objects are ignored.

	@function HasLineOfSight()
	@tparam float roomID Room ID of the first position's room.
	@tparam Vec3 posA First position.
	@tparam Vec3 posB Second position.
	@treturn bool __true__ if there is a line of sight, __false__ if not.
	@usage
	local flamePlinthPos = flamePlinth:GetPosition() + Vec3(0, flamePlinthHeight, 0);
	print(Misc.HasLineOfSight(enemyHead:GetRoomNumber(), enemyHead:GetPosition(), flamePlinthPos))
	*/
	[[nodiscard]] static bool HasLineOfSight(int roomID, const Vec3& posA, const Vec3& posB)
	{
		auto vector0 = posA.ToGameVector();
		auto vector1 = posB.ToGameVector();

		MESH_INFO* meshPtr = nullptr;
		auto vector = Vector3i::Zero;
		return (LOS(&vector0, &vector1) &&
				ObjectOnLOS2(&vector0, &vector1, &vector, &meshPtr) == NO_LOS_ITEM);
	}

	///Vibrate game controller, if function is available and setting is on.
	//@function Vibrate
	//@tparam float strength Strength of the vibration
	//@tparam float time __(default 0.3)__ Time of the vibration, in seconds
	static void Vibrate(float strength, sol::optional<float> time)
	{
		Rumble(strength, time.value_or(0.3f), RumbleMode::Both);
	}

	///Do a full-screen fade-to-black. The screen will remain black until a call to FadeIn.
	//@function FadeOut
	//@tparam float speed (default 1.0). Speed in "amount" per second. A value of 1 will make the fade take one second.
	static void FadeOut(TypeOrNil<float> speed)
	{
		SetScreenFadeOut(USE_IF_HAVE(float, speed, 1.0f) / (float)FPS);
	}

	///Do a full-screen fade-in from black.
	//@function FadeIn
	//@tparam float speed (default 1.0). Speed in "amount" per second. A value of 1 will make the fade take one second.
	static void FadeIn(TypeOrNil<float> speed)
	{
		SetScreenFadeIn(USE_IF_HAVE(float, speed, 1.0f) / (float)FPS);
	}

	///Check if fade out is complete and screen is completely black.
	//@treturn bool state of the fade out
	static bool FadeOutComplete()
	{
		return ScreenFadeCurrent == 0.0f;
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
		AlterFOV(ANGLE(std::clamp(abs(angle), 10.0f, 170.0f)));
	}

	//Get field of view.
	//@function GetFOV
	//@treturn float current FOV angle in degrees
	static float GetFOV()
	{
		return TO_DEGREES(GetCurrentFOV());
	}

	///Shows the mode of the game camera.
	//@function GetCameraType
	//@treturn Misc.CameraType value used by the Main Camera.
	//@usage
	//LevelFuncs.OnControlPhase = function() 
	//	if (Misc.GetCameraType() == CameraType.Combat) then
	//		--Do your Actions here.
	//	end
	//end
	static CameraType GetCameraType()
	{
		return Camera.oldType;
	}
	
	/// Play an audio track
	//@function PlayAudioTrack
	//@tparam string name of track (without file extension) to play
	//@tparam Misc.SoundTrackType type of the audio track to play
	static void PlayAudioTrack(const std::string& trackName, TypeOrNil<SoundTrackType> mode)
	{
		auto playMode = USE_IF_HAVE(SoundTrackType, mode, SoundTrackType::OneShot);
		PlaySoundTrack(trackName, playMode);
	}

	///Set and play an ambient track
	//@function SetAmbientTrack
	//@tparam string name of track (without file extension) to play
	static void SetAmbientTrack(const std::string& trackName)
	{
		PlaySoundTrack(trackName, SoundTrackType::BGM);
	}

	///Stop any audio tracks currently playing
	//@function StopAudioTracks
	static void StopAudioTracks()
	{
		StopSoundTracks();
	}

	///Stop audio track that is currently playing
	//@function StopAudioTrack
	//@tparam Misc.SoundTrackType type of the audio track
	static void StopAudioTrack(TypeOrNil<SoundTrackType> mode)
	{
		auto playMode = USE_IF_HAVE(SoundTrackType, mode, SoundTrackType::OneShot);
		StopSoundTrack(playMode, SOUND_XFADETIME_ONESHOT);
	}

	///Get current loudness level for specified track type
	//@function GetAudioTrackLoudness
	//@tparam Misc.SoundTrackType type of the audio track
	//@treturn float current loudness of a specified audio track
	static float GetAudioTrackLoudness(TypeOrNil<SoundTrackType> mode)
	{
		auto playMode = USE_IF_HAVE(SoundTrackType, mode, SoundTrackType::OneShot);
		return GetSoundTrackLoudness(playMode);
	}

	///Get current subtitle string for a voice track currently playing.
	//Subtitle file must be in .srt format, have same filename as voice track, and be placed in same directory as voice track.
	//Returns nil if no voice track is playing or no subtitle present.
	//@function GetCurrentSubtitle
	//@treturn string current subtitle string
	static TypeOrNil<std::string> GetCurrentVoiceTrackSubtitle()
	{
		auto& result = GetCurrentSubtitle();

		if (result.has_value())
		{
			return result.value();
		}
		else
		{
			return sol::nil;
		}
	}

	/// Play sound effect
	//@function PlaySound
	//@tparam int sound ID to play. Corresponds to the value in the sound XML file or Tomb Editor's "Sound Infos" window.
	////@tparam[opt] Vec3 position The 3D position of the sound, i.e. where the sound "comes from". If not given, the sound will not be positional.
	static void PlaySoundEffect(int id, sol::optional<Vec3> pos)
	{
		SoundEffect(id, pos.has_value() ? &Pose(Vector3i(pos->x, pos->y, pos->z)) : nullptr, SoundEnvironment::Always);
	}

	/// Check if the sound effect is playing
	//@function IsSoundPlaying
	//@tparam int Sound ID to check. Corresponds to the value in the sound XML file or Tomb Editor's "Sound Infos" window.
	static bool IsSoundPlaying(int effectID)
	{
		return (Sound_EffectIsPlaying(effectID, nullptr) != SOUND_NO_CHANNEL);
	}

	/// Check if the audio track is playing
	//@function IsAudioTrackPlaying
	//@tparam string Track filename to check. Should be without extension and without full directory path.
	static bool IsAudioTrackPlaying(const std::string& trackName)
	{
		return Sound_TrackIsPlaying(trackName);
	}

	static bool CheckInput(int actionIndex)
	{
		if (actionIndex > (int)ActionID::Count)
		{
			ScriptAssertF(false, "Input action {} does not exist.", actionIndex);
			return false;
		}

		return true;
	}

	static bool KeyIsHeld(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return false;

		if (IsHeld((ActionID)actionIndex))
			return true;

		return false;
	}

	static bool KeyIsHit(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return false;

		if (IsClicked((ActionID)actionIndex))
			return true;

		return false;
	}

	static void KeyPush(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return;

		ActionQueue[actionIndex] = QueueState::Push;
	}

	static void KeyClear(int actionIndex)
	{
		if (!CheckInput(actionIndex))
			return;

		ActionQueue[actionIndex] = QueueState::Clear;
	}

	///Do FlipMap with specific ID
	//@function FlipMap
	//@tparam int flipmap (ID of flipmap)
	static void FlipMap(int flipmap)
	{
		DoFlipMap(flipmap);
	}

	///Enable FlyBy with specific ID
	//@function PlayFlyBy
	//@tparam short flyby (ID of flyby)
	static void PlayFlyBy(short flyby)
	{
		UseSpotCam = true;
		InitializeSpotCam(flyby);
	}

	///Calculate the distance between two positions.
	//@function CalculateDistance
	//@tparam Vec3 posA first position
	//@tparam Vec3 posB second position
	//@treturn int the direct distance from one position to the other
	static int CalculateDistance(const Vec3& pos1, const Vec3& pos2)
	{
		auto p1 = Vector3(pos1.x, pos1.y, pos1.z);
		auto p2 = Vector3(pos2.x, pos2.y, pos2.z);
		return (int)round(Vector3::Distance(p1, p2));
	}

	// TODO: Deprecated. Also should not use int!
	///Calculate the horizontal distance between two positions.
	//@function CalculateHorizontalDistance
	//@tparam Vec3 posA first position
	//@tparam Vec3 posB second position
	//@treturn float the direct distance on the XZ plane from one position to the other
	static int CalculateHorizontalDistance(const Vec3& pos1, const Vec3& pos2)
	{
		auto p1 = Vector2(pos1.x, pos1.z);
		auto p2 = Vector2(pos2.x, pos2.z);
		return (int)round(Vector2::Distance(p1, p2));
	}

	/// Translate a pair display position coordinates to pixel coordinates.
	//To be used with @{ Strings.DisplayString:SetPosition } and @{ Strings.DisplayString }.
	//@function PercentToScreen
	//@tparam float x X component of the display position.
	//@tparam float y Y component of the display position.
	//@treturn int x X coordinate in pixels.
	//@treturn int y Y coordinate in pixels.
	//@usage	
	//local halfwayX, halfwayY = PercentToScreen(50, 50)
	//local baddy
	//local spawnLocationNullmesh = GetMoveableByName("position_behind_left_pillar")
	//local str1 = DisplayString("You spawned an enemy!", halfwayX, halfwayY, Color(255, 100, 100), false, { DisplayStringOption.SHADOW, DisplayStringOption.CENTER })
	//
	//LevelFuncs.triggerOne = function(obj) 
	//	ShowString(str1, 4)
	//end
	static std::tuple<int, int> PercentToScreen(float x, float y)
	{
		float fWidth = g_Configuration.ScreenWidth;
		float fHeight = g_Configuration.ScreenHeight;
		int resX = (int)std::round(fWidth / 100.0f * x);
		int resY = (int)std::round(fHeight / 100.0f * y);

		return std::make_tuple(resX, resY);
	}

	/// Translate a pair of pixel coordinates to display position coordinates.
	//To be used with @{ Strings.DisplayString:GetPosition }.
	//@function ScreenToPercent
	//@tparam int x X pixel coordinate to translate to display position.
	//@tparam int y Y pixel coordinate to translate to display position.
	//@treturn float x X component of display position.
	//@treturn float y Y component of display position.
	static std::tuple<float, float> ScreenToPercent(int x, int y)
	{
		float fWidth = g_Configuration.ScreenWidth;
		float fHeight = g_Configuration.ScreenHeight;
		float resX = x / fWidth * 100.0f;
		float resY = y / fHeight * 100.0f;
		return std::make_tuple(resX, resY);
	}

	/// Reset object camera back to the player and deactivate object camera.
	//@function ResetObjCamera
	static void ResetObjCamera()
	{
		ObjCamera(LaraItem, 0, LaraItem, 0, false);
	}

	/// Write messages within the Log file
	//@advancedDesc
	//For native Lua handling of errors, see the official Lua website:
	//
	//<a href="https://www.lua.org/pil/8.3.html">Error management</a>
	//
	//<a href="https://www.lua.org/manual/5.4/manual.html#pdf-debug.traceback">debug.traceback</a>
	//@function PrintLog
	//@tparam string message to be displayed within the Log
	//@tparam Misc.LogLevel logLevel log level to be displayed
	//@tparam[opt] bool allowSpam true allows spamming of the message
	// 
	//@usage
	//PrintLog('test info log', LogLevel.INFO)
	//PrintLog('test warning log', LogLevel.WARNING)
	//PrintLog('test error log', LogLevel.ERROR)
	//-- spammed message
	//PrintLog('test spam log', LogLevel.INFO, true)
	// 
	static void PrintLog(const std::string& message, const LogLevel& level, TypeOrNil<bool> allowSpam)
	{
		TENLog(message, level, LogConfig::All, USE_IF_HAVE(bool, allowSpam, false));
	}

	/// Get the projected 2D display space position of a 3D world position. Returns nil if the world position is behind the camera view.
	// @tparam Vec3 worldPos 3D world position.
	// @return Vec2 Projected display space position in percent.
	// @usage 
	//
	// Example: Display a string at the player's position.
	// local string = DisplayString('Example', 0, 0, Color(255, 255, 255), false)
	// local displayPos = GetDisplayPosition(Lara:GetPosition())
	// string:SetPosition(PercentToScreen(displayPos.x, displayPos.y))
	static sol::optional<Vec2> GetDisplayPosition(const Vec3& worldPos)
	{
		auto displayPos = g_Renderer.Get2DPosition(worldPos.ToVector3());
		if (!displayPos.has_value())
			return sol::nullopt;

		return Vec2(
			(displayPos->x / SCREEN_SPACE_RES.x) * 100,
			(displayPos->y / SCREEN_SPACE_RES.y) * 100);
	}

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table tableMisc{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Misc, tableMisc);

		///Vibrate gamepad, if possible.
		//@function Vibrate
		//@tparam float strength
		//@tparam float time (in seconds, default: 0.3)
		tableMisc.set_function(ScriptReserved_Vibrate, &Vibrate);

		tableMisc.set_function(ScriptReserved_FadeIn, &FadeIn);
		tableMisc.set_function(ScriptReserved_FadeOut, &FadeOut);
		tableMisc.set_function(ScriptReserved_FadeOutComplete, &FadeOutComplete);

		tableMisc.set_function(ScriptReserved_SetCineBars, &SetCineBars);

		tableMisc.set_function(ScriptReserved_SetFOV, &SetFOV);
		tableMisc.set_function(ScriptReserved_GetFOV, &GetFOV);
		tableMisc.set_function(ScriptReserved_GetCameraType, &GetCameraType);
		tableMisc.set_function(ScriptReserved_SetAmbientTrack, &SetAmbientTrack);

		tableMisc.set_function(ScriptReserved_PlayAudioTrack, &PlayAudioTrack);
		tableMisc.set_function(ScriptReserved_StopAudioTrack, &StopAudioTrack);
		tableMisc.set_function(ScriptReserved_StopAudioTracks, &StopAudioTracks);
		tableMisc.set_function(ScriptReserved_GetAudioTrackLoudness, &GetAudioTrackLoudness); 
		tableMisc.set_function(ScriptReserved_GetCurrentSubtitle, &GetCurrentVoiceTrackSubtitle);

		tableMisc.set_function(ScriptReserved_PlaySound, &PlaySoundEffect);
		tableMisc.set_function(ScriptReserved_IsSoundPlaying, &IsSoundPlaying);
		tableMisc.set_function(ScriptReserved_IsAudioTrackPlaying, &IsAudioTrackPlaying);

		/// Check if particular action key is held
		//@function KeyIsHeld
		//@tparam Misc.ActionID action action mapping index to check
		tableMisc.set_function(ScriptReserved_KeyIsHeld, &KeyIsHeld);

		/// Check if particular action key was hit (once)
		//@function KeyIsHit
		//@tparam Misc.ActionID action action mapping index to check
		tableMisc.set_function(ScriptReserved_KeyIsHit, &KeyIsHit);

		/// Emulate pushing of a certain action key
		//@function KeyPush
		//@tparam Misc.ActionID action action mapping index to push
		tableMisc.set_function(ScriptReserved_KeyPush, &KeyPush);

		/// Clears particular input from action key
		//@function KeyClear
		//@tparam Misc.ActionID action action mapping index to clear
		tableMisc.set_function(ScriptReserved_KeyClear, &KeyClear);

		tableMisc.set_function(ScriptReserved_CalculateDistance, &CalculateDistance);
		tableMisc.set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);
		tableMisc.set_function(ScriptReserved_HasLineOfSight, &HasLineOfSight);

		tableMisc.set_function(ScriptReserved_PercentToScreen, &PercentToScreen);
		tableMisc.set_function(ScriptReserved_ScreenToPercent, &ScreenToPercent);

		tableMisc.set_function(ScriptReserved_FlipMap, &FlipMap);
		tableMisc.set_function(ScriptReserved_PlayFlyBy, &PlayFlyBy);
		tableMisc.set_function(ScriptReserved_ResetObjCamera, &ResetObjCamera);

		tableMisc.set_function(ScriptReserved_PrintLog, &PrintLog);
		tableMisc.set_function(ScriptReserved_GetDisplayPosition, &GetDisplayPosition);
		tableMisc.set_function(ScriptReserved_GetCursorDisplayPosition, &GetCursorDisplayPosition);

		LuaHandler handler{ state };
		handler.MakeReadOnlyTable(tableMisc, ScriptReserved_ActionID, ACTION_IDS);
		handler.MakeReadOnlyTable(tableMisc, ScriptReserved_CameraType, CAMERA_TYPE);
		handler.MakeReadOnlyTable(tableMisc, ScriptReserved_SoundTrackType, SOUNDTRACK_TYPE);
		handler.MakeReadOnlyTable(tableMisc, ScriptReserved_LogLevel, LOG_LEVEL);
	}
}
