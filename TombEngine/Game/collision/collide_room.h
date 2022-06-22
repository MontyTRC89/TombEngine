#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct ItemInfo;
struct CollisionInfo;
class FloorInfo;
struct ROOM_INFO;
struct MESH_INFO;
enum RoomEnvFlags;

constexpr auto NO_LOWER_BOUND = -NO_HEIGHT;	// used by coll->Setup.LowerFloorBound
constexpr auto NO_UPPER_BOUND = NO_HEIGHT;	// used by coll->Setup.UpperFloorBound
constexpr auto COLLISION_CHECK_DISTANCE = SECTOR(8);

enum CollisionType
{
	CT_NONE = 0,				// 0x00
	CT_FRONT = (1 << 0),		// 0x01
	CT_LEFT = (1 << 1),			// 0x02
	CT_RIGHT = (1 << 2),		// 0x04
	CT_TOP = (1 << 3),			// 0x08
	CT_TOP_FRONT = (1 << 4),	// 0x10
	CT_CLAMP = (1 << 5)			// 0x20
};

enum class CollisionProbeMode
{
	Quadrants,
	FreeForward,
	FreeFlat
};

enum class CornerType
{
	None,
	Inner,
	Outer
};

struct CollisionPosition
{
	int Floor;
	int Ceiling;
	int Bridge;
	float SplitAngle;
	bool FloorSlope;
	bool CeilingSlope;
	bool DiagonalStep;

	bool HasDiagonalSplit() { return SplitAngle == 45.0f * RADIAN || SplitAngle == 135.0f * RADIAN; }
	bool HasFlippedDiagonalSplit() { return HasDiagonalSplit() && SplitAngle != 45.0f * RADIAN; }
};

struct CollisionResult
{
	Vector3Int Coordinates;
	int RoomNumber;

	FloorInfo* Block;
	FloorInfo* BottomBlock;

	CollisionPosition Position;
	Vector2 FloorTilt;			// x = x, y = z
	Vector2 CeilingTilt;		// x = x, y = z
};

struct CollisionSetup
{
	CollisionProbeMode Mode;	// Probe rotation mode
	int   Radius;				// Collision bounds horizontal size
	int   Height;				// Collision bounds vertical size
	short ForwardAngle;			// Forward angle direction

	int LowerFloorBound;		// Borderline floor step-up height 
	int UpperFloorBound;		// Borderline floor step-down height
	int LowerCeilingBound;		// Borderline ceiling step-up height
	int UpperCeilingBound;		// Borderline ceiling step-down height

	bool BlockFloorSlopeUp;		// Treat steep slopes as walls
	bool BlockFloorSlopeDown;	// Treat steep slopes as pits
	bool BlockCeilingSlope;		// Treat steep slopes on ceilings as walls
	bool BlockDeathFloorDown;	// Treat death sectors as pits
	bool BlockMonkeySwingEdge;	// Treat non-monkey sectors as walls
	
	bool EnableObjectPush;		// Can be pushed by objects
	bool EnableSpasm;			// Convulse when pushed

	// Preserve old parameters to restore later
	Vector3Int OldPosition;
	int OldAnimNumber;
	int OldFrameNumber;
	int OldState;
};

struct CollisionInfo
{
	CollisionSetup    Setup;    // In parameters

	CollisionPosition Middle;       
	CollisionPosition MiddleLeft;   
	CollisionPosition MiddleRight;  
	CollisionPosition Front;        
	CollisionPosition FrontLeft;    
	CollisionPosition FrontRight;   

	Vector3Int Shift;
	CollisionType CollisionType;
	Vector2 FloorTilt;				// x = x, y = z
	Vector2 CeilingTilt;			// x = x, y = z
	short NearestLedgeAngle;
	float NearestLedgeDistance;

	bool HitStatic;
	bool HitTallObject;

	bool TriangleAtRight() { return MiddleRight.SplitAngle != 0.0f && MiddleRight.SplitAngle == Middle.SplitAngle; }
	bool TriangleAtLeft() { return MiddleLeft.SplitAngle != 0.0f && MiddleLeft.SplitAngle == Middle.SplitAngle; }
	bool DiagonalStepAtRight() { return MiddleRight.DiagonalStep && TriangleAtRight() && (NearestLedgeAngle % ANGLE(90.0f)); }
	bool DiagonalStepAtLeft()  { return MiddleLeft.DiagonalStep && TriangleAtLeft() && (NearestLedgeAngle % ANGLE(90.0f)); }
};

[[nodiscard]] bool TestItemRoomCollisionAABB(ItemInfo* item);

CollisionResult GetCollision(ItemInfo* item);
CollisionResult GetCollision(ItemInfo* item, short angle, float forward, float up = 0.0f, float right = 0.0f);
CollisionResult GetCollision(Vector3Int pos, int roomNumber, short angle, float forward, float up = 0.0f, float right = 0.0f);
CollisionResult GetCollision(int x, int y, int z, short roomNumber);
CollisionResult GetCollision(FloorInfo* floor, int x, int y, int z);

void  GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, Vector3Int offset, bool resetRoom = false);
void  GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, bool resetRoom = false);
int   GetQuadrant(short angle);
short GetNearestLedgeAngle(ItemInfo* item, CollisionInfo* coll, float& distance);

FloorInfo* GetFloor(int x, int y, int z, short* roomNumber);
int GetFloorHeight(FloorInfo* floor, int x, int y, int z);
int GetCeiling(FloorInfo* floor, int x, int y, int z);
int GetDistanceToFloor(int itemNumber, bool precise = true);
void AlterFloorHeight(ItemInfo* item, int height);

int GetWaterSurface(int x, int y, int z, short roomNumber);
int GetWaterSurface(ItemInfo* item);
int GetWaterDepth(int x, int y, int z, short roomNumber);
int GetWaterDepth(ItemInfo* item);
int GetWaterHeight(int x, int y, int z, short roomNumber);
int GetWaterHeight(ItemInfo* item);

int  FindGridShift(int x, int z);
void ShiftItem(ItemInfo* item, CollisionInfo* coll);
void SnapItemToLedge(ItemInfo* item, CollisionInfo* coll, float offsetMultiplier = 0.0f, bool snapToAngle = true);
void SnapItemToLedge(ItemInfo* item, CollisionInfo* coll, short angle, float offsetMultiplier = 0.0f);
void SnapItemToGrid(ItemInfo* item, CollisionInfo* coll);

void CalculateItemRotationToSurface(ItemInfo* item, float radiusDivisor = 1.0f, short xOffset = 0, short zOffset = 0);

short GetSurfaceAspectAngle(float xTilt, float zTilt);
short GetSurfaceSteepnessAngle(float xTilt, float zTilt);

bool TestEnvironment(RoomEnvFlags environmentType, int x, int y, int z, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, ItemInfo* item);
bool TestEnvironment(RoomEnvFlags environmentType, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, ROOM_INFO* room);
bool TestEnvironmentFlags(RoomEnvFlags environmentType, int flags);

