#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "LEB128.h"
#include "Streams.h"
#include "newtypes.h"
#include "items.h"
#include "room.h"

#define AddPtr(p, t, n) p = (t*)((char*)(p) + (ptrdiff_t)(n));
#define MESHES(slot, mesh) (Objects[slot].meshIndex + mesh)

struct ChunkId;
struct LEB128;
struct SAMPLE_INFO;
struct BOX_INFO;
struct OVERLAP;

typedef struct OBJECT_TEXTURE_VERT
{
	float x;
	float y;
};

typedef struct OBJECT_TEXTURE
{
	int attribute;
	int tileAndFlag;
	int newFlags;
	struct OBJECT_TEXTURE_VERT vertices[4];
	int destination;
};

struct TEXTURE
{
	int width;
	int height;
	std::vector<byte> colorMapData;
	std::vector<byte> normalMapData;
};

struct AIOBJECT
{
	short objectNumber;
	short roomNumber;
	int x;
	int y;
	int z;
	short triggerFlags;
	short flags;
	short yRot;
	short boxNumber;
};

struct CHANGE_STRUCT
{
	short goalAnimState;
	short numberRanges;
	short rangeIndex;
};

struct RANGE_STRUCT
{
	short startFrame;
	short endFrame;
	short linkAnimNum;
	short linkFrameNum;
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

struct MESH_VERTEX
{
	Vector3 position;
	Vector3 normal;
	Vector2 textureCoordinates;
	Vector3 color;
	int bone;
	int indexInPoly;
	int originalIndex;
};

struct MESH
{
	BoundingSphere sphere;
	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector3> colors;
	std::vector<int> bones;
	std::vector<BUCKET> buckets;
};

struct LEVEL
{
	std::vector<TEXTURE> RoomTextures;
	std::vector<TEXTURE> MoveablesTextures;
	std::vector<TEXTURE> StaticsTextures;
	std::vector<TEXTURE> SpritesTextures;
	TEXTURE MiscTextures;
	std::vector<ROOM_INFO> Rooms;
	std::vector<short> FloorData;
	std::vector<MESH> Meshes;
	std::vector<int> Bones;
	std::vector<ANIM_STRUCT> Anims;
	std::vector<CHANGE_STRUCT> Changes;
	std::vector<RANGE_STRUCT> Ranges;
	std::vector<short> Commands;
	std::vector<short> Frames;
	std::vector<OBJECT_TEXTURE> ObjectTextures;
	std::vector<ITEM_INFO> Items;
	std::vector<AIOBJECT> AIObjects;
	std::vector<SPRITE> Sprites;
	std::vector<BOX_INFO> Boxes;
	std::vector<OVERLAP> Overlaps;
	std::vector<int> Zones[5][2];
	std::vector<short> SoundMap;
	std::vector<SAMPLE_INFO> SoundDetails;
	int NumItems;
	int NumSpritesSequences;
};

extern std::vector<int> MoveablesIds;
extern std::vector<int> StaticObjectsIds;
extern char* LevelDataPtr;
extern int IsLevelLoading;
extern LEVEL g_Level;

void LoadTextures();
void LoadRooms();
int LoadItems();
void LoadObjects();
int S_LoadLevelFile(int levelIndex);
void FreeLevel();
void LoadCameras();
void InitialiseLara(int restore);
void LoadSprites();
void GetCarriedItems();
void GetAIPickups();
void LoadBoxes();
void LoadSamples();
size_t ReadFileEx(void* ptr, size_t size, size_t count, FILE* stream);
void LoadSoundEffects();
void LoadAnimatedTextures();
void LoadTextureInfos();
void LoadAIObjects();
FILE* FileOpen(const char* fileName);
void FileClose(FILE* ptr);
void Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize);

unsigned CALLBACK LoadLevel(void* data);

// New functions for loading data from TR5M footer
bool ReadLuaIds(ChunkId* chunkId, int maxSize, int arg);
bool ReadLuaTriggers(ChunkId* chunkId, int maxSize, int arg);
bool ReadNewDataChunks(ChunkId* chunkId, int maxSize, int arg);
void LoadNewData(int size);
