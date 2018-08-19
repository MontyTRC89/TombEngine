#include "sound.h"

HSTREAM BASS_Soundtrack[2];	// A pair of streams used to play one-shot music and BGM

HSTREAM BASS_Mixer_Samples;	// Mixer stream used to mixdown samples
HSTREAM BASS_Mixer_Streams;	// Mixer stream used to mixdown music and BGM
HSTREAM BASS_Mixer_Master;	// Master channel

HFX BASS_FXHandler;
BASS_BFX_FREEVERB BASS_ReverbTypes[NUM_REVERB_TYPES]; // Reverb presets

// Injected function to replace DXCreateSampleADPCM()

bool __cdecl Sound_MakeSample(char *buffer, __int32 compSize, __int32 uncompSize, __int32 currentIndex)
{
	DB_Log(8, "DXCreateSampleADPCM - DLL");

	if (BASS_GetDevice() != -1)
	{
		// In future, if new sound system or file format is introduced, we can add switch case here.
		printf("Loading sample %d  \n", currentIndex);
		return Sound_LoadLegacySample(currentIndex, buffer, compSize, uncompSize);
	}
	return false;
}

long __cdecl SoundEffect(__int32 effectID, PHD_3DPOS* position, __int32 env_flags)
{
	if (effectID >= SOUND_LEGACY_SOUNDMAP_SIZE)
		return 0;

	if (BASS_GetDevice() == -1)
		return 0;

	if (!(env_flags & ENV_FLAG_SFX_ALWAYS))
	{
		// Don't play effect if effect's environment isn't the same as camera position's environment
		// @FIXME: Later redo with proper EQ damping to be able to subtly hear sounds from underwater!
		//if ((env_flags & ENV_FLAG_WATER) != Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER)
		//return 0;
	}

	// Get actual sample index from SoundMap
	int sampleIndex = SampleLUT[effectID];

	// -1 means no such effect exists in level file.
	// We set it to -2 afterwards to prevent further debug message firings.
	if (sampleIndex == -1)
	{
		printf("Non present effect %d \n", effectID);
		SampleLUT[effectID] = -2;
		return 0;
	}
	else if (sampleIndex == -2)
		return 0;

	SAMPLE_INFO *sampleInfo = &SampleInfo[sampleIndex];

	if (sampleInfo->number < 0)
	{
		printf("No valid samples count for effect %d", sampleIndex);
		return 0;
	}

	// Assign all common sample flags for BASS stream.
	DWORD sampleFlags = BASS_SAMPLE_MONO | BASS_SAMPLE_FLOAT | BASS_SAMPLE_3D | BASS_ASYNCFILE | BASS_STREAM_DECODE;

	// Effect's chance to play.
	if ((sampleInfo->randomness) && ((GetRandomControl() & 127) > sampleInfo->randomness))
		return 0;

	int radius = sampleInfo->radius << 10;

	auto truePosition = position ? D3DXVECTOR3(position->xPos, position->yPos, position->zPos) : SOUND_OMNIPRESENT_ORIGIN;

	// Select behaviour based on effect playback type (bytes 0-1 of flags field)
	switch (sampleInfo->flags & 3)
	{
	case SOUND_NORMAL:
		break;

	case SOUND_WAIT:
		if (Sound_EffectIsPlaying(effectID, truePosition) != -1) return 0; // Don't play until stopped
		break;

	case SOUND_RESTART:
		Sound_FreeSlot(Sound_EffectIsPlaying(effectID, truePosition));	// Stop existing and continue
		break;

	case SOUND_LOOPED:
		sampleFlags |= BASS_SAMPLE_LOOP;
		if (Sound_UpdateEffectPosition(Sound_EffectIsPlaying(effectID, truePosition), truePosition))
			return 0;
		break;
	}

	// Don't play sound if it's too far from listener's position.
	if (!Sound_IsInRange(truePosition, radius))
		return 0;

	// Set and randomize volume (if needed)
	float volume = static_cast<float>(sampleInfo->volume) / 255.0f;
	if ((sampleInfo->flags & SOUND_FLAG_RND_GAIN))
		volume -= (static_cast<float>(GetRandomControl()) / static_cast<float>(RAND_MAX)) * SOUND_MAX_GAIN_CHANGE;

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

	// Get to the buffer of the sample
	char* samplePtr = SamplePointer[sampleToPlay];
	if (samplePtr == NULL)
	{
		printf("No buffer address found for sample %d \n", sampleToPlay);
		return 0;
	}

	// We need to set stream buffer to minimum before assigning channel
	// It's okay to use such small buffer, because whole processing happens in RAM.
	BASS_SetConfig(BASS_CONFIG_BUFFER, 10);

	// Create sample's stream and reset buffer back to normal value.
	HSTREAM channel = BASS_StreamCreateFile(true, samplePtr, 0, *(DWORD*)(samplePtr + 4) + 8, sampleFlags);
	BASS_SetConfig(BASS_CONFIG_BUFFER, 300);

	if (Sound_CheckBASSError("Trying to create channel for sample %d", sampleToPlay))
		return 0;

	// Assign stream free callback to clean-up corresponding SoundSlot
	BASS_ChannelSetSync(channel, BASS_SYNC_FREE | BASS_SYNC_ONETIME | BASS_SYNC_MIXTIME, 0, Sound_ClearSoundSlot, (int*)freeSlot);

	// Set pitch/volume settings appropriately
	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_FREQ, 22050.0f * pitch);
	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);

	// Set 3D attributes
	Sound_UpdateEffectPosition(channel, truePosition);
	BASS_ChannelSet3DAttributes(channel, BASS_3DMODE_NORMAL, SOUND_MAXVOL_RADIUS, radius, 360.0f, 360.0f, 1.0f);
	BASS_Apply3D();

	if (Sound_CheckBASSError("Applying 3D attribs on channel %x", channel))
		return 0;

	// Directly feed sample stream to sample mixer channel. Playback starts automatically.
	BASS_Mixer_StreamAddChannel(BASS_Mixer_Samples, channel, BASS_STREAM_AUTOFREE);

	if (Sound_CheckBASSError("Queuing channel %x on sample mixer", freeSlot))
		return 0;

	// Finally ready to play sound, assign it to sound slot.
	SoundSlot[freeSlot].effectID = effectID;
	SoundSlot[freeSlot].origin = truePosition;
	SoundSlot[freeSlot].channel = channel;

	return 1;
}

