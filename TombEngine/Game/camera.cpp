#include "framework.h"
#include "Game/camera.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/Setup.h"
#include "Game/spotcam.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Effects::Environment;
using namespace TEN::Entities::Generic;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Utils;
using TEN::Renderer::g_Renderer;

constexpr auto CAMERA_OBJECT_COLL_DIST_THRESHOLD   = BLOCK(4);
constexpr auto CAMERA_OBJECT_COLL_EXTENT_THRESHOLD = CLICK(0.5f);

constexpr auto SWIVEL_STEP_COUNT = 4;

struct OLD_CAMERA
{
	Pose pos;
	Pose pos2;
	Vector3 target; // LookAt

	// Camera sphere
	short actualAngle	  = 0.0f;
	short actualElevation = 0;
	short targetElevation = 0;
	float targetDistance  = 0.0f;

	// Player anim state
	int ActiveState = 0;
	int TargetState = 0;
};

CAMERA_INFO Camera;
OLD_CAMERA OldCam;
GameVector LastTarget;

Vector3 LastIdeal;
int LastIdealRoomNumber;

int CameraSnaps = 0;
int TargetSnaps = 0;
ObjectCameraInfo ItemCamera;
GameVector ForcedFixedCamera;
int UseForcedFixedCamera;

CameraType BinocularOldCamera;

short CurrentFOV;
short LastFOV;

int RumbleTimer = 0;
int RumbleCounter = 0;

bool  ScreenFadedOut = false;
bool  ScreenFading = false;
float ScreenFadeSpeed = 0;
float ScreenFadeStart = 0;
float ScreenFadeEnd = 0;
float ScreenFadeCurrent = 0;

float CinematicBarsHeight = 0;
float CinematicBarsDestinationHeight = 0;
float CinematicBarsSpeed = 0;

// NOTE: Function label comments will be a temporary reference until a camera class
// is created to keep them neatly organised as private methods. -- Sezz 2024.03.13

// ----------------
// HELPER FUNCTIONS
// ----------------

// IsCameraCollidableItem()
// IsCameraCollideableStatic()

// GetCameraRoomLosIntersect()
// GetCameraCollidableItemPtrs()
// GetCameraCollidableStaticPtrs()
// GetCameraRayBoxIntersect()
// GetCameraObjectLosIntersect()
// GetCameraLosIntersect()
// GetCameraRelativeShift()

static bool IsCameraCollidableItem(const ItemInfo& item)
{
	// 1) Test distance.
	float distSqr = Vector3i::DistanceSquared(item.Pose.Position, Camera.Position);
	if (distSqr >= SQUARE(CAMERA_OBJECT_COLL_DIST_THRESHOLD))
		return false;

	// 2) Check object collidability.
	const auto& object = Objects[item.ObjectNumber];
	if (!item.Collidable || !object.usingDrawAnimatingItem)
		return false;

	// 3) Check object attributes.
	if (object.intelligent || object.isPickup || object.isPuzzleHole || object.collision == nullptr)
		return false;

	// 4) Test if any 2 box extents are smaller than threshold.
	auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
	if ((abs(box.Extents.x) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.y) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD) ||
		(abs(box.Extents.x) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.z) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD) ||
		(abs(box.Extents.y) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.z) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD))
	{
		return false;
	}

	return true;
}

static bool IsCameraCollideableStatic(const MESH_INFO& staticObject)
{
	// 1) Test distance.
	float distSqr = Vector3i::DistanceSquared(Camera.Position, staticObject.pos.Position);
	if (distSqr >= SQUARE(CAMERA_OBJECT_COLL_DIST_THRESHOLD))
		return false;

	// 2) Check if static is visible.
	if (!(staticObject.flags & StaticMeshFlags::SM_VISIBLE))
		return false;

	// 3) Test if any 2 box extents are smaller than threshold.
	auto box = GetBoundsAccurate(staticObject, false).ToBoundingOrientedBox(staticObject.pos);
	if ((abs(box.Extents.x) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.y) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD) ||
		(abs(box.Extents.x) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.z) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD) ||
		(abs(box.Extents.y) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD && abs(box.Extents.z) < CAMERA_OBJECT_COLL_EXTENT_THRESHOLD))
	{
		return false;
	}

	return true;
}

static std::optional<std::pair<Vector3, int>> GetCameraRoomLosIntersect(const Vector3& idealPos, int idealRoomNumber, const Vector3& dir)
{
	auto closestIntersect = std::optional<Vector3>();

	auto origin = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);
	auto target = GameVector(idealPos, idealRoomNumber);

	// 1) Collide axis-aligned walls.
	if (!LOS(&origin, &target))
	{
		float dist = Vector3::Distance(origin.ToVector3(), target.ToVector3());
		//auto closestIntersect = Geometry::TranslatePoint(Camera.LookAt, dir, dist);
		return std::pair(Geometry::TranslatePoint(Camera.LookAt, dir, dist), target.RoomNumber);
	}

	// TODO: Interferes with above LOS.
	// 2) Collide diagonal walls and floors/ceilings.
	/*if (!LOSAndReturnTarget(&origin, &target, 0))
	{
		float dist = Vector3::Distance(origin.ToVector3(), target.ToVector3()) - BUFFER;
		auto closestIntersect = Geometry::TranslatePoint(Camera.LookAt, dir, dist);
	}*/

	// Return intersection.
	if (closestIntersect.has_value())
	{
		int intersectRoomNumber = target.RoomNumber;
		return std::pair(*closestIntersect, intersectRoomNumber);
	}

	// No intersection; return nullopt.
	return std::nullopt;
}

static std::vector<const ItemInfo*> GetCameraCollidableItemPtrs()
{
	auto itemPtrs = std::vector<const ItemInfo*>{};

	// Collect item pointers.
	const auto& neighborRoomNumbers = g_Level.Rooms[Camera.RoomNumber].neighbors;
	for (int itemNumber = 0; itemNumber < g_Level.NumItems; itemNumber++)
	{
		const auto& item = g_Level.Items[itemNumber];

		// 1) Check if item is bridge.
		if (item.IsBridge())
			continue;

		// 2) Check if room is active.
		const auto& room = g_Level.Rooms[item.RoomNumber];
		if (!room.Active())
			continue;

		// 3) Test if item is in neighboring room.
		if (!Contains(neighborRoomNumbers, (int)item.RoomNumber))
			continue;

		// 4) Test if item is camera-collidable.
		if (!IsCameraCollidableItem(item))
			continue;

		itemPtrs.push_back(&item);
	}

	return itemPtrs;
}

static std::vector<const MESH_INFO*> GetCameraCollidableStaticPtrs()
{
	auto staticPtrs = std::vector<const MESH_INFO*>{};

	// Collect static pointers.
	const auto& neighborRoomNumbers = g_Level.Rooms[Camera.RoomNumber].neighbors;
	for (int roomNumber : neighborRoomNumbers)
	{
		// 1) Check if room is active.
		const auto& room = g_Level.Rooms[roomNumber];
		if (!room.Active())
			continue;

		// 2) Run through statics in room.
		for (const auto& staticObject : room.mesh)
		{
			// Test if static is camera-collidable.
			if (!IsCameraCollideableStatic(staticObject))
				continue;

			staticPtrs.push_back(&staticObject);
		}
	}

	return staticPtrs;
}

static std::optional<Vector3> GetCameraRayBoxIntersect(const Vector3& origin, const Vector3& dir, float dist, const BoundingOrientedBox& box)
{
	constexpr auto BUFFER = Vector3(BLOCK(1 / 16.0f));

	// TODO: Try dynamic relative shift instead of offsetting boxes.

	// Calculate collision boxes.
	auto expandedBox = BoundingOrientedBox(box.Center, box.Extents + BUFFER, box.Orientation);
	auto bufferBox = BoundingOrientedBox(box.Center, box.Extents + (BUFFER * 2), box.Orientation);

	// Calculate and return intersection.
	float intersectDist = 0.0f;
	if (bufferBox.Intersects(origin, dir, intersectDist))
	{
		if (intersectDist < dist && intersectDist <= Camera.targetDistance)
		{
			if (expandedBox.Intersects(origin, dir, intersectDist))
				return Geometry::TranslatePoint(origin, dir, intersectDist);
		}
	}

	// No intersection; return nullopt.
	return std::nullopt;
}

