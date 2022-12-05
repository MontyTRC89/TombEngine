#include "framework.h"
#include "Sound/sound.h"

#include <filesystem>
#include <regex>
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara.h"
#include "Game/room.h"
#include "Specific/setup.h"
#include "Specific/configuration.h"
#include "Specific/level.h"
#include "Specific/winmain.h"

HSTREAM BASS_3D_Mixdown;
HFX BASS_FXHandler[(int)SoundFilter::Count];
SoundTrackSlot BASS_Soundtrack[(int)SoundTrackType::Count];
HSAMPLE SamplePointer[SOUND_MAX_SAMPLES];
SoundEffectSlot SoundSlot[SOUND_MAX_CHANNELS];

const BASS_BFX_FREEVERB BASS_ReverbTypes[(int)ReverbType::Count] =    // Reverb presets

{ // Dry Mix | Wet Mix |  Size   |  Damp   |  Width  |  Mode  | Channel
  {  1.0f,     0.20f,     0.05f,    0.90f,    0.7f,     0,      -1     },	// 0 = Outside
  {  1.0f,     0.20f,     0.35f,    0.15f,    0.8f,     0,      -1     },	// 1 = Small room
  {  1.0f,     0.25f,     0.55f,    0.20f,    1.0f,     0,      -1     },	// 2 = Medium room
  {  1.0f,     0.25f,     0.80f,    0.50f,    1.0f,     0,      -1     },	// 3 = Large room
  {  1.0f,     0.25f,     0.90f,    1.00f,    1.0f,     0,      -1     }	// 4 = Pipe
};

const std::string TRACKS_PATH = "Audio\\";

std::map<std::string, int> SoundTrackMap;
std::unordered_map<int, SoundTrackInfo> SoundTracks;
int SecretSoundIndex = 5;
constexpr int LegacyLoopingTrackMin = 98;
constexpr int LegacyLoopingTrackMax = 111;

static int GlobalMusicVolume;
static int GlobalFXVolume;

void SetVolumeMusic(int vol) 
{
	GlobalMusicVolume = vol;

	float fVol = static_cast<float>(vol) / 100.0f;
	if (BASS_ChannelIsActive(BASS_Soundtrack[(int)SoundTrackType::BGM].Channel))
	{
		BASS_ChannelSetAttribute(BASS_Soundtrack[(int)SoundTrackType::BGM].Channel, BASS_ATTRIB_VOL, fVol);
	}
	if (BASS_ChannelIsActive(BASS_Soundtrack[(int)SoundTrackType::OneShot].Channel))
	{
		BASS_ChannelSetAttribute(BASS_Soundtrack[(int)SoundTrackType::OneShot].Channel, BASS_ATTRIB_VOL, fVol);
	}
}

void SetVolumeFX(int vol)
{
	GlobalFXVolume = vol;
}

bool LoadSample(char* pointer, int compSize, int uncompSize, int index)
{
	if (index >= SOUND_MAX_SAMPLES)
	{
		TENLog("Sample index " + std::to_string(index) + " is larger than max. amount of samples", LogLevel::Warning);
		return 0;
	}

	if (pointer == nullptr || compSize <= 0)
	{
		TENLog("Sample size or memory address is incorrect for index " + std::to_string(index), LogLevel::Warning);
		return 0;
	}

	// Load and uncompress sample to 32-bit float format.
	HSAMPLE sample = BASS_SampleLoad(true, pointer, 0, compSize, 1, SOUND_SAMPLE_FLAGS);

	if (!sample)
	{
		TENLog("Error loading sample " + std::to_string(index), LogLevel::Error);
		return false;
	}

	// Paranoid (c) TeslaRus
	// Try to free sample before allocating new one.
	Sound_FreeSample(index);

	BASS_SAMPLE info;
	BASS_SampleGetInfo(sample, &info);
	int finalLength = info.length + 44;	// uncompSize is invalid after 16->32 bit conversion

	if (info.freq != 22050 || info.chans != 1)
	{
		TENLog("Wrong sample parameters, must be 22050 Hz Mono", LogLevel::Error);
		return false;
	}

	// Generate RIFF/WAV header to simplify loading sample data to stream. In case if RIFF/WAV header
	// exists, stream could be completely created just by calling BASS_StreamCreateFile().
	char* uncompBuffer = new char[finalLength];
	ZeroMemory(uncompBuffer, finalLength);
	memcpy(uncompBuffer, "RIFF\0\0\0\0WAVEfmt \20\0\0\0", 20);
	memcpy(uncompBuffer + 36, "data\0\0\0\0", 8);

	WAVEFORMATEX *wf = (WAVEFORMATEX*)(uncompBuffer + 20);

	wf->wFormatTag = 3;
	wf->nChannels = info.chans;
	wf->wBitsPerSample = 32;
	wf->nSamplesPerSec = info.freq;
	wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
	wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;

	// Copy raw PCM data from temporary sample buffer to actual buffer which will be used by engine.
	BASS_SampleGetData(sample, uncompBuffer + 44);
	BASS_SampleFree(sample);

	// Cut off trailing silence from samples to prevent gaps in looped playback
	int cleanLength = info.length;
	for (DWORD i = 4; i < info.length; i += 4)
	{
		float *currentSample = reinterpret_cast<float*>(uncompBuffer + finalLength - i);
		if (*currentSample > SOUND_32BIT_SILENCE_LEVEL || *currentSample < -SOUND_32BIT_SILENCE_LEVEL)
		{
			int alignment = i % wf->nBlockAlign;
			cleanLength -= (i - alignment);
			break;
		}
	}

	// Put data size to header
	*(DWORD*)(uncompBuffer + 4) = cleanLength + 44 - 8;
	*(DWORD*)(uncompBuffer + 40) = cleanLength;

	// Create actual sample
	SamplePointer[index] = BASS_SampleLoad(true, uncompBuffer, 0, cleanLength + 44, 65535, SOUND_SAMPLE_FLAGS | BASS_SAMPLE_3D);
	delete uncompBuffer;

	return true;
}

