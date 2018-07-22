#pragma once

#include "..\Global\global.h"

#include <vector>

#define DoSomethingWithRooms ((void (__cdecl*)()) 0x004774D0)
#define FreeItemsStuff ((void (__cdecl*)(__int32)) 0x00440590)
#define InitialiseClosedDoors ((void (__cdecl*)()) 0x00473600) 
#define CreateSkinningData ((void (__cdecl*)()) 0x00456AE0)
#define ProcessMeshData ((void (__cdecl*)(__int32)) 0x0049A3D0)
#define ReadFileEx ((__int32 (__cdecl*)(void*, __int32, __int32, FILE*)) 0x004E1D20)
#define Decompress ((__int32 (__cdecl*)(byte*, byte*, __int32, __int32)) 0x004A3EF0)
#define ReadRoomOriginal ((__int32 (__cdecl*)(ROOM_INFO*, ROOM_INFO*)) 0x004917D0)
#define ReadRoomsOriginal ((__int32 (__cdecl*)()) 0x004916C0)
#define FileOpen ((FILE* (__cdecl*)(char*)) 0x004A3CD0)
#define FileClose ((void (__cdecl*)(FILE*)) 0x004A3DA0)
#define LoadSprites ((void (__cdecl*)()) 0x004A59D0)
#define LoadCameras ((void (__cdecl*)()) 0x004A5CA0)
#define LoadSoundEffects ((void (__cdecl*)()) 0x004A5D90)
#define LoadBoxes ((void (__cdecl*)()) 0x004A5E50)
#define LoadAnimatedTextures ((void (__cdecl*)()) 0x004A6060)
#define LoadTextureInfos ((void (__cdecl*)()) 0x004A60E0)
#define LoadAIObjects ((void (__cdecl*)()) 0x004A67F0)
#define LoadDemoData ((void (__cdecl*)()) 0x004A67D0)
#define InitialiseLaraLoad ((void (__cdecl*)(__int16)) 0x004568C0)
#define LoadRooms ((void (__cdecl*)()) 0x004A4DA0)
#define InitialiseLara ((void (__cdecl*)(__int16)) 0x00473210)
#define InitialiseGameStuff ((void (__cdecl*)()) 0x004778F0)

using namespace std;

extern byte* Texture32;
extern byte* Texture16;
extern byte* MiscTextures;
extern __int16* MeshData;
extern __int32 MeshDataSize;
extern OBJECT_TEXTURE* NewObjectTextures;
extern uintptr_t hLoadLevel;
extern __int32 NumMeshPointers;
extern __int32* MeshTrees;
extern __int32 NumObjects;
extern __int32 NumStaticObjects;
extern vector<__int32> MoveablesIds;
extern vector<__int32> StaticObjectsIds;
extern __int32* RawMeshPointers;
extern __int16* RawMeshData;

void __cdecl LoadTextures();
__int32 __cdecl LoadRoomsNew();
__int32 __cdecl LoadItems();
void __cdecl LoadObjects();
__int32 __cdecl S_LoadLevelFile(__int32 levelIndex);
void __cdecl FreeLevel();
void __cdecl AdjustUV(__int32 num);

unsigned __stdcall LoadLevel(void* data);

void Inject_RoomLoad();