static std::optional<std::pair<Vector3, int>> GetCameraObjectLosIntersect(const Vector3& idealPos, int idealRoomNumber, const Vector3& dir)
{
	constexpr auto DEBUG_BOX_COLOR = Color(1.0f, 0.0f, 0.0f);

	// Get object pointers.
	auto itemPtrs = GetCameraCollidableItemPtrs();
	auto staticPtrs = GetCameraCollidableStaticPtrs();

	// No objects nearby; return nullopt.
	if (itemPtrs.empty() && staticPtrs.empty())
		return std::nullopt;

	float dist = Vector3::Distance(Camera.LookAt, idealPos);

	auto closestIntersect = std::optional<Vector3>();
	float closestDistSqr = INFINITY;

	// 1) Collide items.
	for (const auto* itemPtr : itemPtrs)
	{
		auto box = GameBoundingBox(itemPtr).ToBoundingOrientedBox(itemPtr->Pose);
		auto intersect = GetCameraRayBoxIntersect(Camera.LookAt, dir, dist, box);
		if (intersect.has_value())
		{
			float distSqr = Vector3::DistanceSquared(Camera.LookAt, *intersect);
			if (distSqr < closestDistSqr)
			{
				closestIntersect = intersect;
				closestDistSqr = distSqr;
			}
		}

		g_Renderer.AddDebugBox(box, DEBUG_BOX_COLOR, RendererDebugPage::CollisionStats);
	}

	// 2) Collide statics.
	for (const auto* staticPtr : staticPtrs)
	{
		auto box = GetBoundsAccurate(*staticPtr, false).ToBoundingOrientedBox(staticPtr->pos);
		auto intersect = GetCameraRayBoxIntersect(Camera.LookAt, dir, dist, box);
		if (intersect.has_value())
		{
			float distSqr = Vector3::DistanceSquared(Camera.LookAt, *intersect);
			if (distSqr < closestDistSqr)
			{
				closestIntersect = intersect;
				closestDistSqr = distSqr;
			}
		}

		g_Renderer.AddDebugBox(box, DEBUG_BOX_COLOR, RendererDebugPage::CollisionStats);
	}

	// Return intersection.
	if (closestIntersect.has_value())
	{
		int intersectRoomNumber = GetCollision(idealPos, idealRoomNumber, -dir, sqrt(closestDistSqr)).RoomNumber;
		return std::pair(*closestIntersect, intersectRoomNumber);
	}

	// No intersection; return nullopt.
	return std::nullopt;
}

static std::optional<std::pair<Vector3, int>> GetCameraLosIntersect(const Vector3& idealPos, int idealRoomNumber, const Vector3& dir)
{
	constexpr auto BUFFER = BLOCK(0.1f);

	auto intersect = std::pair(idealPos, idealRoomNumber);
	bool hasIntersect = false;

	// 1) Collide with room.
	auto roomIntersect = GetCameraRoomLosIntersect(intersect.first, intersect.second, dir);
	if (roomIntersect.has_value())
	{
		intersect = *roomIntersect;
		hasIntersect = true;
	}

	// 2) Collide with objects.
	auto objectIntersect = GetCameraObjectLosIntersect(intersect.first, intersect.second, dir);
	if (objectIntersect.has_value())
	{
		intersect = *objectIntersect;
		hasIntersect = true;
	}

	// Return intersection.
	if (hasIntersect)
	{
		// Apply buffer.
		float dist = Vector3::Distance(Camera.LookAt, intersect.first);
		dist = ((dist - BUFFER) >= BUFFER) ? (dist - BUFFER) : BUFFER;

		intersect.first = Geometry::TranslatePoint(Camera.LookAt, dir, dist);
		intersect.second = GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, dir, dist).RoomNumber;
		return intersect;
	}

	// No intersection; return nullopt.
	return std::nullopt;
}

// TODO
static Vector3 GetCameraRelativeShift(const Vector3& pos, int roomNumber)
{
	constexpr auto BUFFER	  = BLOCK(0.2f);
	constexpr auto BUFFER_MAX = BUFFER * 2;
	constexpr auto RAY_COUNT  = 8;

	auto shift = Vector3::Zero;
	auto pointColl = GetCollision(pos, roomNumber);

	// Calculate vertical shift.
	if (abs(pointColl.Position.Ceiling - pointColl.Position.Floor) < (BUFFER * 2))
	{
		shift.y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
	}
	else if ((pointColl.Position.Floor + BUFFER) > pointColl.Position.Floor)
	{
		shift.y = pointColl.Position.Floor + BUFFER;
	}
	else if ((pointColl.Position.Ceiling - BUFFER) < pointColl.Position.Ceiling)
	{
		shift.y = pointColl.Position.Ceiling - BUFFER;
	}

	// Calculate 2D radius.
	auto screenRes = g_Renderer.GetScreenResolution();
	auto screenAspect = (float)screenRes.x / (float)screenRes.y;
	float radius = BUFFER * screenAspect;

	auto origin = GameVector(pos, roomNumber);

	// Calculate horizontal shift.
	auto intersects = std::vector<Vector3>{};
	for (int i = 0; i < RAY_COUNT; i++)
	{
		short headingAngle = ANGLE((360.0f / RAY_COUNT) * i);
		auto intersectPos = Geometry::TranslatePoint(pos, headingAngle, radius);
		int intersectRoomNumber = GetCollision(pos, roomNumber, headingAngle, radius).RoomNumber;

		auto target = GameVector(intersectPos, intersectRoomNumber);
		if (LOS(&origin, &target))
		{
			float dist = Vector3::Distance(pos, target.ToVector3());
			intersectPos = Geometry::TranslatePoint(pos, headingAngle, dist);
		}
		else
		{
			auto intersectPos = Geometry::TranslatePoint(pos, headingAngle, radius * 2);
		}

		intersects.push_back(intersectPos);
	}

	// Calculate horizontal shift.
	auto average = Vector3::Zero;
	for (const auto& intersect : intersects)
		average += intersect;

	average /= RAY_COUNT;
	shift += average;

	return (pos - shift);
}

std::pair<Vector3, int> CameraCollisionBounds(const Vector3& pos, int roomNumber, int push, bool yFirst)
{
	auto collidedPos = std::pair(pos, roomNumber);
	auto pointColl = CollisionResult{};

	if (yFirst)
	{
		pointColl = GetCollision(pos, roomNumber);

		int buffer = CLICK(1) - 1;
		if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			(collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
		}
		else if ((collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Floor - buffer;
		}
		else if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Ceiling + buffer;
		}
	}

	pointColl = GetCollision(collidedPos.first.x - push, collidedPos.first.y, collidedPos.first.z, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.x = ((int)collidedPos.first.x & (~WALL_MASK)) + push;
	}

	pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z - push, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.z = ((int)collidedPos.first.z & (~WALL_MASK)) + push;
	}

	pointColl = GetCollision(collidedPos.first.x + push, collidedPos.first.y, collidedPos.first.z, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.x = ((int)collidedPos.first.x | WALL_MASK) - push;
	}

	pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z + push, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling)
	{
		collidedPos.first.z = ((int)collidedPos.first.z | WALL_MASK) - push;
	}

	if (!yFirst)
	{
		pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z, roomNumber);

		int buffer = CLICK(1) - 1;
		if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			(collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
		}
		else if ((collidedPos.first.y + buffer) > pointColl.Position.Floor &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Floor - buffer;
		}
		else if ((collidedPos.first.y - buffer) < pointColl.Position.Ceiling &&
			pointColl.Position.Ceiling < pointColl.Position.Floor &&
			pointColl.Position.Ceiling != NO_HEIGHT &&
			pointColl.Position.Floor != NO_HEIGHT)
		{
			collidedPos.first.y = pointColl.Position.Ceiling + buffer;
		}
	}

	pointColl = GetCollision(collidedPos.first.x, collidedPos.first.y, collidedPos.first.z, roomNumber);
	if (collidedPos.first.y > pointColl.Position.Floor ||
		collidedPos.first.y < pointColl.Position.Ceiling ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor)
	{
		return collidedPos;
	}

	collidedPos.second = pointColl.RoomNumber;
	return collidedPos;
}

// ----------------
// CAMERA FUNCTIONS
// ----------------

