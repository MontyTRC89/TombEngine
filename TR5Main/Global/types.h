#pragma once
#include <Windows.h>
#include "enums.h"

#include <fixed_point.h>
#pragma pack(push, 1)
typedef enum TYPE_ZONE
{
	ZONE_NULL = -1,
	ZONE_SKELLY = 0,
	ZONE_BASIC,
	ZONE_FLYER,
	ZONE_HUMAN_CLASSIC,
	ZONE_WATER,
	ZONE_HUMAN_JUMP_AND_MONKEY,
	ZONE_HUMAN_JUMP,
	ZONE_SPIDER,
};

typedef struct vector_t
{
	int vx;
	int vy;
	int vz;
	int pad;
} VECTOR;

typedef struct SPHERE
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	int r; // size=0, offset=12
};

typedef struct svector_t
{
	short vx;
	short vy;
	short vz;
	short pad;
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
	int x;
	int y;
	int z;
} PHD_VECTOR;

typedef struct tr_vertex
{
	int x;
	int y;
	int z;
};

struct MATRIX3D
{
	short m00; // size=0, offset=0
	short m01; // size=0, offset=2
	short m02; // size=0, offset=4
	short m10; // size=0, offset=6
	short m11; // size=0, offset=8
	short m12; // size=0, offset=10
	short m20; // size=0, offset=12
	short m21; // size=0, offset=14
	short m22; // size=0, offset=16
	short pad; // size=0, offset=18
	int tx; // size=0, offset=20
	int ty; // size=0, offset=24
	int tz; // size=0, offset=28
};

typedef struct phd_3dpos_t
{
	int xPos; // off 0 [64]
	int yPos; // off 4 [68]
	int zPos; // off 8 [72]
	short xRot; // off 12 [76]
	short yRot; // off 14 [78]
	short zRot; // off 16 [80]
} PHD_3DPOS;

typedef struct game_vector_t
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	short roomNumber; // size=0, offset=12
	short boxNumber; // size=0, offset=14
} GAME_VECTOR;

typedef struct object_vector
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	short data; // size=0, offset=12
	short flags; // size=0, offset=14
} OBJECT_VECTOR;

typedef struct ilight_t
{
	short x;
	short y;
	short z;
	short pad1;
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
	//PHD_VECTOR hvel;
	byte unknown[24];
} HAIR_STRUCT;

typedef struct box_node_t
{
	short exitBox; // size=0, offset=0
	unsigned short searchNumber; // size=0, offset=2
	short nextExpansion; // size=0, offset=4
	short boxNumber; // size=0, offset=6
} BOX_NODE;

typedef struct box_info_t
{
	unsigned char left; // size=0, offset=0
	unsigned char right; // size=0, offset=1
	unsigned char top; // size=0, offset=2
	unsigned char bottom; // size=0, offset=3
	short height; // size=0, offset=4
	short overlapIndex; // size=0, offset=6
} BOX_INFO;

typedef struct ai_info_t
{
	short zoneNumber; // size=0, offset=0
	short enemyZone; // size=0, offset=2
	int distance; // size=0, offset=4
	int ahead; // size=0, offset=8
	int bite; // size=0, offset=12
	short angle; // size=0, offset=16
	short xAngle; // size=0, offset=18
	short enemyFacing; // size=0, offset=20
} AI_INFO;

typedef struct bite_info_t {				// Offset into given Mesh
	int	x;					// where Baddie kicks off Bite Effect
	int	y;
	int	z;
	int	meshNum;
} BITE_INFO;

typedef struct lot_info_t
{
	BOX_NODE* node; // size=8, offset=0
	short head; // size=0, offset=4
	short tail; // size=0, offset=6
	unsigned short searchNumber; // size=0, offset=8
	unsigned short blockMask; // size=0, offset=10
	short step; // size=0, offset=12
	short drop; // size=0, offset=14
	short zoneCount; // size=0, offset=16
	short targetBox; // size=0, offset=18
	short requiredBox; // size=0, offset=20
	short fly; // size=0, offset=22
	unsigned short canJump : 1; // offset=24.0
	unsigned short canMonkey : 1; // offset=24.1
	unsigned short isAmphibious : 1; // offset=24.2
	unsigned short isJumping : 1; // offset=24.3
	unsigned short isMonkeying : 1; // offset=24.4
	PHD_VECTOR target; // size=12, offset=26
	int zone; // size=4, offset=40
} LOT_INFO;

typedef struct floor_info_t {
	unsigned short index; // size=0, offset=0
	unsigned short fx : 4; // offset=2.0
	unsigned short box : 11; // offset=2.4
	unsigned short stopper : 1; // offset=3.7
	unsigned char pitRoom; // size=0, offset=4
	signed char floor; // size=0, offset=5
	unsigned char skyRoom; // size=0, offset=6
	signed char ceiling; // size=0, offset=7
} FLOOR_INFO;

