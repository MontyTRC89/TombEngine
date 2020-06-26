#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "LEB128.h"
#include "Streams.h"
#include "newtypes.h"
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
	int attribute;
	int tileAndFlag;
	int newFlags;
	struct OBJECT_TEXTURE_VERT vertices[4];
	int destination;
};

#pragma pack(push, 1)
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
#pragma pack(pop)

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

extern short* MeshData;
extern int MeshDataSize;
extern OBJECT_TEXTURE* NewObjectTextures;
extern uintptr_t hLoadLevel;
extern int NumMeshPointers;
extern int* MeshTrees;
extern int NumObjects;
extern int NumStaticObjects;
extern std::vector<int> MoveablesIds;
extern std::vector<int> StaticObjectsIds;
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
extern std::vector<OBJECT_TEXTURE> ObjectTextures;
extern ITEM_INFO* Items;
extern int LevelItems;
extern std::vector<ROOM_INFO> Rooms;
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

extern std::vector<TEXTURE> RoomTextures;
extern std::vector<TEXTURE> MoveablesTextures;
extern std::vector<TEXTURE> StaticsTextures;
extern std::vector<TEXTURE> SpritesTextures;
extern TEXTURE MiscTextures;

extern TrLevel g_Level;

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
void Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize);

unsigned CALLBACK LoadLevel(void* data);

// New functions for loading data from TR5M footer
bool ReadLuaIds(ChunkId* chunkId, int maxSize, int arg);
bool ReadLuaTriggers(ChunkId* chunkId, int maxSize, int arg);
bool ReadNewDataChunks(ChunkId* chunkId, int maxSize, int arg);
void LoadNewData(int size);
