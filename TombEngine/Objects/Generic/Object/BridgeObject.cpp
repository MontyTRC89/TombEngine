#include "framework.h"
#include "Objects/Generic/Object/BridgeObject.h"

// TODO: Uncomment when attractors are complete.
//#include "Game/collision/Attractor.h"
#include "Game/items.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/Pushable/PushableInfo.h"
#include "Objects/Generic/Object/Pushable/PushableObject.h"

// TODO: Uncomment when attractors are complete.
//using namespace TEN::Colllision::Attractor;
using namespace TEN::Math;

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

	// TODO: Uncomment when attractors are complete.
	/*const AttractorObject& BridgeObject::GetAttractor() const
	{
		return _attractor;
	}*/

	void BridgeObject::Initialize(const ItemInfo& item)
	{
		// @BRIDGEME
		// TODO: For some reason, AssignSectors() doesn't work on its own when initialising at level start.
		// Must keep using this for now. -- Sezz 2024.11.04
		UpdateBridgeItem(item);

		UpdateAabb(item);
		InitializeCollisionMesh(item);
		InitializeAttractor(item);
		//AssignSectors(item);

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
		// TODO: Remove.
		UpdateBridgeItem(item, true);

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
		// Determine relative tilt offset.
		auto offset = Vector3::Zero;
		switch (item.ObjectNumber)
		{
		case ID_BRIDGE_TILT1:
			offset = Vector3(0.0f, CLICK(1), 0.0f);
			break;

		case ID_BRIDGE_TILT2:
			offset = Vector3(0.0f, CLICK(2), 0.0f);
			break;

		case ID_BRIDGE_TILT3:
			offset = Vector3(0.0f, CLICK(3), 0.0f);
			break;

		case ID_BRIDGE_TILT4:
			offset = Vector3(0.0f, CLICK(4), 0.0f);
			break;

		default:
			break;
		}

		// Get local AABB corners.
		const auto& bounds = GetAnimFrame(item, 0, 0).BoundingBox;
		auto aabb = BoundingBox(bounds.GetCenter(), bounds.GetExtents());
		auto corners = std::array<Vector3, BoundingBox::CORNER_COUNT>{};
		aabb.GetCorners(corners.data());

		// Offset key corners.
		corners[0] += offset;
		corners[4] += offset;
		corners[2] -= offset;
		corners[6] -= offset;

		// Define collision mesh description.
		auto desc = CollisionMeshDesc();
		desc.InsertTriangle(corners[4], corners[1], corners[0]);
		desc.InsertTriangle(corners[1], corners[4], corners[5]);
		desc.InsertTriangle(corners[6], corners[3], corners[2]);
		desc.InsertTriangle(corners[3], corners[6], corners[7]);
		desc.InsertTriangle(corners[0], corners[1], corners[2]);
		desc.InsertTriangle(corners[0], corners[2], corners[3]);
		desc.InsertTriangle(corners[6], corners[5], corners[4]);
		desc.InsertTriangle(corners[7], corners[6], corners[4]);
		desc.InsertTriangle(corners[0], corners[3], corners[4]);
		desc.InsertTriangle(corners[7], corners[4], corners[3]);
		desc.InsertTriangle(corners[5], corners[2], corners[1]);
		desc.InsertTriangle(corners[2], corners[5], corners[6]);

		// Set collision mesh.
		_collisionMesh = CollisionMesh(item.Pose.Position.ToVector3(), item.Pose.Orientation.ToQuaternion(), desc);
	}

	void BridgeObject::InitializeAttractor(const ItemInfo& item)
	{
		// TODO: Implement once attractors are complete.
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
		// TODO: Uncomment when attractors are complete.
		//_attractor.SetPosition(item.Pose.Position.ToVector3());
		//_attractor.SetOrientation(item.Pose.Orientation.ToQuaternion());
	}

	void BridgeObject::AssignSectors(const ItemInfo& item)
	{
		// Deassign sectors from previous position.
		DeassignSectors(item);
		if (item.Flags & IFLAG_KILLED)
			return;

		// Get OBB.
		auto obb = item.GetObb();

		// Assign sectors.
		int searchDepth = (int)ceil(std::max(std::max(obb.Extents.x, obb.Extents.y), obb.Extents.z) / BLOCK(1));
		auto sectors = GetNeighborSectors(item.Pose.Position, item.RoomNumber, searchDepth);
		for (auto* sector : sectors)
		{
			// Test if AABB intersects sector.
			if (!_aabb.Intersects(sector->Aabb))
				continue;

			// Test if OBB intersects sector.
			if (!obb.Intersects(sector->Aabb))
				continue;

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
