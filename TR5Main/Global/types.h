#pragma once
#include <Windows.h>

#pragma pack(push, 1)
typedef struct vector_t
{
	__int32 vx;
	__int32 vy;
	__int32 vz;
	__int32 pad;
} VECTOR;

typedef struct SPHERE
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int32 r; // size=0, offset=12
};

typedef struct svector_t
{
	__int16 vx;
	__int16 vy;
	__int16 vz;
	__int16 pad;
} SVECTOR;

typedef struct cvector_t
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char cd;
} CVECTOR;

typedef struct phd_vector_t
{
	__int32 x;
	__int32 y;
	__int32 z;
} PHD_VECTOR;

typedef struct tr_vertex
{
	__int32 x;
	__int32 y;
	__int32 z;
};

struct MATRIX3D
{
	__int16 m00; // size=0, offset=0
	__int16 m01; // size=0, offset=2
	__int16 m02; // size=0, offset=4
	__int16 m10; // size=0, offset=6
	__int16 m11; // size=0, offset=8
	__int16 m12; // size=0, offset=10
	__int16 m20; // size=0, offset=12
	__int16 m21; // size=0, offset=14
	__int16 m22; // size=0, offset=16
	__int16 pad; // size=0, offset=18
	__int32 tx; // size=0, offset=20
	__int32 ty; // size=0, offset=24
	__int32 tz; // size=0, offset=28
};

typedef struct phd_3dpos_t
{
	__int32 xPos; // off 0 [64]
	__int32 yPos; // off 4 [68]
	__int32 zPos; // off 8 [72]
	__int16 xRot; // off 12 [76]
	__int16 yRot; // off 14 [78]
	__int16 zRot; // off 16 [80]
} PHD_3DPOS;

typedef struct game_vector_t
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 roomNumber; // size=0, offset=12
	__int16 boxNumber; // size=0, offset=14
} GAME_VECTOR;

typedef struct object_vector
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 data; // size=0, offset=12
	__int16 flags; // size=0, offset=14
} OBJECT_VECTOR;

typedef struct ilight_t
{
	__int16 x;
	__int16 y;
	__int16 z;
	__int16 pad1;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char pad;
} ILIGHT;

typedef struct item_light_t
{
	ILIGHT light[4];
} ITEM_LIGHT;

typedef struct hair_struct
{
	PHD_3DPOS pos;
	byte unknown[24];
} HAIR_STRUCT;

typedef struct box_node_t
{
	__int16 exitBox; // size=0, offset=0
	unsigned __int16 searchNumber; // size=0, offset=2
	__int16 nextExpansion; // size=0, offset=4
	__int16 boxNumber; // size=0, offset=6
} BOX_NODE;

typedef struct box_info_t
{
	unsigned char left; // size=0, offset=0
	unsigned char right; // size=0, offset=1
	unsigned char top; // size=0, offset=2
	unsigned char bottom; // size=0, offset=3
	__int16 height; // size=0, offset=4
	__int16 overlapIndex; // size=0, offset=6
} BOX_INFO;

typedef struct ai_info_t
{
	__int16 zoneNumber; // size=0, offset=0
	__int16 enemyZone; // size=0, offset=2
	__int32 distance; // size=0, offset=4
	__int32 ahead; // size=0, offset=8
	__int32 bite; // size=0, offset=12
	__int16 angle; // size=0, offset=16
	__int16 xAngle; // size=0, offset=18
	__int16 enemyFacing; // size=0, offset=20
} AI_INFO;

typedef struct bite_info_t {				// Offset into given Mesh
	__int32			x;					// where Baddie kicks off Bite Effect
	__int32			y;
	__int32			z;
	__int32			meshNum;
} BITE_INFO;

typedef struct lot_info_t
{
	BOX_NODE* node; // size=8, offset=0
	__int16 head; // size=0, offset=4
	__int16 tail; // size=0, offset=6
	unsigned __int16 searchNumber; // size=0, offset=8
	unsigned __int16 blockMask; // size=0, offset=10
	__int16 step; // size=0, offset=12
	__int16 drop; // size=0, offset=14
	__int16 zoneCount; // size=0, offset=16
	__int16 targetBox; // size=0, offset=18
	__int16 requiredBox; // size=0, offset=20
	__int16 fly; // size=0, offset=22
	unsigned __int16 canJump : 1; // offset=24.0
	unsigned __int16 canMonkey : 1; // offset=24.1
	unsigned __int16 isAmphibious : 1; // offset=24.2
	unsigned __int16 isJumping : 1; // offset=24.3
	unsigned __int16 isMonkeying : 1; // offset=24.4
	PHD_VECTOR target; // size=12, offset=26
	__int32 zone; // size=4, offset=40
} LOT_INFO;

typedef struct floor_info_t {
	unsigned __int16 index; // size=0, offset=0
	unsigned __int16 fx : 4; // offset=2.0
	unsigned __int16 box : 11; // offset=2.4
	unsigned __int16 stopper : 1; // offset=3.7
	unsigned char pitRoom; // size=0, offset=4
	byte floor; // size=0, offset=5
	unsigned char skyRoom; // size=0, offset=6
	byte ceiling; // size=0, offset=7
} FLOOR_INFO;

typedef struct item_info_t {
	__int32 floor; // size=0, offset=0
	__int32 touchBits; // size=0, offset=4
	__int32 meshBits; // size=0, offset=8
	__int16 objectNumber; // size=0, offset=12
	__int16 currentAnimState; // size=0, offset=14
	__int16 goalAnimState; // size=0, offset=16
	__int16 requiredAnimState; // size=0, offset=18
	__int16 animNumber; // size=0, offset=20
	__int16 frameNumber; // size=0, offset=22
	__int16 roomNumber; // size=0, offset=24
	__int16 nextItem; // size=0, offset=26
	__int16 nextActive; // size=0, offset=28
	__int16 speed; // size=0, offset=30
	__int16 fallspeed; // size=0, offset=32
	__int16 hitPoints; // size=0, offset=34
	unsigned __int16 boxNumber; // size=0, offset=36
	__int16 timer; // size=0, offset=38
	__int16 flags; // size=0, offset=40
	__int16 shade; // size=0, offset=42
	__int16 triggerFlags; // size=0, offset=44
	__int16 carriedItem; // size=0, offset=46
	__int16 afterDeath; // size=0, offset=48
	unsigned __int16 firedWeapon; // size=0, offset=50
	__int16 itemFlags[4]; // size=8, offset=52
	void* data; // size=0, offset=60
	PHD_3DPOS pos; // size=20, offset=64
	//ITEM_LIGHT il; // size=48, offset=84
	byte padLight[48];
	unsigned __int32 meshswapMeshbits; // size=0, offset=136 OFF=132
	__int16 drawRoom; // size=0, offset=140 OFF=136
	__int16 TOSSPAD; // size=0, offset=142 OFF=138
	byte pad1[5464]; // OFF=140   5432?
	byte* pointer1;
	byte* pointer2;
	//__int16 status;
	//byte pad2[8];
	unsigned __int32 active : 1; // offset=132.0 OFF=5610
	unsigned __int32 status : 2; // offset=132.1
	unsigned __int32 gravityStatus : 1; // offset=132.3
	unsigned __int32 hitStatus : 1; // offset=132.4
	unsigned __int32 collidable : 1; // offset=132.5
	unsigned __int32 lookedAt : 1; // offset=132.6
	unsigned __int32 dynamicLight : 1; // offset=132.7
	unsigned __int32 poisoned : 1; // offset=133.0
	unsigned __int32 aiBits : 5; // offset=133.1
	unsigned __int32 reallyActive : 1; // offset=133.6
	unsigned __int32 InDrawRoom : 1; // offset=133.7
	__int32 swapMeshFlags;
	byte pad2[4]; // OFF=5614
} ITEM_INFO;