bool SoundEffect(int effectID, Pose* position, SoundEnvironment condition, float pitchMultiplier, float gainMultiplier)
{
	if (!g_Configuration.EnableSound)
		return false;

	if (effectID >= g_Level.SoundMap.size())
		return false;

	if (BASS_GetDevice() == -1)
		return false;

	if (condition != SoundEnvironment::Always)
	{
		// Get current camera room's environment
		auto cameraCondition = TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber) ? SoundEnvironment::Water : SoundEnvironment::Land;

		// Don't play effect if effect's environment isn't the same as camera position's environment
		if (condition != cameraCondition)
			return false;
	}

	// Get actual sample index from SoundMap
	int sampleIndex = g_Level.SoundMap[effectID];

	// -1 means no such effect exists in level file.
	// We set it to -2 afterwards to prevent further debug message firings.
	if (sampleIndex == -1)
	{
		TENLog("Non present effect: " + std::to_string(effectID), LogLevel::Warning);
		g_Level.SoundMap[effectID] = -2;
		return false;
	}
	else if (sampleIndex == -2)
		return false;

	SampleInfo* sampleInfo = &g_Level.SoundDetails[sampleIndex];

	if (sampleInfo->Number < 0)
	{
		TENLog("No valid samples count for effect " + std::to_string(sampleIndex), LogLevel::Warning);
		return false;
	}

	// Assign common sample flags.
	DWORD sampleFlags = SOUND_SAMPLE_FLAGS;

	// Effect's chance to play.
	if ((sampleInfo->Randomness) && ((GetRandomControl() & UCHAR_MAX) > sampleInfo->Randomness))
		return false;

	// Apply 3D attrib only to sound with position property
	if (position)
		sampleFlags |= BASS_SAMPLE_3D;

	// Set & randomize volume (if needed)
	float gain = (static_cast<float>(sampleInfo->Volume) / UCHAR_MAX) * std::clamp(gainMultiplier, SOUND_MIN_PARAM_MULTIPLIER, SOUND_MAX_PARAM_MULTIPLIER);;
	if ((sampleInfo->Flags & SOUND_FLAG_RND_GAIN))
		gain -= (static_cast<float>(GetRandomControl()) / static_cast<float>(RAND_MAX))* SOUND_MAX_GAIN_CHANGE;

	// Set and randomize pitch and additionally multiply by provided value (for vehicles etc)
	float pitch = (1.0f + static_cast<float>(sampleInfo->Pitch) / 127.0f) * std::clamp(pitchMultiplier, SOUND_MIN_PARAM_MULTIPLIER, SOUND_MAX_PARAM_MULTIPLIER);

	// Randomize pitch (if needed)
	if ((sampleInfo->Flags & SOUND_FLAG_RND_PITCH))
		pitch += ((static_cast<float>(GetRandomControl()) / static_cast<float>(RAND_MAX)) - 0.5f) * SOUND_MAX_PITCH_CHANGE * 2.0f;

	// Calculate sound radius and distance to sound
	float radius = (float)(sampleInfo->Radius) * SECTOR(1);
	float distance = Sound_DistanceToListener(position);

	// Don't play sound if it's too far from listener's position.
	if (distance > radius)
		return false;

	// Get final volume of a sound.
	float volume = Sound_Attenuate(gain, distance, radius);

	// Get existing index, if any, of sound which is playing.
	int existingChannel = Sound_EffectIsPlaying(effectID, position);

	// Select behaviour based on effect playback type (bytes 0-1 of flags field)
	auto playType = (SoundPlayMode)(sampleInfo->Flags & 3);
	switch (playType)
	{
	case SoundPlayMode::Normal:
		break;

	case SoundPlayMode::Wait:
		if (existingChannel != -1) // Don't play until stopped
			return false;
		break;

	case SoundPlayMode::Restart:
		if (existingChannel != -1) // Stop existing and continue
			Sound_FreeSlot(existingChannel, SOUND_XFADETIME_CUTSOUND); 
		break;

	case SoundPlayMode::Looped:
		if (existingChannel != -1) // Just update parameters and return, if already playing
		{
			Sound_UpdateEffectPosition(existingChannel, position);
			Sound_UpdateEffectAttributes(existingChannel, pitch, volume);
			return false;
		}
		sampleFlags |= BASS_SAMPLE_LOOP;
		break;
	}

	// Randomly select arbitrary sample from the list, if more than one is present
	int sampleToPlay = 0;
	int numSamples = (sampleInfo->Flags >> 2) & 15;
	if (numSamples == 1)
		sampleToPlay = sampleInfo->Number;
	else
		sampleToPlay = sampleInfo->Number + (int)((GetRandomControl() * numSamples) >> 15);

	// Get free channel to play sample
	int freeSlot = Sound_GetFreeSlot();
	if (freeSlot == -1)
	{
		TENLog("No free channel slot available!", LogLevel::Warning);
		return false;
	}

	// Create sample's stream and reset buffer back to normal value.
	HSTREAM channel = BASS_SampleGetChannel(SamplePointer[sampleToPlay], true);

	if (Sound_CheckBASSError("Trying to create channel for sample %d", false, sampleToPlay))
		return false;

	// Finally ready to play sound, assign it to sound slot.
	SoundSlot[freeSlot].State = SoundState::Idle;
	SoundSlot[freeSlot].EffectID = effectID;
	SoundSlot[freeSlot].Channel = channel;
	SoundSlot[freeSlot].Gain = gain;
	SoundSlot[freeSlot].Origin = position ? Vector3(position->Position.x, position->Position.y, position->Position.z) : SOUND_OMNIPRESENT_ORIGIN;

	if (Sound_CheckBASSError("Applying pitch/gain attribs on channel %x, sample %d", false, channel, sampleToPlay))
		return false;

	// Set looped flag, if necessary
	if (playType == SoundPlayMode::Looped)
		BASS_ChannelFlags(channel, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

	// Play channel
	BASS_ChannelPlay(channel, false);

	if (Sound_CheckBASSError("Queuing channel %x on sample mixer", false, freeSlot))
		return false;

	// Set attributes
	BASS_ChannelSet3DAttributes(channel, position ? BASS_3DMODE_NORMAL : BASS_3DMODE_OFF, SOUND_MAXVOL_RADIUS, radius, 360, 360, 0.0f);
	Sound_UpdateEffectPosition(freeSlot, position, true);
	Sound_UpdateEffectAttributes(freeSlot, pitch, volume);

	if (Sound_CheckBASSError("Applying 3D attribs on channel %x, sound %d", false, channel, effectID))
		return false;

	return true;
}