typedef struct item_info_t {
	int floor; // size=0, offset=0
	int touchBits; // size=0, offset=4
	int meshBits; // size=0, offset=8
	short objectNumber; // size=0, offset=12
	short currentAnimState; // size=0, offset=14
	short goalAnimState; // size=0, offset=16
	short requiredAnimState; // size=0, offset=18
	short animNumber; // size=0, offset=20
	short frameNumber; // size=0, offset=22
	short roomNumber; // size=0, offset=24
	short nextItem; // size=0, offset=26
	short nextActive; // size=0, offset=28
	short speed; // size=0, offset=30
	short fallspeed; // size=0, offset=32
	short hitPoints; // size=0, offset=34
	unsigned short boxNumber; // size=0, offset=36
	short timer; // size=0, offset=38
	short flags; // size=0, offset=40
	short shade; // size=0, offset=42
	short triggerFlags; // size=0, offset=44
	short carriedItem; // size=0, offset=46
	short afterDeath; // size=0, offset=48
	unsigned short firedWeapon; // size=0, offset=50
	short itemFlags[4]; // size=8, offset=52
	void* data; // size=0, offset=60
	PHD_3DPOS pos; // size=20, offset=64
	//ITEM_LIGHT il; // size=48, offset=84
	byte padLight[48];
	unsigned int meshswapMeshbits; // size=0, offset=136 OFF=132
	short drawRoom; // size=0, offset=140 OFF=136
	short TOSSPAD; // size=0, offset=142 OFF=138
	byte pad1[5464]; // OFF=140   5432?
	byte* pointer1;
	byte* pointer2;
	//short status;
	//byte pad2[8];
	unsigned int active : 1; // offset=132.0 OFF=5610
	unsigned int status : 2; // offset=132.1
	unsigned int gravityStatus : 1; // offset=132.3
	unsigned int hitStatus : 1; // offset=132.4
	unsigned int collidable : 1; // offset=132.5
	unsigned int lookedAt : 1; // offset=132.6
	unsigned int dynamicLight : 1; // offset=132.7
	unsigned int poisoned : 1; // offset=133.0
	unsigned int aiBits : 5; // offset=133.1
	unsigned int reallyActive : 1; // offset=133.6
	unsigned int InDrawRoom : 1; // offset=133.7
	int swapMeshFlags;
	byte pad2[4]; // OFF=5614
} ITEM_INFO;

typedef struct creature_info_t 
{
	short jointRotation[4]; // size=8, offset=0
	short maximumTurn; // size=0, offset=8
	short flags; // size=0, offset=10
	unsigned short alerted : 1; // offset=12.0
	unsigned short headLeft : 1; // offset=12.1
	unsigned short headRight : 1; // offset=12.2
	unsigned short reachedGoal : 1; // offset=12.3
	unsigned short hurtByLara : 1; // offset=12.4
	unsigned short patrol2 : 1; // offset=12.5
	unsigned short jumpAhead : 1; // offset=12.6
	unsigned short monkeyAhead : 1; // offset=12.7
	MOOD_TYPE mood; // size=4, offset=14
	ITEM_INFO* enemy; // size=144, offset=18
	ITEM_INFO aiTarget; // size=144, offset=22
	short pad; // size=0, offset=5644
	short itemNum; // size=0, offset=5644
	PHD_VECTOR target; // size=12, offset=5646
	LOT_INFO LOT; // size=44, offset=5658
} CREATURE_INFO;

typedef struct lara_arm_t
{
	short* frameBase; // size=0, offset=0
	short frameNumber; // size=0, offset=4
	short animNumber; // size=0, offset=6
	short lock; // size=0, offset=8
	short yRot; // size=0, offset=10
	short xRot; // size=0, offset=12
	short zRot; // size=0, offset=14
	short flash_gun; // size=0, offset=16
} LARA_ARM;

typedef struct fx_info_t
{
	PHD_3DPOS pos; // size=20, offset=0
	short roomNumber; // size=0, offset=20
	short objectNumber; // size=0, offset=22
	short nextFx; // size=0, offset=24
	short nextActive; // size=0, offset=26
	short speed; // size=0, offset=28
	short fallspeed; // size=0, offset=30
	short frameNumber; // size=0, offset=32
	short counter; // size=0, offset=34
	short shade; // size=0, offset=36
	short flag1; // size=0, offset=38
	short flag2; // size=0, offset=40
} FX_INFO;

