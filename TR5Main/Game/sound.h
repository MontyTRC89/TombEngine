#pragma once

#include <bass.h>
#include <bassmix.h>
#include <bass_fx.h>
#include <d3dx9math.h>
#include "..\Game\control.h"
#include "..\Global\global.h"

#define SOUND_MAX_SAMPLES 3072 // Original was 1024, reallocate original 3-byte DX handle struct to just 1-byte memory pointer
#define SOUND_MAX_CHANNELS  32 // Original was 24, reallocate original 36-byte struct with 26-byte SoundEffectChannel struct

#define SOUND_LEGACY_SOUNDMAP_SIZE	450

#define SOUND_MAXVOL_RANGE			1
#define SOUND_MAXVOL_RADIUS			(SOUND_MAXVOL_RANGE << 10)

#define SOUND_FLAG_NO_PAN			(1<<12)	// Unused flag
#define SOUND_FLAG_RND_PITCH		(1<<13)
#define SOUND_FLAG_RND_GAIN			(1<<14)

#define SOUND_MAX_PITCH_CHANGE		0.09f
#define SOUND_MAX_GAIN_CHANGE		0.0625f

#define SOUND_32BIT_SILENCE_LEVEL	4.9e-04f
#define SOUND_OMNIPRESENT_ORIGIN    D3DXVECTOR3(1.17549e-038f, 1.17549e-038f, 1.17549e-038f)

typedef struct SoundEffectSlot
{
	HSTREAM channel;
	__int32 effectID;
	D3DXVECTOR3 origin;
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
bool __cdecl Sound_MakeSample(char *buffer, __int32 compSize, __int32 uncompSize, __int32 currentIndex);
void __cdecl Sound_FreeSamples();
void __cdecl SOUND_Stop();

static void CALLBACK Sound_ClearSoundSlot(HSYNC handle, DWORD channel, DWORD data, void* slot);

void Sound_Init();
void Sound_DeInit();
bool Sound_CheckBASSError(char* message, ...);
void Sound_UpdateScene();
void Sound_FreeSample(__int32 index);
bool Sound_LoadLegacySample(__int32 index, char *pointer, __int32 compSize, __int32 uncompSize);
int  Sound_GetFreeSlot();
void Sound_FreeSlot(int index);
int  Sound_EffectIsPlaying(int effectID, D3DXVECTOR3 origin);
bool Sound_IsInRange(D3DXVECTOR3 position, float range);
bool Sound_UpdateEffectPosition(int index, D3DXVECTOR3 origin);

void Inject_Sound();