void PauseAllSounds()
{
	BASS_Pause();
}

void ResumeAllSounds()
{
	BASS_Start();
}

void StopSoundEffect(short effectID)
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if (SoundSlot[i].Channel != NULL && SoundSlot[i].EffectID == effectID && BASS_ChannelIsActive(SoundSlot[i].Channel) == BASS_ACTIVE_PLAYING)
			Sound_FreeSlot(i, SOUND_XFADETIME_CUTSOUND);
	}
}

void StopAllSounds()
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
		Sound_FreeSlot(i, SOUND_XFADETIME_CUTSOUND);

	ZeroMemory(SoundSlot, (sizeof(SoundEffectSlot) * SOUND_MAX_CHANNELS));
}

void FreeSamples()
{
	StopAllSounds();
	for (int i = 0; i < SOUND_MAX_SAMPLES; i++)
		Sound_FreeSample(i);
}

void EnumerateLegacyTracks()
{
	auto dir = std::filesystem::path(TRACKS_PATH);
	if (std::filesystem::exists(dir))
	{
		try {
			// capture three-digit filenames, or those which start with three digits.
			std::regex upToThreeDigits("\\\\((\\d{1,3})[^\\.]*)");
			std::smatch result;
			for (const auto& file : std::filesystem::directory_iterator{ dir })
			{
				std::string fileName = file.path().string();
				auto bResult = std::regex_search(fileName, result, upToThreeDigits);
				if (!result.empty())
				{
					// result[0] is the whole match including the leading backslash, so ignore it
					// result[1] is the full file name, not including the extension
					int index = std::stoi(result[2].str());
					SoundTrackInfo s;

					// TRLE default looping tracks
					if (index >= LegacyLoopingTrackMin && index <= LegacyLoopingTrackMax)
					{
						s.Mode = SoundTrackType::BGM;
					}
					s.Name = result[1];
					SoundTracks.insert(std::make_pair(index, s));
					SecretSoundIndex = std::max(SecretSoundIndex, index);
				}
			}
		}
		catch (std::filesystem::filesystem_error const& e)
		{
			TENLog(e.what(), LogLevel::Error, LogConfig::All);
		}
	}
	else
	{
		TENLog("Folder \"" + dir.string() + "\" does not exist. ", LogLevel::Warning, LogConfig::All);
	}

}

