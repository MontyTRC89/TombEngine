#pragma once

#include <bass.h>
#include <bass_fx.h>
#include <d3dx9math.h>

#include "..\Game\control.h"
#include "..\Global\global.h"

#define SOUND_BASS_UNITS			1.0f / 1024.0f	// TR->BASS distance unit coefficient
#define SOUND_MAXVOL_RADIUS			1024.0f			// Max. volume hearing distance
#define SOUND_OMNIPRESENT_ORIGIN    D3DXVECTOR3(1.17549e-038f, 1.17549e-038f, 1.17549e-038f)

#define SOUND_MAX_SAMPLES 3072 // Original was 1024, reallocate original 3-byte DX handle struct to just 1-byte memory pointer
#define SOUND_MAX_CHANNELS  32 // Original was 24, reallocate original 36-byte struct with 24-byte SoundEffectSlot struct

#define SOUND_LEGACY_SOUNDMAP_SIZE	 450
#define SOUND_LEGACY_TRACKTABLE_SIZE 136

#define SOUND_FLAG_NO_PAN			(1<<12)	// Unused flag
#define SOUND_FLAG_RND_PITCH		(1<<13)
#define SOUND_FLAG_RND_GAIN			(1<<14)

#define SOUND_MAX_PITCH_CHANGE		0.09f
#define SOUND_MAX_GAIN_CHANGE		0.0625f

#define SOUND_32BIT_SILENCE_LEVEL	4.9e-04f

#define SOUND_SAMPLE_FLAGS			(BASS_SAMPLE_MONO | BASS_SAMPLE_FLOAT)

typedef struct SoundEffectSlot
{
	short state;
	short effectID;
	float gain;
	HCHANNEL channel;
	D3DXVECTOR3 origin;
};

typedef struct SoundTrackSlot
{
	HSTREAM channel;
	short   trackID;
};

enum sound_track_types
{
	SOUND_TRACK_ONESHOT,
	SOUND_TRACK_BACKGROUND,

	NUM_SOUND_TRACK_TYPES
};

enum sound_filters
{
	SOUND_FILTER_REVERB,
	SOUND_FILTER_COMPRESSOR,

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

#define SayNo ((void (__cdecl*)()) 0x004790E0)

long __cdecl SoundEffect(__int32 effectID, PHD_3DPOS* position, __int32 env_flags);
void __cdecl StopSoundEffect(__int16 effectID);
bool __cdecl Sound_LoadSample(char *buffer, __int32 compSize, __int32 uncompSize, __int32 currentIndex);
void __cdecl Sound_FreeSamples();
void __cdecl SOUND_Stop();
void __cdecl S_CDPlay(short index, unsigned int mode);
void __cdecl S_CDPlayEx(short index, DWORD mask, DWORD unknown);
void __cdecl S_CDStop();

void  Sound_Init();
void  Sound_DeInit();
bool  Sound_CheckBASSError(char* message, bool verbose, ...);
void  Sound_UpdateScene();
void  Sound_FreeSample(__int32 index);
int   Sound_GetFreeSlot();
void  Sound_FreeSlot(int index, unsigned int fadeout = 0);
int   Sound_EffectIsPlaying(int effectID, PHD_3DPOS *position);
float Sound_DistanceToListener(PHD_3DPOS *position);
float Sound_DistanceToListener(D3DXVECTOR3 position);
float Sound_Attenuate(float gain, float distance, float radius);
bool  Sound_UpdateEffectPosition(int index, PHD_3DPOS *position, bool force = false);

void Inject_Sound();