void __cdecl StopSoundEffect(__int16 effectID)
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
		if (SoundSlot[i].effectID == effectID && SoundSlot[i].channel != 0 && BASS_ChannelIsActive(SoundSlot[i].channel))
		{
			BASS_ChannelStop(SoundSlot[i].channel);
			SoundSlot[i].channel = NULL;
			SoundSlot[i].effectID = -1;
		}
}

void __cdecl SOUND_Stop()
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
		if (SoundSlot[i].channel != 0 && BASS_ChannelIsActive(SoundSlot[i].channel))
			BASS_ChannelStop(SoundSlot[i].channel);
	ZeroMemory(SoundSlot, (sizeof(SoundEffectSlot) * SOUND_MAX_CHANNELS));
}

void __cdecl Sound_FreeSamples()
{
	SOUND_Stop();
	for (int i = 0; i < SOUND_MAX_SAMPLES; i++)
		Sound_FreeSample(i);
}

bool Sound_LoadLegacySample(__int32 index, char *pointer, __int32 compSize, __int32 uncompSize)
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
	HSAMPLE sample = BASS_SampleLoad(true, pointer, 0, compSize, 1, BASS_SAMPLE_MONO | BASS_SAMPLE_FLOAT);

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
	char* samplePtr = new char[finalLength];
	SamplePointer[index] = samplePtr;
	ZeroMemory(samplePtr, finalLength);
	memcpy(samplePtr, "RIFF\0\0\0\0WAVEfmt \20\0\0\0", 20);
	memcpy(samplePtr + 36, "data\0\0\0\0", 8);

	WAVEFORMATEX *wf = (WAVEFORMATEX*)(samplePtr + 20);

	wf->wFormatTag = 3;
	wf->nChannels = info.chans;
	wf->wBitsPerSample = 32;
	wf->nSamplesPerSec = info.freq;
	wf->nBlockAlign = wf->nChannels * wf->wBitsPerSample / 8;
	wf->nAvgBytesPerSec = wf->nSamplesPerSec * wf->nBlockAlign;

	// Copy raw PCM data from temporary sample buffer to actual buffer which will be used by engine.
	BASS_SampleGetData(sample, samplePtr + 44);
	BASS_SampleFree(sample);

	// Cut off trailing silence from samples to prevent gaps in looped playback
	int cleanLength = info.length;
	for (int i = 4; i < info.length; i += 4)
	{
		float *currentSample = reinterpret_cast<float*>(samplePtr + finalLength - i);
		if (*currentSample > 4.9e-04f || *currentSample < -4.9e-04f)
		{
			int alignment = i % wf->nBlockAlign;
			cleanLength -= (i - alignment);
			break;
		}
	}

	// Put data size to header
	*(DWORD*)(samplePtr + 4) = cleanLength + 44 - 8;
	*(DWORD*)(samplePtr + 40) = cleanLength;

	printf("Sample loaded into buffer, size %d \n", info.length);
	return true;
}