typedef struct lara_info_t {
	short itemNumber; // size=0, offset=0
	short gunStatus; // size=0, offset=2
	short gunType; // size=0, offset=4
	short requestGunType; // size=0, offset=6
	short lastGunType; // size=0, offset=8
	short calcFallSpeed; // size=0, offset=10
	short waterStatus; // size=0, offset=12
	short climbStatus; // size=0, offset=14
	short poseCount; // size=0, offset=16
	short hitFrame; // size=0, offset=18
	short hitDirection; // size=0, offset=20
	short air; // size=0, offset=22
	short diveCount; // size=0, offset=24
	short deathCount; // size=0, offset=26
	short currentActive; // size=0, offset=28
	short currentXvel; // size=0, offset=30
	short currentYvel; // size=0, offset=32
	short currentZvel; // size=0, offset=34
	short spazEffectCount; // size=0, offset=36
	short flareAge; // size=0, offset=38
	short BurnCount; // size=0, offset=40
	short weaponItem; // size=0, offset=42
	short backGun; // size=0, offset=44
	short flareFrame; // size=0, offset=46
	short poisoned; // size=0, offset=48
	short dpoisoned; // size=0, offset=50
	unsigned char anxiety; // size=0, offset=52
	unsigned char wet[15]; // size=15, offset=53
	unsigned short flareControlLeft : 1; // offset=68.0
	unsigned short unused1 : 1; // offset=68.1
	unsigned short look : 1; // offset=68.2
	unsigned short burn : 1; // offset=68.3
	unsigned short keepDucked : 1; // offset=68.4
	unsigned short isMoving : 1; // offset=68.5
	unsigned short canMonkeySwing : 1; // offset=68.6
	unsigned short burnBlue : 2; // offset=68.7
	unsigned short gassed : 1; // offset=69.1
	unsigned short burnSmoke : 1; // offset=69.2
	unsigned short isDucked : 1; // offset=69.3
	unsigned short hasFired : 1; // offset=69.4
	unsigned short busy : 1; // offset=69.5
	unsigned short litTorch : 1; // offset=69.6
	unsigned short isClimbing : 1; // offset=69.7
	unsigned short fired : 1; // offset=70.0
	int waterSurfaceDist; // size=0, offset=72
	PHD_VECTOR lastPos; // size=12, offset=76
	FX_INFO* spazEffect; // size=44, offset=88
	int meshEffects; // size=0, offset=92
	short* meshPtrs[15]; // size=60, offset=96
	ITEM_INFO* target; // size=144, offset=156
	short targetAngles[2]; // size=4, offset=160
	short turnRate; // size=0, offset=164
	short moveAngle; // size=0, offset=166
	short headYrot; // size=0, offset=168
	short headXrot; // size=0, offset=170
	short headZrot; // size=0, offset=172
	short torsoYrot; // size=0, offset=174
	short torsoXrot; // size=0, offset=176
	short torsoZrot; // size=0, offset=178
	LARA_ARM leftArm; // size=20, offset=180
	LARA_ARM rightArm; // size=20, offset=200
	unsigned short holster; // size=0, offset=220
	CREATURE_INFO* creature; // size=228, offset=224
	int cornerX; // size=0, offset=228
	int cornerZ; // size=0, offset=232
	byte ropeSegment; // size=0, offset=236
	byte ropeDirection; // size=0, offset=237
	short ropeArcFront; // size=0, offset=238
	short ropeArcBack; // size=0, offset=240
	short ropeLastX; // size=0, offset=242
	short ropeMaxXForward; // size=0, offset=244
	short ropeMaxXBackward; // size=0, offset=246
	int ropeDFrame; // size=0, offset=248
	int ropeFrame; // size=0, offset=252
	unsigned short ropeFrameRate; // size=0, offset=256
	unsigned short ropeY; // size=0, offset=258
	int ropePtr; // size=0, offset=260
	void* generalPtr; // size=0, offset=264
	int ropeOffset; // size=0, offset=268
	int ropeDownVel; // size=0, offset=272
	byte ropeFlag; // size=0, offset=276
	byte moveCount; // size=0, offset=277
	int ropeCount; // size=0, offset=280
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
	unsigned short puzzleItemsCombo; // size=0, offset=312
	unsigned short keyItems; // size=0, offset=314
	unsigned short keyItemsCombo; // size=0, offset=316
	unsigned short pickupItems; // size=0, offset=318
	unsigned short pickupItemsCombo; // size=0, offset=320
	short numSmallMedipack; // size=0, offset=322
	short numLargeMedipack; // size=0, offset=324
	short numFlares; // size=0, offset=326
	short numPistolsAmmo; // size=0, offset=328
	short numUziAmmo; // size=0, offset=330
	short numRevolverAmmo; // size=0, offset=332
	short numShotgunAmmo1; // size=0, offset=334
	short numShotgunAmmo2; // size=0, offset=336
	short numHKammo1; // size=0, offset=338
	short numCrossbowAmmo1; // size=0, offset=340
	short numCrossbowAmmo2; // size=0, offset=342
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
	int midFloor; // size=0, offset=0
	int midCeiling; // size=0, offset=4
	int midType; // size=0, offset=8
	int frontFloor; // size=0, offset=12
	int frontCeiling; // size=0, offset=16
	int frontType; // size=0, offset=20
	int leftFloor; // size=0, offset=24
	int leftCeiling; // size=0, offset=28
	int leftType; // size=0, offset=32
	int rightFloor; // size=0, offset=36
	int rightCeiling; // size=0, offset=40
	int rightType; // size=0, offset=44
	int leftFloor2; // size=0, offset=48
	int leftCeiling2; // size=0, offset=52
	int leftType2; // size=0, offset=56
	int rightFloor2; // size=0, offset=60
	int rightCeiling2; // size=0, offset=64
	int rightType2; // size=0, offset=68
	int radius; // size=0, offset=72
	int badPos; // size=0, offset=76
	int badNeg; // size=0, offset=80
	int badCeiling; // size=0, offset=84
	PHD_VECTOR shift; // size=12, offset=88
	PHD_VECTOR old; // size=12, offset=100
	short oldAnimState; // size=0, offset=112
	short oldAnimNumber; // size=0, offset=114
	short oldFrameNumber; // size=0, offset=116
	short facing; // size=0, offset=118
	short quadrant; // size=0, offset=120
	short collType; // size=0, offset=122 USE ENUM CT_*
	short* trigger; // size=0, offset=124
	signed char tiltX; // size=0, offset=128
	signed char tiltZ; // size=0, offset=129
	byte hitByBaddie; // size=0, offset=130
	byte hitStatic; // size=0, offset=131
	unsigned short slopesAreWalls : 2; // offset=132.0
	unsigned short slopesArePits : 1; // offset=132.2
	unsigned short lavaIsPit : 1; // offset=132.3
	unsigned short enableBaddiePush : 1; // offset=132.4
	unsigned short enableSpaz : 1; // offset=132.5
	unsigned short hitCeiling : 1; // offset=132.6
} COLL_INFO;