void PlaySoundTrack(std::string track, SoundTrackType mode, QWORD position)
{
	if (!g_Configuration.EnableSound)
		return;

	if (track.empty())
		return;

	bool crossfade = false;
	DWORD crossfadeTime = 0;
	DWORD flags = BASS_STREAM_AUTOFREE | BASS_SAMPLE_FLOAT | BASS_ASYNCFILE;

	bool channelActive = BASS_ChannelIsActive(BASS_Soundtrack[(int)mode].Channel);
	if (channelActive && BASS_Soundtrack[(int)mode].Track.compare(track) == 0)
		return;

	switch (mode)
	{
	case SoundTrackType::OneShot:
		crossfadeTime = SOUND_XFADETIME_ONESHOT;
		break;

	case SoundTrackType::BGM:
		crossfade = true;
		crossfadeTime = channelActive ? SOUND_XFADETIME_BGM : SOUND_XFADETIME_BGM_START;
		flags |= BASS_SAMPLE_LOOP;
		break;
	}

	if (channelActive)
		BASS_ChannelSlideAttribute(BASS_Soundtrack[(int)mode].Channel, BASS_ATTRIB_VOL, -1.0f, crossfadeTime);
	
	auto fullTrackName = TRACKS_PATH + track + ".ogg";
	if (!std::filesystem::exists(fullTrackName))
	{
		fullTrackName = TRACKS_PATH + track + ".mp3";
		if (!std::filesystem::exists(fullTrackName))
		{
			fullTrackName = TRACKS_PATH + track + ".wav";
			if (!std::filesystem::exists(fullTrackName))
			{
				TENLog("No soundtrack files with name '" + track + "' were found", LogLevel::Warning);
				return;
			}
		}
	}

	auto stream = BASS_StreamCreateFile(false, fullTrackName.c_str(), 0, 0, flags);

	if (Sound_CheckBASSError("Opening soundtrack '%s'", false, fullTrackName.c_str()))
		return;

	float masterVolume = (float)GlobalMusicVolume / 100.0f;

	// Damp BGM track in case one-shot track is about to play.

	if (mode == SoundTrackType::OneShot)
	{
		if (BASS_ChannelIsActive(BASS_Soundtrack[(int)SoundTrackType::BGM].Channel))
			BASS_ChannelSlideAttribute(BASS_Soundtrack[(int)SoundTrackType::BGM].Channel, BASS_ATTRIB_VOL, masterVolume * SOUND_BGM_DAMP_COEFFICIENT, SOUND_XFADETIME_BGM_START);
		BASS_ChannelSetSync(stream, BASS_SYNC_FREE | BASS_SYNC_ONETIME | BASS_SYNC_MIXTIME, 0, Sound_FinishOneshotTrack, NULL);
	}

	// BGM tracks are crossfaded, and additionally shuffled a bit to make things more natural.
	// Think everybody are fed up with same start-up sounds of Caves ambience...

	if (crossfade && BASS_ChannelIsActive(BASS_Soundtrack[(int)SoundTrackType::BGM].Channel))
	{		
		// Crossfade...
		BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, 0.0f);
		BASS_ChannelSlideAttribute(stream, BASS_ATTRIB_VOL, masterVolume, crossfadeTime);

		// Shuffle...
		// Only activates if no custom position is passed as argument.
		if (!position)
		{
			QWORD newPos = BASS_ChannelGetLength(stream, BASS_POS_BYTE) * (static_cast<float>(GetRandomControl()) / static_cast<float>(RAND_MAX));
			BASS_ChannelSetPosition(stream, newPos, BASS_POS_BYTE);
		}
	}
	else
		BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, masterVolume);

	BASS_ChannelPlay(stream, false);

	// Try to restore position, if specified.
	if (position && (BASS_ChannelGetLength(stream, BASS_POS_BYTE) > position))
		BASS_ChannelSetPosition(stream, position, BASS_POS_BYTE);

	if (Sound_CheckBASSError("Playing soundtrack '%s'", true, fullTrackName.c_str()))
		return;

	BASS_Soundtrack[(int)mode].Channel = stream;
	BASS_Soundtrack[(int)mode].Track = track;
}