typedef struct creature_info_t 
{
	__int16 jointRotation[4]; // size=8, offset=0
	__int16 maximumTurn; // size=0, offset=8
	__int16 flags; // size=0, offset=10
	unsigned __int16 alerted : 1; // offset=12.0
	unsigned __int16 headLeft : 1; // offset=12.1
	unsigned __int16 headRight : 1; // offset=12.2
	unsigned __int16 reachedGoal : 1; // offset=12.3
	unsigned __int16 hurtByLara : 1; // offset=12.4
	unsigned __int16 patrol2 : 1; // offset=12.5
	unsigned __int16 jumpAhead : 1; // offset=12.6
	unsigned __int16 monkeyAhead : 1; // offset=12.7
	MOOD_TYPE mood; // size=4, offset=14
	ITEM_INFO* enemy; // size=144, offset=18
	ITEM_INFO aiTarget; // size=144, offset=22
	__int16 pad; // size=0, offset=5644
	__int16 itemNum; // size=0, offset=5644
	PHD_VECTOR target; // size=12, offset=5646
	LOT_INFO LOT; // size=44, offset=5658
} CREATURE_INFO;

typedef struct lara_arm_t
{
	__int16* frameBase; // size=0, offset=0
	__int16 frameNumber; // size=0, offset=4
	__int16 animNumber; // size=0, offset=6
	__int16 lock; // size=0, offset=8
	__int16 yRot; // size=0, offset=10
	__int16 xRot; // size=0, offset=12
	__int16 zRot; // size=0, offset=14
	__int16 flash_gun; // size=0, offset=16
} LARA_ARM;

typedef struct fx_info_t
{
	PHD_3DPOS pos; // size=20, offset=0
	__int16 roomNumber; // size=0, offset=20
	__int16 objectNumber; // size=0, offset=22
	__int16 nextFx; // size=0, offset=24
	__int16 nextActive; // size=0, offset=26
	__int16 speed; // size=0, offset=28
	__int16 fallspeed; // size=0, offset=30
	__int16 frameNumber; // size=0, offset=32
	__int16 counter; // size=0, offset=34
	__int16 shade; // size=0, offset=36
	__int16 flag1; // size=0, offset=38
	__int16 flag2; // size=0, offset=40
} FX_INFO;