static int GetCameraPlayerVerticalOffset(const ItemInfo& item, const CollisionInfo& coll)
{
	constexpr auto VERTICAL_OFFSET_DEFAULT		  = -BLOCK(1 / 16.0f);
	constexpr auto VERTICAL_OFFSET_SWAMP		  = BLOCK(0.4f);
	constexpr auto VERTICAL_OFFSET_MONKEY_SWING	  = BLOCK(0.25f);
	constexpr auto VERTICAL_OFFSET_TREADING_WATER = BLOCK(0.5f);

	const auto& player = GetLaraInfo(item);

	bool isInSwamp = TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber);

	// Determine contextual vertical offset.
	float verticalOffset = coll.Setup.Height;
	if (player.Control.IsMonkeySwinging)
	{
		verticalOffset -= VERTICAL_OFFSET_MONKEY_SWING;
	}
	else if (player.Control.WaterStatus == WaterStatus::TreadWater)
	{
		verticalOffset -= VERTICAL_OFFSET_TREADING_WATER;
	}
	else if (isInSwamp)
	{
		verticalOffset = VERTICAL_OFFSET_SWAMP;
	}
	else
	{
		verticalOffset -= VERTICAL_OFFSET_DEFAULT;
	}

	// Get floor-to-ceiling height.
	auto pointColl = GetCollision(item);
	int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

	// Return appropriate vertical offset.
	return -((verticalOffset < floorToCeilHeight) ? verticalOffset : floorToCeilHeight);
}

void LookCamera(const ItemInfo& playerItem, const CollisionInfo& coll)
{
	constexpr auto POS_LERP_ALPHA	 = 0.25f;
	constexpr auto COLL_PUSH		 = BLOCK(0.2f);
	constexpr auto LOOK_AT_DIST		 = BLOCK(0.5f);
	constexpr auto CAMERA_DIST_COEFF = 0.7f;
	constexpr auto CAMERA_DIST_MAX	 = BLOCK(0.75f);

	const auto& player = GetLaraInfo(playerItem);

	int verticalOffset = GetCameraPlayerVerticalOffset(playerItem, coll);
	auto pivotOffset = Vector3(0.0f, verticalOffset, 0.0f);

	float dist = -std::max(Camera.targetDistance * CAMERA_DIST_COEFF, CAMERA_DIST_MAX);

	// Define absolute camera orientation.
	auto orient = player.Control.Look.Orientation +
		EulerAngles(playerItem.Pose.Orientation.x, playerItem.Pose.Orientation.y, 0) +
		EulerAngles(0, Camera.targetAngle, 0);
	orient.x = std::clamp(orient.x, LOOKCAM_ORIENT_CONSTRAINT.first.x, LOOKCAM_ORIENT_CONSTRAINT.second.x);

	// Determine base position.
	bool isInSwamp = TestEnvironment(ENV_FLAG_SWAMP, playerItem.RoomNumber);
	auto basePos = Vector3(
		playerItem.Pose.Position.x,
		isInSwamp ? g_Level.Rooms[playerItem.RoomNumber].maxceiling : playerItem.Pose.Position.y,
		playerItem.Pose.Position.z);

	// Define landmarks.
	auto pivotPos = Geometry::TranslatePoint(basePos, playerItem.Pose.Orientation.y, pivotOffset);
	auto idealPos = Geometry::TranslatePoint(pivotPos, orient, dist);
	auto lookAtPos = Geometry::TranslatePoint(pivotPos, orient, LOOK_AT_DIST);

	// Determine best position.
	auto origin = GameVector(pivotPos, GetCollision(&playerItem, playerItem.Pose.Orientation.y, pivotOffset.z, pivotOffset.y).RoomNumber);
	auto target = GameVector(idealPos, GetCollision(origin.ToVector3i(), origin.RoomNumber, orient, dist).RoomNumber);

	// Handle room and object collisions.
	LOSAndReturnTarget(&origin, &target, 0);
	//auto collidedPos = CameraCollisionBounds(&target, COLL_PUSH, true);

	// Smoothly update camera position.
	MoveCamera(playerItem, target.ToVector3(), target.RoomNumber, Camera.speed);
	Camera.LookAt = Camera.LookAt + (lookAtPos - Camera.LookAt) * POS_LERP_ALPHA;
	Camera.LookAtRoomNumber = playerItem.RoomNumber;

	LookAt(Camera, 0);
	UpdateMikePos(playerItem);
	Camera.oldType = Camera.type;
}

void LookAt(CAMERA_INFO& camera, short roll)
{
	float fov = TO_RAD(CurrentFOV / 1.333333f);
	float levelFarView = BLOCK(g_GameFlow->GetLevel(CurrentLevel)->GetFarView());

	g_Renderer.UpdateCameraMatrices(&camera, TO_RAD(roll), fov, levelFarView);
}

void AlterFOV(short value, bool store)
{
	if (store)
		LastFOV = value;

	CurrentFOV = value;
}

short GetCurrentFOV()
{
	return CurrentFOV;
}

inline void RumbleFromBounce()
{
	Rumble(std::clamp(abs(Camera.bounce) / 70.0f, 0.0f, 0.8f), 0.2f);
}

void InitializeCamera()
{
	Camera.shift = LaraItem->Pose.Position.y - BLOCK(1);

	LastTarget = GameVector(
		LaraItem->Pose.Position.x,
		Camera.shift,
		LaraItem->Pose.Position.z,
		LaraItem->RoomNumber);

	Camera.LookAt = Vector3(LastTarget.x, Camera.shift, LastTarget.z);
	Camera.LookAtRoomNumber = LaraItem->RoomNumber;

	Camera.Position = Vector3(LastTarget.x, Camera.shift, LastTarget.z - 100);
	Camera.RoomNumber = LaraItem->RoomNumber;

	Camera.targetDistance = BLOCK(1.5f);
	Camera.item = nullptr;
	Camera.numberFrames = 1;
	Camera.type = CameraType::Chase;
	Camera.speed = 1.0f;
	Camera.flags = CameraFlag::None;
	Camera.bounce = 0;
	Camera.number = -1;
	Camera.fixedCamera = false;

	AlterFOV(ANGLE(DEFAULT_FOV));

	UseForcedFixedCamera = 0;
	CalculateCamera(*LaraItem, LaraCollision);

	// Fade in screen.
	SetScreenFadeIn(FADE_SCREEN_SPEED);
}