typedef struct aiobject_t
{
	short objectNumber; // size=0, offset=0
	short roomNumber; // size=0, offset=2
	int x; // size=0, offset=4
	int y; // size=0, offset=8
	int z; // size=0, offset=12
	short triggerFlags; // size=0, offset=16
	short flags; // size=0, offset=18
	short yRot; // size=0, offset=20
	short boxNumber; // size=0, offset=22
} AIOBJECT;

typedef struct object_info_t {
	short nmeshes; // size=0, offset=0
	short meshIndex; // size=0, offset=2
	int boneIndex; // size=0, offset=4
	short* frameBase; // size=0, offset=8
	void(*initialise)(short itemNumber); // size=0, offset=12
	void(*control)(short itemNumber); // size=0, offset=16
	void(*floor)(ITEM_INFO* item, int x, int y, int z, int* height); // size=0, offset=20
	void(*ceiling)(ITEM_INFO* item, int x, int y, int z, int* height); // size=0, offset=24
	void(*drawRoutine)(ITEM_INFO* item); // size=0, offset=28
	void(*collision)(short item_num, ITEM_INFO* laraitem, COLL_INFO* coll); // size=0, offset=32
	short zoneType; // size=0, offset=36
	short animIndex; // size=0, offset=38
	short hitPoints; // size=0, offset=40
	short pivotLength; // size=0, offset=42
	short radius; // size=0, offset=44
	short shadowSize; // size=0, offset=46
	unsigned short biteOffset; // size=0, offset=48
	unsigned short loaded : 1; // offset=50.0
	unsigned short intelligent : 1; // offset=50.1
	unsigned short nonLot : 1; // offset=50.2
	unsigned short savePosition : 1; // offset=50.3
	unsigned short saveHitpoints : 1; // offset=50.4
	unsigned short saveFlags : 1; // offset=50.5
	unsigned short saveAnim : 1; // offset=50.6
	unsigned short semiTransparent : 1; // offset=50.7
	unsigned short waterCreature : 1; // offset=51.0
	unsigned short usingDrawAnimatingItem : 1; // offset=51.1
	unsigned short hitEffect : 2; // offset=51.2
	unsigned short undead : 1; // offset=51.4
	unsigned short saveMesh : 1; // offset=51.5
	void(*drawRoutineExtra)(ITEM_INFO* item); // size=0, offset=52
	unsigned int explodableMeshbits; // size=0, offset=56
	unsigned int padfuck; // size=0, offset=60
} OBJECT_INFO;

typedef struct tr5_room_layer_t   // 56 bytes
{
	unsigned int NumLayerVertices;   // Number of vertices in this layer (4 bytes)
	unsigned short UnknownL1;
	unsigned short NumLayerRectangles; // Number of rectangles in this layer (2 bytes)
	unsigned short NumLayerTriangles;  // Number of triangles in this layer (2 bytes)
	unsigned short UnknownL2;

	unsigned short Filler;             // Always 0
	unsigned short Filler2;            // Always 0

								 // The following 6 floats define the bounding box for the layer

	float    LayerBoundingBoxX1;
	float    LayerBoundingBoxY1;
	float    LayerBoundingBoxZ1;
	float    LayerBoundingBoxX2;
	float    LayerBoundingBoxY2;
	float    LayerBoundingBoxZ2;

	unsigned int Filler3;     // Always 0 (4 bytes)
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
	unsigned int Colour;     // 32-bit colour
} tr5_room_vertex;

typedef struct mesh_info_t
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	short yRot; // size=0, offset=12
	short shade; // size=0, offset=14
	short Flags; // size=0, offset=16
	short staticNumber; // size=0, offset=18
} MESH_INFO;

typedef struct light_info_t
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	unsigned char Type; // size=0, offset=12
	unsigned char r; // size=0, offset=13
	unsigned char g; // size=0, offset=14
	unsigned char b; // size=0, offset=15
	short nx; // size=0, offset=16
	short ny; // size=0, offset=18
	short nz; // size=0, offset=20
	short Intensity; // size=0, offset=22
	unsigned char Inner; // size=0, offset=24
	unsigned char Outer; // size=0, offset=25
	short FalloffScale; // size=0, offset=26
	short Length; // size=0, offset=28
	short Cutoff; // size=0, offset=30
} LIGHTINFO;

struct tr4_mesh_face3    // 10 bytes
{
	short Vertices[3];
	short Texture;
	short Effects;    // TR4-5 ONLY: alpha blending and environment mapping strength
};

struct tr4_mesh_face4    // 12 bytes
{
	short Vertices[4];
	short Texture;
	short Effects;
};

struct tr_room_portal  // 32 bytes
{
	short  AdjoiningRoom; // Which room this portal leads to
	tr_vertex Normal;
	tr_vertex Vertices[4];
};