void PlaySoundTrack(std::string track, short mask)
{
	// If track name was included in script, play it as registered track and take mask into account.
	// Otherwise, play it once without registering anywhere.

	if (SoundTrackMap.count(track))
		PlaySoundTrack(SoundTrackMap[track], mask);
	else
		PlaySoundTrack(track, SoundTrackType::OneShot);
}

void PlaySoundTrack(int index, short mask)
{
	if (SoundTracks.find(index) == SoundTracks.end())
	{
		static int lastAttemptedIndex = -1;
		if (lastAttemptedIndex != index)
		{
			TENLog("No track registered with index " + std::to_string(index), LogLevel::Error);
			lastAttemptedIndex = index;
		}
		return;
	}
	
	// Check and modify soundtrack map mask, if needed.
	// If existing mask is unmodified (same activation mask setup), track won't play.

	if (mask && !(SoundTracks[index].Mode == SoundTrackType::BGM))
	{
		int filteredMask = (mask >> 8) & 0x3F;
		if ((SoundTracks[index].Mask & filteredMask) == filteredMask)
			return;	// Mask is the same, don't play it.

		SoundTracks[index].Mask |= filteredMask;
	}

	PlaySoundTrack(SoundTracks[index].Name, SoundTracks[index].Mode);
}

void StopSoundTracks()
{
	StopSoundTrack(SoundTrackType::OneShot, SOUND_XFADETIME_ONESHOT);
	StopSoundTrack(SoundTrackType::BGM, SOUND_XFADETIME_ONESHOT);
}

void StopSoundTrack(SoundTrackType mode, int fadeoutTime)
{
	// Do fadeout.
	BASS_ChannelSlideAttribute(BASS_Soundtrack[(int)mode].Channel, BASS_ATTRIB_VOL | BASS_SLIDE_LOG, -1.0f, fadeoutTime);

	BASS_Soundtrack[(int)mode].Track = {};
	BASS_Soundtrack[(int)mode].Channel = NULL;
}

void ClearSoundTrackMasks()
{
	for (auto& track : SoundTracks) { track.second.Mask = 0; }
}

// Returns specified soundtrack type's stem name and playhead position.
// To be used with savegames. To restore soundtrack, use PlaySoundtrack function with playhead position passed as 3rd argument.

std::pair<std::string, QWORD> GetSoundTrackNameAndPosition(SoundTrackType type)
{
	auto track = BASS_Soundtrack[(int)type];

	if (track.Track.empty() || !BASS_ChannelIsActive(track.Channel))
		return std::pair<std::string, QWORD>();

	std::filesystem::path path = track.Track;
	return std::pair<std::string, QWORD>(path.stem().string(), BASS_ChannelGetPosition(track.Channel, BASS_POS_BYTE));
}

static void CALLBACK Sound_FinishOneshotTrack(HSYNC handle, DWORD channel, DWORD data, void* userData)
{
	if (BASS_ChannelIsActive(BASS_Soundtrack[(int)SoundTrackType::BGM].Channel))
		BASS_ChannelSlideAttribute(BASS_Soundtrack[(int)SoundTrackType::BGM].Channel, BASS_ATTRIB_VOL, (float)GlobalMusicVolume / 100.0f, SOUND_XFADETIME_BGM_START);
}

void Sound_FreeSample(int index)
{
	if (SamplePointer[index] != NULL)
	{
		BASS_SampleFree(SamplePointer[index]);
		SamplePointer[index] = NULL;
	}
}