void MoveCamera(const ItemInfo& playerItem, Vector3 ideal, int idealRoomNumber, float speed)
{
	constexpr auto BUFFER = CLICK(1) - 1;

	const auto& player = GetLaraInfo(playerItem);

	if (player.Control.Look.IsUsingBinoculars)
		speed = 1.0f;

	UpdateMikePos(playerItem);

	if (OldCam.pos.Orientation != playerItem.Pose.Orientation ||
		OldCam.pos2.Orientation.x != player.ExtraHeadRot.x ||
		OldCam.pos2.Orientation.y != player.ExtraHeadRot.y ||
		OldCam.pos2.Position.x != player.ExtraTorsoRot.x ||
		OldCam.pos2.Position.y != player.ExtraTorsoRot.y ||
		OldCam.pos.Position != playerItem.Pose.Position ||
		OldCam.ActiveState != playerItem.Animation.ActiveState ||
		OldCam.TargetState != playerItem.Animation.TargetState ||
		OldCam.targetDistance != Camera.targetDistance ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target != Camera.LookAt ||
		Camera.oldType != Camera.type ||
		player.Control.Look.IsUsingBinoculars)
	{
		OldCam.pos.Orientation = playerItem.Pose.Orientation;
		OldCam.pos2.Orientation.x = player.ExtraHeadRot.x;
		OldCam.pos2.Orientation.y = player.ExtraHeadRot.y;
		OldCam.pos2.Position.x = player.ExtraTorsoRot.x;
		OldCam.pos2.Position.y = player.ExtraTorsoRot.y;
		OldCam.pos.Position = playerItem.Pose.Position;
		OldCam.ActiveState = playerItem.Animation.ActiveState;
		OldCam.TargetState = playerItem.Animation.TargetState;
		OldCam.targetDistance = Camera.targetDistance;
		OldCam.targetElevation = Camera.targetElevation;
		OldCam.actualElevation = Camera.actualElevation;
		OldCam.actualAngle = Camera.actualAngle;
		OldCam.target = Camera.LookAt;
		LastIdeal = ideal;
		LastIdealRoomNumber = idealRoomNumber;
	}
	else
	{
		ideal = LastIdeal;
		idealRoomNumber = LastIdealRoomNumber;
	}

	// Translate camera.
	Camera.Position = Vector3::Lerp(Camera.Position, ideal, 1.0f / speed);
	Camera.RoomNumber = idealRoomNumber;

	// Bounce.
	if (Camera.bounce)
	{
		if (Camera.bounce <= 0)
		{
			int bounce = -Camera.bounce;
			int bounce2 = bounce / 2;

			Camera.LookAt.x += GetRandomControl() % bounce - bounce2;
			Camera.LookAt.y += GetRandomControl() % bounce - bounce2;
			Camera.LookAt.z += GetRandomControl() % bounce - bounce2;
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.Position.y += Camera.bounce;
			Camera.LookAt.y += Camera.bounce;
			Camera.bounce = 0;
		}
	}

	// Avoid entering swamp rooms.
	int vPos = Camera.Position.y;
	if (TestEnvironment(ENV_FLAG_SWAMP, Camera.RoomNumber))
		vPos = g_Level.Rooms[Camera.RoomNumber].y - BUFFER;

	// Handle position out of room bounds.
	auto pointColl = GetCollision(Camera.Position.x, vPos, Camera.Position.z, Camera.RoomNumber);
	if (vPos > pointColl.Position.Floor ||
		vPos < pointColl.Position.Ceiling)
	{
		auto origin0 = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);
		auto target0 = GameVector(Camera.Position, Camera.RoomNumber);
		LOSAndReturnTarget(&origin0, &target0, 0);
		Camera.Position = target0.ToVector3();
		Camera.RoomNumber = target0.RoomNumber;

		float dist = Vector3::Distance(Camera.Position, ideal);
		if (dist < BLOCK(0.5f))
		{
			auto origin = GameVector(ideal, idealRoomNumber);
			auto target = GameVector(Camera.Position, Camera.RoomNumber);

			// Snap to within room bounds.
			if (!LOSAndReturnTarget(&origin, &target, 0) && ++CameraSnaps >= 8)
			{
				Camera.Position = ideal;
				Camera.RoomNumber = idealRoomNumber;
				CameraSnaps = 0;
			}
		}
	}

	// Handle narrow space between floor and ceiling.
	pointColl = GetCollision(Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.RoomNumber);
	if ((Camera.Position.y - BUFFER) < pointColl.Position.Ceiling &&
		(Camera.Position.y + BUFFER) > pointColl.Position.Floor &&
		pointColl.Position.Ceiling < pointColl.Position.Floor &&
		pointColl.Position.Ceiling != NO_HEIGHT && pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.Position.y = (pointColl.Position.Floor + pointColl.Position.Ceiling) / 2;
	}
	else if ((Camera.Position.y + BUFFER) > pointColl.Position.Floor &&
		pointColl.Position.Ceiling < pointColl.Position.Floor &&
		pointColl.Position.Ceiling != NO_HEIGHT && pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.Position.y = pointColl.Position.Floor - BUFFER;
	}
	else if ((Camera.Position.y - BUFFER) < pointColl.Position.Ceiling &&
		pointColl.Position.Ceiling < pointColl.Position.Floor &&
		pointColl.Position.Ceiling != NO_HEIGHT &&	pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.Position.y = pointColl.Position.Ceiling + BUFFER;
	}
	else if (pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT || pointColl.Position.Ceiling == NO_HEIGHT)
	{
		Camera.Position = ideal;
		Camera.RoomNumber = idealRoomNumber;
	}

	Camera.RoomNumber = GetCollision(Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.RoomNumber).RoomNumber;
	LookAt(Camera, 0);
	Camera.oldType = Camera.type;
}

void ObjCamera(ItemInfo* item, int boneID, ItemInfo* targetItem, int targetBoneID, bool cond)
{
	// item and targetItem remain same object until it becomes possible to extend targetItem to another object.
	// Activates code below -> void CalculateCamera().
	ItemCamera.ItemCameraOn = cond;

	UpdateCameraSphere(*LaraItem);

	// Get position of mesh 0.	
	auto pos = GetJointPosition(item, 0, Vector3i::Zero);
	auto target = GameVector(pos, item->RoomNumber);

	Camera.fixedCamera = true;

	MoveObjCamera(&target, item, boneID, targetItem, targetBoneID);
	Camera.timer = -1;
}

void ClearObjCamera()
{
	ItemCamera.ItemCameraOn = false;
}

void MoveObjCamera(GameVector* ideal, ItemInfo* item, int boneID, ItemInfo* targetItem, int targetBoneID)
{
	float speed = 1.0f;

	//Get mesh1 to attach camera to
	//Vector3i pos = Vector3i::Zero;
	auto pos = GetJointPosition(item, boneID, Vector3i::Zero).ToVector3();
	//Get mesh2 to attach target to
	//Vector3i pos2 = Vector3i::Zero;
	auto pos2 = GetJointPosition(targetItem, targetBoneID, Vector3i::Zero).ToVector3();

	if (OldCam.pos.Position != pos ||
		OldCam.targetDistance  != Camera.targetDistance  ||
		OldCam.targetElevation != Camera.targetElevation ||
		OldCam.actualElevation != Camera.actualElevation ||
		OldCam.actualAngle != Camera.actualAngle ||
		OldCam.target != Camera.LookAt ||
		Camera.oldType != Camera.type ||
		Lara.Control.Look.IsUsingBinoculars)
	{
		OldCam.pos.Position = pos;
		OldCam.targetDistance = Camera.targetDistance;
		OldCam.targetElevation = Camera.targetElevation;
		OldCam.actualElevation = Camera.actualElevation;
		OldCam.actualAngle = Camera.actualAngle;
		OldCam.target = Camera.LookAt;
		LastIdeal = pos;
		LastIdealRoomNumber = ideal->RoomNumber;
		LastTarget = Vector3i(pos2);
	}
	else
	{
		pos  = LastIdeal;
		pos2 = LastTarget.ToVector3();
		ideal->RoomNumber = LastIdealRoomNumber;
	}

	Camera.Position += (ideal->ToVector3() - Camera.Position) / speed;
	Camera.RoomNumber = GetCollision(Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.RoomNumber).RoomNumber;
	LookAt(Camera, 0);

	auto angle = Camera.LookAt - Camera.Position;
	auto position = Vector3i(Camera.LookAt - Camera.Position);

	// write last frame camera angle to LastAngle to compare if next frame camera angle has a bigger step than 100.
	// To make camera movement smoother a speed of 2 is used.
	// While for big camera angle steps (cuts) -
	// the speed is set to 1 to make the cut immediatelly.
	constexpr int angleThresholdDegrees = 100;

	if (LastTarget.x - Camera.LookAt.x > angleThresholdDegrees ||
		LastTarget.y - Camera.LookAt.y > angleThresholdDegrees ||
		LastTarget.z - Camera.LookAt.z > angleThresholdDegrees)
	{
		speed = 1;
	}
	else
	{
		speed = 2;
	}

	// Move lookAt.
	Camera.LookAt.x += (pos2.x - Camera.LookAt.x) / speed;
	Camera.LookAt.y += (pos2.y - Camera.LookAt.y) / speed;
	Camera.LookAt.z += (pos2.z - Camera.LookAt.z) / speed;

	if (ItemCamera.LastAngle != position)
	{
		ItemCamera.LastAngle = Vector3i(ItemCamera.LastAngle.x = angle.x, 
										ItemCamera.LastAngle.y = angle.y, 
										ItemCamera.LastAngle.z = angle.z);
	}
}

void RefreshFixedCamera(int cameraID)
{
	const auto& camera = g_Level.Cameras[cameraID];

	auto origin = GameVector(camera.Position, camera.RoomNumber);
	int moveSpeed = camera.Speed * 8 + 1;

	MoveCamera(*LaraItem, origin.ToVector3(), origin.RoomNumber, moveSpeed);
}

static void ClampCameraAltitudeAngle(bool isUnderwater)
{
	constexpr auto MODERN_CAMERA_ABOVE_WATER_ANGLE_CONSTRAINT = std::pair<short, short>(ANGLE(-80.0f), ANGLE(70.0f));
	constexpr auto MODERN_CAMERA_UNDERWATER_ANGLE_CONSTRAINT  = std::pair<short, short>(ANGLE(-80.0f), ANGLE(80.0f));
	constexpr auto TANK_CAMERA_ANGLE_CONSTRAINT				  = std::pair<short, short>(ANGLE(-85.0f), ANGLE(85.0f));

	const auto& angleConstraint = IsUsingModernControls() ?
		(isUnderwater ? MODERN_CAMERA_UNDERWATER_ANGLE_CONSTRAINT : MODERN_CAMERA_ABOVE_WATER_ANGLE_CONSTRAINT) :
		TANK_CAMERA_ANGLE_CONSTRAINT;

	if (Camera.actualElevation > angleConstraint.second)
	{
		Camera.actualElevation = angleConstraint.second;
	}
	else if (Camera.actualElevation < angleConstraint.first)
	{
		Camera.actualElevation = angleConstraint.first;
	}
}