typedef struct lara_info_t {
	__int16 itemNumber; // size=0, offset=0
	__int16 gunStatus; // size=0, offset=2
	__int16 gunType; // size=0, offset=4
	__int16 requestGunType; // size=0, offset=6
	__int16 lastGunType; // size=0, offset=8
	__int16 calcFallSpeed; // size=0, offset=10
	__int16 waterStatus; // size=0, offset=12
	__int16 climbStatus; // size=0, offset=14
	__int16 poseCount; // size=0, offset=16
	__int16 hitFrame; // size=0, offset=18
	__int16 hitDirection; // size=0, offset=20
	__int16 air; // size=0, offset=22
	__int16 diveCount; // size=0, offset=24
	__int16 deathCount; // size=0, offset=26
	__int16 currentActive; // size=0, offset=28
	__int16 currentXvel; // size=0, offset=30
	__int16 currentYvel; // size=0, offset=32
	__int16 currentZvel; // size=0, offset=34
	__int16 spazEffectCount; // size=0, offset=36
	__int16 flareAge; // size=0, offset=38
	__int16 BurnCount; // size=0, offset=40
	__int16 weaponItem; // size=0, offset=42
	__int16 backGun; // size=0, offset=44
	__int16 flareFrame; // size=0, offset=46
	__int16 poisoned; // size=0, offset=48
	__int16 dpoisoned; // size=0, offset=50
	unsigned char anxiety; // size=0, offset=52
	unsigned char wet[15]; // size=15, offset=53
	unsigned __int16 flareControlLeft : 1; // offset=68.0
	unsigned __int16 unused1 : 1; // offset=68.1
	unsigned __int16 look : 1; // offset=68.2
	unsigned __int16 burn : 1; // offset=68.3
	unsigned __int16 keepDucked : 1; // offset=68.4
	unsigned __int16 isMoving : 1; // offset=68.5
	unsigned __int16 canMonkeySwing : 1; // offset=68.6
	unsigned __int16 burnBlue : 2; // offset=68.7
	unsigned __int16 gassed : 1; // offset=69.1
	unsigned __int16 burnSmoke : 1; // offset=69.2
	unsigned __int16 isDucked : 1; // offset=69.3
	unsigned __int16 hasFired : 1; // offset=69.4
	unsigned __int16 busy : 1; // offset=69.5
	unsigned __int16 litTorch : 1; // offset=69.6
	unsigned __int16 isClimbing : 1; // offset=69.7
	unsigned __int16 fired : 1; // offset=70.0
	__int32 waterSurfaceDist; // size=0, offset=72
	PHD_VECTOR lastPos; // size=12, offset=76
	FX_INFO* spazEffect; // size=44, offset=88
	__int32 meshEffects; // size=0, offset=92
	__int16* meshPtrs[15]; // size=60, offset=96
	ITEM_INFO* target; // size=144, offset=156
	__int16 targetAngles[2]; // size=4, offset=160
	__int16 turnRate; // size=0, offset=164
	__int16 moveAngle; // size=0, offset=166
	__int16 headYrot; // size=0, offset=168
	__int16 headXrot; // size=0, offset=170
	__int16 headZrot; // size=0, offset=172
	__int16 torsoYrot; // size=0, offset=174
	__int16 torsoXrot; // size=0, offset=176
	__int16 torsoZrot; // size=0, offset=178
	LARA_ARM leftArm; // size=20, offset=180
	LARA_ARM rightArm; // size=20, offset=200
	unsigned __int16 holster; // size=0, offset=220
	CREATURE_INFO* creature; // size=228, offset=224
	__int32 cornerX; // size=0, offset=228
	__int32 cornerZ; // size=0, offset=232
	byte ropeSegment; // size=0, offset=236
	byte ropeDirection; // size=0, offset=237
	__int16 ropeArcFront; // size=0, offset=238
	__int16 ropeArcBack; // size=0, offset=240
	__int16 ropeLastX; // size=0, offset=242
	__int16 ropeMaxXForward; // size=0, offset=244
	__int16 ropeMaxXBackward; // size=0, offset=246
	__int32 ropeDFrame; // size=0, offset=248
	__int32 ropeFrame; // size=0, offset=252
	unsigned __int16 ropeFrameRate; // size=0, offset=256
	unsigned __int16 ropeY; // size=0, offset=258
	__int32 ropePtr; // size=0, offset=260
	void* generalPtr; // size=0, offset=264
	__int32 ropeOffset; // size=0, offset=268
	__int32 ropeDownVel; // size=0, offset=272
	byte ropeFlag; // size=0, offset=276
	byte moveCount; // size=0, offset=277
	__int32 ropeCount; // size=0, offset=280
	byte skelebob; // size=0, offset=284
	byte pistolsTypeCarried; // size=0, offset=285
	byte uzisTypeCarried; // size=0, offset=286
	byte shotgunTypeCarried; // size=0, offset=287
	byte crossbowTypeCarried; // size=0, offset=288
	byte HKtypeCarried; // size=0, offset=289
	byte sixshooterTypeCarried; // size=0, offset=290
	byte laserSight; // size=0, offset=291
	byte silencer; // size=0, offset=292
	byte binoculars; // size=0, offset=293
	byte crowbar; // size=0, offset=294
	byte examine1; // size=0, offset=295
	byte examine2; // size=0, offset=296
	byte examine3; // size=0, offset=297
	byte wetcloth; // size=0, offset=298
	byte bottle; // size=0, offset=299
	byte puzzleItems[12]; // size=12, offset=300
	unsigned __int16 puzzleItemsCombo; // size=0, offset=312
	unsigned __int16 keyItems; // size=0, offset=314
	unsigned __int16 keyItemsCombo; // size=0, offset=316
	unsigned __int16 pickupItems; // size=0, offset=318
	unsigned __int16 pickupItemsCombo; // size=0, offset=320
	__int16 numSmallMedipack; // size=0, offset=322
	__int16 numLargeMedipack; // size=0, offset=324
	__int16 numFlares; // size=0, offset=326
	__int16 numPistolsAmmo; // size=0, offset=328
	__int16 numUziAmmo; // size=0, offset=330
	__int16 numRevolverAmmo; // size=0, offset=332
	__int16 numShotgunAmmo1; // size=0, offset=334
	__int16 numShotgunAmmo2; // size=0, offset=336
	__int16 numHKammo1; // size=0, offset=338
	__int16 numCrossbowAmmo1; // size=0, offset=340
	__int16 numCrossbowAmmo2; // size=0, offset=342
	byte location; // size=0, offset=344
	byte highestLocation; // size=0, offset=345
	byte locationPad; // size=0, offset=346
	unsigned char tightRopeOnCount; // size=0, offset=347
	unsigned char tightRopeOff; // size=0, offset=348
	unsigned char tightRopeFall; // size=0, offset=349
	unsigned char chaffTimer; // size=0, offset=350
} LARA_INFO;

typedef struct coll_info_t
{
	__int32 midFloor; // size=0, offset=0
	__int32 midCeiling; // size=0, offset=4
	__int32 midType; // size=0, offset=8
	__int32 frontFloor; // size=0, offset=12
	__int32 frontCeiling; // size=0, offset=16
	__int32 frontType; // size=0, offset=20
	__int32 leftFloor; // size=0, offset=24
	__int32 leftCeiling; // size=0, offset=28
	__int32 leftType; // size=0, offset=32
	__int32 rightFloor; // size=0, offset=36
	__int32 rightCeiling; // size=0, offset=40
	__int32 rightType; // size=0, offset=44
	__int32 leftFloor2; // size=0, offset=48
	__int32 leftCeiling2; // size=0, offset=52
	__int32 leftType2; // size=0, offset=56
	__int32 rightFloor2; // size=0, offset=60
	__int32 rightCeiling2; // size=0, offset=64
	__int32 rightType2; // size=0, offset=68
	__int32 radius; // size=0, offset=72
	__int32 badPos; // size=0, offset=76
	__int32 badNeg; // size=0, offset=80
	__int32 badCeiling; // size=0, offset=84
	PHD_VECTOR shift; // size=12, offset=88
	PHD_VECTOR old; // size=12, offset=100
	__int16 oldAnimState; // size=0, offset=112
	__int16 oldAnimNumber; // size=0, offset=114
	__int16 oldFrameNumber; // size=0, offset=116
	__int16 facing; // size=0, offset=118
	__int16 quadrant; // size=0, offset=120
	__int16 collType; // size=0, offset=122 USE ENUM CT_*
	__int16* trigger; // size=0, offset=124
	byte tiltX; // size=0, offset=128
	byte tiltZ; // size=0, offset=129
	byte hitByBaddie; // size=0, offset=130
	byte hitStatic; // size=0, offset=131
	unsigned __int16 slopesAreWalls : 2; // offset=132.0
	unsigned __int16 slopesArePits : 1; // offset=132.2
	unsigned __int16 lavaIsPit : 1; // offset=132.3
	unsigned __int16 enableBaddiePush : 1; // offset=132.4
	unsigned __int16 enableSpaz : 1; // offset=132.5
	unsigned __int16 hitCeiling : 1; // offset=132.6
} COLL_INFO;

typedef struct aiobject_t
{
	__int16 objectNumber; // size=0, offset=0
	__int16 roomNumber; // size=0, offset=2
	__int32 x; // size=0, offset=4
	__int32 y; // size=0, offset=8
	__int32 z; // size=0, offset=12
	__int16 triggerFlags; // size=0, offset=16
	__int16 flags; // size=0, offset=18
	__int16 yRot; // size=0, offset=20
	__int16 boxNumber; // size=0, offset=22
} AIOBJECT;

