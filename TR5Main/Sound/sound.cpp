#include "framework.h"
#include "winmain.h"
#include <filesystem>
#include "Sound/sound.h"
#include "lara.h"
#include "camera.h"
#include "control/flipmap.h"
#include "configuration.h"
#include "level.h"

using std::vector;
using std::unordered_map;
using std::string;

HSTREAM BASS_3D_Mixdown;
HFX BASS_FXHandler[NUM_SOUND_FILTERS];
SoundTrackSlot BASS_Soundtrack[NUM_SOUND_TRACK_TYPES];
HSAMPLE SamplePointer[SOUND_MAX_SAMPLES];
SoundEffectSlot SoundSlot[SOUND_MAX_CHANNELS];

const BASS_BFX_FREEVERB BASS_ReverbTypes[NUM_REVERB_TYPES] =    // Reverb presets

{ // Dry Mix | Wet Mix |  Size   |  Damp   |  Width  |  Mode  | Channel
  {  1.0f,     0.20f,     0.05f,    0.90f,    0.7f,     0,      -1     },	// 0 = Outside
  {  1.0f,     0.20f,     0.35f,    0.15f,    0.8f,     0,      -1     },	// 1 = Small room
  {  1.0f,     0.25f,     0.55f,    0.20f,    1.0f,     0,      -1     },	// 2 = Medium room
  {  1.0f,     0.25f,     0.80f,    0.50f,    1.0f,     0,      -1     },	// 3 = Large room
  {  1.0f,     0.25f,     0.90f,    1.00f,    1.0f,     0,      -1     }	// 4 = Pipe
};

unordered_map<string, AudioTrack> SoundTracks;
std::string CurrentLoopedSoundTrack;

static int GlobalMusicVolume;
static int GlobalFXVolume;

void SetVolumeMusic(int vol) 
{
	GlobalMusicVolume = vol;

	float fVol = static_cast<float>(vol) / 100.0f;
	if (BASS_ChannelIsActive(BASS_Soundtrack[SOUND_TRACK_BGM].channel))
	{
		BASS_ChannelSetAttribute(BASS_Soundtrack[SOUND_TRACK_BGM].channel, BASS_ATTRIB_VOL, fVol);
	}
	if (BASS_ChannelIsActive(BASS_Soundtrack[SOUND_TRACK_ONESHOT].channel))
	{
		BASS_ChannelSetAttribute(BASS_Soundtrack[SOUND_TRACK_ONESHOT].channel, BASS_ATTRIB_VOL, fVol);
	}
}

void SetVolumeFX(int vol)
{
	GlobalFXVolume = vol;
}