struct tr_room_sector // 8 bytes
{
	unsigned short FDindex;    // Index into FloorData[]
	unsigned short BoxIndex;   // Index into Boxes[] (-1 if none)
	byte  RoomBelow;  // 255 is none
	INT8   Floor;      // Absolute height of floor
	byte  RoomAbove;  // 255 if none
	INT8   Ceiling;    // Absolute height of ceiling
};

struct tr5_room_light   // 88 bytes
{
	float x, y, z;       // Position of light, in world coordinates
	float r, g, b;       // Colour of the light

	int Separator;    // Dummy value = 0xCDCDCDCD

		float In;            // Cosine of the IN value for light / size of IN value
	float Out;           // Cosine of the OUT value for light / size of OUT value
	float RadIn;         // (IN radians) * 2
	float RadOut;        // (OUT radians) * 2
	float Range;         // Range of light

	float dx, dy, dz;    // Direction - used only by sun and spot lights
	int x2, y2, z2;    // Same as position, only in integer.
	int dx2, dy2, dz2; // Same as direction, only in integer.

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
	unsigned short FileNameLen; // size=0, offset=12
	unsigned short ScriptLen; // size=0, offset=14
};

typedef struct room_info_t {
	short* data; // size=0, offset=0
	short* door; // size=0, offset=4
	FLOOR_INFO* floor; // size=8, offset=8
	void* somePointer; // size=32, offset=12
	MESH_INFO* mesh; // size=20, offset=16
	int x; // size=0, offset=20
	int y; // size=0, offset=24
	int z; // size=0, offset=28
	int minfloor; // size=0, offset=32
	int maxceiling; // size=0, offset=36
	short xSize; // size=0, offset=42
	short ySize; // size=0, offset=40
	CVECTOR ambient; // size=4, offset=44
	short numLights; // size=0, offset=48
	short numMeshes; // size=0, offset=50
	unsigned char reverbType; // size=0, offset=52
	unsigned char flipNumber; // size=0, offset=53
	byte meshEffect; // size=0, offset=54
	byte bound_active; // size=0, offset=55
	short left; // size=0, offset=56
	short right; // size=0, offset=58
	short top; // size=0, offset=60
	short bottom; // size=0, offset=62
	short testLeft; // size=0, offset=64
	short testRight; // size=0, offset=66
	short testTop; // size=0, offset=68
	short testBottom; // size=0, offset=70
	short itemNumber; // size=0, offset=72
	short fxNumber; // size=0, offset=74
	short flippedRoom; // size=0, offset=76
	unsigned short flags; // size=0, offset=78

	unsigned int Unknown1;
	unsigned int Unknown2;     // Always 0
	unsigned int Unknown3;     // Always 0

	unsigned int Separator;    // 0xCDCDCDCD

	unsigned short Unknown4;
	unsigned short Unknown5;

	float RoomX;
	float RoomY;
	float RoomZ;

	unsigned int Separator1[4]; // Always 0xCDCDCDCD
	unsigned int Separator2;    // 0 for normal rooms and 0xCDCDCDCD for null rooms
	unsigned int Separator3;    // Always 0xCDCDCDCD

	unsigned int NumRoomTriangles;
	unsigned int NumRoomRectangles;

	tr5_room_light* light;     // Always 0

	unsigned int LightDataSize;
	unsigned int NumLights2;    // Always same as NumLights

	unsigned int Unknown6;

	int RoomYTop;
	int RoomYBottom;

	unsigned int NumLayers;

	tr5_room_layer* LayerOffset;
	tr5_room_vertex* VerticesOffset;
	void* PolyOffset;
	void* PolyOffset2;   // Same as PolyOffset

	int NumVertices;

	int Separator5[4];  // Always 0xCDCDCDCD

	/*tr5_room_light* Lights;    // Data for the lights (88 bytes * NumRoomLights)
	tr_room_sector* SectorList; // List of sectors in this room

	short NumPortals;                 // Number of visibility portals to other rooms
	tr_room_portal Portals[NumPortals];  // List of visibility portals

	short Separator;  // Always 0xCDCD

	tr3_room_staticmesh StaticMeshes[NumStaticMeshes];   // List of static meshes

	tr5_room_layer Layers[NumLayers]; // Data for the room layers (volumes) (56 bytes * NumLayers)

	uint8_t Faces[(NumRoomRectangles * sizeof(tr_face4) + NumRoomTriangles * sizeof(tr_face3)];

	tr5_room_vertex Vertices[NumVertices];*/
} ROOM_INFO;

typedef struct anim_struct_t
{
	short* framePtr; // size=0, offset=0
	short interpolation; // size=0, offset=4
	short currentAnimState; // size=0, offset=6
	int velocity; // size=0, offset=8
	int acceleration; // size=0, offset=12
	int Xvelocity; // size=0, offset=16
	int Xacceleration; // size=0, offset=20
	short frameBase; // size=0, offset=24
	short frameEnd; // size=0, offset=26
	short jumpAnimNum; // size=0, offset=28
	short jumpFrameNum; // size=0, offset=30
	short numberChanges; // size=0, offset=32
	short changeIndex; // size=0, offset=34
	short numberCommands; // size=0, offset=36
	short commandIndex; // size=0, offset=38
} ANIM_STRUCT;

