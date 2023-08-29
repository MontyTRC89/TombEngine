#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Sounds.h"

#include "Sound/sound.h"
#include "Specific/level.h"

namespace TEN::Entities::Generic
{
	PushableSoundData GetPushableSfxData(const MaterialType material)
	{
		static const PushableSoundData SOUND_DATA_DEFAULT =
		{
			SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL
		};

		static const std::unordered_map<MaterialType, PushableSoundData> SOUND_DATA_MAP =
		{
			{ MaterialType::Mud, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_MUD, SFX_TEN_PUSHABLES_STOP_MUD, SFX_TEN_PUSHABLES_COLLIDE_MUD } },
			{ MaterialType::Snow, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_SNOW, SFX_TEN_PUSHABLES_STOP_SNOW, SFX_TEN_PUSHABLES_COLLIDE_SNOW } },
			{ MaterialType::Sand, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_SAND, SFX_TEN_PUSHABLES_STOP_SAND, SFX_TEN_PUSHABLES_COLLIDE_SAND } },
			{ MaterialType::Gravel, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_GRAVEL, SFX_TEN_PUSHABLES_STOP_GRAVEL, SFX_TEN_PUSHABLES_COLLIDE_GRAVEL } },
			{ MaterialType::Ice, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_ICE, SFX_TEN_PUSHABLES_STOP_ICE, SFX_TEN_PUSHABLES_COLLIDE_ICE } },
			{ MaterialType::Water, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_WATER, SFX_TEN_PUSHABLES_STOP_WATER, SFX_TEN_PUSHABLES_COLLIDE_WATER } },
			{ MaterialType::Stone, PushableSoundData{ SFX_TEN_PUSHABLES_STOP_MOVE_STONE, SFX_TEN_PUSHABLES_STOP_STONE, SFX_TEN_PUSHABLES_COLLIDE_STONE } },
			{ MaterialType::Wood, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_WOOD, SFX_TEN_PUSHABLES_STOP_WOOD, SFX_TEN_PUSHABLES_COLLIDE_WOOD } },
			{ MaterialType::Metal, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_METAL, SFX_TEN_PUSHABLES_STOP_METAL, SFX_TEN_PUSHABLES_COLLIDE_METAL } },
			{ MaterialType::Marble, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_MARBLE, SFX_TEN_PUSHABLES_STOP_MARBLE, SFX_TEN_PUSHABLES_COLLIDE_MARBLE } },
			{ MaterialType::Grass, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_GRASS, SFX_TEN_PUSHABLES_STOP_GRASS, SFX_TEN_PUSHABLES_COLLIDE_GRASS } },
			{ MaterialType::Concrete, PushableSoundData{ SFX_TEN_PUSHABLES_MOVE_CONCRETE, SFX_TEN_PUSHABLES_STOP_CONCRETE, SFX_TEN_PUSHABLES_COLLIDE_CONCRETE } },
			{ MaterialType::OldWood, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::OldMetal, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom1, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom2, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom3, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom4, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom5, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom6, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom7, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } },
			{ MaterialType::Custom8, PushableSoundData{ SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL } }
		};

		auto it = SOUND_DATA_MAP.find(material);
		//return ((it != SOUND_DATA_MAP.end()) ? it->second : SOUND_DATA_DEFAULT);
		return SOUND_DATA_DEFAULT;
	}

	int GetPushableSfx(PushableSoundType soundType, const Vector3i& pos, const short roomNumber)
	{
		auto pointColl = GetCollision(pos);
		auto material = pointColl.BottomBlock->Material;

		switch (soundType)
		{
		case PushableSoundType::Loop:
			return GetPushableSfxData(material).LoopSfx;

		case PushableSoundType::Stop:
			return GetPushableSfxData(material).StopSfx;

		case PushableSoundType::Fall:
			return GetPushableSfxData(material).LandSfx;
		
		default:
			TENLog("Missing pushable sfx.", LogLevel::Error, LogConfig::All, true);
			return 0;
		}
	}

	void PushablesManageSounds(int itemNumber, PushableInfo& pushable)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		
		if (pushable.CurrentSoundState == PushableSoundState::Moving)
		{
			SoundEffect(GetPushableSfx(Loop, pushableItem.Pose.Position, pushableItem.RoomNumber), &pushableItem.Pose, SoundEnvironment::Always);
		}
		else if (pushable.CurrentSoundState == PushableSoundState::Stopping)
		{
			pushable.CurrentSoundState = PushableSoundState::None;
			SoundEffect(GetPushableSfx(Stop, pushableItem.Pose.Position, pushableItem.RoomNumber), &pushableItem.Pose, SoundEnvironment::Always);
		}
		else if (pushable.CurrentSoundState == PushableSoundState::Falling)
		{
			pushable.CurrentSoundState = PushableSoundState::None;
			SoundEffect(GetPushableSfx(Fall, pushableItem.Pose.Position, pushableItem.RoomNumber), &pushableItem.Pose, SoundEnvironment::Always);
		}
		else if (pushable.CurrentSoundState == PushableSoundState::WaterRipples)
		{
			pushable.CurrentSoundState = PushableSoundState::None;
			SoundEffect(SFX_TR4_LARA_WADE, &pushableItem.Pose, SoundEnvironment::Always);
		}
	}
}
