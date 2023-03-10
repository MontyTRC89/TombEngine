#include "framework.h"
#include "Game/effects/hair.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Renderer/Renderer11.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Environment;
using TEN::Renderer::g_Renderer;

namespace TEN::Effects::Hair
{
	// DEBUG
	static bool drawSpheres = false;

	HairEffectController HairEffect = {};

	void HairUnit::Update(const ItemInfo& item, int hairUnitIndex)
	{
		const auto& player = GetLaraInfo(item);

		bool isYoung = (g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);

		// Get world matrix from head bone.
		auto worldMatrix = Matrix::Identity;
		g_Renderer.GetBoneMatrix(player.ItemNumber, LM_HEAD, &worldMatrix);

		// Apply base offset to world matrix.
		auto relOffset = this->GetRelBaseOffset(hairUnitIndex, isYoung);
		worldMatrix = Matrix::CreateTranslation(relOffset) * worldMatrix;

		// Set position of base segment.
		auto basePos = worldMatrix.Translation();
		this->Segments[0].Position = basePos;

		if (IsInitialized)
		{
			this->IsInitialized = false;

			// Update segment positions.
			for (int i = 0; i < Segments.size() - 1; i++)
			{
				auto& segment = this->Segments[i];
				auto& nextSegment = this->Segments[i + 1];

				// NOTE: Joint offset determines segment length.
				auto jointOffset = GetJointOffset(ID_HAIR, i);

				worldMatrix = Matrix::CreateTranslation(segment.Position);
				worldMatrix = segment.Orientation.ToRotationMatrix() * worldMatrix;
				worldMatrix = Matrix::CreateTranslation(jointOffset) * worldMatrix;

				nextSegment.Position = worldMatrix.Translation();
			}
		}
		else
		{
			auto* framePtr = this->GetFramePtr(item);

			// Get water height.
			auto pos = item.Pose.Position + framePtr->boundingBox.GetCenter();
			int roomNumber = item.RoomNumber;
			int waterHeight = GetWaterHeight(pos.x, pos.y, pos.z, roomNumber);

			// Get collision spheres.
			auto spheres = GetSpheres(item, isYoung);

			// Update segments.
			for (int i = 1; i < Segments.size(); i++)
			{
				auto& segment = this->Segments[i];
				auto& prevSegment = this->Segments[i - 1];

				// TR3 UPV uses a hack which forces player water status to dry. 
				// Therefore, cannot directly use water status value to determine enrironment.
				bool isOnLand = (player.Control.WaterStatus == WaterStatus::Dry &&
								 (player.Vehicle == -1 || g_Level.Items[player.Vehicle].ObjectNumber != ID_UPV));

				// Handle segment room collision.
				this->CollideSegmentWithRoom(segment, waterHeight, roomNumber, isOnLand);

				// Handle segment sphere collision.
				this->CollideSegmentWithSpheres(segment, spheres);

				// Calculate orientation.
				prevSegment.Orientation = this->GetOrientation(prevSegment.Position, segment.Position);

				// Calculate world matrix.
				worldMatrix = Matrix::CreateTranslation(prevSegment.Position);
				worldMatrix = prevSegment.Orientation.ToRotationMatrix() * worldMatrix;

				auto jointOffset = ((i - 1) == (Segments.size() - 1)) ?
					GetJointOffset(ID_HAIR, (i - 1) - 1) :
					GetJointOffset(ID_HAIR, (i - 1));
				worldMatrix = Matrix::CreateTranslation(jointOffset) * worldMatrix;

				segment.Position = worldMatrix.Translation();
				segment.Velocity = segment.Position - Segments[0].Velocity;

				// DEBUG
				//if (drawSpheres)
					//g_Renderer.AddSphere(segment.Position, 20, Vector4::One);
			}
		}
	}

	Vector3 HairUnit::GetRelBaseOffset(int hairUnitIndex, bool isYoung)
	{
		auto relOffset = Vector3::Zero;
		if (isYoung)
		{
			switch (hairUnitIndex)
			{
			// Left pigtail offset.
			case 0:
				relOffset = Vector3(-52.0f, -48.0f, -50.0f);
				break;

			// Right pigtail offset.
			case 1:
				relOffset = Vector3(44.0f, -48.0f, -50.0f);
				break;
			}
		}
		else
		{
			// Center braid offset.
			relOffset = Vector3(-4.0f, -4.0f, -48.0f);
		}

		return relOffset;
	}

	AnimFrame* HairUnit::GetFramePtr(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

		// TODO: Not needed?
		AnimFrame* framePtr = nullptr;
		if (player.HitDirection >= 0)
		{
			int spasm = 0;
			switch (player.HitDirection)
			{
			case NORTH:
				spasm = (player.Control.IsLow) ? 294 : 125;
				break;

			case SOUTH:
				spasm = (player.Control.IsLow) ? 293 : 126;
				break;

			case EAST:
				spasm = (player.Control.IsLow) ? 295 : 127;
				break;

			default:
				spasm = (player.Control.IsLow) ? 296 : 128;
				break;
			}

			framePtr = &g_Level.Frames[g_Level.Anims[spasm].FramePtr + player.HitFrame];
		}
		else
		{
			framePtr = GetBestFrame(&item);
		}

		return framePtr;
	}