typedef struct sparks_t
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	short xVel; // size=0, offset=12
	short yVel; // size=0, offset=14
	short zVel; // size=0, offset=16
	short gravity; // size=0, offset=18
	short rotAng; // size=0, offset=20
	short flags; // size=0, offset=22
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
	int shift; // size=0, offset=40
	int flags; // size=0, offset=44
	int fixedCamera; // size=0, offset=48
	int numberFrames; // size=0, offset=52
	int bounce; // size=0, offset=56
	int underwater; // size=0, offset=60
	int targetDistance; // size=0, offset=64
	short targetAngle; // size=0, offset=68
	short targetElevation; // size=0, offset=70
	short actualElevation; // size=0, offset=72
	short actualAngle; // size=0, offset=74
	short laraNode; // size=0, offset=76
	short box; // size=0, offset=78
	short number; // size=0, offset=80
	short last; // size=0, offset=82
	short timer; // size=0, offset=84
	short speed; // size=0, offset=86
	short targetspeed; // size=0, offset=88
	ITEM_INFO* item; // size=144, offset=92
	ITEM_INFO* lastItem; // size=144, offset=96
	OBJECT_VECTOR* fixed; // size=16, offset=100
	int mikeAtLara; // size=0, offset=104
	PHD_VECTOR mikePos; // size=12, offset=108
} CAMERA_INFO;

typedef struct static_info_t
{
	short meshNumber;
	short flags;
	short xMinp;
	short xMaxp;
	short yMinp;
	short yMaxp;
	short zMinp;
	short zMaxp;
	short xMinc;
	short xMaxc;
	short yMinc;
	short yMaxc;
	short zMinc;
	short zMaxc;
} STATIC_INFO;

typedef struct sample_info_t
{
	short number;
	unsigned char volume;
	byte radius;
	byte randomness;
	signed char pitch;
	short flags;
} SAMPLE_INFO;

typedef struct change_struct_t
{
	short goalAnimState; // size=0, offset=0
	short numberRanges; // size=0, offset=2
	short rangeIndex; // size=0, offset=4
} CHANGE_STRUCT;

typedef struct range_struct_t
{
	short startFrame; // size=0, offset=0
	short endFrame; // size=0, offset=2
	short linkAnimNum; // size=0, offset=4
	short linkFrameNum; // size=0, offset=6
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
	short               Attribute;
	short               TileAndFlag;
	short               NewFlags;

	tr_object_texture_vert Vertices[4]; // The four corners of the texture

	int               OriginalU;
	int               OriginalV;
	int               Width;     // Actually width-1
	int               Height;    // Actually height-1

	short Padding;
};

typedef struct stats_t {
	unsigned int Timer; // size=0, offset=0
	unsigned int Distance; // size=0, offset=4
	unsigned int AmmoUsed; // size=0, offset=8
	unsigned int AmmoHits; // size=0, offset=12
	unsigned short Kills; // size=0, offset=16
	unsigned char Secrets; // size=0, offset=18
	unsigned char HealthUsed; // size=0, offset=19
} STATS;

typedef struct savegame_info
{
	short Checksum; // size=0, offset=0
	unsigned short VolumeCD; // size=0, offset=2
	unsigned short VolumeFX; // size=0, offset=4
	short ScreenX; // size=0, offset=6
	short ScreenY; // size=0, offset=8
	unsigned char ControlOption; // size=0, offset=10
	unsigned char VibrateOn; // size=0, offset=11
	unsigned char AutoTarget; // size=0, offset=12
	LARA_INFO Lara; // size=352, offset=16
	STATS Level; // size=20, offset=368
	STATS Game; // size=20, offset=388
	short WeaponObject; // size=0, offset=408
	short WeaponAnim; // size=0, offset=410
	short WeaponFrame; // size=0, offset=412
	short WeaponCurrent; // size=0, offset=414
	short WeaponGoal; // size=0, offset=416
	unsigned int CutSceneTriggered1; // size=0, offset=420
	unsigned int CutSceneTriggered2; // size=0, offset=424
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
	short attribute;
	short tileAndFlag;
	short newFlags;
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
	short on; // size=0, offset=32
};

struct SHOCKWAVE_STRUCT
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	short innerRad; // size=0, offset=12
	short outerRad; // size=0, offset=14
	short xRot; // size=0, offset=16
	short flags; // size=0, offset=18
	unsigned char r; // size=0, offset=20
	unsigned char g; // size=0, offset=21
	unsigned char b; // size=0, offset=22
	unsigned char life; // size=0, offset=23
	short speed; // size=0, offset=24
	short temp; // size=0, offset=26
};

struct GUNSHELL_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	short fallspeed; // size=0, offset=20
	short roomNumber; // size=0, offset=22
	short speed; // size=0, offset=24
	short counter; // size=0, offset=26
	short dirXrot; // size=0, offset=28
	short objectNumber; // size=0, offset=30
};

struct BUBBLE_STRUCT
{
	PHD_VECTOR pos; // size=12, offset=0
	short roomNumber; // size=0, offset=12
	numeric::Fixed<9, 7> speed; // size=0, offset=14
	short size; // size=0, offset=16
	short dsize; // size=0, offset=18
	unsigned char shade; // size=0, offset=20
	numeric::Fixed<4, 4> vel; // size=0, offset=21
	unsigned char yRot; // size=0, offset=22
	byte flags; // size=0, offset=23
	numeric::Fixed<6,2> xVel; // size=0, offset=24
	numeric::Fixed<6, 2> yVel; // size=0, offset=26
	numeric::Fixed<6, 2> zVel; // size=0, offset=28
	short pad; // size=0, offset=30
};

