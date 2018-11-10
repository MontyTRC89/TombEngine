#pragma once

#include "..\Game\sound.h"
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
//#define LevelDataPtr				VAR_U_(0x00874964, char*)
#define NumberRooms					VAR_U_(0x0087514C, __int16)
#define nAnimUVRanges				VAR_U_(0x0087495C, __int32)
#define LevelFilePtr				VAR_U_(0x00875164, FILE*)
//#define StaticObjects				ARRAY_(0x00874988, STATIC_INFO, [70])
#define NumberCameras				VAR_U_(0x00EEFAC0, __int32)
#define Cameras						VAR_U_(0x00EEF9A2, OBJECT_VECTOR*)
#define NumberSpotcams				VAR_U_(0x00E4F428, __int32)

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
#define SetDebounce					VAR_U_(0x00EEEA38, __int32)

// Game
#define FlipStatus					VAR_U_(0x00EEEAE0, int)
#define FlipStats					ARRAY_(0x00EEF060, __int32, [255])
#define WindowsHandle				VAR_U_(0x00D9AB3C, HWND)
#define SoundActive					VAR_U_(0x0051D004, __int32)
#define DoTheGame					VAR_U_(0x00876C40, __int32)
#define CurrentLevel				VAR_U_(0x00E5C2D0, __int8)
#define CutSeqNum					VAR_U_(0x0051CAA8, __int32)
#define CutSeqTriggered				VAR_U_(0x0051CAAC, __int32)
#define GlobalPlayingCutscene		VAR_U_(0x0051CAB0, __int32)
#define Wibble						VAR_U_(0x0051CDF0, __int32)
#define CdFlags						ARRAY_(0x00EEEA40, byte, [136])

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

// Sound
#define SampleLUT					VAR_U_(0x00E528A4, __int16*)
#define SampleInfo					VAR_U_(0x00E528A8, SAMPLE_INFO*)
#define SamplePointer               ARRAY_(0x0086BEF0, HSAMPLE, [SOUND_MAX_SAMPLES])
#define SoundSlot					ARRAY_(0x00E52430, SoundEffectSlot, [SOUND_MAX_CHANNELS])

#define TrackNamePrefix				VAR_U_(0x00511828, char)
#define TrackNameTable				ARRAY_(0x005108C0, char*, [SOUND_LEGACY_TRACKTABLE_SIZE])
#define TrackMap					ARRAY_(0x00EEEA40, byte,  [SOUND_LEGACY_TRACKTABLE_SIZE])

#define IsAtmospherePlaying			VAR_U_(0x00EEEFFC, byte)
#define CurrentAtmosphere			VAR_U_(0x00EEEB90, byte)
#define GlobalMusicVolume			VAR_U_(0x00517B68, unsigned int)
#define GlobalFXVolume				VAR_U_(0x00517B6C, unsigned int)

// Gameflow
#define gfFilenameOffset			VAR_U_(0x00E5C34C, __int16*)
#define gfFilenameWad				VAR_U_(0x00E5C2CC, char*)
#define AllStrings					VAR_U_(0x00E5C310, char*)
#define AllStringsOffsets			VAR_U_(0x00E5C2B8, __int16*)

#define RenderLoadBar				VAR_U_(0x008FBDC0, __int32)
//#define IsLevelLoading				VAR_U_(0x00874968, __int32)
//#define ThreadId					VAR_U_(0x00874978, unsigned int)
#define WeatherType					VAR_U_(0x00EEF4A0, byte)
#define CreditsDone					VAR_U_(0x00E6D838, byte)
#define CanLoad						VAR_U_(0x0051CE54, byte)
#define Savegame					VAR_U_(0x00E52EB3, SAVEGAME_INFO)
#define GlobalLastInventoryItem		VAR_U_(0x00508E14, __int32)
#define GlobalEnterInventory		VAR_U_(0x00508E18, __int32)
#define InventoryItemChosen			VAR_U_(0x00508E1C, __int32)
#define DelCutSeqPlayer				VAR_U_(0x0051CE2C, __int16)
#define TitleControlsLockedOut		VAR_U_(0x00E5C2A8, byte)

#define TrackCameraInit				VAR_U_(0x0051D010, __int32)
#define CheckTrigger				VAR_U_(0x0051D014, __int32)
#define UseSpotCam					VAR_U_(0x0051D018, __int32)
#define DisableLaraControl			VAR_U_(0x0051D01C, __int32)
#define FramesCount					VAR_U_(0x0051CA78, __int32)
#define GlobalCounter				VAR_U_(0x0051CA58, __int16)

#define LevelComplete				VAR_U_(0x00E5C2F0, byte)
#define ResetFlag					VAR_U_(0x0051CA6C, __int32)
//#define GameFlow					VAR_U_(0x00E5C2BC, GAMEFLOW*)
#define GameMode					VAR_U_(0x005082C0, byte)
#define nFrames						VAR_U_(0x005082C4, __int32)
#define GameStatus					VAR_U_(0x00E5C2B0, __int32)
#define GotLaraSpheres				VAR_U_(0x00E51F2E, byte)