bool Sound_LoadSample(char *pointer, int compSize, int uncompSize, int index)	// Replaces DXCreateSampleADPCM()
{
	if (index >= SOUND_MAX_SAMPLES)
	{
		printf("Sample index is larger than max. amount of samples (%d) \n", index);
		return 0;
	}

	if (pointer == NULL || compSize <= 0)
	{
		printf("Sample size or memory address is incorrect \n", index);
		return 0;
	}

	// Load and uncompress sample to 32-bit float format
	HSAMPLE sample = BASS_SampleLoad(true, pointer, 0, compSize, 1, SOUND_SAMPLE_FLAGS);

	if (!sample)
	{
		printf("Error loading sample %d \n", index);
		return false;
	}

	// Paranoid (c) TeslaRus
	// Try to free sample before allocating new one
	Sound_FreeSample(index);

	BASS_SAMPLE info;
	BASS_SampleGetInfo(sample, &info);
	int finalLength = info.length + 44;	// uncompSize is invalid after 16->32 bit conversion

	if (info.freq != 22050 || info.chans != 1)
	{
		printf("Wrong sample parameters, must be 22050 Hz Mono \n");
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
	for (int i = 4; i < info.length; i += 4)
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
	return true;
}

long SoundEffect(int effectID, PHD_3DPOS* position, int env_flags)
{
	if (effectID >= g_Level.SoundMap.size())
		return 0;

	if (BASS_GetDevice() == -1)
		return 0;

	if (!(env_flags & SFX_ALWAYS))
	{
		// Don't play effect if effect's environment isn't the same as camera position's environment
		if ((env_flags & ENV_FLAG_WATER) != (g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER))
			return 0;
	}

	// Get actual sample index from SoundMap
	int sampleIndex = g_Level.SoundMap[effectID];

	// -1 means no such effect exists in level file.
	// We set it to -2 afterwards to prevent further debug message firings.
	if (sampleIndex == -1)
	{
		printf("Non present effect %d \n", effectID);
		g_Level.SoundMap[effectID] = -2;
		return 0;
	}
	else if (sampleIndex == -2)
		return 0;

	SAMPLE_INFO *sampleInfo = &g_Level.SoundDetails[sampleIndex];

	if (sampleInfo->number < 0)
	{
		printf("No valid samples count for effect %d", sampleIndex);
		return 0;
	}

	// Assign common sample flags.
	DWORD sampleFlags = SOUND_SAMPLE_FLAGS;

	// Effect's chance to play.
	if ((sampleInfo->randomness) && ((GetRandomControl() & 127) > sampleInfo->randomness))
		return 0;

	// Apply 3D attrib only to sound with position property
	if (position)
		sampleFlags |= BASS_SAMPLE_3D;

	// Select behaviour based on effect playback type (bytes 0-1 of flags field)
	int playType = sampleInfo->flags & 3;
	switch (playType)
	{
	case SOUND_NORMAL:
		break;

	case SOUND_WAIT:
		if (Sound_EffectIsPlaying(effectID, position) != -1) return 0; // Don't play until stopped
		break;

	case SOUND_RESTART:
		Sound_FreeSlot(Sound_EffectIsPlaying(effectID, position), SOUND_XFADETIME_CUTSOUND); // Stop existing and continue
		break;

	case SOUND_LOOPED:
		if (Sound_UpdateEffectPosition(Sound_EffectIsPlaying(effectID, position), position))
			return 0;
		sampleFlags |= BASS_SAMPLE_LOOP;
		break;
	}

	float radius   = (float)(sampleInfo->radius) * 1024.0f;
	float distance = Sound_DistanceToListener(position);

	// Don't play sound if it's too far from listener's position.
	if (distance > radius)
		return 0;

	// Set and randomize volume (if needed)
	float gain = static_cast<float>(sampleInfo->volume) / 255.0f;
	if ((sampleInfo->flags & SOUND_FLAG_RND_GAIN))
		gain -= (static_cast<float>(GetRandomControl()) / static_cast<float>(RAND_MAX)) * SOUND_MAX_GAIN_CHANGE;

	// Set and randomize pitch (if needed)
	float pitch = 1.0f + static_cast<float>(sampleInfo->pitch) / 127.0f;
	if ((sampleInfo->flags & SOUND_FLAG_RND_PITCH))
		pitch += ((static_cast<float>(GetRandomControl()) / static_cast<float>(RAND_MAX)) - 0.5f) * SOUND_MAX_PITCH_CHANGE * 2.0f;

	// Randomly select arbitrary sample from the list, if more than one is present
	int sampleToPlay = 0;
	int numSamples = (sampleInfo->flags >> 2) & 15;
	if (numSamples == 1)
		sampleToPlay = sampleInfo->number;
	else
		sampleToPlay = sampleInfo->number + (int)((GetRandomControl() * numSamples) >> 15);

	// Get free channel to play sample
	int freeSlot = Sound_GetFreeSlot();
	if (freeSlot == -1)
	{
		printf("No free channel slot available!");
		return 0;
	}

	// Create sample's stream and reset buffer back to normal value.
	HSTREAM channel = BASS_SampleGetChannel(SamplePointer[sampleToPlay], true);

	if (Sound_CheckBASSError("Trying to create channel for sample %d", false, sampleToPlay))
		return 0;

	// Set pitch/volume settings appropriately
	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_FREQ, 22050.0f * pitch);
	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, Sound_Attenuate(gain, distance, radius));

	if (Sound_CheckBASSError("Applying pitch/gain attribs on channel %x, sample %d", false, channel, sampleToPlay))
		return 0;

	// Finally ready to play sound, assign it to sound slot.
	SoundSlot[freeSlot].state = SOUND_STATE_IDLE;
	SoundSlot[freeSlot].effectID = effectID;
	SoundSlot[freeSlot].channel = channel;
	SoundSlot[freeSlot].gain = gain;
	SoundSlot[freeSlot].origin = position ? Vector3(position->xPos, position->yPos, position->zPos) : SOUND_OMNIPRESENT_ORIGIN;
	
	// Set 3D attributes
	BASS_ChannelSet3DAttributes(channel, position ? BASS_3DMODE_NORMAL : BASS_3DMODE_OFF, SOUND_MAXVOL_RADIUS, radius, 360, 360, 0.0f);
	Sound_UpdateEffectPosition(freeSlot, position, true);

	if (Sound_CheckBASSError("Applying 3D attribs on channel %x, sound %d", false, channel, effectID))
		return 0;

	// Set looped flag, if necessary
	if(playType == SOUND_LOOPED)
		BASS_ChannelFlags(channel, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

	// Play channel
	BASS_ChannelPlay(channel, false);

	if (Sound_CheckBASSError("Queuing channel %x on sample mixer", false, freeSlot))
		return 0;

	return 1;
}

