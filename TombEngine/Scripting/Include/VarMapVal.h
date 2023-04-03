#pragma once

struct MESH_INFO;
struct LevelCameraInfo;
struct SinkInfo;
struct SoundSourceInfo;
struct TriggerVolume;
struct AI_OBJECT;
struct ROOM_INFO;

using VarMapVal = std::variant<
	short,
	std::reference_wrapper<MESH_INFO>,
	std::reference_wrapper<LevelCameraInfo>,
	std::reference_wrapper<SinkInfo>,
	std::reference_wrapper<SoundSourceInfo>,
	std::reference_wrapper<TriggerVolume>,
	std::reference_wrapper<AI_OBJECT>,
	std::reference_wrapper<ROOM_INFO>>;