#define EffectRoutines				ARRAY_(0x00507964, EFFECT_ROUTINE, [100])
#define DashTimer					VAR_U_(0x00E5BF04, __int16)
#define LaraCollisionRoutines		ARRAY_(0x005089A8, LARA_COLLISION_ROUTINE, [500])
#define LaraControlRoutines			ARRAY_(0x0050877C, LARA_CONTROL_ROUTINE, [500])

#define ShockWaves					ARRAY_(0x00E4C1A0, SHOCKWAVE_STRUCT, [16])
//#define Drips						ARRAY_(0x00E4D740, DRIP_STRUCT, [32])
#define Bubbles						ARRAY_(0x00E4D160, BUBBLE_STRUCT, [40])
#define Ripples						ARRAY_(0x00E5C600, RIPPLE_STRUCT, [32])
#define Blood						ARRAY_(0x00E4C1A0, BLOOD_STRUCT, [32])

#define CurrentFOV					VAR_U_(0x00E4F504, __int16)

#define SpotCam						ARRAY_(0x00E4F6C0, SPOTCAM, [64])
#define CurrentSplineCamera			VAR_U_(0x00E4F500, __int16)
#define OldHitPoints				VAR_U_(0x005084DC, __int16)

#define GameTimer					VAR_U_(0x00E5C27C, __int32)

// Inventory
#define InventoryObjectsList		ARRAY_(0x00508E38, INVOBJ, [100])
#define Friggrimmer					VAR_U_(0x00E598F4, byte)
#define InventoryRings				ARRAY_(0x00E59900, INVENTORYRING*, [2])

#define PickupX						VAR_U_(0x00E5BF38, __int16)
#define CurrentPickup				VAR_U_(0x00E5BF3C, __int16)
#define Pickups						ARRAY_(0x00E5BF40, DISPLAY_PICKUP, [8])
#define PickupVel					VAR_U_(0x00E5BF3A, __int16)

#define GnFrameCounter				VAR_U_(0x00E4B0FC, __int32)
#define	gfLevelFlags				VAR_U_(0x00E5C2A0, __int32)

#define OptionAutoTarget			VAR_U_(0x00D9AC30, __int32)

#define PhdLeft						VAR_U_(0x0055D20C, __int32)
#define PhdTop						VAR_U_(0x0051D0A8, __int32)
#define PhdRight					VAR_U_(0x0055DA3C, __int32)
#define PhdBottom					VAR_U_(0x0055D204, __int32)
#define BoundList					ARRAY_(0x0051CB5C, __int32, [128])
#define BoundStart					VAR_U_(0x0051CD60, __int32)
#define BoundEnd					VAR_U_(0x0051CD64, __int32)
#define NumberDrawnRooms			VAR_U_(0x00E6CAFC, __int32)
#define DrawnRooms					ARRAY_(0x00E6D760, __int16, [256])
#define CurrentRoom					VAR_U_(0x00E6D754, __int16)
#define Outside						VAR_U_(0x00E6CAF8, __int32)
#define OutsideTop					VAR_U_(0x00E6E4E0, __int32)
#define OutsideLeft					VAR_U_(0x00E6D83C, __int32)
#define OutsideBottom				VAR_U_(0x00E6D738, __int32)
#define OutsideRight				VAR_U_(0x00E6E4C0, __int32)
#define Underwater					VAR_U_(0x00E6E4B4, __int32)
#define Unknown_00E6CAE8			VAR_U_(0x00E6CAE8, __int32)

#define Fires						ARRAY_(0x00E4C7A0, FIRE_LIST, [32])
#define Dynamics					ARRAY_(0x00E6C3E0, DYNAMIC, [64])
#define NumDynamics					VAR_U_(0x00E6D82C, __int32)

#define Sprites						VAR_U_(0x008751B4, SPRITE*)
#define FireSparks					ARRAY_(0x00E4CE40, FIRE_SPARKS, [20])
#define NextFreeFireSpark			VAR_U_(0x0050A17C, __int16)
#define SmokeSparks					ARRAY_(0x00E4B940, SMOKE_SPARKS, [32])
#define Blood						ARRAY_(0x00E4C9C0, BLOOD_STRUCT, [32])
#define GunShells					ARRAY_(0x00E4BEC0, GUNSHELL_STRUCT, [24])
#define Sparks						ARRAY_(0x00E5F380, SPARKS, [128])
#define Drips						ARRAY_(0x00E4D740, DRIP_STRUCT, [32])
#define Bubbles						ARRAY_(0x00E4D160, BUBBLE_STRUCT, [40])
#define Splashes					ARRAY_(0x00E6CA00, SPLASH_STRUCT, [4])
#define Ripples						ARRAY_(0x00E5C600, RIPPLE_STRUCT, [32])