void StopSoundEffect(short effectID)
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
		if (SoundSlot[i].effectID == effectID && SoundSlot[i].channel != NULL && BASS_ChannelIsActive(SoundSlot[i].channel) == BASS_ACTIVE_PLAYING)
			Sound_FreeSlot(i, SOUND_XFADETIME_CUTSOUND);
}

void Sound_Stop()
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
		if (SoundSlot[i].channel != NULL && BASS_ChannelIsActive(SoundSlot[i].channel))
			BASS_ChannelStop(SoundSlot[i].channel);
	ZeroMemory(SoundSlot, (sizeof(SoundEffectSlot) * SOUND_MAX_CHANNELS));
}

void Sound_FreeSamples()
{
	Sound_Stop();
	for (int i = 0; i < SOUND_MAX_SAMPLES; i++)
		Sound_FreeSample(i);
}

void PlaySoundTrack(short track, short flags)
{
	S_CDPlayEx(track, flags, SOUND_TRACK_ONESHOT);
}

void S_CDPlay(std::string track, unsigned int mode)
{
	bool crossfade = false;
	DWORD crossfadeTime;
	DWORD flags = BASS_STREAM_AUTOFREE | BASS_SAMPLE_FLOAT | BASS_ASYNCFILE;

	mode = (mode >= NUM_SOUND_TRACK_TYPES) ? SOUND_TRACK_BGM : mode;

	bool channelActive = BASS_ChannelIsActive(BASS_Soundtrack[mode].channel);
	if (channelActive && BASS_Soundtrack[mode].track.compare(track) == 0)
		return;

	switch (mode)
	{
		case SOUND_TRACK_ONESHOT:
			crossfadeTime = SOUND_XFADETIME_ONESHOT;
			break;

		case SOUND_TRACK_BGM:
			crossfade = true;
			crossfadeTime = channelActive ? SOUND_XFADETIME_BGM : SOUND_XFADETIME_BGM_START;
			flags |= BASS_SAMPLE_LOOP;
			CurrentLoopedSoundTrack = track;
			break;
	}

	if(channelActive)
		BASS_ChannelSlideAttribute(BASS_Soundtrack[mode].channel, BASS_ATTRIB_VOL, -1.0f, crossfadeTime);
	
	static char fullTrackName[1024];
	char const* name = track.c_str();

	snprintf(fullTrackName, sizeof(fullTrackName), TRACKS_PREFIX, name, "ogg");
	if (!std::filesystem::exists(fullTrackName))
	{
		snprintf(fullTrackName, sizeof(fullTrackName), TRACKS_PREFIX, name, "mp3");
		if (!std::filesystem::exists(fullTrackName))
		{
			snprintf(fullTrackName, sizeof(fullTrackName), TRACKS_PREFIX, name, "wav");
			if (!std::filesystem::exists(fullTrackName))
			{
				return;
			}
		}
	}

	auto stream = BASS_StreamCreateFile(false, fullTrackName, 0, 0, flags);

	if (Sound_CheckBASSError("Opening soundtrack %s", false, fullTrackName))
		return;

	float masterVolume = (float)GlobalMusicVolume / 100.0f;

	// Damp BGM track in case one-shot track is about to play.

	if (mode == SOUND_TRACK_ONESHOT)
	{
		if (BASS_ChannelIsActive(BASS_Soundtrack[SOUND_TRACK_BGM].channel))
			BASS_ChannelSlideAttribute(BASS_Soundtrack[SOUND_TRACK_BGM].channel, BASS_ATTRIB_VOL, masterVolume * SOUND_BGM_DAMP_COEFFICIENT, SOUND_XFADETIME_BGM_START);
		BASS_ChannelSetSync(stream, BASS_SYNC_FREE | BASS_SYNC_ONETIME | BASS_SYNC_MIXTIME, 0, Sound_FinishOneshotTrack, NULL);
	}

	// BGM tracks are crossfaded, and additionally shuffled a bit to make things more natural.
	// Think everybody are fed up with same start-up sounds of Caves ambience...

	if (crossfade)
	{		
		// Crossfade...
		BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, 0.0f);
		BASS_ChannelSlideAttribute(stream, BASS_ATTRIB_VOL, masterVolume, crossfadeTime);

		// Shuffle...
		QWORD newPos = BASS_ChannelGetLength(stream, BASS_POS_BYTE) * (static_cast<float>(GetRandomControl()) / static_cast<float>(RAND_MAX));
		BASS_ChannelSetPosition(stream, newPos, BASS_POS_BYTE);
	}
	else
		BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, masterVolume);

	BASS_ChannelPlay(stream, false);

	if (Sound_CheckBASSError("Playing soundtrack %s", false, fullTrackName))
		return;

	BASS_Soundtrack[mode].channel = stream;
	BASS_Soundtrack[mode].track = track;
}