void Sound_FreeSample(__int32 index)
{
	if (SamplePointer[index] != NULL)
	{
		delete(SamplePointer[index]);
		SamplePointer[index] = NULL;
	}
}

// Get first free (non-playing) sound slot.
// If no free slots found, now try to hijack slot which is as far from listener as possible

int Sound_GetFreeSlot()
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if ((SoundSlot[i].channel == NULL) ||
			!BASS_ChannelIsActive(SoundSlot[i].channel))
			return i;
	}

	// No free slots, hijack now.

	float minDistance = 0;
	int farSlot = -1;

	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		float distance = D3DXVec3Length(&D3DXVECTOR3(SoundSlot[i].origin - D3DXVECTOR3(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z)));

		if (distance > minDistance)
		{
			minDistance = distance;
			farSlot = i;
		}
	}

	printf("Hijacked sound effect slot %d", farSlot);
	return farSlot;
}

// Returns slot ID in which effect is playing, if found. If not found, returns -1.
// We use origin position as a reference, because in original TRs it's not possible to clearly
// identify what's the source of the producing effect.

int Sound_EffectIsPlaying(int effectID, D3DXVECTOR3 origin = SOUND_OMNIPRESENT_ORIGIN)
{
	for (int i = 0; i < SOUND_MAX_CHANNELS; i++)
	{
		if (SoundSlot[i].effectID == effectID)
		{
			if (SoundSlot[i].channel == NULL)	// Free channel
				continue;

			if (BASS_ChannelIsActive(SoundSlot[i].channel))
			{
				// Check if effect origin is equal OR in nearest possible hearing range.

				if (SoundSlot[i].origin == origin ||
					D3DXVec3Length(&D3DXVECTOR3(SoundSlot[i].origin - origin)) < SOUND_MAXVOL_RADIUS)
					return i;
			}
			else
				SoundSlot[i].channel = NULL; // WTF, let's clean this up
		}
	}

	return -1;
}

// Checks if position is in hearing range of listener.

bool Sound_IsInRange(D3DXVECTOR3 position, float range)
{
	auto distance = D3DXVec3Length(&D3DXVECTOR3(D3DXVECTOR3(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z) - position));
	return (distance <= range);
}

// Stop and free desired sound slot.

void Sound_FreeSlot(int index)
{
	if (index > SOUND_MAX_CHANNELS || index < 0)
		return;

	BASS_ChannelStop(SoundSlot[index].channel);
	BASS_StreamFree(SoundSlot[index].channel);
	SoundSlot[index].channel = NULL;
}

// Update sound position in a level.

bool Sound_UpdateEffectPosition(int index, D3DXVECTOR3 origin)
{
	if (index > SOUND_MAX_CHANNELS || index < 0)
		return false;

	// Omnipresent samples update their origin to current camera position.
	if(origin == SOUND_OMNIPRESENT_ORIGIN)
		BASS_ChannelSet3DPosition(SoundSlot[index].channel, &BASS_3DVECTOR(Camera.mikePos.x, Camera.mikePos.y, Camera.mikePos.z), NULL, NULL);
	else
		BASS_ChannelSet3DPosition(SoundSlot[index].channel, &BASS_3DVECTOR(origin.x, origin.y, origin.z), NULL, NULL);

	return true;
}

// Update whole sound scene in a level.
// Must be called every frame to update camera position and 3D parameters.

void Sound_UpdateScene()
{
	static int currentReverb = -1;

	if (currentReverb == -1 || Rooms[Camera.pos.roomNumber].reverbType != currentReverb)
	{
		currentReverb = Rooms[Camera.pos.roomNumber].reverbType;
		if (currentReverb < NUM_REVERB_TYPES)
			BASS_FXSetParameters(BASS_FXHandler, &BASS_ReverbTypes[currentReverb]);
	}

	BASS_Set3DPosition(&BASS_3DVECTOR(Camera.pos.x, Camera.pos.y, Camera.pos.z), NULL, NULL, NULL);
	BASS_Apply3D();
}

// Initialize BASS engine and also prepare all sound data.
// Called once on engine start-up.

