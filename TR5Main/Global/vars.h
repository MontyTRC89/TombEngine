#pragma once

#include "..\Renderer\Renderer.h"

// Camera
#define Camera						VAR_U_(0x00EEF940, CAMERA_INFO)
#define ForcedFixedCamera			VAR_U_(0x00EEFA20, GAME_VECTOR)
#define UseForcedFixedCamera		VAR_U_(0x00EEFA50, __int8)

// Memory
#define MallocBuffer				VAR_U_(0x00E4B10C, char*)
#define MallocSize					VAR_U_(0x00E4B058, __int32)
#define MallocPtr					VAR_U_(0x00E4B0DC, char*)
#define MallocFree					VAR_U_(0x00E4B0F4, __int32)
#define MallocUsed					VAR_U_(0x00E4B0F0, __int32)

// Items & Effects
#define Items						VAR_U_(0x00EEEFF0, ITEM_INFO*)
#define Effects						VAR_U_(0x00E5C5E0, FX_INFO*)
#define InItemControlLoop			VAR_U_(0x0051CA53, byte)
#define ItemNewRoomNo				VAR_U_(0x0051CA54, short)
#define ItemNewRooms				ARRAY_(0x00EEF4C0, short, [512])
#define NextFxActive				VAR_U_(0x00E5C5FC, short)
#define NextFxFree					VAR_U_(0x00E5C5F4, short)
#define NextItemActive				VAR_U_(0x00E5BF22, short)
#define NextItemFree				VAR_U_(0x00E5BF16, short)
#define SlotsUsed					VAR_U_(0x0051CEE8, int)
#define nAIObjects					VAR_U_(0x00E5B842, int)
#define TriggerIndex				VAR_U_(0x00EEEF9C, __int16*)

// Level data
#define Boxes						VAR_U_(0x00EEFB64, BOX_INFO*)
#define NumberBoxes					VAR_U_(0x00EEFB68, int)
#define GroundZones					ARRAY_(0x00EEFB20, __int16*, [10])
#define Overlaps					VAR_U_(0x00EEFB5C, __int16*)
#define LevelItems					VAR_U_(0x0051CEB8, int)
#define BaddieSlots					VAR_U_(0x00E5B834, CREATURE_INFO*)
#define AIObjects					VAR_U_(0x00E5B844, AIOBJECT*)
#define Rooms						VAR_U_(0x00875154, ROOM_INFO*)
#define Objects						ARRAY_(0x0086CF50, OBJECT_INFO, [460])
#define Anims						VAR_U_(0x00875158, ANIM_STRUCT*)
#define Bones						VAR_U_(0x00875178, __int32*)
#define Changes						VAR_U_(0x0087515C, CHANGE_STRUCT*)
#define Ranges						VAR_U_(0x00875160, RANGE_STRUCT*)
#define MeshBase					VAR_U_(0x0087516C, __int16*)
#define Meshes						VAR_U_(0x00875170, __int16**)
#define Frames						VAR_U_(0x00875174, __int16*)
#define Commands					VAR_U_(0x0087517C, __int16*)
#define MeshesCount					VAR_U_(0x00875140, __int32)
#define AnimationsCount				VAR_U_(0x00875150, __int32)
#define FloorData					VAR_U_(0x00875168, __int16*)
#define ObjectTextures				VAR_U_(0x008751B0, OBJECT_TEXTURE*)
#define RoomLightsCount				VAR_U_(0x0087B0EC, __int32)
#define LevelDataPtr				VAR_U_(0x00874964, char*)
#define NumberRooms					VAR_U_(0x0087514C, __int16)
#define nAnimUVRanges				VAR_U_(0x0087495C, __int32)
#define LevelFilePtr				VAR_U_(0x00875164, FILE*)
//#define StaticObjects				ARRAY_(0x00874988, STATIC_INFO, [70])

// Lara
#define LaraItem					VAR_U_(0x00E5BF08, ITEM_INFO*)
#define Lara						VAR_U_(0x00E5BD60, LARA_INFO)
#define LaraDrawType				VAR_U_(0x00EEEAD0, __int8)
#define Hairs						ARRAY_(0x00E5C000, HAIR_STRUCT, [14])

#define SequenceUsed				ARRAY_(0x00E4EA78, __int8, [6])
#define SequenceResults				ARRAY_(0x00E4EA80, __int8, [27])
#define gfNumMips					VAR_U_(0x0051CE37, __int8)
#define CurrentSequence				VAR_U_(0x00E4EA61, __int8)
#define Frames						VAR_U_(0x00875174, __int16*)
#define gfMips						ARRAY_(0x00E5C2C0, __int8, [8])
#define OldPickupPos				VAR_U_(0x00E59700, GAME_VECTOR)
#define Rats						VAR_U_(0x00EEEFEC, char*)
#define Bats						VAR_U_(0x00EEEFE8, char*)
#define Spiders						VAR_U_(0x00EEF45C, char*)
#define SetDebounce					VAR_U_(0x00EEEA38, __int32)