static void HandleCameraFollow(const ItemInfo& playerItem, bool isCombatCamera)
{
	constexpr auto LOOK_AT_DIST = BLOCK(0.5f);

	// Move camera.
	if (IsUsingModernControls() || Camera.IsControllingTankCamera)
	{
		// Calcuate ideal position and its direction from lookAt.
		auto dir = -EulerAngles(Camera.actualElevation, Camera.actualAngle, 0).ToDirection();
		auto idealPos = Geometry::TranslatePoint(Camera.LookAt, dir, Camera.targetDistance);
		int idealRoomNumber = GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, dir, Camera.targetDistance).RoomNumber;

		// Calculate LOS intersection.
		auto intersect = GetCameraLosIntersect(idealPos, idealRoomNumber, dir);
		if (intersect.has_value())
		{
			idealPos = intersect->first;
			idealRoomNumber = intersect->second;
		}

		// Update camera position.
		float speedCoeff = (Camera.type != CameraType::Look) ? 0.2f : 1.0f;
		MoveCamera(playerItem, idealPos, idealRoomNumber, Camera.speed * speedCoeff);

		// Calculate lookAt.
		auto lookAtDir = Camera.LookAt - Camera.Position;
		lookAtDir.Normalize();
		auto lookAtBackup = Camera.LookAt;
		int lookAtRoomNumberBackup = Camera.LookAtRoomNumber;
		auto lookAtPos = Geometry::TranslatePoint(Camera.LookAt, lookAtDir, LOOK_AT_DIST);

		// TODO: Ceilings not handled correctly.
		// Handle look at.
		Camera.LookAt = lookAtPos;
		Camera.LookAtRoomNumber = playerItem.RoomNumber;
		LookAt(Camera, 0);
		Camera.LookAt = lookAtBackup;
		Camera.LookAtRoomNumber = lookAtRoomNumberBackup;
	}
	else
	{
		auto farthestIdealPos = std::pair<Vector3, int>(Camera.Position, Camera.RoomNumber);
		short farthestIdealAzimuthAngle = Camera.actualAngle;
		float farthestDistSqr = INFINITY;

		// Determine ideal position around player.
		for (int i = 0; i < SWIVEL_STEP_COUNT; i++)
		{
			short azimuthAngle = (i == 0) ? Camera.actualAngle : ANGLE(90.0f * (i - 1));
			auto dir = -EulerAngles(Camera.actualElevation, azimuthAngle, 0).ToDirection();
			auto idealPos = Geometry::TranslatePoint(Camera.LookAt, dir, Camera.targetDistance);
			int idealRoomNumber = GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, dir, Camera.targetDistance).RoomNumber;

			// Assess room LOS.
			auto lookAt = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);
			auto roomLosIntersect = GameVector(idealPos, idealRoomNumber);

			bool los = LOSAndReturnTarget(&lookAt, &roomLosIntersect, 0);
			float dist = Vector3::Distance(lookAt.ToVector3(), roomLosIntersect.ToVector3());
			idealPos = Geometry::TranslatePoint(Camera.LookAt, dir, dist);

			if (los)
			{
				if (i == 0 || LOSAndReturnTarget(&roomLosIntersect, &GameVector(Camera.Position, Camera.RoomNumber), 0))
				{
					// Position directly behind is ideal; set and break loop.
					if (i == 0)
					{
						farthestIdealPos = std::pair(idealPos, roomLosIntersect.RoomNumber);
						farthestIdealAzimuthAngle = azimuthAngle;
						break;
					}

					float distSqr = Vector3::DistanceSquared(Camera.Position, idealPos);
					if (distSqr < farthestDistSqr)
					{
						farthestIdealPos = std::pair(idealPos, roomLosIntersect.RoomNumber);
						farthestIdealAzimuthAngle = azimuthAngle;
						farthestDistSqr = distSqr;
					}
				}
			}
			else if (i == 0)
			{
				float distSqr = Vector3::DistanceSquared(Camera.LookAt, idealPos);
				if (distSqr > SQUARE(BLOCK(0.75f)))
				{
					farthestIdealPos = std::pair(idealPos, roomLosIntersect.RoomNumber);
					farthestIdealAzimuthAngle = azimuthAngle;
					break;
				}
			}
		}

		Camera.actualAngle = farthestIdealAzimuthAngle;
		//CameraCollisionBounds(&farthestIdealPos, CLICK(1.5f), true);

		if (isCombatCamera)
		{
			// Snap position of fixed camera type.
			if (Camera.oldType == CameraType::Fixed)
				Camera.speed = 1.0f;
		}

		// Calculate LOS intersection.
		auto dir = -EulerAngles(Camera.actualElevation, farthestIdealAzimuthAngle, 0).ToDirection();
		auto intersect = GetCameraLosIntersect(farthestIdealPos.first, farthestIdealPos.second, dir);
		if (intersect.has_value())
			farthestIdealPos = *intersect;

		MoveCamera(playerItem, farthestIdealPos.first, farthestIdealPos.second, Camera.speed);
	}
}

void ChaseCamera(const ItemInfo& playerItem)
{
	const auto& player = GetLaraInfo(playerItem);

	if (Camera.targetElevation == 0)
		Camera.targetElevation = ANGLE(-10.0f);

	Camera.targetElevation += playerItem.Pose.Orientation.x;
	UpdateCameraSphere(playerItem);
	ClampCameraAltitudeAngle(player.Control.WaterStatus == WaterStatus::Underwater);

	auto pointColl = GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, 0, 0, CLICK(1));

	if (TestEnvironment(ENV_FLAG_SWAMP, pointColl.RoomNumber))
		Camera.LookAt.y = g_Level.Rooms[pointColl.RoomNumber].maxceiling - CLICK(1);

	int vPos = Camera.LookAt.y;
	pointColl = GetCollision(Camera.LookAt.x, vPos, Camera.LookAt.z, Camera.LookAtRoomNumber);
	if (((vPos < pointColl.Position.Ceiling || pointColl.Position.Floor < vPos) || pointColl.Position.Floor <= pointColl.Position.Ceiling) ||
		(pointColl.Position.Floor == NO_HEIGHT || pointColl.Position.Ceiling == NO_HEIGHT))
	{
		TargetSnaps++;
		Camera.LookAt = LastTarget.ToVector3();
		Camera.LookAtRoomNumber = LastTarget.RoomNumber;
	}
	else
	{
		TargetSnaps = 0;
	}

	HandleCameraFollow(playerItem, false);
}

void CombatCamera(const ItemInfo& playerItem)
{
	const auto& player = GetLaraInfo(playerItem);

	Camera.LookAt.x = playerItem.Pose.Position.x;
	Camera.LookAt.z = playerItem.Pose.Position.z;

	if (player.TargetEntity != nullptr)
	{
		Camera.targetAngle = player.TargetArmOrient.y;
		Camera.targetElevation = player.TargetArmOrient.x + playerItem.Pose.Orientation.x;
	}
	else
	{
		Camera.targetAngle = player.ExtraHeadRot.y + player.ExtraTorsoRot.y;
		Camera.targetElevation = player.ExtraHeadRot.x + player.ExtraTorsoRot.x + playerItem.Pose.Orientation.x - ANGLE(15.0f);
	}

	auto pointColl = GetCollision(Camera.LookAt, Camera.LookAtRoomNumber, 0, 0, CLICK(1));
	if (TestEnvironment(ENV_FLAG_SWAMP, pointColl.RoomNumber))
		Camera.LookAt.y = g_Level.Rooms[pointColl.RoomNumber].y - CLICK(1);

	pointColl = GetCollision(Camera.LookAt.x, Camera.LookAt.y, Camera.LookAt.z, Camera.LookAtRoomNumber);
	Camera.LookAtRoomNumber = pointColl.RoomNumber;

	int buffer = CLICK(0.25f);
	if ((pointColl.Position.Ceiling + buffer) > (pointColl.Position.Floor - buffer) &&
		pointColl.Position.Floor != NO_HEIGHT &&
		pointColl.Position.Ceiling != NO_HEIGHT)
	{
		Camera.LookAt.y = (pointColl.Position.Ceiling + pointColl.Position.Floor) / 2;
		Camera.targetElevation = 0;
	}
	else if (Camera.LookAt.y > (pointColl.Position.Floor - buffer) &&
		pointColl.Position.Floor != NO_HEIGHT)
	{
		Camera.LookAt.y = pointColl.Position.Floor - buffer;
		Camera.targetElevation = 0;
	}
	else if (Camera.LookAt.y < (pointColl.Position.Ceiling + buffer) &&
		pointColl.Position.Ceiling != NO_HEIGHT)
	{
		Camera.LookAt.y = pointColl.Position.Ceiling + buffer;
		Camera.targetElevation = 0;
	}

	int y = Camera.LookAt.y;
	pointColl = GetCollision(Camera.LookAt.x, y, Camera.LookAt.z, Camera.LookAtRoomNumber);
	Camera.LookAtRoomNumber = pointColl.RoomNumber;

	if (y < pointColl.Position.Ceiling ||
		y > pointColl.Position.Floor ||
		pointColl.Position.Ceiling >= pointColl.Position.Floor ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT)
	{
		TargetSnaps++;
		Camera.LookAt = LastTarget.ToVector3();
		Camera.LookAtRoomNumber = LastTarget.RoomNumber;
	}
	else
	{
		TargetSnaps = 0;
	}

	UpdateCameraSphere(playerItem);
	ClampCameraAltitudeAngle(player.Control.WaterStatus == WaterStatus::Underwater);

	Camera.targetDistance = BLOCK(1.5f);

	HandleCameraFollow(playerItem, true);
}