typedef struct object_info_t {
	__int16 nmeshes; // size=0, offset=0
	__int16 meshIndex; // size=0, offset=2
	__int32 boneIndex; // size=0, offset=4
	__int16* frameBase; // size=0, offset=8
	void(*initialise)(__int16 itemNumber); // size=0, offset=12
	void(*control)(__int16 itemNumber); // size=0, offset=16
	void(*floor)(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, int* height); // size=0, offset=20
	void(*ceiling)(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, int* height); // size=0, offset=24
	void(*drawRoutine)(ITEM_INFO* item); // size=0, offset=28
	void(*collision)(__int16 item_num, ITEM_INFO* laraitem, COLL_INFO* coll); // size=0, offset=32
	__int16 objectMip; // size=0, offset=36
	__int16 animIndex; // size=0, offset=38
	__int16 hitPoints; // size=0, offset=40
	__int16 pivotLength; // size=0, offset=42
	__int16 radius; // size=0, offset=44
	__int16 shadowSize; // size=0, offset=46
	unsigned __int16 biteOffset; // size=0, offset=48
	unsigned __int16 loaded : 1; // offset=50.0
	unsigned __int16 intelligent : 1; // offset=50.1
	unsigned __int16 nonLot : 1; // offset=50.2
	unsigned __int16 savePosition : 1; // offset=50.3
	unsigned __int16 saveHitpoints : 1; // offset=50.4
	unsigned __int16 saveFlags : 1; // offset=50.5
	unsigned __int16 saveAnim : 1; // offset=50.6
	unsigned __int16 semiTransparent : 1; // offset=50.7
	unsigned __int16 waterCreature : 1; // offset=51.0
	unsigned __int16 usingDrawanimatingItem : 1; // offset=51.1
	unsigned __int16 hitEffect : 2; // offset=51.2
	unsigned __int16 undead : 1; // offset=51.4
	unsigned __int16 saveMesh : 1; // offset=51.5
	void(*drawRoutineExtra)(ITEM_INFO* item); // size=0, offset=52
	unsigned __int32 explodableMeshbits; // size=0, offset=56
	unsigned __int32 padfuck; // size=0, offset=60
} OBJECT_INFO;

typedef struct tr5_room_layer_t   // 56 bytes
{
	unsigned __int32 NumLayerVertices;   // Number of vertices in this layer (4 bytes)
	unsigned __int16 UnknownL1;
	unsigned __int16 NumLayerRectangles; // Number of rectangles in this layer (2 bytes)
	unsigned __int16 NumLayerTriangles;  // Number of triangles in this layer (2 bytes)
	unsigned __int16 UnknownL2;

	unsigned __int16 Filler;             // Always 0
	unsigned __int16 Filler2;            // Always 0

								 // The following 6 floats define the bounding box for the layer

	float    LayerBoundingBoxX1;
	float    LayerBoundingBoxY1;
	float    LayerBoundingBoxZ1;
	float    LayerBoundingBoxX2;
	float    LayerBoundingBoxY2;
	float    LayerBoundingBoxZ2;

	unsigned __int32 Filler3;     // Always 0 (4 bytes)
	void* VerticesOffset;
	void* PolyOffset;
	void* PolyOffset2;
} tr5_room_layer;

typedef struct tr5_vertex_t   // 12 bytes
{
	float x;
	float y;
	float z;
} tr5_vertex;

typedef struct tr5_room_vertex_t  // 28 bytes
{
	tr5_vertex Vertex;     // Vertex is now floating-point
	tr5_vertex Normal;
	unsigned __int32 Colour;     // 32-bit colour
} tr5_room_vertex;

typedef struct mesh_info_t
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 yRot; // size=0, offset=12
	__int16 shade; // size=0, offset=14
	__int16 Flags; // size=0, offset=16
	__int16 staticNumber; // size=0, offset=18
} MESH_INFO;

typedef struct light_info_t
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	unsigned char Type; // size=0, offset=12
	unsigned char r; // size=0, offset=13
	unsigned char g; // size=0, offset=14
	unsigned char b; // size=0, offset=15
	__int16 nx; // size=0, offset=16
	__int16 ny; // size=0, offset=18
	__int16 nz; // size=0, offset=20
	__int16 Intensity; // size=0, offset=22
	unsigned char Inner; // size=0, offset=24
	unsigned char Outer; // size=0, offset=25
	__int16 FalloffScale; // size=0, offset=26
	__int16 Length; // size=0, offset=28
	__int16 Cutoff; // size=0, offset=30
} LIGHTINFO;

struct tr4_mesh_face3    // 10 bytes
{
	__int16 Vertices[3];
	__int16 Texture;
	__int16 Effects;    // TR4-5 ONLY: alpha blending and environment mapping strength
};

struct tr4_mesh_face4    // 12 bytes
{
	__int16 Vertices[4];
	__int16 Texture;
	__int16 Effects;
};

struct tr_room_portal  // 32 bytes
{
	__int16  AdjoiningRoom; // Which room this portal leads to
	tr_vertex Normal;
	tr_vertex Vertices[4];
};

struct tr_room_sector // 8 bytes
{
	unsigned __int16 FDindex;    // Index into FloorData[]
	unsigned __int16 BoxIndex;   // Index into Boxes[] (-1 if none)
	byte  RoomBelow;  // 255 is none
	INT8   Floor;      // Absolute height of floor
	byte  RoomAbove;  // 255 if none
	INT8   Ceiling;    // Absolute height of ceiling
};

struct tr5_room_light   // 88 bytes
{
	float x, y, z;       // Position of light, in world coordinates
	float r, g, b;       // Colour of the light

	__int32 Separator;    // Dummy value = 0xCDCDCDCD

		float In;            // Cosine of the IN value for light / size of IN value
	float Out;           // Cosine of the OUT value for light / size of OUT value
	float RadIn;         // (IN radians) * 2
	float RadOut;        // (OUT radians) * 2
	float Range;         // Range of light

	float dx, dy, dz;    // Direction - used only by sun and spot lights
	__int32 x2, y2, z2;    // Same as position, only in integer.
	__int32 dx2, dy2, dz2; // Same as direction, only in integer.

	byte LightType;

	byte Filler[3];     // Dummy values = 3 x 0xCD
};

struct GAMEFLOW
{
	unsigned int CheatEnabled : 1; // offset=0.0
	unsigned int LoadSaveEnabled : 1; // offset=0.1
	unsigned int TitleEnabled : 1; // offset=0.2
	unsigned int PlayAnyLevel : 1; // offset=0.3
	unsigned int Language : 3; // offset=0.4
	unsigned int DemoDisc : 1; // offset=0.7
	unsigned int Unused : 24; // offset=1.0
	unsigned int InputTimeout; // size=0, offset=4
	unsigned char SecurityTag; // size=0, offset=8
	unsigned char nLevels; // size=0, offset=9
	unsigned char nFileNames; // size=0, offset=10
	unsigned char Pad; // size=0, offset=11
	unsigned __int16 FileNameLen; // size=0, offset=12
	unsigned __int16 ScriptLen; // size=0, offset=14
};