// Game
#define FlipStatus					VAR_U_(0x00EEEAE0, int)
#define WindowsHandle				VAR_U_(0x00D9AB3C, HWND)
#define SoundActive					VAR_U_(0x0051D004, __int32)
#define DoTheGame					VAR_U_(0x00876C40, __int32)
#define CurrentLevel				VAR_U_(0x00E5C2D0, __int8)
#define CutSeqNum					VAR_U_(0x0051CAA8, __int32)
#define CutSeqTriggered				VAR_U_(0x0051CAAC, __int32)
#define GlobalPlayingCutscene		VAR_U_(0x0051CAB0, __int32)
#define Wibble						VAR_U_(0x0051CDF0, __int32)
// Input
#define DbInput						VAR_U_(0x00878DAC, __int32)
#define TrInput						VAR_U_(0x00878D98, __int32)

// Math
#define MatrixStack					ARRAY_(0x0055D66C, __int32, [480])
#define MatrixPtr					VAR_U_(0x00E4B0F8, __int32*)
#define DxMatrixPtr					VAR_U_(0x0055DA2C, byte*)

// Unknown
#define Unk_876C48					VAR_U_(0x00876C48, __int32)
#define Unk_007E7FE8				VAR_U_(0x007E7FE8, __int32)

// Gameflow
#define gfFilenameOffset			VAR_U_(0x00E5C34C, __int16*)
#define gfFilenameWad				VAR_U_(0x00E5C2CC, char*)

#define RenderLoadBar				VAR_U_(0x008FBDC0, __int32)
#define IsLevelLoading				VAR_U_(0x00874968, __int32)
#define ThreadId					VAR_U_(0x00874978, unsigned int)
#define WeatherType					VAR_U_(0x00EEF4A0, byte)
#define CreditsDone					VAR_U_(0x00E6D838, byte)
#define CanLoad						VAR_U_(0x0051CE54, byte)
#define Savegame					VAR_U_(0x00E52EB3, SAVEGAME_INFO)
#define GlobalLastInventoryItem		VAR_U_(0x00508E14, __int32)
#define GlobalEnterInventory		VAR_U_(0x00508E18, __int32)
#define DelCutSeqPlayer				VAR_U_(0x0051CE2C, __int16)
#define TitleControlsLockedOut		VAR_U_(0x00E5C2A8, byte)
#define IsAtmospherePlaying			VAR_U_(0x00EEEFFC, byte)

#define TrackCameraInit				VAR_U_(0x0051D010, __int32)
#define CheckTrigger				VAR_U_(0x0051D014, __int32)
#define UseSpotCam					VAR_U_(0x0051D018, __int32)
#define DisableLaraControl			VAR_U_(0x0051D01C, __int32)
#define FramesCount					VAR_U_(0x0051CA78, __int32)
#define GlobalCounter				VAR_U_(0x0051CA58, __int16)

#define LevelComplete				VAR_U_(0x00E5C2F0, byte)
#define ResetFlag					VAR_U_(0x0051CA6C, __int32)
#define GameFlow					VAR_U_(0x00E5C2BC, GAMEFLOW*)
#define GameMode					VAR_U_(0x005082C0, byte)
#define nFrames						VAR_U_(0x005082C4, __int32)
#define GameStatus					VAR_U_(0x00E5C2B0, __int32)
#define GotLaraSpheres				VAR_U_(0x00E51F2E, byte)

#define EffectRoutines				ARRAY_(0x00507964, ITEM_INFO*, [1]) // TODO: redefine this array properly
#define DashTimer					VAR_U_(0x00E5BF04, __int16)

#define ShockWaves					ARRAY_(0x00E4C1A0, SHOCKWAVE_STRUCT, [16])
//#define Drips						ARRAY_(0x00E4D740, DRIP_STRUCT, [32])
#define Bubbles						ARRAY_(0x00E4D160, BUBBLE_STRUCT, [40])
#define Ripples						ARRAY_(0x00E5C600, RIPPLE_STRUCT, [32])
#define Blood						ARRAY_(0x00E4C1A0, BLOOD_STRUCT, [32])

#define CurrentFOV					VAR_U_(0x00E4F504, __int16)

#define SpotCam						ARRAY_(0x00E4F6C0, SPOTCAM, [64])
#define CurrentSplineCamera			VAR_U_(0x00E4F500, __int16)
#define OldHitPoints				VAR_U_(0x005084DC, __int16)

#define AllStrings					VAR_U_(0x00E5C310, char*)
#define AllStringsOffsets			VAR_U_(0x00E5C2B8, __int16*)

#define GameTimer					VAR_U_(0x00E5C27C, __int32)

extern bool MonksAttackLara;

// Remapped variables
extern __int32 NumItems;
extern __int32 dword_874254;
extern __int32 unk_87435C;
extern ITEM_INFO* Targets[NUM_SLOTS];
extern STATIC_INFO StaticObjects[NUM_STATICS];