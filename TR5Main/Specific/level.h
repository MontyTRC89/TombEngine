#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "LEB128.h"
#include "Streams.h"

#include "items.h"
#include "room.h"

#define AddPtr(p, t, n) p = (t*)((char*)(p) + (ptrdiff_t)(n));
#define MESHES(slot, mesh) Meshes[Objects[slot].meshIndex + mesh]

struct ChunkId;
struct LEB128;

typedef struct OBJECT_TEXTURE_VERT
{
	float x;
	float y;
};

typedef struct OBJECT_TEXTURE
{
	short attribute;
	short tileAndFlag;
	short newFlags;
	struct OBJECT_TEXTURE_VERT vertices[4];
};

struct tr_object_texture_vert
{
	byte Xcoordinate; // 1 if Xpixel is the low value, 255 if Xpixel is the high value in the object texture
	byte Xpixel;
	byte Ycoordinate; // 1 if Ypixel is the low value, 255 if Ypixel is the high value in the object texture
	byte Ypixel;
};

struct tr4_object_texture
{
	short Attribute;
	short TileAndFlag;
	short NewFlags;
	tr_object_texture_vert Vertices[4]; // The four corners of the texture
	int OriginalU;
	int OriginalV;
	int Width;     // Actually width-1
	int Height;    // Actually height-1
	short Padding;
};

typedef struct AIOBJECT
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

typedef struct CHANGE_STRUCT
{
	short goalAnimState;
	short numberRanges;
	short rangeIndex;
};

typedef struct RANGE_STRUCT
{
	short startFrame;
	short endFrame;
	short linkAnimNum;
	short linkFrameNum;
};

typedef struct SPRITE
{
	short tile;
	byte x;
	byte y;
	short width;
	short height;
	float left;
	float top;
	float right;
	float bottom;
};

extern byte* Texture32;
extern byte* Texture16;
extern byte* MiscTextures;
extern short* MeshData;
extern int MeshDataSize;
extern OBJECT_TEXTURE* NewObjectTextures;
extern uintptr_t hLoadLevel;
extern int NumMeshPointers;
extern int* MeshTrees;
extern int NumObjects;
extern int NumStaticObjects;
extern vector<int> MoveablesIds;
extern vector<int> StaticObjectsIds;
extern int* RawMeshPointers;
extern short* RawMeshData;
extern int NumObjectTextures;
extern char* LevelDataPtr;
extern int IsLevelLoading;
extern int NumTextureTiles;
extern int g_NumSprites;
extern int g_NumSpritesSequences;
extern short* MeshBase;
extern short** Meshes;
extern int NumItems;
extern OBJECT_TEXTURE* ObjectTextures;
extern ITEM_INFO* Items;
extern int LevelItems;
extern int NumberRooms;
extern ROOM_INFO* Rooms;
extern ANIM_STRUCT* Anims;
extern CHANGE_STRUCT* Changes;
extern RANGE_STRUCT* Ranges;
extern short* Commands;
extern int* Bones;
extern short* Frames;
extern int AnimationsCount;
extern short* FloorData;
extern int nAIObjects;
extern AIOBJECT* AIObjects;
extern SPRITE* Sprites;

void LoadTextures();
void LoadRooms();
int LoadItems();
void LoadObjects();
int S_LoadLevelFile(int levelIndex);
void FreeLevel();
void AdjustUV(int num);
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
void FixUpRoom(ROOM_INFO* room, ROOM_INFO* roomData);
void Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize);

unsigned CALLBACK LoadLevel(void* data);

// New functions for loading data from TR5M footer
bool ReadLuaIds(ChunkId* chunkId, int maxSize, int arg);
bool ReadLuaTriggers(ChunkId* chunkId, int maxSize, int arg);
bool ReadNewDataChunks(ChunkId* chunkId, int maxSize, int arg);
void LoadNewData(int size);