void S_CDPlayEx(std::string track, DWORD mask, DWORD unknown)
{
	// Check and modify soundtrack map mask, if needed.
	// If existing mask is unmodified (same activation mask setup), track won't play.
	if (!SoundTracks[track].looped)
	{
		byte filteredMask = (mask >> 8) & 0x3F;
		if ((SoundTracks[track].Mask & filteredMask) == filteredMask)
			return;	// Mask is the same, don't play it.

		SoundTracks[track].Mask |= filteredMask;
	}

	S_CDPlay(track, SoundTracks[track].looped);
}

// Legacy!
void S_CDPlay(int index, unsigned int mode)
{
	std::pair<const std::string, AudioTrack>& track = *std::next(SoundTracks.begin(), index);
	S_CDPlay(track.first, mode);
}

void S_CDPlayEx(int index, DWORD mask, DWORD unknown)
{
	std::pair<const std::string, AudioTrack>& track = *std::next(SoundTracks.begin(), index);
	S_CDPlayEx(track.first, mask, unknown);
}

void StopSoundTracks()
{
	// Do quick fadeouts.
	BASS_ChannelSlideAttribute(BASS_Soundtrack[SOUND_TRACK_ONESHOT].channel, BASS_ATTRIB_VOL | BASS_SLIDE_LOG, -1.0f, SOUND_XFADETIME_ONESHOT);
	BASS_ChannelSlideAttribute(BASS_Soundtrack[SOUND_TRACK_BGM].channel, BASS_ATTRIB_VOL | BASS_SLIDE_LOG, -1.0f, SOUND_XFADETIME_ONESHOT);

	BASS_Soundtrack[SOUND_TRACK_ONESHOT].track = "";
	BASS_Soundtrack[SOUND_TRACK_ONESHOT].channel = NULL;
	BASS_Soundtrack[SOUND_TRACK_BGM].track = "";
	BASS_Soundtrack[SOUND_TRACK_BGM].channel = NULL;
}

static void CALLBACK Sound_FinishOneshotTrack(HSYNC handle, DWORD channel, DWORD data, void* userData)
{
	if (BASS_ChannelIsActive(BASS_Soundtrack[SOUND_TRACK_BGM].channel))
		BASS_ChannelSlideAttribute(BASS_Soundtrack[SOUND_TRACK_BGM].channel, BASS_ATTRIB_VOL, (float)GlobalMusicVolume / 100.0f, SOUND_XFADETIME_BGM_START);
	
	BASS_Soundtrack[SOUND_TRACK_ONESHOT].track = "";
	BASS_Soundtrack[SOUND_TRACK_ONESHOT].channel = NULL;
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
		if (SoundSlot[i].channel == NULL || !BASS_ChannelIsActive(SoundSlot[i].channel))
			return i;
	}

	// No free slots, hijack now.

	float minDistance = 0;
	int farSlot = -1;

	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		float distance = Vector3(SoundSlot[i].origin - Vector3(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z)).Length();
		if (distance > minDistance)
		{
			minDistance = distance;
			farSlot = i;
		}
	}

	printf("Hijacking sound effect slot %d  \n", farSlot);
	Sound_FreeSlot(farSlot, SOUND_XFADETIME_HIJACKSOUND);
	return farSlot;
}

// Returns slot ID in which effect is playing, if found. If not found, returns -1.
// We use origin position as a reference, because in original TRs it's not possible to clearly
// identify what's the source of the producing effect.