typedef struct room_info_t {
	__int16* data; // size=0, offset=0
	__int16* door; // size=0, offset=4
	FLOOR_INFO* floor; // size=8, offset=8
	void* somePointer; // size=32, offset=12
	MESH_INFO* mesh; // size=20, offset=16
	__int32 x; // size=0, offset=20
	__int32 y; // size=0, offset=24
	__int32 z; // size=0, offset=28
	__int32 minfloor; // size=0, offset=32
	__int32 maxceiling; // size=0, offset=36
	__int16 xSize; // size=0, offset=42
	__int16 ySize; // size=0, offset=40
	CVECTOR ambient; // size=4, offset=44
	__int16 numLights; // size=0, offset=48
	__int16 numMeshes; // size=0, offset=50
	unsigned char reverbType; // size=0, offset=52
	unsigned char flipNumber; // size=0, offset=53
	byte meshEffect; // size=0, offset=54
	byte bound_active; // size=0, offset=55
	__int16 left; // size=0, offset=56
	__int16 right; // size=0, offset=58
	__int16 top; // size=0, offset=60
	__int16 bottom; // size=0, offset=62
	__int16 testLeft; // size=0, offset=64
	__int16 testRight; // size=0, offset=66
	__int16 testTop; // size=0, offset=68
	__int16 testBottom; // size=0, offset=70
	__int16 itemNumber; // size=0, offset=72
	__int16 fxNumber; // size=0, offset=74
	__int16 flippedRoom; // size=0, offset=76
	unsigned __int16 flags; // size=0, offset=78

	unsigned __int32 Unknown1;
	unsigned __int32 Unknown2;     // Always 0
	unsigned __int32 Unknown3;     // Always 0

	unsigned __int32 Separator;    // 0xCDCDCDCD

	unsigned __int16 Unknown4;
	unsigned __int16 Unknown5;

	float RoomX;
	float RoomY;
	float RoomZ;

	unsigned __int32 Separator1[4]; // Always 0xCDCDCDCD
	unsigned __int32 Separator2;    // 0 for normal rooms and 0xCDCDCDCD for null rooms
	unsigned __int32 Separator3;    // Always 0xCDCDCDCD

	unsigned __int32 NumRoomTriangles;
	unsigned __int32 NumRoomRectangles;

	tr5_room_light* light;     // Always 0

	unsigned __int32 LightDataSize;
	unsigned __int32 NumLights2;    // Always same as NumLights

	unsigned __int32 Unknown6;

	__int32 RoomYTop;
	__int32 RoomYBottom;

	unsigned __int32 NumLayers;

	tr5_room_layer* LayerOffset;
	tr5_room_vertex* VerticesOffset;
	void* PolyOffset;
	void* PolyOffset2;   // Same as PolyOffset

	__int32 NumVertices;

	__int32 Separator5[4];  // Always 0xCDCDCDCD

	/*tr5_room_light* Lights;    // Data for the lights (88 bytes * NumRoomLights)
	tr_room_sector* SectorList; // List of sectors in this room

	__int16 NumPortals;                 // Number of visibility portals to other rooms
	tr_room_portal Portals[NumPortals];  // List of visibility portals

	__int16 Separator;  // Always 0xCDCD

	tr3_room_staticmesh StaticMeshes[NumStaticMeshes];   // List of static meshes

	tr5_room_layer Layers[NumLayers]; // Data for the room layers (volumes) (56 bytes * NumLayers)

	uint8_t Faces[(NumRoomRectangles * sizeof(tr_face4) + NumRoomTriangles * sizeof(tr_face3)];

	tr5_room_vertex Vertices[NumVertices];*/
} ROOM_INFO;

typedef struct anim_struct_t
{
	__int16* framePtr; // size=0, offset=0
	__int16 interpolation; // size=0, offset=4
	__int16 currentAnimState; // size=0, offset=6
	__int32 velocity; // size=0, offset=8
	__int32 acceleration; // size=0, offset=12
	__int32 Xvelocity; // size=0, offset=16
	__int32 Xacceleration; // size=0, offset=20
	__int16 frameBase; // size=0, offset=24
	__int16 frameEnd; // size=0, offset=26
	__int16 jumpAnimNum; // size=0, offset=28
	__int16 jumpFrameNum; // size=0, offset=30
	__int16 numberChanges; // size=0, offset=32
	__int16 changeIndex; // size=0, offset=34
	__int16 numberCommands; // size=0, offset=36
	__int16 commandIndex; // size=0, offset=38
} ANIM_STRUCT;

typedef struct sparks_t
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 xVel; // size=0, offset=12
	__int16 yVel; // size=0, offset=14
	__int16 zVel; // size=0, offset=16
	__int16 gravity; // size=0, offset=18
	__int16 rotAng; // size=0, offset=20
	__int16 flags; // size=0, offset=22
	unsigned char sSize; // size=0, offset=24
	unsigned char dSize; // size=0, offset=25
	unsigned char size; // size=0, offset=26
	unsigned char friction; // size=0, offset=27
	unsigned char scalar; // size=0, offset=28
	unsigned char def; // size=0, offset=29
	byte rotAdd; // size=0, offset=30
	byte maxYvel; // size=0, offset=31
	unsigned char on; // size=0, offset=32
	unsigned char sR; // size=0, offset=33
	unsigned char sG; // size=0, offset=34
	unsigned char sB; // size=0, offset=35
	unsigned char dR; // size=0, offset=36
	unsigned char dG; // size=0, offset=37
	unsigned char dB; // size=0, offset=38
	unsigned char r; // size=0, offset=39
	unsigned char g; // size=0, offset=40
	unsigned char b; // size=0, offset=41
	unsigned char colFadeSpeed; // size=0, offset=42
	unsigned char fadeToBlack; // size=0, offset=43
	unsigned char sLife; // size=0, offset=44
	unsigned char life; // size=0, offset=45
	unsigned char transType; // size=0, offset=46
	unsigned char extras; // size=0, offset=47
	byte dynamic; // size=0, offset=48
	unsigned char fxObj; // size=0, offset=49
	unsigned char roomNumber; // size=0, offset=50
	unsigned char nodeNumber; // size=0, offset=51
} SPARKS;

typedef struct camera_info_t
{
	GAME_VECTOR pos; // size=16, offset=0
	GAME_VECTOR target; // size=16, offset=16
	CAMERA_TYPE type; // size=4, offset=32
	CAMERA_TYPE oldType; // size=4, offset=36
	__int32 shift; // size=0, offset=40
	__int32 flags; // size=0, offset=44
	__int32 fixedCamera; // size=0, offset=48
	__int32 numberFrames; // size=0, offset=52
	__int32 bounce; // size=0, offset=56
	__int32 underwater; // size=0, offset=60
	__int32 targetDistance; // size=0, offset=64
	__int16 targetAngle; // size=0, offset=68
	__int16 targetElevation; // size=0, offset=70
	__int16 actualElevation; // size=0, offset=72
	__int16 actualAngle; // size=0, offset=74
	__int16 laraNode; // size=0, offset=76
	__int16 box; // size=0, offset=78
	__int16 number; // size=0, offset=80
	__int16 last; // size=0, offset=82
	__int16 timer; // size=0, offset=84
	__int16 speed; // size=0, offset=86
	__int16 targetspeed; // size=0, offset=88
	ITEM_INFO* item; // size=144, offset=92
	ITEM_INFO* lastItem; // size=144, offset=96
	OBJECT_VECTOR* fixed; // size=16, offset=100
	__int32 mikeAtLara; // size=0, offset=104
	PHD_VECTOR mikePos; // size=12, offset=108
} CAMERA_INFO;

