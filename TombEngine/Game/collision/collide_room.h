#pragma once
#include "Math/Math.h"
#include "Math/Math.h"

struct ItemInfo;
struct CollisionInfo;
class FloorInfo;
struct ROOM_INFO;
struct MESH_INFO;
enum RoomEnvFlags;

constexpr auto NO_LOWER_BOUND = -NO_HEIGHT;	// Used by coll->Setup.LowerFloorBound.
constexpr auto NO_UPPER_BOUND = NO_HEIGHT;	// Used by coll->Setup.UpperFloorBound.
constexpr auto COLLISION_CHECK_DISTANCE = SECTOR(8);

enum CollisionType
{
	CT_NONE		 = 0,
	CT_FRONT	 = (1 << 0),
	CT_LEFT		 = (1 << 1),
	CT_RIGHT	 = (1 << 2),
	CT_TOP		 = (1 << 3),
	CT_TOP_FRONT = (1 << 4),
	CT_CLAMP	 = (1 << 5)
};

enum class CollisionProbeMode
{
	None,
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

	bool HasDiagonalSplit()		   { return ((SplitAngle == (45.0f * RADIAN)) || (SplitAngle == (135.0f * RADIAN))); }
	bool HasFlippedDiagonalSplit() { return (HasDiagonalSplit() && (SplitAngle != (45.0f * RADIAN))); }
};

struct CollisionResult
{
	Vector3i Coordinates;
	int RoomNumber;

	FloorInfo* Block;
	FloorInfo* BottomBlock;

	CollisionPosition Position;
	Vector2 FloorTilt;	 // x = x, y = z
	Vector2 CeilingTilt; // x = x, y = z
};

struct CollisionSetup
{
	CollisionProbeMode Mode = CollisionProbeMode::None; // Probe mode.

	int   Radius	   = 0; // Collision bounds radius.
	int   Height	   = 0; // Collision bounds height.
	short ForwardAngle = 0; // 2D heading angle.

	int UpperFloorBound	  = 0; // Borderline floor step-down height.
	int LowerFloorBound	  = 0; // Borderline floor step-up height.
	int UpperCeilingBound = 0; // Borderline ceiling step-down height.
	int LowerCeilingBound = 0; // Borderline ceiling step-up height.

	bool BlockFloorSlopeUp	  = false; // Block floor slopes above.
	bool BlockFloorSlopeDown  = false; // Block floor slopes below.
	bool BlockCeilingSlope	  = false; // Block ceiling slopes.
	bool BlockDeathFloorDown  = false; // Block death floors.
	bool BlockMonkeySwingEdge = false; // Block non-monkey sectors.
	
	bool EnableObjectPush = false; // Can be pushed by objects.
	bool EnableSpasm	  = false; // Convulse when pushed.

	// Preserve previous parameters to restore later.
	Vector3i OldPosition	= Vector3i::Zero;
	int		 OldAnimNumber	= 0;
	int		 OldFrameNumber = 0;
	int		 OldState		= 0;
};

struct CollisionInfo
{
	CollisionSetup Setup; // In parameters.

	CollisionPosition Middle;       
	CollisionPosition MiddleLeft;   
	CollisionPosition MiddleRight;  
	CollisionPosition Front;        
	CollisionPosition FrontLeft;    
	CollisionPosition FrontRight;   

	Vector3i Shift;
	CollisionType CollisionType;
	Vector2 FloorTilt;	 // x = x, y = z
	Vector2 CeilingTilt; // x = x, y = z
	short NearestLedgeAngle;
	float NearestLedgeDistance;

	bool HitStatic;
	bool HitTallObject;

	bool TriangleAtRight()	   { return ((MiddleRight.SplitAngle != 0.0f) && (MiddleRight.SplitAngle == Middle.SplitAngle)); }
	bool TriangleAtLeft()	   { return ((MiddleLeft.SplitAngle != 0.0f) && (MiddleLeft.SplitAngle == Middle.SplitAngle)); }
	bool DiagonalStepAtRight() { return (MiddleRight.DiagonalStep && TriangleAtRight() && (NearestLedgeAngle % ANGLE(90.0f))); }
	bool DiagonalStepAtLeft()  { return (MiddleLeft.DiagonalStep && TriangleAtLeft() && (NearestLedgeAngle % ANGLE(90.0f))); }
};

[[nodiscard]] bool TestItemRoomCollisionAABB(ItemInfo* item);

CollisionResult GetCollision(ItemInfo* item);
CollisionResult GetCollision(ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
CollisionResult GetCollision(Vector3i pos, int roomNumber, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
CollisionResult GetCollision(int x, int y, int z, short roomNumber);
CollisionResult GetCollision(FloorInfo* floor, int x, int y, int z);

void  GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, const Vector3i& offset, bool resetRoom = false);
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

void AlignEntityToSurface(ItemInfo* item, const Vector2& ellipse, float alpha = 0.75f, float constraintAngle = 70.0f);

bool TestEnvironment(RoomEnvFlags environmentType, int x, int y, int z, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, Vector3i pos, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, ItemInfo* item);
bool TestEnvironment(RoomEnvFlags environmentType, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, ROOM_INFO* room);
bool TestEnvironmentFlags(RoomEnvFlags environmentType, int flags);

