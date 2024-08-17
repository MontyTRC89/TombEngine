#include "framework.h"
#include "Objects/Generic/Object/BridgeObject.h"

//#include "Game/collision/Attractor.h"
#include "Game/items.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/Pushable/PushableInfo.h"
#include "Objects/Generic/Object/Pushable/PushableObject.h"
#include "Physics/Physics.h"
#include "Specific/level.h"

//using namespace TEN::Colllision::Attractor;
using namespace TEN::Math;
using namespace TEN::Physics;

namespace TEN::Entities::Generic
{
	const BoundingBox& BridgeObject::GetAabb() const
	{
		return _aabb;
	}

	const CollisionMesh& BridgeObject::GetCollisionMesh() const
	{
		return _collisionMesh;
	}

	/*const AttractorObject& BridgeObject::GetAttractor() const
	{
		reutrn _attractor;
	}*/

	void BridgeObject::Initialize(const ItemInfo& item)
	{
		UpdateAabb(item);
		InitializeCollisionMesh(item);
		InitializeAttractor(item);
		AssignSectors(item);

		auto& room = g_Level.Rooms[item.RoomNumber];
		room.Bridges.Insert(item.Index, item.GetAabb());

		_prevPose = item.Pose;
		_prevRoomNumber = item.RoomNumber;
	}

	void BridgeObject::Update(const ItemInfo& item)
	{
		if (item.Pose == _prevPose && item.RoomNumber == _prevRoomNumber)
			return;

		UpdateItemRoom(item.Index);
		UpdateAabb(item);
		UpdateCollisionMesh(item);
		UpdateAttractor(item);
		AssignSectors(item);

		auto& room = g_Level.Rooms[item.RoomNumber];
		auto& prevRoom = g_Level.Rooms[_prevRoomNumber];

		if (item.RoomNumber == _prevRoomNumber)
		{
			if (item.Pose != _prevPose)
				room.Bridges.Move(item.Index, item.GetAabb());
		}
		else
		{
			room.Bridges.Insert(item.Index, item.GetAabb());
			prevRoom.Bridges.Remove(item.Index);
		}

		_prevPose = item.Pose;
		_prevRoomNumber = item.RoomNumber;
		_prevPose = item.Pose;
		_prevRoomNumber = item.RoomNumber;
		_prevAabb = _aabb;
		_prevObb = item.GetObb();
	}

	void BridgeObject::DeassignSectors(const ItemInfo& item) const
	{
		// Deassign sectors.
		unsigned int sectorSearchDepth = (unsigned int)ceil(std::max(std::max(_prevObb.Extents.x, _prevObb.Extents.y), _prevObb.Extents.z) / BLOCK(1));
		auto sectors = GetNeighborSectors(_prevPose.Position, _prevRoomNumber, sectorSearchDepth);
		for (auto* sector : sectors)
		{
			// Test if previous AABB intersects sector.
			if (!_prevAabb.Intersects(sector->Aabb))
				continue;

			// Remove previous bridge assignment if within sector.
			if (_prevObb.Intersects(sector->Aabb))
				sector->RemoveBridge(item.Index);
		}
	}

