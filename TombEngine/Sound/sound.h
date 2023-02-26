#pragma once
#include <bass.h>
#include <bass_fx.h>

#include "Game/control/control.h"
#include "Sound/sound_effects.h"

using std::string;

constexpr auto SOUND_BASS_UNITS              = 1.0f / 1024.0f;	// TR->BASS distance unit coefficient
constexpr auto SOUND_MAXVOL_RADIUS           = 1024.0f;		// Max. volume hearing distance
constexpr auto SOUND_OMNIPRESENT_ORIGIN      = Vector3(1.17549e-038f, 1.17549e-038f, 1.17549e-038f);
constexpr auto SOUND_MAX_SAMPLES             = 8192; // Original was 1024, reallocate original 3-dword DX handle struct to just 1-dword memory pointer
constexpr auto SOUND_MAX_CHANNELS            = 32; // Original was 24, reallocate original 36-byte struct with 24-byte SoundEffectSlot struct
constexpr auto SOUND_LEGACY_SOUNDMAP_SIZE    = 450;
constexpr auto SOUND_NEW_SOUNDMAP_MAX_SIZE   = 4096;
constexpr auto SOUND_LEGACY_TRACKTABLE_SIZE  = 136;
constexpr auto SOUND_FLAG_NO_PAN             = (1<<12);	// Unused flag
constexpr auto SOUND_FLAG_RND_PITCH          = (1<<13);
constexpr auto SOUND_FLAG_RND_GAIN           = (1<<14);
constexpr auto SOUND_MAX_PITCH_CHANGE        = 0.09f;
constexpr auto SOUND_MAX_GAIN_CHANGE         = 0.0625f;
constexpr auto SOUND_32BIT_SILENCE_LEVEL     = 4.9e-04f;
constexpr auto SOUND_SAMPLE_FLAGS            = (BASS_SAMPLE_MONO | BASS_SAMPLE_FLOAT);
constexpr auto SOUND_XFADETIME_BGM           = 5000;
constexpr auto SOUND_XFADETIME_BGM_START     = 1500;
constexpr auto SOUND_XFADETIME_ONESHOT       = 200;
constexpr auto SOUND_XFADETIME_CUTSOUND      = 100;
constexpr auto SOUND_XFADETIME_HIJACKSOUND   = 50;
constexpr auto SOUND_BGM_DAMP_COEFFICIENT    = 0.5f;
constexpr auto SOUND_MIN_PARAM_MULTIPLIER    = 0.05f;
constexpr auto SOUND_MAX_PARAM_MULTIPLIER    = 5.0f;

enum class SoundTrackType
{
	OneShot,
	BGM,
	Count
};

enum SampleRate
{
	TRLE_QUALITY_SAMPLES = 22050,
	HIGH_QUALITY_SAMPLES = 44100,
};

enum Channels
{
	MONO = 1,
	STEREO = 2
};

enum class SoundEnvironment
{
	Land,
	Water,
	Always
};

enum class SoundPlayMode
{
	Normal,
	Wait,
	Restart,
	Looped
};

enum class ReverbType
{
	Outside,  // 0x00   no reverberation
	Small,	  // 0x01   little reverberation
	Medium,   // 0x02
	Large,	  // 0x03
	Pipe,	  // 0x04   highest reverberation, almost never used
	Count
};

enum class SoundState
{
	Idle,
	Ending,
	Ended
};

enum class SoundFilter
{
	Reverb,
	Compressor,
	Lowpass,
	Count
};

struct SoundEffectSlot
{
	SoundState State;
	short EffectID;
	float Gain;
	HCHANNEL Channel;
	Vector3 Origin;
};

struct SoundTrackSlot
{
	HSTREAM Channel;
	std::string Track;
};

struct SampleInfo
{
	short Number;
	byte Volume;
	byte Radius;
	byte Randomness;
	signed char Pitch;
	short Flags;
};

struct SoundTrackInfo
{
	std::string Name{};
	SoundTrackType Mode{ SoundTrackType::OneShot };
	int Mask{ 0 };
};

struct SoundSourceInfo
{
	Vector3i Position = Vector3i::Zero;
	int		 SoundID = 0;
	int		 Flags = 0;
	string	 Name = "";

	SoundSourceInfo()
	{
	}

	SoundSourceInfo(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
	}

	SoundSourceInfo(int xPos, int yPos, int zPos, short soundID)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->SoundID = soundID;
	}

	SoundSourceInfo(int xPos, int yPos, int zPos, short soundID, short newflags)
	{
		this->Position = Vector3i(xPos, yPos, zPos);
		this->SoundID = soundID;
		this->Flags = newflags;
	}
};

extern std::map<std::string, int> SoundTrackMap;
extern std::unordered_map<int, SoundTrackInfo> SoundTracks;
extern int SecretSoundIndex;

bool SoundEffect(int effectID, Pose* position, SoundEnvironment condition = SoundEnvironment::Land, float pitchMultiplier = 1.0f, float gainMultiplier = 1.0f);
void StopSoundEffect(short effectID);
bool LoadSample(char *buffer, int compSize, int uncompSize, int currentIndex);
void FreeSamples();
void StopAllSounds();
void PauseAllSounds();
void ResumeAllSounds();

void PlaySoundTrack(std::string trackName, SoundTrackType mode, QWORD position = 0);
void PlaySoundTrack(std::string trackName, short mask = 0);
void PlaySoundTrack(int index, short mask = 0);
void StopSoundTrack(SoundTrackType mode, int fadeoutTime);
void StopSoundTracks();
void ClearSoundTrackMasks();
void PlaySecretTrack();
void SayNo();
void PlaySoundSources();
int  GetShatterSound(int shatterID);
void EnumerateLegacyTracks();

std::pair<std::string, QWORD> GetSoundTrackNameAndPosition(SoundTrackType type);

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
int   Sound_EffectIsPlaying(int effectID, Pose *position);
float Sound_DistanceToListener(Pose *position);
float Sound_DistanceToListener(Vector3 position);
float Sound_Attenuate(float gain, float distance, float radius);
bool  Sound_UpdateEffectPosition(int index, Pose *position, bool force = false);
bool  Sound_UpdateEffectAttributes(int index, float pitch, float gain);