int Sound_EffectIsPlaying(int effectID, PHD_3DPOS *position)
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if (SoundSlot[i].effectID == effectID)
		{
			if (SoundSlot[i].channel == NULL)	// Free channel
				continue;

			if (BASS_ChannelIsActive(SoundSlot[i].channel))
			{
				// Only check position on 3D samples. 2D samples stop immediately.

				BASS_CHANNELINFO info;
				BASS_ChannelGetInfo(SoundSlot[i].channel, &info);
				if (!(info.flags & BASS_SAMPLE_3D) || !position)
					return i;

				// Check if effect origin is equal OR in nearest possible hearing range.

				Vector3 origin = Vector3(position->xPos, position->yPos, position->zPos);
				if (SoundSlot[i].origin == origin ||
					Vector3(SoundSlot[i].origin - origin).Length() < SOUND_MAXVOL_RADIUS)
					return i;
			}
			else
				SoundSlot[i].channel = NULL; // WTF, let's clean this up
		}
	}
	return -1;
}

// Gets the distance to the source.

float Sound_DistanceToListener(PHD_3DPOS *position)
{
	if (!position) return 0.0f;	// Assume sound is 2D menu sound
	return Sound_DistanceToListener(Vector3(position->xPos, position->yPos, position->zPos));
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

	if (fadeout > 0)
		BASS_ChannelSlideAttribute(SoundSlot[index].channel, BASS_ATTRIB_VOL, -1.0f, fadeout);
	else
		BASS_ChannelStop(SoundSlot[index].channel);

	SoundSlot[index].channel = NULL;
	SoundSlot[index].state = SOUND_STATE_IDLE;
	SoundSlot[index].effectID = -1;
}

// Update sound position in a level.

bool Sound_UpdateEffectPosition(int index, PHD_3DPOS *position, bool force)
{
	if (index > SOUND_MAX_CHANNELS || index < 0)
		return false;

	if (position)
	{
		BASS_CHANNELINFO info;
		BASS_ChannelGetInfo(SoundSlot[index].channel, &info);
		if ((info.flags & BASS_SAMPLE_3D) && 
			!(!force && SoundSlot[index].origin.x == position->xPos &&
						SoundSlot[index].origin.y == position->yPos &&
						SoundSlot[index].origin.z == position->zPos))
		{
			auto pos = BASS_3DVECTOR(position->xPos, position->yPos, position->zPos);
			auto rot = BASS_3DVECTOR(position->xRot, position->yRot, position->zRot);
			BASS_ChannelSet3DPosition(SoundSlot[index].channel, &pos, &rot, NULL);
			BASS_Apply3D();
		}
	}

	// Reset activity flag, important for looped samples
	if (BASS_ChannelIsActive(SoundSlot[index].channel))
		SoundSlot[index].state = SOUND_STATE_IDLE;

	return true;
}

// Update whole sound scene in a level.
// Must be called every frame to update camera position and 3D parameters.

void Sound_UpdateScene()
{
	// Apply environmental effects

	static int currentReverb = -1;

	if (currentReverb == -1 || g_Level.Rooms[Camera.pos.roomNumber].reverbType != currentReverb)
	{
		currentReverb = g_Level.Rooms[Camera.pos.roomNumber].reverbType;
		if (currentReverb < NUM_REVERB_TYPES)
			BASS_FXSetParameters(BASS_FXHandler[SOUND_FILTER_REVERB], &BASS_ReverbTypes[currentReverb]);
	}

	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if ((SoundSlot[i].channel != NULL) && (BASS_ChannelIsActive(SoundSlot[i].channel) == BASS_ACTIVE_PLAYING))
		{
			SAMPLE_INFO *sampleInfo = &g_Level.SoundDetails[g_Level.SoundMap[SoundSlot[i].effectID]];

			// Stop and clean up sounds which were in ending state in previous frame.
			// In case sound is looping, make it ending unless they are re-fired in next frame.

			if (SoundSlot[i].state == SOUND_STATE_ENDING)
			{
				SoundSlot[i].state = SOUND_STATE_ENDED;
				Sound_FreeSlot(i, SOUND_XFADETIME_CUTSOUND);
				continue;
			}
			else if (sampleInfo->flags & 3 == SOUND_LOOPED)
				SoundSlot[i].state = SOUND_STATE_ENDING;

			// Calculate attenuation and clean up sounds which are out of listener range (only for 3D sounds).

			if (SoundSlot[i].origin != SOUND_OMNIPRESENT_ORIGIN)
			{
				float radius = (float)(sampleInfo->radius) * 1024.0f;
				float distance = Sound_DistanceToListener(SoundSlot[i].origin);

				if (distance > radius)
				{
					Sound_FreeSlot(i);
					continue;
				}
				else
					BASS_ChannelSetAttribute(SoundSlot[i].channel, BASS_ATTRIB_VOL, Sound_Attenuate(SoundSlot[i].gain, distance, radius));
			}
		}
	}

	// Apply current listener position.

	Vector3 at = Vector3(Camera.target.x, Camera.target.y, Camera.target.z) -
		Vector3(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z);
	at.Normalize();
	auto mikePos = BASS_3DVECTOR( Camera.mikePos.x,		// Pos
		Camera.mikePos.y,
		Camera.mikePos.z);
	auto laraVel = BASS_3DVECTOR(Lara.currentXvel,		// Vel
		Lara.currentYvel,
		Lara.currentZvel);
	auto atVec = BASS_3DVECTOR(at.x, at.y, at.z);			// At
	auto upVec = BASS_3DVECTOR(0.0f, 1.0f, 0.0f);		// Up
	BASS_Set3DPosition(&mikePos,
					   &laraVel,
					   &atVec,
					   &upVec);
	BASS_Apply3D();
}