	void BridgeObject::InitializeCollisionMesh(const ItemInfo& item)
	{
		constexpr auto UP_NORMAL	  = Vector3(0.0f, -1.0f, 0.0f);
		constexpr auto DOWN_NORMAL	  = Vector3(0.0f, 1.0f, 0.0f);
		constexpr auto FORWARD_NORMAL = Vector3(0.0f, 0.0f, 1.0f);
		constexpr auto BACK_NORMAL	  = Vector3(0.0f, 0.0f, -1.0f);
		constexpr auto LEFT_NORMAL	  = Vector3(-1.0f, 0.0f, 0.0f);
		constexpr auto RIGHT_NORMAL	  = Vector3(1.0f, 0.0f, 0.0f);

		// Determine relative tilt offset.
		auto offset = Vector3::Zero;
		auto tiltRotMatrix = Matrix::Identity;
		switch (item.ObjectNumber)
		{
		case ID_BRIDGE_TILT1:
			offset = Vector3(0.0f, CLICK(1), 0.0f);
			tiltRotMatrix = EulerAngles(0, 0, ANGLE(-45.0f * 0.25f)).ToRotationMatrix();
			break;

		case ID_BRIDGE_TILT2:
			offset = Vector3(0.0f, CLICK(2), 0.0f);
			tiltRotMatrix = EulerAngles(0, 0, ANGLE(-45.0f * 0.5f)).ToRotationMatrix();
			break;

		case ID_BRIDGE_TILT3:
			offset = Vector3(0.0f, CLICK(3), 0.0f);
			tiltRotMatrix = EulerAngles(0, 0, ANGLE(-45.0f * 0.75f)).ToRotationMatrix();
			break;

		case ID_BRIDGE_TILT4:
			offset = Vector3(0.0f, CLICK(4), 0.0f);
			tiltRotMatrix = EulerAngles(0, 0, ANGLE(-45.0f)).ToRotationMatrix();
			break;

		default:
			break;
		}

		// Get local AABB corners.
		auto bounds = GetAnimFrame(item, 0, 0).BoundingBox;
		auto aabb = BoundingBox(bounds.GetCenter(), bounds.GetExtents());
		auto corners = std::array<Vector3, BoundingOrientedBox::CORNER_COUNT>{};
		aabb.GetCorners(corners.data());

		// Offset key corners.
		corners[0] += offset;
		corners[4] += offset;
		corners[2] -= offset;
		corners[6] -= offset;

		// Build collision mesh.
		_collisionMesh.InsertTriangle(corners[0], corners[1], corners[4], Vector3::Transform(UP_NORMAL, tiltRotMatrix));
		_collisionMesh.InsertTriangle(corners[1], corners[4], corners[5], Vector3::Transform(UP_NORMAL, tiltRotMatrix));
		_collisionMesh.InsertTriangle(corners[2], corners[3], corners[6], Vector3::Transform(DOWN_NORMAL, tiltRotMatrix));
		_collisionMesh.InsertTriangle(corners[3], corners[6], corners[7], Vector3::Transform(DOWN_NORMAL, tiltRotMatrix));
		_collisionMesh.InsertTriangle(corners[0], corners[1], corners[2], FORWARD_NORMAL);
		_collisionMesh.InsertTriangle(corners[0], corners[2], corners[3], FORWARD_NORMAL);
		_collisionMesh.InsertTriangle(corners[4], corners[5], corners[6], BACK_NORMAL);
		_collisionMesh.InsertTriangle(corners[4], corners[6], corners[7], BACK_NORMAL);
		_collisionMesh.InsertTriangle(corners[0], corners[3], corners[4], LEFT_NORMAL);
		_collisionMesh.InsertTriangle(corners[3], corners[4], corners[7], LEFT_NORMAL);
		_collisionMesh.InsertTriangle(corners[1], corners[2], corners[5], RIGHT_NORMAL);
		_collisionMesh.InsertTriangle(corners[2], corners[5], corners[6], RIGHT_NORMAL);
		_collisionMesh.Cook();

		// Set collision mesh position and orientation.
		_collisionMesh.SetPosition(item.Pose.Position.ToVector3());
		_collisionMesh.SetOrientation(item.Pose.Orientation.ToQuaternion());
	}

	void BridgeObject::InitializeAttractor(const ItemInfo& item)
	{
		// TODO
	}

	void BridgeObject::UpdateAabb(const ItemInfo& item)
	{
		_aabb = Geometry::GetBoundingBox(item.GetObb());
	}

	void BridgeObject::UpdateCollisionMesh(const ItemInfo& item)
	{
		_collisionMesh.SetPosition(item.Pose.Position.ToVector3());
		_collisionMesh.SetOrientation(item.Pose.Orientation.ToQuaternion());
	}

	void BridgeObject::UpdateAttractor(const ItemInfo& item)
	{
		// TODO
	}

	void BridgeObject::AssignSectors(const ItemInfo& item)
	{
		// Deassign sectors at previous position.
		DeassignSectors(item);
		if (item.Flags & IFLAG_KILLED)
			return;

		// Get OBB.
		auto obb = item.GetObb();

		// Assign sectors.
		int sectorSearchDepth = (int)ceil(std::max(std::max(obb.Extents.x, obb.Extents.y), obb.Extents.z) / BLOCK(1));
		auto sectors = GetNeighborSectors(item.Pose.Position, item.RoomNumber, sectorSearchDepth);
		for (auto* sector : sectors)
		{
			// Test if AABB intersects sector.
			if (!_aabb.Intersects(sector->Aabb))
				continue;

			// Add bridge assignment if within sector.
			if (obb.Intersects(sector->Aabb))
				sector->AddBridge(item.Index);
		}
	}

	const BridgeObject& GetBridgeObject(const ItemInfo& item)
	{
		// HACK: Pushable bridges.
		if (item.Data.is<PushableInfo>())
		{
			const auto& pushable = GetPushableInfo(item);
			return pushable.Bridge;
		}

		return (BridgeObject&)item.Data;
	}

	BridgeObject& GetBridgeObject(ItemInfo& item)
	{
		// HACK: Pushable bridges.
		if (item.Data.is<PushableInfo>())
		{
			auto& pushable = GetPushableInfo(item);
			return pushable.Bridge;
		}

		return (BridgeObject&)item.Data;
	}
}
