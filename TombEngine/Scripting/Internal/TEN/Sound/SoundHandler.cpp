#include "framework.h"
#include "Scripting/Internal/TEN/Sound/SoundHandler.h"

#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Sound/sound.h"
#include "Scripting/Internal/TEN/Vec3/Vec3.h"
#include "Scripting/Internal/TEN/Sound/SoundTrackTypes.h"

/***
Functions to manage sounds.
@tentable Sound 
@pragma nostrip
*/

namespace Sound
{
	static void PlayAudioTrack(const std::string& trackName, TypeOrNil<SoundTrackType> mode)
	{
		auto playMode = USE_IF_HAVE(SoundTrackType, mode, SoundTrackType::OneShot);
		PlaySoundTrack(trackName, playMode);
	}

	static void SetAmbientTrack(const std::string& trackName)
	{
		PlaySoundTrack(trackName, SoundTrackType::BGM);
	}

	static void StopAudioTracks()
	{
		StopSoundTracks();
	}

	static void StopAudioTrack(TypeOrNil<SoundTrackType> mode)
	{
		auto playMode = USE_IF_HAVE(SoundTrackType, mode, SoundTrackType::OneShot);
		StopSoundTrack(playMode, SOUND_XFADETIME_ONESHOT);
	}

	static float GetAudioTrackLoudness(TypeOrNil<SoundTrackType> mode)
	{
		auto playMode = USE_IF_HAVE(SoundTrackType, mode, SoundTrackType::OneShot);
		return GetSoundTrackLoudness(playMode);
	}

	static void PlaySoundEffect(int id, sol::optional<Vec3> p)
	{
		SoundEffect(id, p.has_value() ? &Pose(p.value().x, p.value().y, p.value().z) : nullptr, SoundEnvironment::Always);
	}

	static bool IsSoundPlaying(int effectID)
	{
		return (Sound_EffectIsPlaying(effectID, nullptr) != SOUND_NO_CHANNEL);
	}

	static bool IsAudioTrackPlaying(const std::string& trackName)
	{
		return Sound_TrackIsPlaying(trackName);
	}

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

	void Register(sol::state* state, sol::table& parent)
	{
		sol::table tableMisc{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Sound, tableMisc);

		/// Play an audio track
		//@function PlayAudioTrack
		//@tparam string name of track (without file extension) to play
		//@tparam Sound.SoundTrackType type of the audio track to play
		tableMisc.set_function(ScriptReserved_PlayAudioTrack, &PlayAudioTrack);

		///Set and play an ambient track
		//@function SetAmbientTrack
		//@tparam string name of track (without file extension) to play
		tableMisc.set_function(ScriptReserved_SetAmbientTrack, &SetAmbientTrack);

		///Stop any audio tracks currently playing
		//@function StopAudioTracks
		tableMisc.set_function(ScriptReserved_StopAudioTracks, &StopAudioTracks);

		///Stop audio track that is currently playing
		//@function StopAudioTrack
		//@tparam Misc.SoundTrackType type of the audio track
		tableMisc.set_function(ScriptReserved_StopAudioTrack, &StopAudioTrack);

		///Get current loudness level for specified track type
		//@function GetAudioTrackLoudness
		//@tparam Misc.SoundTrackType type of the audio track
		//@treturn float current loudness of a specified audio track
		tableMisc.set_function(ScriptReserved_GetAudioTrackLoudness, &GetAudioTrackLoudness);

		/// Play sound effect
		//@function PlaySound
		//@tparam int sound ID to play. Corresponds to the value in the sound XML file or Tomb Editor's "Sound Infos" window.
		////@tparam[opt] Vec3 position The 3D position of the sound, i.e. where the sound "comes from". If not given, the sound will not be positional.
		tableMisc.set_function(ScriptReserved_PlaySound, &PlaySoundEffect);

		/// Check if the sound effect is playing
		//@function IsSoundPlaying
		//@tparam int Sound ID to check. Corresponds to the value in the sound XML file or Tomb Editor's "Sound Infos" window.
		tableMisc.set_function(ScriptReserved_IsSoundPlaying, &IsSoundPlaying);

		/// Check if the audio track is playing
		//@function IsAudioTrackPlaying
		//@tparam string Track filename to check. Should be without extension and without full directory path.
		tableMisc.set_function(ScriptReserved_IsAudioTrackPlaying, &IsAudioTrackPlaying);

		///Get current subtitle string for a voice track currently playing.
		//Subtitle file must be in .srt format, have same filename as voice track, and be placed in same directory as voice track.
		//Returns nil if no voice track is playing or no subtitle present.
		//@function GetCurrentSubtitle
		//@treturn string current subtitle string
		tableMisc.set_function(ScriptReserved_GetCurrentSubtitle, &GetCurrentVoiceTrackSubtitle);

		LuaHandler handler{ state };
		handler.MakeReadOnlyTable(tableMisc, ScriptReserved_SoundTrackType, SOUNDTRACK_TYPE);
	}
};