typedef struct static_info_t
{
	__int16 meshNumber;
	__int16 flags;
	__int16 xMinp;
	__int16 xMaxp;
	__int16 yMinp;
	__int16 yMaxp;
	__int16 zMinp;
	__int16 zMaxp;
	__int16 xMinc;
	__int16 xMaxc;
	__int16 yMinc;
	__int16 yMaxc;
	__int16 zMinc;
	__int16 zMaxc;
} STATIC_INFO;

typedef struct sample_info_t
{
	__int16 number;
	unsigned char volume;
	byte radius;
	byte randomness;
	signed char pitch;
	__int16 flags;
} SAMPLE_INFO;

typedef struct change_struct_t
{
	__int16 goalAnimState; // size=0, offset=0
	__int16 numberRanges; // size=0, offset=2
	__int16 rangeIndex; // size=0, offset=4
} CHANGE_STRUCT;

typedef struct range_struct_t
{
	__int16 startFrame; // size=0, offset=0
	__int16 endFrame; // size=0, offset=2
	__int16 linkAnimNum; // size=0, offset=4
	__int16 linkFrameNum; // size=0, offset=6
} RANGE_STRUCT;

struct tr_object_texture_vert // 4 bytes
{
	byte Xcoordinate; // 1 if Xpixel is the low value, 255 if Xpixel is the high value in the object texture
	byte Xpixel;
	byte Ycoordinate; // 1 if Ypixel is the low value, 255 if Ypixel is the high value in the object texture
	byte Ypixel;
};

struct tr4_object_texture // 38 bytes
{
	__int16               Attribute;
	__int16               TileAndFlag;
	__int16               NewFlags;

	tr_object_texture_vert Vertices[4]; // The four corners of the texture

	__int32               OriginalU;
	__int32               OriginalV;
	__int32               Width;     // Actually width-1
	__int32               Height;    // Actually height-1

	__int16 Padding;
};

typedef struct stats_t {
	unsigned __int32 Timer; // size=0, offset=0
	unsigned __int32 Distance; // size=0, offset=4
	unsigned __int32 AmmoUsed; // size=0, offset=8
	unsigned __int32 AmmoHits; // size=0, offset=12
	unsigned __int16 Kills; // size=0, offset=16
	unsigned char Secrets; // size=0, offset=18
	unsigned char HealthUsed; // size=0, offset=19
} STATS;

typedef struct savegame_info
{
	__int16 Checksum; // size=0, offset=0
	unsigned __int16 VolumeCD; // size=0, offset=2
	unsigned __int16 VolumeFX; // size=0, offset=4
	__int16 ScreenX; // size=0, offset=6
	__int16 ScreenY; // size=0, offset=8
	unsigned char ControlOption; // size=0, offset=10
	unsigned char VibrateOn; // size=0, offset=11
	unsigned char AutoTarget; // size=0, offset=12
	LARA_INFO Lara; // size=352, offset=16
	STATS Level; // size=20, offset=368
	STATS Game; // size=20, offset=388
	__int16 WeaponObject; // size=0, offset=408
	__int16 WeaponAnim; // size=0, offset=410
	__int16 WeaponFrame; // size=0, offset=412
	__int16 WeaponCurrent; // size=0, offset=414
	__int16 WeaponGoal; // size=0, offset=416
	unsigned __int32 CutSceneTriggered1; // size=0, offset=420
	unsigned __int32 CutSceneTriggered2; // size=0, offset=424
	byte GameComplete; // size=0, offset=428
	unsigned char LevelNumber; // size=0, offset=429
	unsigned char CampaignSecrets[4]; // size=4, offset=430
	unsigned char TLCount; // size=0, offset=434
} SAVEGAME_INFO;

struct OBJECT_TEXTURE_VERT
{
	float x;
	float y;
};

struct OBJECT_TEXTURE
{
	__int16 attribute;
	__int16 tileAndFlag;
	__int16 newFlags;
	struct OBJECT_TEXTURE_VERT vertices[4];
};

struct WINAPP
{
	HINSTANCE				hInstance;
	int		nFillMode;
	WNDCLASS WindowClass;
	HWND WindowHandle;
	bool bNoFocus;
	bool isInScene;
};

struct GUNFLASH_STRUCT
{
	MATRIX3D matrix; // size=32, offset=0
	__int16 on; // size=0, offset=32
};

struct SHOCKWAVE_STRUCT
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 innerRad; // size=0, offset=12
	__int16 outerRad; // size=0, offset=14
	__int16 xRot; // size=0, offset=16
	__int16 flags; // size=0, offset=18
	unsigned char r; // size=0, offset=20
	unsigned char g; // size=0, offset=21
	unsigned char b; // size=0, offset=22
	unsigned char life; // size=0, offset=23
	__int16 speed; // size=0, offset=24
	__int16 temp; // size=0, offset=26
};

struct GUNSHELL_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	__int16 fallspeed; // size=0, offset=20
	__int16 roomNumber; // size=0, offset=22
	__int16 speed; // size=0, offset=24
	__int16 counter; // size=0, offset=26
	__int16 dirXrot; // size=0, offset=28
	__int16 objectNumber; // size=0, offset=30
};

struct BUBBLE_STRUCT
{
	PHD_VECTOR pos; // size=12, offset=0
	__int16 roomNumber; // size=0, offset=12
	__int16 speed; // size=0, offset=14
	__int16 size; // size=0, offset=16
	__int16 dsize; // size=0, offset=18
	unsigned char shade; // size=0, offset=20
	unsigned char vel; // size=0, offset=21
	unsigned char yRot; // size=0, offset=22
	byte flags; // size=0, offset=23
	__int16 xVel; // size=0, offset=24
	__int16 yVel; // size=0, offset=26
	__int16 zVel; // size=0, offset=28
	__int16 pad; // size=0, offset=30
};

