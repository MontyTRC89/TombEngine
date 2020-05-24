#pragma once

#include "global.h"

#include <vector>
#include <map>

#include "IO/ChunkId.h"
#include "IO/ChunkReader.h"
#include "IO/LEB128.h"

struct ChunkId;
struct LEB128;

#define AddPtr(p, t, n) p = (t*)((char*)(p) + (ptrdiff_t)(n));

using namespace std;

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

unsigned __stdcall LoadLevel(void* data);

// New functions for loading data from TR5M footer
bool ReadLuaIds(ChunkId* chunkId, int maxSize, int arg);
bool ReadLuaTriggers(ChunkId* chunkId, int maxSize, int arg);
bool ReadNewDataChunks(ChunkId* chunkId, int maxSize, int arg);
void LoadNewData(int size);
