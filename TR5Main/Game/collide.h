#pragma once
#include "Specific\phd_global.h"
#include "trmath.h"
struct ITEM_INFO;
struct COLL_INFO;
struct FLOOR_INFO;
struct MESH_INFO;

// used by coll->Setup.BadHeightUp
#define NO_BAD_POS (-NO_HEIGHT)
// used by coll->Setup.BadHeightDown
#define NO_BAD_NEG NO_HEIGHT

#define MAX_COLLIDED_OBJECTS 1024

#define COLLISION_CHECK_DISTANCE 6144

enum COLL_TYPE
{
	CT_NONE = 0,				// 0x00
	CT_FRONT = (1 << 0),		// 0x01
	CT_LEFT = (1 << 1),			// 0x02
	CT_RIGHT = (1 << 2),		// 0x04
	CT_TOP = (1 << 3),			// 0x08
	CT_TOP_FRONT = (1 << 4),	// 0x10
	CT_CLAMP = (1 << 5)			// 0x20
};

enum HEIGHT_TYPE
{
	WALL,
	SMALL_SLOPE,
	BIG_SLOPE,
	DIAGONAL,
	SPLIT_TRI
};

struct COLL_POSITION
{
	int Floor;
	int Ceiling;
	int SplitFloor;
	int SplitCeiling;
	HEIGHT_TYPE Type;
};

struct COLL_RESULT
{
	int RoomNumber;

	FLOOR_INFO* Block;
	FLOOR_INFO* BottomBlock;

	COLL_POSITION Position;

	int TiltX;
	int TiltZ;
};

struct COLL_SETUP
{
	bool SlopesAreWalls;    // Treat steep slopes as walls
	bool SlopesArePits;     // Treat steep slopes as pits
	bool DeathIsPit;        // Treat death sectors as pits
	bool EnableObjectPush;  // Can be pushed by objects
	bool EnableSpaz;        // Push is treated as hurt
						    
	int   Radius;           // Collision bounds horizontal size
	short ForwardAngle;     // Forward angle direction
	int   BadHeightUp;      // Borderline step-up height 
	int   BadHeightDown;    // Borderline step-down height
	int   BadCeilingHeight; // Borderline ceiling height

	PHD_VECTOR OldPosition; // Preserve old parameters to restore later
	short OldAnimState;
	short OldAnimNumber;
	short OldFrameNumber;
};

struct COLL_INFO
{
	COLL_SETUP    Setup;    // In parameters

	COLL_POSITION Middle;       
	COLL_POSITION MiddleLeft;   
	COLL_POSITION MiddleRight;  
	COLL_POSITION Front;        
	COLL_POSITION FrontLeft;    
	COLL_POSITION FrontRight;   

	PHD_VECTOR Shift;
	COLL_TYPE CollisionType;
	int TiltX;
	int TiltZ;

	bool HitStatic;
	bool HitTallBounds;
	int ObjectHeadroom;
};

struct OBJECT_COLLISION_BOUNDS
{
	BOUNDING_BOX boundingBox;
	short rotX1;
	short rotX2;
	short rotY1;
	short rotY2;
	short rotZ1;
	short rotZ2;
};

constexpr auto MAX_ITEMS = 1024;
constexpr auto ITEM_RADIUS_YMAX = SECTOR(3);

extern BOUNDING_BOX GlobalCollisionBounds;
extern ITEM_INFO* CollidedItems[MAX_ITEMS];
extern MESH_INFO* CollidedMeshes[MAX_ITEMS];

void GenericSphereBoxCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void UpdateLaraRoom(ITEM_INFO* item, int height);
COLL_RESULT GetCollisionResult(FLOOR_INFO* floor, int x, int y, int z);
COLL_RESULT GetCollisionResult(int x, int y, int z, short roomNumber);
COLL_RESULT GetCollisionResult(ITEM_INFO* item);
int FindGridShift(int x, int z); 
int TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius);
int ItemPushItem(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, bool spazon, char bigpush);
int ItemPushStatic(ITEM_INFO* l, MESH_INFO* mesh, COLL_INFO* coll);
void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l);
int TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* l);
int Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd);
int MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius);
void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, PHD_VECTOR offset, int objectHeight);
void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, int objectHeight);
void GetObjectCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, PHD_VECTOR offset, int objectHeight);
void GetObjectCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, int objectHeight);
void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
void DoObjectCollision(ITEM_INFO* item, COLL_INFO* coll);
bool ItemNearLara(PHD_3DPOS* pos, int radius);
bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius);
bool SnapToQuadrant(short& angle, int interval);
int GetQuadrant(short angle);
bool SnapToDiagonal(short& angle, int interval);
void CalcItemToFloorRotation(ITEM_INFO* item, int radiusDivide = 1);
Vector2 GetDiagonalIntersect(int xPos, int zPos, int splitType, int radius, short yRot); // find xPos, zPos that intersects with diagonal on sector
Vector2 GetOrthogonalIntersect(int xPos, int zPos, int radius, short yRot); // find xPos, zPos near sector bound, offset by radius;
bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, COLL_INFO* coll);
void CollideSolidStatics(ITEM_INFO* item, COLL_INFO* coll);