struct SPLASH_STRUCT
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 innerRad; // size=0, offset=12
	__int16 innerSize; // size=0, offset=14
	__int16 innerRadVel; // size=0, offset=16
	__int16 innerYVel; // size=0, offset=18
	__int16 innerY; // size=0, offset=20
	__int16 middleRad; // size=0, offset=22
	__int16 middleSize; // size=0, offset=24
	__int16 middleRadVel; // size=0, offset=26
	__int16 middleYVel; // size=0, offset=28
	__int16 middleY; // size=0, offset=30
	__int16 outerRad; // size=0, offset=32
	__int16 outerSize; // size=0, offset=34
	__int16 outerRadVel; // size=0, offset=36
	byte flags; // size=0, offset=38
	unsigned char life; // size=0, offset=39
};

struct DRIP_STRUCT
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	byte On; // size=0, offset=12
	byte R; // size=0, offset=13
	byte G; // size=0, offset=14
	byte B; // size=0, offset=15
	__int16 Yvel; // size=0, offset=16
	byte Gravity; // size=0, offset=18
	byte Life; // size=0, offset=19
	__int16 RoomNumber; // size=0, offset=20
	byte Outside; // size=0, offset=22
	byte Pad; // size=0, offset=23
};

struct RIPPLE_STRUCT
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	byte flags; // size=0, offset=12
	unsigned char life; // size=0, offset=13
	unsigned char size; // size=0, offset=14
	unsigned char init; // size=0, offset=15
};

struct SPLASH_SETUP
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 innerRad; // size=0, offset=12
	__int16 innerSize; // size=0, offset=14
	__int16 innerRadVel; // size=0, offset=16
	__int16 innerYVel; // size=0, offset=18
	__int16 pad1; // size=0, offset=20
	__int16 middleRad; // size=0, offset=22
	__int16 middleSize; // size=0, offset=24
	__int16 middleRadVel; // size=0, offset=26
	__int16 middleYVel; // size=0, offset=28
	__int16 pad2; // size=0, offset=30
	__int16 outerRad; // size=0, offset=32
	__int16 outerSize; // size=0, offset=34
	__int16 outerRadVel; // size=0, offset=36
	__int16 pad3; // size=0, offset=38
};

struct FIRE_LIST
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	byte on; // size=0, offset=12
	byte size; // size=0, offset=13
	__int16 room_number; // size=0, offset=14
};

struct FIRE_SPARKS
{
	__int16 x; // size=0, offset=0
	__int16 y; // size=0, offset=2
	__int16 z; // size=0, offset=4
	__int16 xVel; // size=0, offset=6
	__int16 yVel; // size=0, offset=8
	__int16 zVel; // size=0, offset=10
	__int16 gravity; // size=0, offset=12
	__int16 rotAng; // size=0, offset=14
	__int16 flags; // size=0, offset=16
	unsigned char sSize; // size=0, offset=18
	unsigned char dSize; // size=0, offset=19
	unsigned char size; // size=0, offset=20
	unsigned char friction; // size=0, offset=21
	unsigned char scalar; // size=0, offset=22
	unsigned char def; // size=0, offset=23
	byte rotAdd; // size=0, offset=24
	byte maxYvel; // size=0, offset=25
	unsigned char on; // size=0, offset=26
	unsigned char sR; // size=0, offset=27
	unsigned char sG; // size=0, offset=28
	unsigned char sB; // size=0, offset=29
	unsigned char dR; // size=0, offset=30
	unsigned char dG; // size=0, offset=31
	unsigned char dB; // size=0, offset=32
	unsigned char r; // size=0, offset=33
	unsigned char g; // size=0, offset=34
	unsigned char b; // size=0, offset=35
	unsigned char colFadeSpeed; // size=0, offset=36
	unsigned char fadeToBlack; // size=0, offset=37
	unsigned char sLife; // size=0, offset=38
	unsigned char life; // size=0, offset=39
};

struct SMOKE_SPARKS
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 Xvel; // size=0, offset=12
	__int16 Yvel; // size=0, offset=14
	__int16 Zvel; // size=0, offset=16
	__int16 Gravity; // size=0, offset=18
	__int16 RotAng; // size=0, offset=20
	__int16 Flags; // size=0, offset=22
	unsigned char sSize; // size=0, offset=24
	unsigned char dSize; // size=0, offset=25
	unsigned char Size; // size=0, offset=26
	unsigned char Friction; // size=0, offset=27
	unsigned char Scalar; // size=0, offset=28
	unsigned char Def; // size=0, offset=29
	byte RotAdd; // size=0, offset=30
	byte MaxYvel; // size=0, offset=31
	unsigned char On; // size=0, offset=32
	unsigned char sShade; // size=0, offset=33
	unsigned char dShade; // size=0, offset=34
	unsigned char Shade; // size=0, offset=35
	unsigned char ColFadeSpeed; // size=0, offset=36
	unsigned char FadeToBlack; // size=0, offset=37
	byte sLife; // size=0, offset=38
	byte Life; // size=0, offset=39
	unsigned char TransType; // size=0, offset=40
	unsigned char FxObj; // size=0, offset=41
	unsigned char NodeNumber; // size=0, offset=42
	unsigned char mirror; // size=0, offset=43
};

struct BLOOD_STRUCT
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int16 Xvel; // size=0, offset=12
	__int16 Yvel; // size=0, offset=14
	__int16 Zvel; // size=0, offset=16
	__int16 Gravity; // size=0, offset=18
	__int16 RotAng; // size=0, offset=20
	unsigned char sSize; // size=0, offset=22
	unsigned char dSize; // size=0, offset=23
	unsigned char Size; // size=0, offset=24
	unsigned char Friction; // size=0, offset=25
	byte RotAdd; // size=0, offset=26
	unsigned char On; // size=0, offset=27
	unsigned char sShade; // size=0, offset=28
	unsigned char dShade; // size=0, offset=29
	unsigned char Shade; // size=0, offset=30
	unsigned char ColFadeSpeed; // size=0, offset=31
	unsigned char FadeToBlack; // size=0, offset=32
	byte sLife; // size=0, offset=33
	byte Life; // size=0, offset=34
	byte Pad; // size=0, offset=35
};

typedef struct SPOTCAM
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	__int32 tx; // size=0, offset=12
	__int32 ty; // size=0, offset=16
	__int32 tz; // size=0, offset=20
	unsigned char sequence; // size=0, offset=24
	unsigned char camera; // size=0, offset=25
	__int16 fov; // size=0, offset=26
	__int16 roll; // size=0, offset=28
	__int16 timer; // size=0, offset=30
	__int16 speed; // size=0, offset=32
	__int16 flags; // size=0, offset=34
	__int16 roomNumber; // size=0, offset=36
	__int16 pad; // size=0, offset=38
};

typedef struct INVOBJ
{
	__int16 objectNumber;
	__int16 yOff;
	__int16 scale1;
	__int16 yRot;
	__int16 xRot;
	__int16 zRot;
	__int16 flags;
	__int16 objectName;
	__int32 meshBits;
};