void Sound_Init()
{
	// Clean up channels array 
	ZeroMemory(SoundSlot, (sizeof(SoundEffectSlot) * SOUND_MAX_CHANNELS));
	BASS_Init(-1, 44100, BASS_DEVICE_3D, WindowsHandle, NULL);

	if (Sound_CheckBASSError("Initializing BASS sound device"))
		return;

	// Set minimum latency and 2 threads for updating.
	// Most of modern PCs already have multi-core CPUs, so why not parallelize updating?
	BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 2);
	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 5);

	// Create both sample and track mixer channels and make them online forever.
	// For realtime mixer channels, we need minimum buffer latency. It shouldn't affect reliability.
	BASS_SetConfig(BASS_CONFIG_BUFFER, 10);
	BASS_Mixer_Samples = BASS_Mixer_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT | BASS_MIXER_NONSTOP | BASS_STREAM_DECODE);
	BASS_Mixer_Streams = BASS_Mixer_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT | BASS_MIXER_NONSTOP | BASS_STREAM_DECODE);

	// For master mix, we need slightly more buffer size, or else stuttering happens.
	BASS_SetConfig(BASS_CONFIG_BUFFER, 40);
	BASS_Mixer_Master = BASS_Mixer_StreamCreate(44100, 2, BASS_SAMPLE_FLOAT | BASS_MIXER_NONSTOP);

	// Reset buffer back to normal value.
	BASS_SetConfig(BASS_CONFIG_BUFFER, 300);

	if (Sound_CheckBASSError("Creating mixer channels"))
		return;

	// Mixdown soundtrack and sample channels to master, and start the mixer.
	BASS_Mixer_StreamAddChannel(BASS_Mixer_Master, BASS_Mixer_Streams, BASS_STREAM_AUTOFREE);
	BASS_Mixer_StreamAddChannel(BASS_Mixer_Master, BASS_Mixer_Samples, BASS_STREAM_AUTOFREE);
	BASS_ChannelPlay(BASS_Mixer_Master, false);

	if (Sound_CheckBASSError("Starting mixdown"))
		return;

	// Set up reverb presets
	BASS_ReverbTypes[RVB_OUTSIDE].fDryMix = 1.0f;
	BASS_ReverbTypes[RVB_OUTSIDE].fWetMix = 0.03f;
	BASS_ReverbTypes[RVB_OUTSIDE].fWidth = 1.0f;
	BASS_ReverbTypes[RVB_OUTSIDE].fRoomSize = 0.55f;
	BASS_ReverbTypes[RVB_OUTSIDE].fDamp = 0.25f;
	BASS_ReverbTypes[RVB_OUTSIDE].lChannel = -1;

	BASS_ReverbTypes[RVB_SMALL_ROOM].fDryMix = 1.0f;
	BASS_ReverbTypes[RVB_SMALL_ROOM].fWetMix = 0.1f;
	BASS_ReverbTypes[RVB_SMALL_ROOM].fWidth = 1.0f;
	BASS_ReverbTypes[RVB_SMALL_ROOM].fRoomSize = 0.3f;
	BASS_ReverbTypes[RVB_SMALL_ROOM].fDamp = 0.15f;
	BASS_ReverbTypes[RVB_SMALL_ROOM].lChannel = -1;

	BASS_ReverbTypes[RVB_MEDIUM_ROOM].fDryMix = 1.0f;
	BASS_ReverbTypes[RVB_MEDIUM_ROOM].fWetMix = 0.15f;
	BASS_ReverbTypes[RVB_MEDIUM_ROOM].fWidth = 1.0f;
	BASS_ReverbTypes[RVB_MEDIUM_ROOM].fRoomSize = 0.5f;
	BASS_ReverbTypes[RVB_MEDIUM_ROOM].fDamp = 0.25f;
	BASS_ReverbTypes[RVB_MEDIUM_ROOM].lChannel = -1;

	BASS_ReverbTypes[RVB_LARGE_ROOM].fDryMix = 1.0f;
	BASS_ReverbTypes[RVB_LARGE_ROOM].fWetMix = 0.2f;
	BASS_ReverbTypes[RVB_LARGE_ROOM].fWidth = 1.0f;
	BASS_ReverbTypes[RVB_LARGE_ROOM].fRoomSize = 0.78f;
	BASS_ReverbTypes[RVB_LARGE_ROOM].fDamp = 0.2f;
	BASS_ReverbTypes[RVB_LARGE_ROOM].lChannel = -1;

	BASS_ReverbTypes[RVB_PIPE].fDryMix = 1.0f;
	BASS_ReverbTypes[RVB_PIPE].fWetMix = 0.7f;
	BASS_ReverbTypes[RVB_PIPE].fWidth = 1.0f;
	BASS_ReverbTypes[RVB_PIPE].fRoomSize = 0.8f;
	BASS_ReverbTypes[RVB_PIPE].fDamp = 0.6f;
	BASS_ReverbTypes[RVB_PIPE].lChannel = -1;

	// Test parallel streaming of two tracks
	//HSTREAM track1 = BASS_StreamCreateFile(false, "094.ogg", 0, 0, BASS_SAMPLE_LOOP | BASS_STREAM_DECODE);
	//HSTREAM track2 = BASS_StreamCreateFile(false, "108.ogg", 0, 0, BASS_SAMPLE_LOOP | BASS_STREAM_DECODE);
	//BASS_ChannelSetAttribute(track1, BASS_ATTRIB_VOL, 1.0f);
	//BASS_ChannelSetAttribute(track2, BASS_ATTRIB_VOL, 1.0f);
	//BASS_Mixer_StreamAddChannel(trackStream, track1, BASS_STREAM_AUTOFREE);
	//BASS_Mixer_StreamAddChannel(trackStream, track2, BASS_STREAM_AUTOFREE);

	// Initialize BASS_FX plugin
	BASS_FX_GetVersion();

	// Test reverb on sample channel
	BASS_FXHandler = BASS_ChannelSetFX(BASS_Mixer_Samples, BASS_FX_BFX_FREEVERB, 0);
}