static EulerAngles GetCameraControlRotation()
{
	constexpr auto AXIS_SENSITIVITY_COEFF = 25.0f;
	constexpr auto SMOOTHING_FACTOR		  = 8.0f;

	bool isUsingMouse = (GetCameraAxis() == Vector2::Zero);
	auto axisSign = Vector2(g_Configuration.InvertCameraXAxis ? -1 : 1, g_Configuration.InvertCameraYAxis ? -1 : 1);

	// Calculate axis.
	auto axis = (isUsingMouse ? GetMouseAxis() : GetCameraAxis()) * axisSign;
	float sensitivity = AXIS_SENSITIVITY_COEFF / (1.0f + (abs(axis.x) + abs(axis.y)));
	axis *= sensitivity * (isUsingMouse ? SMOOTHING_FACTOR : 1.0f); // TODO: Add camera sensitivity setting? Unify mouse and analog stick somehow.

	// Calculate and return rotation.
	return EulerAngles(ANGLE(axis.x), ANGLE(axis.y), 0);
}

static bool CanControlTankCamera()
{
	if (!g_Configuration.EnableTankCameraControl)
		return false;

	bool isUsingMouse = (GetCameraAxis() == Vector2::Zero);
	const auto& axis = isUsingMouse ? GetMouseAxis() : GetCameraAxis();

	// Test if player is stationary.
	if (!IsWakeActionHeld() && (axis != Vector2::Zero || Camera.IsControllingTankCamera))
		return true;

	// Test if player is moving and camera or mouse axis isn't zero.
	if (IsWakeActionHeld() && axis != Vector2::Zero)
		return true;

	return false;
}

void UpdateCameraSphere(const ItemInfo& playerItem)
{
	constexpr auto COMBAT_CAMERA_REBOUND_ALPHA = 0.3f;
	constexpr auto ALTITUDE_ROT_ALPHA		   = 1 / 8.0f;

	if (Camera.laraNode != -1)
	{
		auto origin = GetJointPosition(playerItem, Camera.laraNode, Vector3i::Zero);
		auto target = GetJointPosition(playerItem, Camera.laraNode, Vector3i(0, -CLICK(1), BLOCK(2)));
		auto deltaPos = target - origin;

		Camera.actualAngle = Camera.targetAngle + FROM_RAD(atan2(deltaPos.x, deltaPos.z));
		Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) * ALTITUDE_ROT_ALPHA;
	}
	else
	{
		if (IsUsingModernControls())
		{
			auto rot = GetCameraControlRotation();

			if (IsPlayerInCombat(playerItem))
			{
				short azimuthRot = Geometry::GetShortestAngle(Camera.actualAngle, (playerItem.Pose.Orientation.y + Camera.targetAngle) + rot.x);
				short altitudeRot = Geometry::GetShortestAngle(Camera.actualElevation, Camera.targetElevation - rot.y);

				Camera.actualAngle += azimuthRot * COMBAT_CAMERA_REBOUND_ALPHA;
				Camera.actualElevation += altitudeRot * COMBAT_CAMERA_REBOUND_ALPHA;
			}
			else
			{
				Camera.actualAngle += rot.x;
				Camera.actualElevation -= rot.y;
			}
		}
		else
		{
			if (CanControlTankCamera())
			{
				auto rot = GetCameraControlRotation();

				Camera.actualAngle += rot.x;
				Camera.actualElevation -= rot.y;
				Camera.IsControllingTankCamera = true;
			}
			else
			{
				Camera.actualAngle = playerItem.Pose.Orientation.y + Camera.targetAngle;
				Camera.actualElevation += (Camera.targetElevation - Camera.actualElevation) * ALTITUDE_ROT_ALPHA;
				Camera.IsControllingTankCamera = false;
			}
		}
	}
}

void FixedCamera()
{
	// Fixed cameras before TR3 had optional "movement" effect. 
	// Later for some reason it was forced to always be 1, and actual speed value
	// from camera trigger was ignored. In TEN, speed value was moved out out of legacy
	// floordata trigger to camera itself to make use of it again. Still, by default,
	// value is 1 for UseForcedFixedCamera hack.

	float speed = 1.0f;

	auto origin = GameVector::Zero;
	if (UseForcedFixedCamera)
	{
		origin = ForcedFixedCamera;
	}
	else
	{
		const auto& camera = g_Level.Cameras[Camera.number];

		origin = GameVector(camera.Position, camera.RoomNumber);
		speed = (camera.Speed * 8) + 1.0f; // Multiply original speed by 8 to comply with original bitshifted speed from TR1-2.
	}

	Camera.fixedCamera = true;

	MoveCamera(*LaraItem, origin.ToVector3(), origin.RoomNumber, speed);

	if (Camera.timer)
	{
		if (!--Camera.timer)
			Camera.timer = -1;
	}
}

void BounceCamera(ItemInfo* item, int bounce, float distMax)
{
	float dist = Vector3i::Distance(item->Pose.Position.ToVector3(), Camera.Position);
	if (dist < distMax)
	{
		if (distMax == -1)
		{
			Camera.bounce = bounce;
		}
		else
		{
			Camera.bounce = -(bounce * (distMax - dist) / distMax);
		}
	}
	else if (distMax == -1)
	{
		Camera.bounce = bounce;
	}
}

void BinocularCamera(ItemInfo* item)
{
	auto& player = GetLaraInfo(*item);

	if (!player.Control.Look.IsUsingLasersight)
	{
		if (IsClicked(In::Deselect) ||
			IsClicked(In::Draw) ||
			IsClicked(In::Look) ||
			IsHeld(In::Flare))
		{
			ResetPlayerFlex(item);
			player.Control.Look.OpticRange = 0;
			player.Control.Look.IsUsingBinoculars = false;
			player.Inventory.IsBusy = false;

			Camera.type = BinocularOldCamera;
			AlterFOV(LastFOV);
			return;
		}
	}

	AlterFOV(7 * (ANGLE(11.5f) - player.Control.Look.OpticRange), false);

	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y - CLICK(2);
	int z = item->Pose.Position.z;

	auto probe = GetCollision(x, y, z, item->RoomNumber);
	if (probe.Position.Ceiling <= (y - CLICK(1)))
	{
		y -= CLICK(1);
	}
	else
	{
		y = probe.Position.Ceiling + CLICK(0.25f);
	}

	Camera.Position.x = x;
	Camera.Position.y = y;
	Camera.Position.z = z;
	Camera.RoomNumber = probe.RoomNumber;

	float l = BLOCK(20.25f) * phd_cos(player.Control.Look.Orientation.x);
	float tx = x + l * phd_sin(item->Pose.Orientation.y + player.Control.Look.Orientation.y);
	float ty = y - BLOCK(20.25f) * phd_sin(player.Control.Look.Orientation.x);
	float tz = z + l * phd_cos(item->Pose.Orientation.y + player.Control.Look.Orientation.y);

	if (Camera.oldType == CameraType::Fixed)
	{
		Camera.LookAt.x = tx;
		Camera.LookAt.y = ty;
		Camera.LookAt.z = tz;
		Camera.LookAtRoomNumber = item->RoomNumber;
	}
	else
	{
		Camera.LookAt.x += (tx - Camera.LookAt.x) / 4;
		Camera.LookAt.y += (ty - Camera.LookAt.y) / 4;
		Camera.LookAt.z += (tz - Camera.LookAt.z) / 4;
		Camera.LookAtRoomNumber = item->RoomNumber;
	}

	if (Camera.bounce &&
		Camera.type == Camera.oldType)
	{
		if (Camera.bounce <= 0)
		{
			Camera.LookAt.x += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.LookAt.y += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.LookAt.z += (CLICK(0.25f) / 4) * (GetRandomControl() % (-Camera.bounce) - (-Camera.bounce / 2));
			Camera.bounce += 5;
			RumbleFromBounce();
		}
		else
		{
			Camera.bounce = 0;
			Camera.LookAt.y += Camera.bounce;
		}
	}

	Camera.LookAtRoomNumber = GetCollision(Camera.Position.x, Camera.Position.y, Camera.Position.z, Camera.LookAtRoomNumber).RoomNumber;
	LookAt(Camera, 0);
	UpdateMikePos(*item);
	Camera.oldType = Camera.type;

	auto origin0 = GameVector(Camera.Position, Camera.RoomNumber);
	auto target0 = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);
	GetTargetOnLOS(&origin0, &target0, false, false);
	Camera.LookAt = target0.ToVector3();
	Camera.LookAtRoomNumber = target0.RoomNumber;

	if (IsHeld(In::Action))
	{
		auto origin = Vector3i(Camera.Position);
		auto target = Vector3i(Camera.LookAt);
		LaraTorch(&origin, &target, player.ExtraHeadRot.y, 192);
	}
}

