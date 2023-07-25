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
		int LoopSfx = 0; // Looped sound during moving.
		int StopSfx = 0; // Ending sound after movement.
		int LandSfx = 0; // Landing sound following drop.
	};

	extern const std::unordered_map<MaterialType, PushableSoundData> SOUND_DATA_MAP;
	extern const PushableSoundData SOUND_DATA_DEFAULT;

	PushableSoundData GetPushableSfxData(const MaterialType material);
	int	 GetPushableSfx(PushableSoundType soundType, const Vector3i& pos, const short roomNumber);
	void PushablesManageSounds(int itemNumber, PushableInfo& pushable);
}