struct SPLASH_STRUCT
{
	float x;
	float y;
	float z;
	float innerRad;
	float innerRadVel;
	float heightVel;
	float heightSpeed;
	float height;
	float outerRad;
	float outerRadVel;
	float animationSpeed;
	float animationPhase;
	short spriteSequenceStart;
	short spriteSequenceEnd;
	unsigned short life;
	bool isRipple;
	bool isActive;
};

struct DRIP_STRUCT
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	byte on; // size=0, offset=12
	byte r; // size=0, offset=13
	byte g; // size=0, offset=14
	byte b; // size=0, offset=15
	short yVel; // size=0, offset=16
	byte gravity; // size=0, offset=18
	byte life; // size=0, offset=19
	short roomNumber; // size=0, offset=20
	byte outside; // size=0, offset=22
	byte pad; // size=0, offset=23
};

struct RIPPLE_STRUCT
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	byte flags; // size=0, offset=12
	unsigned char life; // size=0, offset=13
	unsigned char size; // size=0, offset=14
	unsigned char init; // size=0, offset=15
};

struct SPLASH_SETUP
{
	float x;
	float y;
	float z;
	float splashPower;
	float innerRadius;

};

struct FIRE_LIST
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	byte on; // size=0, offset=12
	byte size; // size=0, offset=13
	short roomNumber; // size=0, offset=14
};

struct FIRE_SPARKS
{
	short x; // size=0, offset=0
	short y; // size=0, offset=2
	short z; // size=0, offset=4
	short xVel; // size=0, offset=6
	short yVel; // size=0, offset=8
	short zVel; // size=0, offset=10
	short gravity; // size=0, offset=12
	short rotAng; // size=0, offset=14
	short flags; // size=0, offset=16
	unsigned char sSize; // size=0, offset=18
	unsigned char dSize; // size=0, offset=19
	unsigned char size; // size=0, offset=20
	unsigned char friction; // size=0, offset=21
	unsigned char scalar; // size=0, offset=22
	unsigned char def; // size=0, offset=23
	signed char rotAdd; // size=0, offset=24
	signed char maxYvel; // size=0, offset=25
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
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	numeric::Fixed<12 ,4> xVel; // size=0, offset=12
	numeric::Fixed<12, 4> yVel; // size=0, offset=14
	numeric::Fixed<12, 4> zVel; // size=0, offset=16
	numeric::Fixed<8, 8> gravity; // size=0, offset=18
	short rotAng; // size=0, offset=20
	short flags; // size=0, offset=22
	byte sSize; // size=0, offset=24
	byte dSize; // size=0, offset=25
	byte size; // size=0, offset=26
	numeric::Fixed<2,6> friction; // size=0, offset=27
	byte scalar; // size=0, offset=28
	byte def; // size=0, offset=29
	byte rotAdd; // size=0, offset=30
	numeric::Fixed<4, 4> maxYvel; // size=0, offset=31
	byte on; // size=0, offset=32
	byte sShade; // size=0, offset=33
	byte dShade; // size=0, offset=34
	byte shade; // size=0, offset=35
	byte colFadeSpeed; // size=0, offset=36
	byte fadeToBlack; // size=0, offset=37
	byte sLife; // size=0, offset=38
	byte life; // size=0, offset=39
	byte transType; // size=0, offset=40
	byte fxObj; // size=0, offset=41
	byte nodeNumber; // size=0, offset=42
	byte mirror; // size=0, offset=43
};

struct BLOOD_STRUCT
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	short xVel; // size=0, offset=12
	short yVel; // size=0, offset=14
	short zVel; // size=0, offset=16
	short gravity; // size=0, offset=18
	short rotAng; // size=0, offset=20
	unsigned char sSize; // size=0, offset=22
	unsigned char dSize; // size=0, offset=23
	unsigned char size; // size=0, offset=24
	unsigned char friction; // size=0, offset=25
	byte rotAdd; // size=0, offset=26
	unsigned char on; // size=0, offset=27
	unsigned char sShade; // size=0, offset=28
	unsigned char dShade; // size=0, offset=29
	unsigned char shade; // size=0, offset=30
	unsigned char colFadeSpeed; // size=0, offset=31
	unsigned char fadeToBlack; // size=0, offset=32
	byte sLife; // size=0, offset=33
	byte life; // size=0, offset=34
	byte pad; // size=0, offset=35
};

typedef struct SPOTCAM
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	int tx; // size=0, offset=12
	int ty; // size=0, offset=16
	int tz; // size=0, offset=20
	unsigned char sequence; // size=0, offset=24
	unsigned char camera; // size=0, offset=25
	short fov; // size=0, offset=26
	short roll; // size=0, offset=28
	short timer; // size=0, offset=30
	short speed; // size=0, offset=32
	short flags; // size=0, offset=34
	short roomNumber; // size=0, offset=36
	short pad; // size=0, offset=38
};

typedef struct INVOBJ
{
	short objectNumber;
	short yOff;
	short scale1;
	short yRot;
	short xRot;
	short zRot;
	short flags;
	short objectName;
	int meshBits;
};

