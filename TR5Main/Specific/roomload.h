#pragma once

#include "..\Global\global.h"

#include <vector>
#include <map>

#include "IO/ChunkId.h"
#include "IO/ChunkReader.h"
#include "IO/LEB128.h"

struct ChunkId;
struct LEB128;

#define DoSomethingWithRooms ((void (__cdecl*)()) 0x004774D0)
#define FreeItemsStuff ((void (__cdecl*)(int)) 0x00440590)
#define InitialiseClosedDoors ((void (__cdecl*)()) 0x00473600) 
#define CreateSkinningData ((void (__cdecl*)()) 0x00456AE0)
#define ProcessMeshData ((void (__cdecl*)(int)) 0x0049A3D0)
#define ReadFileEx ((int (__cdecl*)(void*, int, int, FILE*)) 0x004E1D20)
#define Decompress ((int (__cdecl*)(byte*, byte*, int, int)) 0x004A3EF0)
#define ReadRoomOriginal ((int (__cdecl*)(ROOM_INFO*, ROOM_INFO*)) 0x004917D0)
#define ReadRoomsOriginal ((int (__cdecl*)()) 0x004916C0)
#define FileOpen ((FILE* (__cdecl*)(char*)) 0x004A3CD0)
#define FileClose ((void (__cdecl*)(FILE*)) 0x004A3DA0)
#define LoadSoundEffects ((void (__cdecl*)()) 0x004A5D90)
#define LoadBoxes ((void (__cdecl*)()) 0x004A5E50)
#define LoadAnimatedTextures ((void (__cdecl*)()) 0x004A6060)
#define LoadTextureInfos ((void (__cdecl*)()) 0x004A60E0)
#define LoadAIObjects ((void (__cdecl*)()) 0x004A67F0)
#define LoadDemoData ((void (__cdecl*)()) 0x004A67D0)
#define LoadSamples ((void (__cdecl*)()) 0x004A6880)
#define InitialiseLaraLoad ((void (__cdecl*)(short)) 0x004568C0)
#define LoadRooms ((void (__cdecl*)()) 0x004A4DA0)
#define InitialiseGameStuff ((void (__cdecl*)()) 0x004778F0)
#define InitialiseGameFlags ((void (__cdecl*)()) 0x00477880)
#define InitialiseLaraMeshes ((void (__cdecl*)()) 0x00455680)
#define InitialiseLaraAnims ((void (__cdecl*)(ITEM_INFO*)) 0x00456900)

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

void LoadTextures();
int LoadRoomsNew();
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

unsigned __stdcall LoadLevel(void* data);

// New functions for loading data from TR5M footer
bool ReadLuaIds(ChunkId* chunkId, int maxSize, int arg);
bool ReadLuaTriggers(ChunkId* chunkId, int maxSize, int arg);
bool ReadNewDataChunks(ChunkId* chunkId, int maxSize, int arg);
void LoadNewData(int size);

void Inject_RoomLoad();