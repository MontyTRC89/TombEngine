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
	static bool drawSpheres = false;

	HairEffectController HairEffect = {};

	void HairUnit::HairSegment::CollideSpheres(const std::vector<SPHERE>& spheres)
	{
		// Handle sphere collision.
		for (const auto& sphere : spheres)
		{
			auto spherePos = Vector3(sphere.x, sphere.y, sphere.z);
			auto direction = Position - spherePos;

			float distance = Vector3::Distance(Position, spherePos);
			if (distance < sphere.r)
			{
				// Avoid division by zero.
				if (distance == 0.0f)
					distance = 1.0f;

				this->Position = spherePos + (direction * (sphere.r / distance));
			}
		}
	}

	void HairUnit::Update(const ItemInfo& item, int hairUnitIndex)
	{
		const auto& player = GetLaraInfo(item);

		bool isYoung = (g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);

		auto worldMatrix = Matrix::Identity;
		g_Renderer.GetBoneMatrix(player.ItemNumber, LM_HEAD, &worldMatrix);

		if (hairUnitIndex)
		{
			worldMatrix = Matrix::CreateTranslation(44.0f, -48.0f, -50.0f) * worldMatrix;
		}
		else if (isYoung)
		{
			worldMatrix = Matrix::CreateTranslation(-52.0f, -48.0f, -50.0f) * worldMatrix;
		}
		else
		{
			worldMatrix = Matrix::CreateTranslation(-4.0f, -4.0f, -48.0f) * worldMatrix;
		}

		auto pos = worldMatrix.Translation();

		int* bonePtr = &g_Level.Bones[Objects[ID_HAIR].boneIndex];

		if (IsInitialized)
		{
			this->IsInitialized = false;
			this->Segments[0].Position = pos;

			// Update segment positions.
			for (int i = 0; i < SEGMENT_COUNT_MAX; i++, bonePtr += 4)
			{
				worldMatrix = Matrix::CreateTranslation(Segments[i].Position);
				worldMatrix = Segments[i].Orientation.ToRotationMatrix() * worldMatrix;
				worldMatrix = Matrix::CreateTranslation(*(bonePtr + 1), *(bonePtr + 2), *(bonePtr + 3)) * worldMatrix;

				this->Segments[i + 1].Position = worldMatrix.Translation();
			}
		}
		else
		{
			auto* framePtr = this->GetFramePtr(item);

			this->Segments[0].Position = pos;

			auto pos = item.Pose.Position + framePtr->boundingBox.GetCenter();
			int roomNumber = item.RoomNumber;
			int waterHeight = GetWaterHeight(pos.x, pos.y, pos.z, roomNumber);

			auto spheres = GetSpheres(item, isYoung);

			// Update segments.
			for (int i = 1; i < SEGMENT_COUNT_MAX + 1; i++, bonePtr += 4)
			{
				auto& segment = this->Segments[i];
				auto& prevSegment = this->Segments[i - 1];

				this->Segments[0].Velocity = segment.Position;

				auto pointColl = GetCollision(segment.Position.x, segment.Position.y, segment.Position.z, item.RoomNumber);
				int floorHeight = pointColl.Position.Floor;

				segment.Position += segment.Velocity * 0.75f;

				// TR3 UPV uses a hack which forces Lara water status to dry. 
				// Therefore, we can't directly use water status value to determine hair mode.
				bool dryMode = ((player.Control.WaterStatus == WaterStatus::Dry) &&
								(player.Vehicle == -1 || g_Level.Items[player.Vehicle].ObjectNumber != ID_UPV));
				if (dryMode)
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

				// Handle sphere collision.
				segment.CollideSpheres(spheres);

				// Old method.
				//if (false)
				{
					// Calculate 2D distance (on XZ plane) between segments.
					float distance2D = Vector2::Distance(
						Vector2(segment.Position.x, segment.Position.z),
						Vector2(prevSegment.Position.x, prevSegment.Position.z));

					// Calculate segment orientation.
					// BUG: phd_atan causes twisting!
					prevSegment.Orientation = EulerAngles(
						-(short)phd_atan(
							distance2D,
							segment.Position.y - prevSegment.Position.y),
						(short)phd_atan(
							segment.Position.z - prevSegment.Position.z,
							segment.Position.x - prevSegment.Position.x),
						0);
				}

				// New method.
				//prevSegment.Orientation = Geometry::GetOrientToPoint(prevSegment.Position, segment.Position);

				worldMatrix = Matrix::CreateTranslation(prevSegment.Position);
				worldMatrix = prevSegment.Orientation.ToRotationMatrix() * worldMatrix;

				if (i == SEGMENT_COUNT_MAX)
					worldMatrix = Matrix::CreateTranslation(*(bonePtr - 3), *(bonePtr - 2), *(bonePtr - 1)) * worldMatrix;
				else
					worldMatrix = Matrix::CreateTranslation(*(bonePtr + 1), *(bonePtr + 2), *(bonePtr + 3)) * worldMatrix;

				segment.Position = worldMatrix.Translation();
				segment.Velocity = segment.Position - Segments[0].Velocity;

				// DEBUG
				if (drawSpheres)
					g_Renderer.AddSphere(segment.Position, 20, Vector4::One);
			}
		}
	}

	AnimFrame* HairUnit::GetFramePtr(const ItemInfo& item)
	{
		const auto& player = GetLaraInfo(item);

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

	std::vector<SPHERE> HairUnit::GetSpheres(const ItemInfo& item, bool isYoung)
	{
		constexpr auto SPHERE_COUNT = 6;

		auto spheres = std::vector<SPHERE>{};
		spheres.reserve(SPHERE_COUNT);

		// Hips sphere.
		auto* meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_HIPS]];
		auto pos = GetJointPosition(item, LM_HIPS, Vector3i(meshPtr->sphere.Center)).ToVector3();
		spheres.push_back(SPHERE(pos, meshPtr->sphere.Radius));

		// Torso sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_TORSO]];
		pos = GetJointPosition(item, LM_TORSO, Vector3i(meshPtr->sphere.Center) + Vector3i(-10, 0, 25)).ToVector3();
		spheres.push_back(SPHERE(pos, meshPtr->sphere.Radius));
		if (isYoung)
			spheres.back().r = spheres.back().r - ((spheres.back().r / 4) + (spheres.back().r / 8));

		// Head sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_HEAD]];
		pos = GetJointPosition(item, LM_HEAD, Vector3i(meshPtr->sphere.Center) + Vector3i(-2, 0, 0)).ToVector3();
		spheres.push_back(SPHERE(pos, meshPtr->sphere.Radius));

		// Right arm sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_RINARM]];
		pos = GetJointPosition(item, LM_RINARM, Vector3i(meshPtr->sphere.Center)).ToVector3();
		spheres.push_back(SPHERE(pos, (meshPtr->sphere.Radius / 3.0f) * 4));

		// Left arm sphere.
		meshPtr = &g_Level.Meshes[item.Model.MeshIndex[LM_LINARM]];
		pos = GetJointPosition(item, LM_LINARM, Vector3i(meshPtr->sphere.Center)).ToVector3();
		spheres.push_back(SPHERE(pos, (meshPtr->sphere.Radius / 3.0f) * 4));

		if (isYoung)
		{
			spheres[1].x = (spheres[1].x + spheres[2].x) / 2;
			spheres[1].y = (spheres[1].y + spheres[2].y) / 2;
			spheres[1].z = (spheres[1].z + spheres[2].z) / 2;
		}

		// Neck sphere.
		spheres.push_back(SPHERE(
			Vector3(
				(spheres[2].x * 2) + spheres[1].x,
				(spheres[2].y * 2) + spheres[1].y,
				(spheres[2].z * 2) + spheres[1].z) / 3,
			isYoung ? 0 : int(float(spheres[2].r * 3) / 4)));

		return spheres;
	}

	void HairEffectController::Initialize()
	{
		constexpr auto ORIENT_DEFAULT = EulerAngles(ANGLE(-90.0f), 0, 0);

		bool isYoung = (g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);

		// Initialize hair units.
		bool isHead = false;
		for (auto& unit : Units)
		{
			int* bonePtr = &g_Level.Bones[Objects[ID_HAIR].boneIndex];

			unit.IsEnabled = (!isHead || isYoung);
			unit.IsInitialized = true;
			
			// Initialize segments.
			for (auto& segment : unit.Segments)
			{
				segment.Position = Vector3(*(bonePtr + 1), *(bonePtr + 2), *(bonePtr + 3));
				segment.Orientation = ORIENT_DEFAULT;
				segment.Velocity = Vector3::Zero;
			}

			isHead = true;
		}
	}

	void HairEffectController::Update(ItemInfo& item, bool isYoung)
	{
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
