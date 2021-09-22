#pragma once
#include <bass.h>
#include <bass_fx.h>
#include "control/control.h"
#include "sound_effects.h"

enum SFX_TYPES
{
	SFX_LANDANDWATER = 0,
	SFX_LANDONLY = (1 << 14),
	SFX_WATERONLY = (2 << 14)
};

#define SFX_ALWAYS 2

#define SOUND_BASS_UNITS			 1.0f / 1024.0f	// TR->BASS distance unit coefficient
#define SOUND_MAXVOL_RADIUS			 1024.0f			// Max. volume hearing distance
#define SOUND_OMNIPRESENT_ORIGIN     Vector3(1.17549e-038f, 1.17549e-038f, 1.17549e-038f)
#define SOUND_MAX_SAMPLES			 8192 // Original was 1024, reallocate original 3-dword DX handle struct to just 1-dword memory pointer
#define SOUND_MAX_CHANNELS			 32 // Original was 24, reallocate original 36-byte struct with 24-byte SoundEffectSlot struct
#define SOUND_LEGACY_SOUNDMAP_SIZE	 450
#define SOUND_NEW_SOUNDMAP_MAX_SIZE	 4096
#define SOUND_LEGACY_TRACKTABLE_SIZE 136
#define SOUND_FLAG_NO_PAN			 (1<<12)	// Unused flag
#define SOUND_FLAG_RND_PITCH		 (1<<13)
#define SOUND_FLAG_RND_GAIN			 (1<<14)
#define SOUND_MAX_PITCH_CHANGE		 0.09f
#define SOUND_MAX_GAIN_CHANGE		 0.0625f
#define SOUND_32BIT_SILENCE_LEVEL	 4.9e-04f
#define SOUND_SAMPLE_FLAGS			 (BASS_SAMPLE_MONO | BASS_SAMPLE_FLOAT)
#define SOUND_XFADETIME_BGM			 5000
#define SOUND_XFADETIME_BGM_START	 1500
#define SOUND_XFADETIME_ONESHOT		 200
#define SOUND_XFADETIME_CUTSOUND	 100
#define SOUND_XFADETIME_HIJACKSOUND	 50
#define SOUND_BGM_DAMP_COEFFICIENT	 0.6f

#define TRACK_FOUND_SECRET			"073_Secret"
#define TRACKS_PREFIX				"Audio\\%s.%s"

struct SoundEffectSlot
{
	short state;
	short effectID;
	float gain;
	HCHANNEL channel;
	Vector3 origin;
};

struct SoundTrackSlot
{
	HSTREAM channel;
	std::string track;
};

enum sound_track_types
{
	SOUND_TRACK_ONESHOT,
	SOUND_TRACK_BGM,

	NUM_SOUND_TRACK_TYPES
};

enum sound_filters
{
	SOUND_FILTER_REVERB,
	SOUND_FILTER_COMPRESSOR,
	SOUND_FILTER_LOWPASS,

	NUM_SOUND_FILTERS
};

enum sound_states
{
	SOUND_STATE_IDLE,
	SOUND_STATE_ENDING,
	SOUND_STATE_ENDED
};

enum sound_flags
{ 
	SOUND_NORMAL, 
	SOUND_WAIT, 
	SOUND_RESTART, 
	SOUND_LOOPED 
};

enum reverb_type
{
	RVB_OUTSIDE,	   // 0x00   no reverberation
	RVB_SMALL_ROOM,	   // 0x01   little reverberation
	RVB_MEDIUM_ROOM,   // 0x02
	RVB_LARGE_ROOM,	   // 0x03
	RVB_PIPE,		   // 0x04   highest reverberation, almost never used

	NUM_REVERB_TYPES
};

struct SAMPLE_INFO
{
	short number;
	byte volume;
	byte radius;
	byte randomness;
	signed char pitch;
	short flags;
};

struct AudioTrack
{
	std::string Name;
	byte Mask;
	bool looped;
};

extern std::unordered_map<std::string, AudioTrack> SoundTracks;
extern std::string CurrentLoopedSoundTrack;

long SoundEffect(int effectID, PHD_3DPOS* position, int env_flags, float pitchMultiplier = 1.0f, float gainMultiplier = 1.0f);
void StopSoundEffect(short effectID);
bool Sound_LoadSample(char *buffer, int compSize, int uncompSize, int currentIndex);
void Sound_FreeSamples();
void Sound_Stop();

void PlaySoundTrack(short track, short flags);
void S_CDPlay(std::string trackName, unsigned int mode);
void S_CDPlayEx(std::string trackName, DWORD mask, DWORD unknown);
void S_CDPlay(int index, unsigned int mode);
void S_CDPlayEx(int index, DWORD mask, DWORD unknown);
void StopSoundTracks();
void SayNo();
void PlaySoundSources();
int  GetShatterSound(int shatterID);

static void CALLBACK Sound_FinishOneshotTrack(HSYNC handle, DWORD channel, DWORD data, void* userData);

void  SetVolumeMusic(int vol);
void  SetVolumeFX(int vol);

void  Sound_Init();
void  Sound_DeInit();
bool  Sound_CheckBASSError(const char* message, bool verbose, ...);
void  Sound_UpdateScene();
void  Sound_FreeSample(int index);
int   Sound_GetFreeSlot();
void  Sound_FreeSlot(int index, unsigned int fadeout = 0);
int   Sound_EffectIsPlaying(int effectID, PHD_3DPOS *position);
float Sound_DistanceToListener(PHD_3DPOS *position);
float Sound_DistanceToListener(Vector3 position);
float Sound_Attenuate(float gain, float distance, float radius);
bool  Sound_UpdateEffectPosition(int index, PHD_3DPOS *position, bool force = false);
bool  Sound_UpdateEffectAttributes(int index, float pitch, float gain);