// Get first free (non-playing) sound slot.
// If no free slots found, now try to hijack slot which is as far from listener as possible

int Sound_GetFreeSlot()
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if (SoundSlot[i].Channel == NULL || !BASS_ChannelIsActive(SoundSlot[i].Channel))
			return i;
	}

	// No free slots, hijack now.

	float minDistance = 0;
	int farSlot = -1;

	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		float distance = Vector3(SoundSlot[i].Origin - Vector3(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z)).Length();
		if (distance > minDistance)
		{
			minDistance = distance;
			farSlot = i;
		}
	}

	TENLog("Hijacking sound effect slot " + std::to_string(farSlot), LogLevel::Info);
	Sound_FreeSlot(farSlot, SOUND_XFADETIME_HIJACKSOUND);
	return farSlot;
}

// Returns slot ID in which effect is playing, if found. If not found, returns -1.
// We use origin position as a reference, because in original TRs it's not possible to clearly
// identify what's the source of the producing effect.

int Sound_EffectIsPlaying(int effectID, Pose *position)
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if (SoundSlot[i].EffectID == effectID)
		{
			if (SoundSlot[i].Channel == NULL)	// Free channel
				continue;

			if (BASS_ChannelIsActive(SoundSlot[i].Channel))
			{
				// Only check position on 3D samples. 2D samples stop immediately.

				BASS_CHANNELINFO info;
				BASS_ChannelGetInfo(SoundSlot[i].Channel, &info);
				if (!(info.flags & BASS_SAMPLE_3D) || !position)
					return i;

				// Check if effect origin is equal OR in nearest possible hearing range.

				Vector3 origin = Vector3(position->Position.x, position->Position.y, position->Position.z);
				if (Vector3::Distance(origin, SoundSlot[i].Origin) < SOUND_MAXVOL_RADIUS)
					return i;
			}
			else
				SoundSlot[i].Channel = NULL; // WTF, let's clean this up
		}
	}
	return -1;
}

// Gets the distance to the source.

float Sound_DistanceToListener(Pose *position)
{
	if (!position) return 0.0f;	// Assume sound is 2D menu sound
	return Sound_DistanceToListener(Vector3(position->Position.x, position->Position.y, position->Position.z));
}
float Sound_DistanceToListener(Vector3 position)
{
	return Vector3(Vector3(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z) - position).Length();
}

// Calculate attenuated volume.

float Sound_Attenuate(float gain, float distance, float radius)
{
	float result = gain * (1.0f - (distance / radius));
	result = result < 0 ? 0.0f : (result > 1.0f ? 1.0f : result);
	return result * ((float)GlobalFXVolume / 100.0f);
}

// Stop and free desired sound slot.

void Sound_FreeSlot(int index, unsigned int fadeout)
{
	if (index > SOUND_MAX_CHANNELS || index < 0)
		return;

	if (SoundSlot[index].Channel != NULL && BASS_ChannelIsActive(SoundSlot[index].Channel))
	{
		if (fadeout > 0)
			BASS_ChannelSlideAttribute(SoundSlot[index].Channel, BASS_ATTRIB_VOL, -1.0f, fadeout);
		else
			BASS_ChannelStop(SoundSlot[index].Channel);
	}

	SoundSlot[index].Channel = NULL;
	SoundSlot[index].State = SoundState::Idle;
	SoundSlot[index].EffectID = -1;
}

// Update sound position in a level.

bool Sound_UpdateEffectPosition(int index, Pose *position, bool force)
{
	if (index > SOUND_MAX_CHANNELS || index < 0)
		return false;

	if (position)
	{
		BASS_CHANNELINFO info;
		BASS_ChannelGetInfo(SoundSlot[index].Channel, &info);
		if (info.flags & BASS_SAMPLE_3D)
		{
			SoundSlot[index].Origin.x = position->Position.x;
			SoundSlot[index].Origin.y = position->Position.y;
			SoundSlot[index].Origin.z = position->Position.z;

			auto pos = BASS_3DVECTOR(position->Position.x, position->Position.y, position->Position.z);
			auto rot = BASS_3DVECTOR(position->Orientation.x, position->Orientation.y, position->Orientation.z);
			BASS_ChannelSet3DPosition(SoundSlot[index].Channel, &pos, &rot, NULL);
			BASS_Apply3D();
		}
	}

	// Reset activity flag, important for looped samples
	if (BASS_ChannelIsActive(SoundSlot[index].Channel))
		SoundSlot[index].State = SoundState::Idle;

	return true;
}