void ConfirmCameraTargetPos()
{
	auto pos = Vector3(
		LaraItem->Pose.Position.x,
		LaraItem->Pose.Position.y - (LaraCollision.Setup.Height / 2),
		LaraItem->Pose.Position.z);

	if (Camera.laraNode != -1)
	{
		Camera.LookAt = pos;
	}
	else
	{
		Camera.LookAt = Vector3(LaraItem->Pose.Position.x, (Camera.LookAt.y + pos.y) / 2, LaraItem->Pose.Position.z);
	}

	int y = Camera.LookAt.y;
	auto pointColl = GetCollision(Camera.LookAt.x, y, Camera.LookAt.z, Camera.LookAtRoomNumber);
	if (y < pointColl.Position.Ceiling ||
		pointColl.Position.Floor < y ||
		pointColl.Position.Floor <= pointColl.Position.Ceiling ||
		pointColl.Position.Floor == NO_HEIGHT ||
		pointColl.Position.Ceiling == NO_HEIGHT)
	{
		Camera.LookAt = pos;
	}
}

static void UpdatePlayerRefCameraOrient(ItemInfo& item)
{
	auto& player = GetLaraInfo(item);

	bool isFixedCameraSwitch = (Camera.type == CameraType::Fixed && (Camera.oldType == CameraType::Chase || Camera.oldType == CameraType::Combat)) ||
							   ((Camera.type == CameraType::Chase || Camera.type == CameraType::Combat) && Camera.oldType == CameraType::Fixed);
	bool hasMoveAction = (GetMoveAxis() != Vector2::Zero || IsHeld(In::StepLeft) || IsHeld(In::StepRight));

	if (isFixedCameraSwitch && hasMoveAction)
	{
		player.Control.LockRefCameraOrient = true;
	}
	else if (!hasMoveAction)
	{
		player.Control.LockRefCameraOrient = false;
	}

	if (!player.Control.LockRefCameraOrient)
		player.Control.RefCameraOrient = EulerAngles(Camera.actualElevation, Camera.actualAngle, 0);
}

void CalculateCamera(ItemInfo& playerItem, const CollisionInfo& coll)
{
	auto& player = GetLaraInfo(playerItem);

	if (player.Control.Look.IsUsingBinoculars)
	{
		BinocularCamera(&playerItem);
		return;
	}

	if (ItemCamera.ItemCameraOn)
		return;

	if (UseForcedFixedCamera != 0)
	{
		Camera.type = CameraType::Fixed;
		if (Camera.oldType != CameraType::Fixed)
			Camera.speed = 1.0f;
	}

	// Play water sound effect if camera is in water room.
	if (TestEnvironment(ENV_FLAG_WATER, Camera.RoomNumber))
	{
		SoundEffect(SFX_TR4_UNDERWATER, nullptr, SoundEnvironment::Always);
		if (Camera.underwater == false)
			Camera.underwater = true;
	}
	else
	{
		if (Camera.underwater == true)
			Camera.underwater = false;
	}

	const ItemInfo* itemPtr = nullptr;
	bool isFixedCamera = false;
	if (Camera.item != nullptr && (Camera.type == CameraType::Fixed || Camera.type == CameraType::Heavy))
	{
		itemPtr = Camera.item;
		isFixedCamera = true;
	}
	else
	{
		itemPtr = &playerItem;
		isFixedCamera = false;
	}

	// TODO: Use DX box.
	auto box = GameBoundingBox(itemPtr).ToBoundingOrientedBox(itemPtr->Pose);
	auto bounds = GameBoundingBox(itemPtr);

	int x = 0;
	int y = itemPtr->Pose.Position.y + bounds.Y2 + ((bounds.Y1 - bounds.Y2) / 2 * 1.5f);
	int z = 0;
	if (itemPtr->IsLara())
	{
		float heightCoeff = IsUsingModernControls() ? 0.9f : 0.75f;
		int offset = GetCameraPlayerVerticalOffset(*itemPtr, coll) * heightCoeff;
		y = itemPtr->Pose.Position.y + offset;
	}

	// Make player look toward target item.
	if (Camera.item != nullptr)
	{
		if (!isFixedCamera)
		{
			auto deltaPos = Camera.item->Pose.Position - itemPtr->Pose.Position;
			float dist = Vector3i::Distance(Camera.item->Pose.Position, itemPtr->Pose.Position);

			auto lookOrient = EulerAngles(
				phd_atan(dist, y - (bounds.Y1 + bounds.Y2) / 2 - Camera.item->Pose.Position.y),
				phd_atan(deltaPos.z, deltaPos.x) - itemPtr->Pose.Orientation.y,
				0) / 2;

			if (lookOrient.y > ANGLE(-50.0f) &&	lookOrient.y < ANGLE(50.0f) &&
				lookOrient.z > ANGLE(-85.0f) && lookOrient.z < ANGLE(85.0f))
			{
				short angleDelta = lookOrient.y - player.ExtraHeadRot.y;
				if (angleDelta > ANGLE(4.0f))
				{
					player.ExtraHeadRot.y += ANGLE(4.0f);
				}
				else if (angleDelta < ANGLE(-4.0f))
				{
					player.ExtraHeadRot.y -= ANGLE(4.0f);
				}
				else
				{
					player.ExtraHeadRot.y += angleDelta;
				}
				player.ExtraTorsoRot.y = player.ExtraHeadRot.y;

				angleDelta = lookOrient.z - player.ExtraHeadRot.x;
				if (angleDelta > ANGLE(4.0f))
				{
					player.ExtraHeadRot.x += ANGLE(4.0f);
				}
				else if (angleDelta < ANGLE(-4.0f))
				{
					player.ExtraHeadRot.x -= ANGLE(4.0f);
				}
				else
				{
					player.ExtraHeadRot.x += angleDelta;
				}
				player.ExtraTorsoRot.x = player.ExtraHeadRot.x;

				player.Control.Look.Orientation = lookOrient;

				Camera.type = CameraType::Look;
				Camera.item->LookedAt = true;
			}
		}
	}

	if (Camera.type == CameraType::Look ||
		Camera.type == CameraType::Combat)
	{
		if (Camera.type == CameraType::Combat)
		{
			LastTarget = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);

			if (!IsUsingModernControls())
				y -= CLICK(1);
		}

		Camera.LookAtRoomNumber = itemPtr->RoomNumber;

		if (Camera.fixedCamera || player.Control.Look.IsUsingBinoculars)
		{
			Camera.LookAt.y = y;
			Camera.speed = 1.0f;
		}
		else
		{
			Camera.LookAt.y += (y - Camera.LookAt.y) / 4;
			Camera.speed = (Camera.type != CameraType::Look) ? 8.0f : 4.0f;
		}

		Camera.fixedCamera = false;
		if (Camera.type == CameraType::Look)
		{
			LookCamera(*itemPtr, coll);
		}
		else
		{
			CombatCamera(*itemPtr);
		}
	}
	else
	{
		LastTarget = GameVector(Camera.LookAt, Camera.LookAtRoomNumber);

		Camera.LookAtRoomNumber = itemPtr->RoomNumber;
		Camera.LookAt.y = y;

		x = itemPtr->Pose.Position.x;
		z = itemPtr->Pose.Position.z;

		// -- Troye 2022.8.7
		if (Camera.flags == CameraFlag::FollowCenter)
		{
			int shift = (bounds.Z1 + bounds.Z2) / 2;
			x += shift * phd_sin(itemPtr->Pose.Orientation.y);
			z += shift * phd_cos(itemPtr->Pose.Orientation.y);
		}

		Camera.LookAt.x = x;
		Camera.LookAt.z = z;

		// CameraFlag::FollowCenter sets target on item and
		// ConfirmCameraTargetPos() overrides this target, hence flag check. -- Troye 2022.8.7
		if (itemPtr->IsLara() && Camera.flags != CameraFlag::FollowCenter)
			ConfirmCameraTargetPos();

		if (isFixedCamera == Camera.fixedCamera)
		{
			Camera.fixedCamera = false;
			if (Camera.speed != 1.0f &&
				!player.Control.Look.IsUsingBinoculars)
			{
				if (TargetSnaps <= 8)
				{
					x = LastTarget.x + ((x - LastTarget.x) / 4);
					y = LastTarget.y + ((y - LastTarget.y) / 4);
					z = LastTarget.z + ((z - LastTarget.z) / 4);

					Camera.LookAt.x = x;
					Camera.LookAt.y = y;
					Camera.LookAt.z = z;
				}
				else
				{
					TargetSnaps = 0;
				}
			}
		}
		else
		{
			Camera.fixedCamera = true;
			Camera.speed = 1.0f;
		}

		Camera.LookAtRoomNumber = GetCollision(x, y, z, Camera.LookAtRoomNumber).RoomNumber;

		if (abs(LastTarget.x - Camera.LookAt.x) < 4 &&
			abs(LastTarget.y - Camera.LookAt.y) < 4 &&
			abs(LastTarget.z - Camera.LookAt.z) < 4)
		{
			Camera.LookAt.x = LastTarget.x;
			Camera.LookAt.y = LastTarget.y;
			Camera.LookAt.z = LastTarget.z;
		}

		if (Camera.type != CameraType::Chase && Camera.flags != CameraFlag::ChaseObject)
		{
			FixedCamera();
		}
		else
		{
			ChaseCamera(*itemPtr);
		}
	}

	Camera.fixedCamera = isFixedCamera;
	Camera.last = Camera.number;

	if ((Camera.type != CameraType::Heavy || Camera.timer == -1) &&
		playerItem.HitPoints > 0)
	{
		Camera.type = CameraType::Chase;
		Camera.speed = 10.0f;
		Camera.number = -1;
		Camera.lastItem = Camera.item;
		Camera.item = nullptr;
		Camera.targetElevation = 0;
		Camera.targetAngle = 0;
		Camera.targetDistance = BLOCK(1.5f);
		Camera.flags = CameraFlag::None;
		Camera.laraNode = -1;
	}

	UpdatePlayerRefCameraOrient(playerItem);
}

