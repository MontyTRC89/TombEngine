#pragma once
#include "Game/animation.h"
#include "Game/control/volumetriggerer.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Sound/sound.h"
#include "Specific/IO/ChunkId.h"
#include "Specific/IO/ChunkReader.h"
#include "Specific/IO/LEB128.h"
#include "Specific/IO/Streams.h"
#include "Specific/newtypes.h"

#define AddPtr(p, t, n) p = (t*)((char*)(p) + (ptrdiff_t)(n));
#define MESHES(slot, mesh) (Objects[slot].meshIndex + mesh)

#define MAX_ZONES 6

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
	PoseData pos;
	short triggerFlags;
	short flags;
	int boxNumber;
	std::string luaName;
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

struct LEVEL
{
	std::vector<TEXTURE> RoomTextures;
	std::vector<TEXTURE> MoveablesTextures;
	std::vector<TEXTURE> StaticsTextures;
	std::vector<TEXTURE> AnimatedTextures;
	std::vector<TEXTURE> SpritesTextures;
	TEXTURE SkyTexture;
	std::vector<ROOM_INFO> Rooms;
	std::vector<short> FloorData;
	std::vector<MESH> Meshes;
	std::vector<int> Bones;
	std::vector<AnimData> Anims;
	std::vector<StateDispatchData> Changes;
	std::vector<StateDispatchRangeData> Ranges;
	std::vector<short> Commands;
	std::vector<AnimFrame> Frames;
	std::vector<ItemInfo> Items;
	std::vector<AI_OBJECT> AIObjects;
	std::vector<SPRITE> Sprites;
	std::vector<LevelCameraInfo> Cameras;
	std::vector<SinkInfo> Sinks;
	std::vector<SoundSourceInfo> SoundSources;
	std::vector<BOX_INFO> Boxes;
	std::vector<OVERLAP> Overlaps;
	std::vector<int> Zones[MAX_ZONES][2];
	std::vector<short> SoundMap;
	std::vector<SampleInfo> SoundDetails;
	std::vector<ANIMATED_TEXTURES_SEQUENCE> AnimatedTexturesSequences;
	std::vector<VolumeEventSet> EventSets;
	int NumItems;
};

extern std::vector<int> MoveablesIds;
extern std::vector<int> StaticObjectsIds;
extern bool IsLevelLoading;
extern LEVEL g_Level;

size_t ReadFileEx(void* ptr, size_t size, size_t count, FILE* stream);
FILE* FileOpen(const char* fileName);
void FileClose(FILE* ptr);
bool Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize);

int LoadLevelFile(int levelIndex);
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
void LoadAIObjects();

void GetCarriedItems();
void GetAIPickups();
void BuildOutsideRoomsTable();

unsigned _stdcall LoadLevel(void* data);