// Stop all sounds and streams, if any, unplug all channels from the mixer and unload BASS engine.
// Must be called on engine quit.

void Sound_DeInit()
{
	// Unplug channels from the mixer and free them
	BASS_Mixer_ChannelRemove(BASS_Mixer_Samples);
	BASS_Mixer_ChannelRemove(BASS_Mixer_Streams);
	BASS_Mixer_ChannelRemove(BASS_Mixer_Master);
	BASS_StreamFree(BASS_Mixer_Streams);
	BASS_StreamFree(BASS_Mixer_Samples);
	BASS_StreamFree(BASS_Mixer_Master);

	BASS_Free();
}

bool Sound_CheckBASSError(char* message, ...)
{
	va_list argptr;
	static char data[4096];

	int bassError = BASS_ErrorGetCode();
	if (bassError != 0)
	{
		va_start(argptr, message);
		int32_t written = vsprintf(data, message, argptr);	// TODO: replace with debug/console/message output later...
		va_end(argptr);

		snprintf(data + written, sizeof(data) - written, ": error #%d \n", bassError);
		printf(data);
	}
	return bassError != 0;
}

// Callback function which is called when sound playback finishes.
// We need to set channel handle to 0 in case slot array will be checked for currently playing effects.

static void CALLBACK Sound_ClearSoundSlot(HSYNC handle, DWORD channel, DWORD data, void* slot)
{
	SoundSlot[(int&)slot].channel = NULL;
}

void Inject_Sound()
{
	INJECT(0x00479060, SOUND_Stop);
	INJECT(0x004790A0, SOUND_Stop);			// SOUND_Init, seems no difference from SOUND_Stop
	INJECT(0x00478FE0, StopSoundEffect);
	INJECT(0x00478570, SoundEffect);
	INJECT(0x004A3510, Sound_MakeSample);	// DXCreateSampleADPCM
	INJECT(0x004A3AA0, Sound_FreeSamples);	// ReleaseDXSoundBuffers

	//INJECT(0x004A3100, EmptySoundProc);		// DXDSCreate
	//INJECT(0x004A3190, EmptySoundProc);		// DXCreateSample
	//INJECT(0x004A3030, EmptySoundProc);		// DXSetOutputFormat
	//INJECT(0x004A2E30, EmptySoundProc);		// SetSoundOutputFormat
	//INJECT(0x004A3300, EmptySoundProc);		// StreamOpen
	//INJECT(0x004A3470, EmptySoundProc);		// StreamClose
	//INJECT(0x004931A0, EmptySoundProc);		// ACMClose
	//INJECT(0x00493490, EmptySoundProc);		// ACMStream
	//INJECT(0x00492DA0, EmptySoundProc);		// ACMInit
	//INJECT(0x00492C20, EmptySoundProc);		// SetupNotifications
	//INJECT(0x00493990, EmptySoundProc);		// StartAddress
	//INJECT(0x00492990, EmptySoundProc);		// S_CDPlay
	//INJECT(0x004929E0, EmptySoundProc);		// S_CDStop
	//INJECT(0x00492B60, EmptySoundProc);		// fnCallback
}