#define SkyPos1						VAR_U_(0x00E6E4B0, __int16)
#define SkyPos2						VAR_U_(0x00E6D73E, __int16)
#define SkyVelocity1				VAR_U_(0x00E5C276, signed char)
#define SkyVelocity2				VAR_U_(0x00E5C275, signed char)

#define SkyColor1					VAR_U_(0x00E5C2A4, CVECTOR)
#define SkyColor2					VAR_U_(0x00E5C280, CVECTOR)
#define SkyStormColor				ARRAY_(0x00E6CAF0, __int16, [3])
#define StormTimer					VAR_U_(0x0051CD5C, __int16)
#define Unk_00E6D74C				VAR_U_(0x00E6D74C, __int16)
#define Unk_00E6D73C				VAR_U_(0x00E6D73C, __int16)
#define Unk_00E6D74C				VAR_U_(0x00E6D74C, __int16)
#define Unk_00E6E4DC				VAR_U_(0x00E6E4DC, __int16)

#define CurrentAtmosphere			VAR_U_(0x00EEEB90, byte)
#define IsAtmospherePlaying			VAR_U_(0x00EEEFFC, byte)

#define SmokeCountL					VAR_U_(0x00E6C9E8, byte)
#define SmokeCountR					VAR_U_(0x00E6C9EC, byte)
#define SplashCount					VAR_U_(0x0051CDF8, byte)
#define WeaponDelay					VAR_U_(0x0051CA52, byte)

#define HealtBarTimer				VAR_U_(0x0051CEA8, __int32)

#define AnimatedTextureRanges		VAR_U_(0x00D9ADA0, __int16*)
//#define Savegame					VAR_U_(0x00E52EB3, savegame_info);
#define gfRequiredStartPos			VAR_U_(0x00E5C2B4, byte)
#define gfInitialiseGame			VAR_U_(0x005082C1, byte)

#define TorchRoom					VAR_U_(0x00507AB0, __int16)
#define WeaponEnemyTimer			VAR_U_(0x0080E016, byte)

#define HeightType					VAR_U_(0x00EEEFF4, __int32)
#define HeavyTriggered				VAR_U_(0x00EEEA34, byte)
#define FlipEffect					VAR_U_(0x00506C60, __int32)
#define FlipTimer					VAR_U_(0x0051CA68, __int32)
#define Unknown_00EEF99A			VAR_U_(0x00EEF99A, byte)
#define SpotCamRemap				ARRAY_(0x00E4F4F0, byte, [16])
#define CameraCnt					ARRAY_(0x00E51F10, byte, [16])
#define LastSpotCam					VAR_U_(0x00E51F2C, __int16)
#define FlipMap						ARRAY_(0x00EEEBA0, __int32, [255])
#define TriggerTimer				VAR_U_(0x0051CA5A, byte)
#define JustLoaded					VAR_U_(0x0051D001, byte)
//#define Weapons						ARRAY_(0x005085B0, WEAPON_INFO, [9])
#define SmokeWeapon					VAR_U_(0x00E6CAA0, __int32)
#define HKTimer						VAR_U_(0x0051CEC9, __int32)
#define HKFlag						VAR_U_(0x0051CECC, __int32)
#define HKFlag2						VAR_U_(0x0051CEC8, byte)

#define Ropes						ARRAY_(0x00E54CC0, ROPE_STRUCT, [12])
#define NumRopes					VAR_U_(0x0051CA60, __int32)

#define PhdWidth					VAR_U_(0x0055D29C, __int32)
#define PhdHeight					VAR_U_(0x0055D164, __int32)
#define PhdPerspective				VAR_U_(0x0055D208, __int32)

#define Bats						VAR_U_(0x00EEEFE8, BAT_STRUCT*)
#define Rats						VAR_U_(0x00EEEFEC, RAT_STRUCT*)
#define Spiders						VAR_U_(0x00EEF45C, SPIDER_STRUCT*)

#define PoisonFlags					VAR_U_(0x00E5BF3E, byte)
#define SmashedMeshCount			VAR_U_(0x0051CA5C, __int16)
#define SmashedMesh					ARRAY_(0x00EEF8C0, MESH_INFO*, [16])
#define SmashedMeshRoom				ARRAY_(0x00EEF480, __int16, [16])

#define Debris						ARRAY_(0x00E8CAC0, DEBRIS_STRUCT, [256])

extern bool MonksAttackLara;

// Remapped variables
extern __int32 NumItems;
extern __int32 dword_874254;
extern __int32 unk_87435C;
extern ITEM_INFO* Targets[NUM_SLOTS];
extern STATIC_INFO StaticObjects[NUM_STATICS];