	std::vector<BoundingSphere> HairUnit::GetSpheres(const ItemInfo& item, bool isYoung)
	{
		constexpr auto SPHERE_COUNT = 6;

		auto spheres = std::vector<BoundingSphere>{};
		spheres.reserve(SPHERE_COUNT);

		// Hips sphere.
		auto* meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_HIPS]];
		auto pos = GetJointPosition(item, LM_HIPS, Vector3i(meshPtr->sphere.Center)).ToVector3();
		spheres.push_back(BoundingSphere(pos, meshPtr->sphere.Radius));

		// Torso sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_TORSO]];
		pos = GetJointPosition(item, LM_TORSO, Vector3i(meshPtr->sphere.Center) + Vector3i(-10, 0, 25)).ToVector3();
		spheres.push_back(BoundingSphere(pos, meshPtr->sphere.Radius));
		if (isYoung)
			spheres.back().Radius = spheres.back().Radius - ((spheres.back().Radius / 4) + (spheres.back().Radius / 8));

		// Head sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_HEAD]];
		pos = GetJointPosition(item, LM_HEAD, Vector3i(meshPtr->sphere.Center) + Vector3i(-2, 0, 0)).ToVector3();
		spheres.push_back(BoundingSphere(pos, meshPtr->sphere.Radius));

		// Right arm sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_RINARM]];
		pos = GetJointPosition(item, LM_RINARM, Vector3i(meshPtr->sphere.Center)).ToVector3();
		spheres.push_back(BoundingSphere(pos, (meshPtr->sphere.Radius / 3.0f) * 4));

		// Left arm sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_LINARM]];
		pos = GetJointPosition(item, LM_LINARM, Vector3i(meshPtr->sphere.Center)).ToVector3();
		spheres.push_back(BoundingSphere(pos, (meshPtr->sphere.Radius / 3.0f) * 4));

		if (isYoung)
			spheres[1].Center = (spheres[1].Center + spheres[2].Center) / 2;

		// Neck sphere.
		spheres.push_back(BoundingSphere(
			(spheres[1].Center + (spheres[2].Center * 2)) / 3,
			isYoung ? 0 : int(float(spheres[2].Radius * 3) / 4)));

		return spheres;
	}

	EulerAngles HairUnit::GetOrientation(const Vector3& origin, const Vector3& target)
	{
		// Calculate 2D distance between segments.
		float distance2D = Vector2::Distance(
			Vector2(target.x, target.z),
			Vector2(origin.x, origin.z));

		// Calculate segment orientation.
		// BUG: Aggressive gimbal lock causes major twisting.
		return EulerAngles(
			-(short)phd_atan(
				distance2D,
				target.y - origin.y),
			(short)phd_atan(
				target.z - origin.z,
				target.x - origin.x),
			0);
	}

	void HairUnit::CollideSegmentWithRoom(HairSegment& segment, int waterHeight, int roomNumber, bool isOnLand)
	{
		constexpr auto VELOCITY_COEFF = 0.75f;

		auto pointColl = GetCollision(segment.Position.x, segment.Position.y, segment.Position.z, roomNumber);
		int floorHeight = pointColl.Position.Floor;

		this->Segments[0].Velocity = segment.Position;
		segment.Position += segment.Velocity * VELOCITY_COEFF;

		// Land collision.
		if (isOnLand)
		{
			// Let wind affect position.
			if (TestEnvironment(ENV_FLAG_WIND, pointColl.RoomNumber))
				segment.Position += Weather.Wind() * 2;

			// Apply gravity.
			segment.Position.y += HAIR_GRAVITY;

			// Float on water surface.
			if (waterHeight != NO_HEIGHT && segment.Position.y > waterHeight)
			{
				segment.Position.y = waterHeight;
			}
			// Avoid clipping through floor.
			else if (floorHeight > Segments[0].Position.y && segment.Position.y > floorHeight)
			{
				segment.Position = Segments[0].Velocity;
			}
		}
		// Water collision.
		else
		{
			if (segment.Position.y < waterHeight)
			{
				segment.Position.y = waterHeight;
			}
			else if (segment.Position.y > floorHeight)
			{
				segment.Position.y = floorHeight;
			}
		}
	}

	void HairUnit::CollideSegmentWithSpheres(HairSegment& segment, const std::vector<BoundingSphere>& spheres)
	{
		for (const auto& sphere : spheres)
		{
			auto direction = segment.Position - sphere.Center;

			float distance = Vector3::Distance(segment.Position, sphere.Center);
			if (distance < sphere.Radius)
			{
				// Avoid division by zero.
				if (distance == 0.0f)
					distance = 1.0f;

				// Push segment away from sphere.
				segment.Position = sphere.Center + (direction * (sphere.Radius / distance));
			}
		}
	}

	void HairEffectController::Initialize()
	{
		constexpr auto ORIENT_DEFAULT = EulerAngles(ANGLE(-90.0f), 0, 0);

		bool isYoung = (g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);

		// Initialize hair units.
		bool isHead = false;
		for (auto& unit : Units)
		{
			unit.IsEnabled = (!isHead || isYoung);
			unit.IsInitialized = true;
			
			unsigned int segmentCount = Objects[ID_HAIR].nmeshes;
			unit.Segments.resize(segmentCount);

			// Initialize segments.
			for (auto& segment : unit.Segments)
			{
				auto jointOffset = GetJointOffset(ID_HAIR, 0);

				segment.Position = jointOffset;
				segment.Orientation = ORIENT_DEFAULT;
				segment.Velocity = Vector3::Zero;
			}

			isHead = true;
		}
	}

	void HairEffectController::Update(ItemInfo& item, bool isYoung)
	{
		if (Units[0].Segments.empty())
			return;

		for (int i = 0; i < Units.size(); i++)
		{
			// DEBUG
			if (i == 0)
				drawSpheres = true;
			else
				drawSpheres = false;

			this->Units[i].Update(item, i);

			if (isYoung && i == 1)
				this->Units[i].Update(item, i);
		}
	}
}