typedef struct OBJLIST {
	short inventoryItem;
	short yRot;
	short bright;
};

typedef struct COMBINELIST {
	int combineRoutine;
	short item1;
	short item2;
	short combinedItem;
};

typedef struct INVENTORYRING {
	OBJLIST currentObjectList[100];
	int ringActive;
	int objectListMovement;
	int currentObjectInList;
	int numObjectsInList;
};

typedef struct DISPLAY_PICKUP {
	short life;
	short objectNumber;
};

typedef struct DYNAMIC
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	byte on; // size=0, offset=12
	byte r; // size=0, offset=13
	byte g; // size=0, offset=14
	byte b; // size=0, offset=15
	short falloff; // size=0, offset=16
	byte used; // size=0, offset=18
	byte pad1[1]; // size=1, offset=19
	int FalloffScale; // size=0, offset=20
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

struct SHATTER_ITEM
{
	SPHERE sphere; // size=16, offset=0
	ITEM_LIGHT* il; // size=48, offset=16
	short* meshp; // size=0, offset=20
	int bit; // size=0, offset=24
	short yRot; // size=0, offset=28
	short flags; // size=0, offset=30
};

struct WEAPON_INFO
{
	short lockAngles[4]; // size=8, offset=0
	short leftAngles[4]; // size=8, offset=8
	short rightAngles[4]; // size=8, offset=16
	short aimSpeed; // size=0, offset=24
	short shotAccuracy; // size=0, offset=26
	short gunHeight; // size=0, offset=28
	short targetDist; // size=0, offset=30
	byte damage; // size=0, offset=32
	byte recoilFrame; // size=0, offset=33
	byte flashTime; // size=0, offset=34
	byte drawFrame; // size=0, offset=35
	short sampleNum; // size=0, offset=36
};



struct RAT_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	short roomNumber; // size=0, offset=20
	short speed; // size=0, offset=22
	short fallspeed; // size=0, offset=24
	byte on; // size=0, offset=26
	byte flags; // size=0, offset=27
};

struct BAT_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	short roomNumber; // size=0, offset=20
	short speed; // size=0, offset=22
	short counter; // size=0, offset=24
	short laraTarget; // size=0, offset=26
	byte xTarget; // size=0, offset=28
	byte zTarget; // size=0, offset=29
	byte on; // size=0, offset=30
	byte flags; // size=0, offset=31
};

struct SPIDER_STRUCT
{
	PHD_3DPOS pos; // size=20, offset=0
	short roomNumber; // size=0, offset=20
	short speed; // size=0, offset=22
	short fallspeed; // size=0, offset=24
	byte on; // size=0, offset=26
	byte flags; // size=0, offset=27
};

struct DEBRIS_STRUCT
{
	void* textInfo; // size=0, offset=0
	int x; // size=0, offset=4
	int y; // size=0, offset=8
	int z; // size=0, offset=12
	short xyzOffsets1[3]; // size=6, offset=16
	short dir; // size=0, offset=22
	short xyzOffsets2[3]; // size=6, offset=24
	short speed; // size=0, offset=30
	short xyzOffsets3[3]; // size=6, offset=32
	short yVel; // size=0, offset=38
	short gravity; // size=0, offset=40
	short roomNumber; // size=0, offset=42
	byte on; // size=0, offset=44
	byte xRot; // size=0, offset=45
	byte yRot; // size=0, offset=46
	byte r; // size=0, offset=47
	byte g; // size=0, offset=48
	byte b; // size=0, offset=49
	byte pad[22]; // size=22, offset=50
};

typedef struct BOUNDING_BOX {
	short X1;
	short X2;
	short Y1;
	short Y2;
	short Z1;
	short Z2;
};

struct QUAKE_CAMERA {
	GAME_VECTOR spos;
	GAME_VECTOR epos;
};

struct DOORPOS_DATA
{
	FLOOR_INFO* floor; // size=8, offset=0
	FLOOR_INFO data; // size=8, offset=4
	short block; // size=0, offset=12
};

struct DOOR_DATA
{
	DOORPOS_DATA d1; // size=16, offset=0
	DOORPOS_DATA d1flip; // size=16, offset=16
	DOORPOS_DATA d2; // size=16, offset=32
	DOORPOS_DATA d2flip; // size=16, offset=48
	short opened; // size=0, offset=64
	short* dptr1; // size=0, offset=68
	short* dptr2; // size=0, offset=72
	short* dptr3; // size=0, offset=76
	short* dptr4; // size=0, offset=80
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
	short objectNum;
	char draw1Anim2;
	char draw1Anim;
	char draw2Anim;
	char recoilAnim;
};

struct SP_DYNAMIC
{
	unsigned char On; // size=0, offset=0
	unsigned char Falloff; // size=0, offset=1
	unsigned char R; // size=0, offset=2
	unsigned char G; // size=0, offset=3
	unsigned char B; // size=0, offset=4
	unsigned char Flags; // size=0, offset=5
	unsigned char Pad[2]; // size=2, offset=6
};

typedef void (cdecl *EFFECT_ROUTINE)(ITEM_INFO*);
typedef void (cdecl *LARA_COLLISION_ROUTINE)(ITEM_INFO*, COLL_INFO*);
typedef void (cdecl *LARA_CONTROL_ROUTINE)(ITEM_INFO*, COLL_INFO*);

#pragma pack(pop)