typedef struct OBJLIST {
	__int16 inventoryItem;
	__int16 yRot;
	__int16 bright;
};

typedef struct COMBINELIST {
	__int32 combineRoutine;
	__int16 item1;
	__int16 item2;
	__int16 combinedItem;
};

typedef struct INVENTORYRING {
	OBJLIST currentObjectList[100];
	__int32 ringActive;
	__int32 objectListMovement;
	__int32 currentObjectInList;
	__int32 numObjectsInList;
};

typedef struct DISPLAY_PICKUP {
	__int16 life;
	__int16 objectNumber;
};

typedef struct DYNAMIC
{
	__int32 x; // size=0, offset=0
	__int32 y; // size=0, offset=4
	__int32 z; // size=0, offset=8
	byte on; // size=0, offset=12
	byte r; // size=0, offset=13
	byte g; // size=0, offset=14
	byte b; // size=0, offset=15
	__int16 falloff; // size=0, offset=16
	byte used; // size=0, offset=18
	byte pad1[1]; // size=1, offset=19
	__int32 FalloffScale; // size=0, offset=20
};

typedef struct SPRITE
{
	__int16 tile;
	byte x;
	byte y;
	__int16 width;
	__int16 height;
	float left;
	float top;
	float right;
	float bottom;
};

struct SHATTER_ITEM
{
	SPHERE sphere; // size=16, offset=0
	ITEM_LIGHT* il; // size=48, offset=16
	__int16* meshp; // size=0, offset=20
	__int32 bit; // size=0, offset=24
	__int16 yRot; // size=0, offset=28
	__int16 flags; // size=0, offset=30
};

struct WEAPON_INFO
{
	__int16 lockAngles[4]; // size=8, offset=0
	__int16 leftAngles[4]; // size=8, offset=8
	__int16 rightAngles[4]; // size=8, offset=16
	__int16 aimSpeed; // size=0, offset=24
	__int16 shotAccuracy; // size=0, offset=26
	__int16 gunHeight; // size=0, offset=28
	__int16 targetDist; // size=0, offset=30
	byte damage; // size=0, offset=32
	byte recoilFrame; // size=0, offset=33
	byte flashTime; // size=0, offset=34
	byte drawFrame; // size=0, offset=35
	__int16 sampleNum; // size=0, offset=36
};

struct ROPE_STRUCT
{
	PHD_VECTOR segment[24]; // size=288, offset=0
	PHD_VECTOR velocity[24]; // size=288, offset=288
	PHD_VECTOR normalisedSegment[24]; // size=288, offset=576
	PHD_VECTOR meshSegment[24]; // size=288, offset=864
	PHD_VECTOR position; // size=12, offset=1152
	PHD_VECTOR Unknown[24];
	__int32 segmentLength; // size=0, offset=1164
	__int16 active; // size=0, offset=1168
	__int16 coiled; // size=0, offset=1170
};

struct RAT_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	__int16 roomNumber; // size=0, offset=20
	__int16 speed; // size=0, offset=22
	__int16 fallspeed; // size=0, offset=24
	byte on; // size=0, offset=26
	byte flags; // size=0, offset=27
};

struct BAT_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	__int16 roomNumber; // size=0, offset=20
	__int16 speed; // size=0, offset=22
	__int16 counter; // size=0, offset=24
	__int16 laraTarget; // size=0, offset=26
	byte xTarget; // size=0, offset=28
	byte zTarget; // size=0, offset=29
	byte on; // size=0, offset=30
	byte flags; // size=0, offset=31
};

struct SPIDER_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	__int16 roomNumber; // size=0, offset=20
	__int16 speed; // size=0, offset=22
	__int16 fallspeed; // size=0, offset=24
	byte on; // size=0, offset=26
	byte flags; // size=0, offset=27
};

struct DEBRIS_STRUCT
{
	void* textInfo; // size=0, offset=0
	__int32 x; // size=0, offset=4
	__int32 y; // size=0, offset=8
	__int32 z; // size=0, offset=12
	__int16 XYZOffsets1[3]; // size=6, offset=16
	__int16 Dir; // size=0, offset=22
	__int16 XYZOffsets2[3]; // size=6, offset=24
	__int16 Speed; // size=0, offset=30
	__int16 XYZOffsets3[3]; // size=6, offset=32
	__int16 Yvel; // size=0, offset=38
	__int16 Gravity; // size=0, offset=40
	__int16 RoomNumber; // size=0, offset=42
	byte On; // size=0, offset=44
	byte XRot; // size=0, offset=45
	byte YRot; // size=0, offset=46
	byte r; // size=0, offset=47
	byte g; // size=0, offset=48
	byte b; // size=0, offset=49
	byte Pad[22]; // size=22, offset=50
};

typedef struct BOUNDING_BOX {
	__int16 X1;
	__int16 X2;
	__int16 Y1;
	__int16 Y2;
	__int16 Z1;
	__int16 Z2;
};

struct QUAKE_CAMERA {
	GAME_VECTOR spos;
	GAME_VECTOR epos;
};

struct DOORPOS_DATA
{
	FLOOR_INFO* floor; // size=8, offset=0
	FLOOR_INFO data; // size=8, offset=4
	__int16 block; // size=0, offset=12
};

struct DOOR_DATA
{
	DOORPOS_DATA d1; // size=16, offset=0
	DOORPOS_DATA d1flip; // size=16, offset=16
	DOORPOS_DATA d2; // size=16, offset=32
	DOORPOS_DATA d2flip; // size=16, offset=48
	__int16 opened; // size=0, offset=64
	__int16* dptr1; // size=0, offset=68
	__int16* dptr2; // size=0, offset=72
	__int16* dptr3; // size=0, offset=76
	__int16* dptr4; // size=0, offset=80
	byte dn1; // size=0, offset=84
	byte dn2; // size=0, offset=85
	byte dn3; // size=0, offset=86
	byte dn4; // size=0, offset=87
	ITEM_INFO* item; // size=144, offset=88
};

struct SUBSUIT_INFO
{
	short XRot; // size=0, offset=0
	short dXRot; // size=0, offset=2
	short XRotVel; // size=0, offset=4
	short Vel[2]; // size=4, offset=6
	short YVel; // size=0, offset=10
};

struct PISTOL_DEF {
	__int16 objectNum;
	char draw1Anim2;
	char draw1Anim;
	char draw2Anim;
	char recoilAnim;
};

typedef void (cdecl *EFFECT_ROUTINE)(ITEM_INFO*);
typedef void (cdecl *LARA_COLLISION_ROUTINE)(ITEM_INFO*, COLL_INFO*);
typedef void (cdecl *LARA_CONTROL_ROUTINE)(ITEM_INFO*, COLL_INFO*);

#pragma pack(pop)