// Initialize BASS engine and also prepare all sound data.
// Called once on engine start-up.

void Sound_Init()
{
	BASS_Init(g_Configuration.SoundDevice, 44100, BASS_DEVICE_3D, WindowsHandle, NULL);
	if (Sound_CheckBASSError("Initializing BASS sound device", true))
		return;

	// Initialize BASS_FX plugin
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

	// Initialize channels and tracks array
	ZeroMemory(BASS_Soundtrack, (sizeof(HSTREAM) * NUM_SOUND_TRACK_TYPES));
	ZeroMemory(SoundSlot, (sizeof(SoundEffectSlot) * SOUND_MAX_CHANNELS));

	// Attach reverb effect to 3D channel
 	BASS_FXHandler[SOUND_FILTER_REVERB] = BASS_ChannelSetFX(BASS_3D_Mixdown, BASS_FX_BFX_FREEVERB, 0);
	BASS_FXSetParameters(BASS_FXHandler[SOUND_FILTER_REVERB], &BASS_ReverbTypes[RVB_OUTSIDE]);

	if (Sound_CheckBASSError("Attaching environmental FX", true))
		return;

	// Apply slight compression to 3D channel
	BASS_FXHandler[SOUND_FILTER_COMPRESSOR] = BASS_ChannelSetFX(BASS_3D_Mixdown, BASS_FX_BFX_COMPRESSOR2, 1);
	auto comp = BASS_BFX_COMPRESSOR2{ 4.0f, -18.0f, 1.5f, 10.0f, 100.0f, -1 };
	BASS_FXSetParameters(BASS_FXHandler[SOUND_FILTER_COMPRESSOR], &comp);

	if (Sound_CheckBASSError("Attaching compressor", true))
		return;

	return;
}

// Stop all sounds and streams, if any, unplug all channels from the mixer and unload BASS engine.
// Must be called on engine quit.

void Sound_DeInit()
{
	BASS_Free();
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
	SoundEffect(SFX_TR4_LARA_NO, NULL, SFX_ALWAYS);
}

int GetShatterSound(int shatterID)
{
	auto fxID = StaticObjects[shatterID].shatterSound;
	if (fxID != -1 && fxID < NUM_SFX)
		return fxID;

	if (shatterID < 3)
		return SFX_TR5_SMASH_WOOD;
	else
		return SFX_TR5_SMASH_GLASS;
}

void PlaySoundSources()
{
	for (size_t i = 0; i < g_Level.SoundSources.size(); i++)
	{
		SOUND_SOURCE_INFO* sound = &g_Level.SoundSources[i];

		short t = sound->flags & 31;
		short group = t & 1;
		group += t & 2;
		group += ((t >> 2) & 1) * 3;
		group += ((t >> 3) & 1) * 4;
		group += ((t >> 4) & 1) * 5;

		if (!FlipStats[group] && (sound->flags & 128) == 0)
			continue;
		else if (FlipStats[group] && (sound->flags & 128) == 0)
			continue;

		SoundEffect(sound->soundId, (PHD_3DPOS*)&sound->x, 0);
	}
}