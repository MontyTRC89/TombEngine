#pragma once
#include "Game/animation.h"
#include "Game/control/event.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/room.h"
#include "Sound/sound.h"
#include "Specific/IO/ChunkId.h"
#include "Specific/IO/ChunkReader.h"
#include "Specific/IO/LEB128.h"
#include "Specific/IO/Streams.h"
#include "Specific/LevelCameraInfo.h"
#include "Specific/newtypes.h"

using namespace TEN::Control::Volumes;

struct ChunkId;
struct LEB128;
struct SampleInfo;
struct SinkInfo;
struct BOX_INFO;
struct OVERLAP;

struct TEXTURE
{
	int width;
	int height;
	std::vector<byte> colorMapData;
	std::vector<byte> normalMapData;
};

struct ANIMATED_TEXTURES_FRAME
{
	float x1;
	float y1;
	float x2;
	float y2;
	float x3;
	float y3;
	float x4;
	float y4;
};

struct ANIMATED_TEXTURES_SEQUENCE
{
	int atlas;
	int Fps;
	int numFrames;
	std::vector<ANIMATED_TEXTURES_FRAME> frames;
};

struct AI_OBJECT
{
	GAME_OBJECT_ID objectNumber;
	short roomNumber;
	Pose pos;
	short triggerFlags;
	short flags;
	int boxNumber;
	std::string Name;
};

struct SPRITE
{
	int tile;
	float x1;
	float y1;
	float x2;
	float y2;
	float x3;
	float y3;
	float x4;
	float y4;
};

struct MESH
{
	LIGHT_MODES lightMode;
	BoundingSphere sphere;
	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector3> colors;
	std::vector<Vector3> effects; // X = glow, Y = move, Z = refract
	std::vector<int> bones;
	std::vector<BUCKET> buckets;
};

// LevelData
struct LEVEL
{
	// Object data
	int					  NumItems = 0;
	std::vector<ItemInfo> Items	   = {};
	std::vector<MESH>	  Meshes   = {};
	std::vector<int>	  Bones	   = {};

	// Animation data
	std::vector<AnimData>				Anims	 = {};
	std::vector<AnimFrame>				Frames	 = {};
	std::vector<StateDispatchData>		Changes	 = {};
	std::vector<StateDispatchRangeData> Ranges	 = {};
	std::vector<short>					Commands = {};

	// Collision data
	std::vector<ROOM_INFO> Rooms	 = {};
	std::vector<short>	   FloorData = {};
	std::vector<SinkInfo>  Sinks	 = {};

	// Pathfinding data
	std::vector<BOX_INFO> Boxes	   = {};
	std::vector<OVERLAP>  Overlaps = {};
	std::vector<int>	  Zones[(int)ZoneType::MaxZone][2] = {};

	// Sound data
	std::vector<short>			 SoundMap	  = {};
	std::vector<SoundSourceInfo> SoundSources = {};
	std::vector<SampleInfo>		 SoundDetails = {};

	// Misc. data
	std::vector<LevelCameraInfo> Cameras   = {};
	std::vector<EventSet>		 GlobalEventSets = {};
	std::vector<EventSet>		 VolumeEventSets = {};
	std::vector<int>			 LoopedEventSetIndices = {};
	std::vector<AI_OBJECT>		 AIObjects = {};
	std::vector<SPRITE>			 Sprites   = {};

	// Texture data
	TEXTURE				 SkyTexture		   = {};
	std::vector<TEXTURE> RoomTextures	   = {};
	std::vector<TEXTURE> MoveablesTextures = {};
	std::vector<TEXTURE> StaticsTextures   = {};
	std::vector<TEXTURE> AnimatedTextures  = {};
	std::vector<TEXTURE> SpritesTextures   = {};
	std::vector<ANIMATED_TEXTURES_SEQUENCE> AnimatedTexturesSequences = {};
};

extern std::vector<int> MoveablesIds;
extern std::vector<int> StaticObjectsIds;
extern LEVEL g_Level;

inline std::future<bool> LevelLoadTask;

size_t ReadFileEx(void* ptr, size_t size, size_t count, FILE* stream);
FILE* FileOpen(const char* fileName);
void FileClose(FILE* ptr);
bool Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize);

bool LoadLevelFile(int levelIndex);
void FreeLevel();

void LoadTextures();
void LoadRooms();
void LoadItems();
void LoadObjects();
void LoadCameras();
void LoadSprites();
void LoadBoxes();
void LoadSamples();
void LoadSoundSources();
void LoadAnimatedTextures();
void LoadEventSets();
void LoadAIObjects();

void LoadPortal(ROOM_INFO& room);

void GetCarriedItems();
void GetAIPickups();
void BuildOutsideRoomsTable();