// Update gain and pitch.
bool  Sound_UpdateEffectAttributes(int index, float pitch, float gain)
{
	if (index > SOUND_MAX_CHANNELS || index < 0)
		return false;

	BASS_ChannelSetAttribute(SoundSlot[index].Channel, BASS_ATTRIB_FREQ, 22050.0f * pitch);
	BASS_ChannelSetAttribute(SoundSlot[index].Channel, BASS_ATTRIB_VOL, gain);

	return true;
}

// Update whole sound scene in a level.
// Must be called every frame to update camera position and 3D parameters.

void Sound_UpdateScene()
{
	if (!g_Configuration.EnableSound)
		return;

	// Apply environmental effects

	static int currentReverb = -1;
	auto roomReverb = g_Configuration.EnableReverb ? g_Level.Rooms[Camera.pos.RoomNumber].reverbType : (int)ReverbType::Small;

	if (currentReverb == -1 || roomReverb != currentReverb)
	{
		currentReverb = roomReverb;
		if (currentReverb < (int)ReverbType::Count)
			BASS_FXSetParameters(BASS_FXHandler[(int)SoundFilter::Reverb], &BASS_ReverbTypes[(int)currentReverb]);
	}

	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if ((SoundSlot[i].Channel != NULL) && (BASS_ChannelIsActive(SoundSlot[i].Channel) == BASS_ACTIVE_PLAYING))
		{
			SampleInfo* sampleInfo = &g_Level.SoundDetails[g_Level.SoundMap[SoundSlot[i].EffectID]];

			// Stop and clean up sounds which were in ending state in previous frame.
			// In case sound is looping, make it ending unless they are re-fired in next frame.

			if (SoundSlot[i].State == SoundState::Ending)
			{
				SoundSlot[i].State = SoundState::Ended;
				Sound_FreeSlot(i, SOUND_XFADETIME_CUTSOUND);
				continue;
			}
			else if ((SoundPlayMode)(sampleInfo->Flags & 3) == SoundPlayMode::Looped)
				SoundSlot[i].State = SoundState::Ending;

			// Calculate attenuation and clean up sounds which are out of listener range (only for 3D sounds).

			if (SoundSlot[i].Origin != SOUND_OMNIPRESENT_ORIGIN)
			{
				float radius = (float)(sampleInfo->Radius) * 1024.0f;
				float distance = Sound_DistanceToListener(SoundSlot[i].Origin);

				if (distance > radius)
				{
					Sound_FreeSlot(i);
					continue;
				}
				else
					BASS_ChannelSetAttribute(SoundSlot[i].Channel, BASS_ATTRIB_VOL, Sound_Attenuate(SoundSlot[i].Gain, distance, radius));
			}
		}
	}

	// Apply current listener position.

	Vector3 at = Vector3(Camera.target.x, Camera.target.y, Camera.target.z) -
		Vector3(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z);
	at.Normalize();
	auto mikePos = BASS_3DVECTOR(					// Pos
		Camera.mikePos.x,
		Camera.mikePos.y,
		Camera.mikePos.z);
	auto laraVel = BASS_3DVECTOR(					// Vel
		Lara.WaterCurrentPull.x,
		Lara.WaterCurrentPull.y,
		Lara.WaterCurrentPull.z);
	auto atVec = BASS_3DVECTOR(at.x, at.y, at.z);	// At
	auto upVec = BASS_3DVECTOR(0.0f, 1.0f, 0.0f);	// Up
	BASS_Set3DPosition(&mikePos,
					   &laraVel,
					   &atVec,
					   &upVec);
	BASS_Apply3D();
}

// Initialise BASS engine and also prepare all sound data.
// Called once on engine start-up.