bool TestBoundsCollideCamera(const GameBoundingBox& bounds, const Pose& pose, float radius)
{
	auto sphere = BoundingSphere(Camera.Position, radius);
	return sphere.Intersects(bounds.ToBoundingOrientedBox(pose));
}

void UpdateMikePos(const ItemInfo& item)
{
	if (Camera.mikeAtLara)
	{
		Camera.mikePos = item.Pose.Position.ToVector3();
		Camera.actualAngle = item.Pose.Orientation.y;

		if (item.IsLara())
		{
			const auto& player = GetLaraInfo(item);
			Camera.actualAngle += player.ExtraHeadRot.y + player.ExtraTorsoRot.y;
		}
	}
	else
	{
		float dist = Vector3::DistanceSquared(Camera.LookAt, OldCam.target);

		// TODO: Better method.
		// Recalculate azimuth angle.
		if (((IsUsingModernControls() && !IsPlayerStrafing(*LaraItem)) || !IsUsingModernControls()) &&
			dist > 1.0f)
		{
			auto deltaPos = Camera.Position - Camera.Position;
			short targetAzimuthAngle = FROM_RAD(atan2(deltaPos.x, deltaPos.z));

			float alpha = 1.0f / Camera.speed;
			//Camera.actualAngle += Geometry::GetShortestAngle(Camera.actualAngle, targetAzimuthAngle) * alpha;
		}

		int perspective = ((g_Configuration.ScreenWidth / 2) * phd_cos(CurrentFOV / 2)) / phd_sin(CurrentFOV / 2);
		Camera.mikePos.x = Camera.Position.x + (perspective * phd_sin(Camera.actualAngle));
		Camera.mikePos.z = Camera.Position.z + (perspective * phd_cos(Camera.actualAngle));
		Camera.mikePos.y = Camera.Position.y;
	}
}

void RumbleScreen()
{
	if (!(GlobalCounter & 0x1FF))
		SoundEffect(SFX_TR5_KLAXON, nullptr, SoundEnvironment::Land, 0.25f);

	if (RumbleTimer >= 0)
		RumbleTimer++;

	if (RumbleTimer > 450)
	{
		if (!(GetRandomControl() & 0x1FF))
		{
			RumbleCounter = 0;
			RumbleTimer = -32 - (GetRandomControl() & 0x1F);
			return;
		}
	}

	if (RumbleTimer < 0)
	{
		if (RumbleCounter >= abs(RumbleTimer))
		{
			Camera.bounce = -(GetRandomControl() % abs(RumbleTimer));
			RumbleTimer++;
		}
		else
		{
			RumbleCounter++;
			Camera.bounce = -(GetRandomControl() % RumbleCounter);
		}
	}
}

void SetScreenFadeOut(float speed, bool force)
{
	if (ScreenFading && !force)
		return;

	ScreenFading = true;
	ScreenFadeStart = 1.0f;
	ScreenFadeEnd = 0;
	ScreenFadeSpeed = speed;
	ScreenFadeCurrent = ScreenFadeStart;
}

void SetScreenFadeIn(float speed, bool force)
{
	if (ScreenFading && !force)
		return;

	ScreenFading = true;
	ScreenFadeStart = 0.0f;
	ScreenFadeEnd = 1.0f;
	ScreenFadeSpeed = speed;
	ScreenFadeCurrent = ScreenFadeStart;
}

void SetCinematicBars(float height, float speed)
{
	CinematicBarsDestinationHeight = height;
	CinematicBarsSpeed = speed;
}

void ClearCinematicBars()
{
	CinematicBarsHeight = 0.0f;
	CinematicBarsDestinationHeight = 0.0f;
	CinematicBarsSpeed = 0.0f;
}

void UpdateFadeScreenAndCinematicBars()
{
	if (CinematicBarsDestinationHeight < CinematicBarsHeight)
	{
		CinematicBarsHeight -= CinematicBarsSpeed;
		if (CinematicBarsDestinationHeight > CinematicBarsHeight)
			CinematicBarsHeight = CinematicBarsDestinationHeight;
	}
	else if (CinematicBarsDestinationHeight > CinematicBarsHeight)
	{
		CinematicBarsHeight += CinematicBarsSpeed;
		if (CinematicBarsDestinationHeight < CinematicBarsHeight)
			CinematicBarsHeight = CinematicBarsDestinationHeight;
	}

	int prevScreenFadeCurrent = ScreenFadeCurrent;

	if (ScreenFadeEnd != 0 && ScreenFadeEnd >= ScreenFadeCurrent)
	{
		ScreenFadeCurrent += ScreenFadeSpeed;
		if (ScreenFadeCurrent > ScreenFadeEnd)
		{
			ScreenFadeCurrent = ScreenFadeEnd;
			if (prevScreenFadeCurrent >= ScreenFadeCurrent)
			{
				ScreenFadedOut = true;
				ScreenFading = false;
			}

		}
	}
	else if (ScreenFadeEnd < ScreenFadeCurrent)
	{
		ScreenFadeCurrent -= ScreenFadeSpeed;
		if (ScreenFadeCurrent < ScreenFadeEnd)
		{
			ScreenFadeCurrent = ScreenFadeEnd;
			ScreenFading = false;
		}
	}
}

float GetParticleDistanceFade(const Vector3i& pos)
{
	constexpr auto PARTICLE_FADE_THRESHOLD = BLOCK(14);

	float dist = Vector3::Distance(Camera.Position, pos.ToVector3());
	if (dist <= PARTICLE_FADE_THRESHOLD)
		return 1.0f;

	return std::clamp(1.0f - ((dist - PARTICLE_FADE_THRESHOLD) / CAMERA_OBJECT_COLL_DIST_THRESHOLD), 0.0f, 1.0f);
}
