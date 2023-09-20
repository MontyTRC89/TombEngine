#pragma once
#include "Sound/sound.h"

namespace TEN::Entities::Generic
{
	enum PushableSoundType
	{
		Loop,
		Stop,
		Fall
	};

	struct PushableSoundData
	{
		int LoopSfx = 0; // Looped sound while during movement.
		int StopSfx = 0; // Ending sound after movement.
		int LandSfx = 0; // Landing sound following drop.
	};

	extern const std::unordered_map<MaterialType, PushableSoundData> SOUND_DATA_MAP;
	extern const PushableSoundData									 SOUND_DATA_DEFAULT;

	PushableSoundData GetPushableSoundData(MaterialType material);
	int				  GetPushableSound(PushableSoundType soundType, const Vector3i& pos, const short roomNumber);

	void HandlePushableSounds(int itemNumber, PushableInfo& pushable);
}
