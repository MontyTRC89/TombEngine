#pragma once
#include "Specific\phd_global.h"
#include "trmath.h"

struct ITEM_INFO;
struct COLL_INFO;
struct FLOOR_INFO;
struct MESH_INFO;

constexpr auto NO_BAD_POS = (-NO_HEIGHT); // used by coll->Setup.BadHeightDown
constexpr auto NO_BAD_NEG = NO_HEIGHT;    // used by coll->Setup.BadHeightUp
constexpr auto MAX_COLLIDED_OBJECTS = 1024;
constexpr auto COLLISION_CHECK_DISTANCE = WALL_SIZE * 8;
constexpr auto ITEM_RADIUS_YMAX = SECTOR(3);

extern BOUNDING_BOX GlobalCollisionBounds;
extern ITEM_INFO* CollidedItems[MAX_COLLIDED_OBJECTS];
extern MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

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

enum class COLL_PROBE_MODE
{
	QUADRANTS,
	FREE_FORWARD,
	FREE_FLAT
};

enum class SPLAT_COLL
{
	NONE,
	WALL,
	STEP
};

enum class CORNER_RESULT
{
	NONE,
	INNER,
	OUTER
};

struct COLL_POSITION
{
	int Floor;
	int Ceiling;
	int Bridge;
	float SplitAngle;
	bool Slope;
	bool DiagonalStep;

	bool HasDiagonalSplit() { return SplitAngle == 45.0f * RADIAN || SplitAngle == 135.0f * RADIAN; }
	bool HasFlippedDiagonalSplit() { return HasDiagonalSplit() && SplitAngle != 45.0f * RADIAN; }
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
	COLL_PROBE_MODE Mode;   // Probe rotation mode

	bool SlopesAreWalls;    // Treat steep slopes as walls
	bool SlopesArePits;     // Treat steep slopes as pits
	bool DeathFlagIsPit;    // Treat death sectors as pits
	bool EnableObjectPush;  // Can be pushed by objects
	bool EnableSpaz;        // Push is treated as hurt
						    
	int   Radius;           // Collision bounds horizontal size
	int   Height;			// Collision bounds vertical size
	short ForwardAngle;     // Forward angle direction
	int   BadHeightDown;    // Borderline step-up height 
	int   BadHeightUp;      // Borderline step-down height
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
	short NearestLedgeAngle;
	float NearestLedgeDistance;
	int TiltX;
	int TiltZ;

	bool HitStatic;
	bool HitTallObject;

	bool TriangleAtRight() { return MiddleRight.SplitAngle != 0.0f && MiddleRight.SplitAngle == Middle.SplitAngle; }
	bool TriangleAtLeft() { return MiddleLeft.SplitAngle != 0.0f && MiddleLeft.SplitAngle == Middle.SplitAngle; }
	bool DiagonalStepAtRight() { return MiddleRight.DiagonalStep && TriangleAtRight() && (NearestLedgeAngle % ANGLE(90)); }
	bool DiagonalStepAtLeft()  { return MiddleLeft.DiagonalStep && TriangleAtLeft() && (NearestLedgeAngle % ANGLE(90)); }
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

void GenericSphereBoxCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
bool GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
bool TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void MoveItem(ITEM_INFO* item, short angle, int x, int y = 0);
void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, float offsetMultiplier = 0.0f);
void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, short angle, float offsetMultiplier = 0.0f);
void SnapItemToGrid(ITEM_INFO* item, COLL_INFO* coll);
COLL_RESULT GetCollisionResult(ITEM_INFO* item, short angle, int dist, int height);
COLL_RESULT GetCollisionResult(FLOOR_INFO* floor, int x, int y, int z);
COLL_RESULT GetCollisionResult(int x, int y, int z, short roomNumber);
COLL_RESULT GetCollisionResult(ITEM_INFO* item);
int FindGridShift(int x, int z); 
bool TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius);
bool ItemPushItem(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, bool spazon, char bigpush);
bool ItemPushStatic(ITEM_INFO* l, MESH_INFO* mesh, COLL_INFO* coll);
void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l);
bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* l);
bool Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd);
bool MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
bool TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius);
void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, PHD_VECTOR offset, bool resetRoom = false);
void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, bool resetRoom = false);
void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
void DoObjectCollision(ITEM_INFO* item, COLL_INFO* coll);
bool ItemNearLara(PHD_3DPOS* pos, int radius);
bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius);
int GetQuadrant(short angle);
void CalcItemToFloorRotation(ITEM_INFO* item, int radiusDivide = 1);
short GetNearestLedgeAngle(ITEM_INFO* item, COLL_INFO* coll, float& dist);
bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, COLL_INFO* coll);
void CollideSolidStatics(ITEM_INFO* item, COLL_INFO* coll);