void Sound_Init()
{
	if (!g_Configuration.EnableSound)
		return;

	BASS_Init(g_Configuration.SoundDevice, 44100, BASS_DEVICE_3D, WindowsHandle, NULL);
	if (Sound_CheckBASSError("Initializing BASS sound device", true))
		return;

	// Initialise BASS_FX plugin
	BASS_FX_GetVersion();
	if (Sound_CheckBASSError("Initializing FX plugin", true))
		return;

	// Set 3D world parameters.
	// Rolloff is lessened since we have own attenuation implementation.
	BASS_Set3DFactors(SOUND_BASS_UNITS, 1.5f, 0.5f);	
	BASS_SetConfig(BASS_CONFIG_3DALGORITHM, BASS_3DALG_FULL);

	// Set minimum latency and 2 threads for updating.
	// Most of modern PCs already have multi-core CPUs, so why not parallelize updating?
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 2);
	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10);

	// Create 3D mixdown channel and make it play forever.
	// For realtime mixer channels, we need minimum buffer latency. It shouldn't affect reliability.
	BASS_SetConfig(BASS_CONFIG_BUFFER, 40);
	BASS_3D_Mixdown = BASS_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT, STREAMPROC_DEVICE_3D, NULL);
	BASS_ChannelPlay(BASS_3D_Mixdown, false);

	// Reset buffer back to normal value.
	BASS_SetConfig(BASS_CONFIG_BUFFER, 300);

	if (Sound_CheckBASSError("Starting 3D mixdown", true))
		return;

	// Initialise channels and tracks array
	ZeroMemory(BASS_Soundtrack, (sizeof(HSTREAM) * (int)SoundTrackType::Count));
	ZeroMemory(SoundSlot, (sizeof(SoundEffectSlot) * SOUND_MAX_CHANNELS));

	// Attach reverb effect to 3D channel
 	BASS_FXHandler[(int)SoundFilter::Reverb] = BASS_ChannelSetFX(BASS_3D_Mixdown, BASS_FX_BFX_FREEVERB, 0);
	BASS_FXSetParameters(BASS_FXHandler[(int)SoundFilter::Reverb], &BASS_ReverbTypes[(int)ReverbType::Outside]);

	if (Sound_CheckBASSError("Attaching environmental FX", true))
		return;

	// Apply slight compression to 3D channel
	BASS_FXHandler[(int)SoundFilter::Compressor] = BASS_ChannelSetFX(BASS_3D_Mixdown, BASS_FX_BFX_COMPRESSOR2, 1);
	auto comp = BASS_BFX_COMPRESSOR2{ 4.0f, -18.0f, 1.5f, 10.0f, 100.0f, -1 };
	BASS_FXSetParameters(BASS_FXHandler[(int)SoundFilter::Compressor], &comp);

	if (Sound_CheckBASSError("Attaching compressor", true))
		return;

	return;
}

// Stop all sounds and streams, if any, unplug all channels from the mixer and unload BASS engine.
// Must be called on engine quit.

void Sound_DeInit()
{
	if (g_Configuration.EnableSound)
	{
		TENLog("Shutting down BASS...", LogLevel::Info);
		BASS_Free();
	}
}

bool Sound_CheckBASSError(const char* message, bool verbose, ...)
{
	va_list argptr;
	static char data[4096];

	int bassError = BASS_ErrorGetCode();
	if (verbose || bassError)
	{
		va_start(argptr, verbose);
		int32_t written = vsprintf(data, (char*)message, argptr);	// @TODO: replace with debug/console/message output later...
		va_end(argptr);
		snprintf(data + written, sizeof(data) - written, bassError ? ": error #%d" : ": success", bassError);
		TENLog(data, bassError ? LogLevel::Error : LogLevel::Info);
	}
	return bassError != 0;
}

void SayNo()
{
	SoundEffect(SFX_TR4_LARA_NO_ENGLISH, NULL, SoundEnvironment::Always);
}

void PlaySecretTrack()
{
	if (SoundTracks.find(SecretSoundIndex) == SoundTracks.end())
	{
		TENLog("No secret soundtrack index was found!", LogLevel::Warning);
		return;
	}

	// Secret soundtrack should be always last one on a list.	
	PlaySoundTrack(SoundTracks.at(SecretSoundIndex).Name, SoundTrackType::OneShot);
}

int GetShatterSound(int shatterID)
{
	auto fxID = StaticObjects[shatterID].shatterSound;
	if (fxID != -1 && fxID < NUM_SFX)
		return fxID;

	if (shatterID < 3)
		return SFX_TR5_SMASH_WOOD;
	else
		return SFX_TR4_SMASH_ROCK;
}

void PlaySoundSources()
{
	static constexpr int PLAY_ALWAYS    = 0x8000;
	static constexpr int PLAY_BASE_ROOM = 0x4000;
	static constexpr int PLAY_FLIP_ROOM = 0x2000;

	for (size_t i = 0; i < g_Level.SoundSources.size(); i++)
	{
		const auto& sound = g_Level.SoundSources[i];

		int group = sound.Flags & 0x1FFF;

		if (!FlipStats[group] && (sound.Flags & PLAY_FLIP_ROOM))
			continue;
		else if (FlipStats[group] && (sound.Flags & PLAY_BASE_ROOM))
			continue;

		SoundEffect(sound.SoundID, (Pose*)&sound.Position);
	}
}
