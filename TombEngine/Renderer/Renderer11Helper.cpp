#include "framework.h"

#include <algorithm>

#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Objects/TR3/Vehicles/big_gun.h"
#include "Objects/TR3/Vehicles/big_gun_info.h"
#include "Objects/TR3/Vehicles/quad_bike.h"
#include "Objects/TR3/Vehicles/quad_bike_info.h"
#include "Objects/TR3/Vehicles/rubber_boat.h"
#include "Objects/TR3/Vehicles/rubber_boat_info.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR3/Vehicles/upv_info.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/jeep_info.h"
#include "Objects/TR4/Vehicles/motorbike.h"
#include "Objects/TR4/Vehicles/motorbike_info.h"
#include "Math/Math.h"
#include "Renderer/RenderView/RenderView.h"
#include "Renderer/Renderer11.h"
#include "Specific/configuration.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Math;

extern GameConfiguration g_Configuration;
extern ScriptInterfaceFlowHandler *g_GameFlow;

namespace TEN::Renderer
{
	void Renderer11::UpdateAnimation(RendererItem* rItem, RendererObject& rObject, const AnimFrameInterpData& frameData, int mask, bool useObjectWorldRotation)
	{
		static auto boneIndices = std::vector<int>{};
		boneIndices.clear();
		
		RendererBone* bones[MAX_BONES] = {};
		int nextBone = 0;

		auto* transforms = ((rItem == nullptr) ? rObject.AnimationTransforms.data() : &rItem->AnimationTransforms[0]);

		// Push.
		bones[nextBone++] = rObject.Skeleton;

		while (nextBone != 0)
		{
			// Pop last bone in stack.
			auto* bonePtr = bones[--nextBone];

			// Check nullptr, otherwise inventory crashes.
			if (bonePtr == nullptr)
				return;

			if (frameData.FramePtr0->BoneOrientations.size() <= bonePtr->Index ||
				(frameData.Alpha != 0.0f && frameData.FramePtr0->BoneOrientations.size() <= bonePtr->Index))
			{
				TENLog(
					"Attempted to animate object with ID " + GetObjectName((GAME_OBJECT_ID)rItem->ObjectNumber) +
					" using incorrect animation data. Bad animations set for slot?",
					LogLevel::Error);

				return;
			}

			bool calculateMatrix = (mask >> bonePtr->Index) & 1;
			if (calculateMatrix)
			{
				auto offset0 = frameData.FramePtr0->Offset;
				auto rotMatrix = Matrix::CreateFromQuaternion(frameData.FramePtr0->BoneOrientations[bonePtr->Index]);
				
				if (frameData.Alpha != 0.0f)
				{
					auto offset1 = frameData.FramePtr1->Offset;
					offset0 = Vector3::Lerp(offset0, offset1, frameData.Alpha);

					auto rotMatrix2 = Matrix::CreateFromQuaternion(frameData.FramePtr1->BoneOrientations[bonePtr->Index]);

					auto quat1 = Quaternion::CreateFromRotationMatrix(rotMatrix);
					auto quat2 = Quaternion::CreateFromRotationMatrix(rotMatrix2);
					auto quat3 = Quaternion::Slerp(quat1, quat2, frameData.Alpha);

					rotMatrix = Matrix::CreateFromQuaternion(quat3);
				}

				auto tMatrix = (bonePtr == rObject.Skeleton) ? Matrix::CreateTranslation(offset0) : Matrix::Identity;

				auto extraRotMatrix = Matrix::CreateFromQuaternion(bonePtr->ExtraRotation);

				if (useObjectWorldRotation)
				{
					auto scale = Vector3::Zero;
					auto inverseQuat = Quaternion::Identity;
					auto translation = Vector3::Zero;
					transforms[bonePtr->Parent->Index].Invert().Decompose(scale, inverseQuat, translation);

					rotMatrix = rotMatrix * extraRotMatrix * Matrix::CreateFromQuaternion(inverseQuat);
				}
				else
				{
					rotMatrix = extraRotMatrix * rotMatrix;
				}

				if (bonePtr != rObject.Skeleton)
					transforms[bonePtr->Index] = rotMatrix * bonePtr->Transform;
				else
					transforms[bonePtr->Index] = rotMatrix * tMatrix;

				if (bonePtr != rObject.Skeleton)
					transforms[bonePtr->Index] = transforms[bonePtr->Index] * transforms[bonePtr->Parent->Index];
			}

			boneIndices.push_back(bonePtr->Index);

			// Push.
			for (auto*& child : bonePtr->Children)
				bones[nextBone++] = child;
		}

		// Apply mutators on top.
		if (rItem != nullptr) 
		{
			const auto& nativeItem = g_Level.Items[rItem->ItemNumber];

			if (nativeItem.Model.Mutators.size() == boneIndices.size())
			{
				for (const int& i : boneIndices)
				{
					const auto& mutator = nativeItem.Model.Mutators[i];

					if (mutator.IsEmpty())
						continue;

					auto rotMatrix = mutator.Rotation.ToRotationMatrix();
					auto scaleMatrix = Matrix::CreateScale(mutator.Scale);
					auto tMatrix = Matrix::CreateTranslation(mutator.Offset);

					transforms[i] = rotMatrix * scaleMatrix * tMatrix * transforms[i];
				}
			}
		}
	}

	void Renderer11::UpdateItemAnimations(int itemNumber, bool force)
	{
		auto* itemToDraw = &m_items[itemNumber];
		auto* nativeItem = &g_Level.Items[itemNumber];

		// TODO: hack for fixing a bug, check again if needed
		itemToDraw->ItemNumber = itemNumber;

		// Lara has her own routine
		if (nativeItem->ObjectNumber == ID_LARA)
			return;

		// Has been already done?
		if (!force && itemToDraw->DoneAnimations)
			return;

		itemToDraw->DoneAnimations = true;

		auto* obj = &Objects[nativeItem->ObjectNumber];
		auto& moveableObj = *m_moveableObjects[nativeItem->ObjectNumber];

		// Copy meshswaps
		itemToDraw->MeshIndex = nativeItem->Model.MeshIndex;

		if (obj->animIndex == -1)
			return;

		// Apply extra rotations
		int lastJoint = 0;
		for (int j = 0; j < moveableObj.LinearizedBones.size(); j++)
		{
			auto* currentBone = moveableObj.LinearizedBones[j];

			auto prevRotation = currentBone->ExtraRotation;
			currentBone->ExtraRotation = Quaternion::Identity;
				
			nativeItem->Data.apply(
				[&j, &currentBone](QuadBikeInfo& quadBike)
				{
					if (j == 3 || j == 4)
					{
						currentBone->ExtraRotation = EulerAngles(quadBike.RearRot, 0, 0).ToQuaternion();
					}
					else if (j == 6 || j == 7)
					{
						currentBone->ExtraRotation = EulerAngles(quadBike.FrontRot, quadBike.TurnRate * 2, 0).ToQuaternion();
					}
				},
				[&j, &currentBone](JeepInfo& jeep)
				{
					switch(j)
					{
					case 9:
						currentBone->ExtraRotation = EulerAngles(jeep.FrontRightWheelRotation, jeep.TurnRate * 4, 0).ToQuaternion();
						break;

					case 10:
						currentBone->ExtraRotation = EulerAngles(jeep.FrontLeftWheelRotation, jeep.TurnRate * 4, 0).ToQuaternion();
						break;

					case 12:
						currentBone->ExtraRotation = EulerAngles(jeep.BackRightWheelRotation, 0, 0).ToQuaternion();
						break;

					case 13:
						currentBone->ExtraRotation = EulerAngles(jeep.BackLeftWheelRotation, 0, 0).ToQuaternion();
						break;
					}
				},
				[&j, &currentBone](MotorbikeInfo& bike)
				{
					switch (j)
					{
					case 2:
						currentBone->ExtraRotation = EulerAngles(bike.RightWheelsRotation, bike.TurnRate * 8, 0).ToQuaternion();
						break;

					case 4:
						currentBone->ExtraRotation = EulerAngles(bike.RightWheelsRotation, 0, 0).ToQuaternion();
						break;

					case 8:
						currentBone->ExtraRotation = EulerAngles(bike.LeftWheelRotation, 0, 0).ToQuaternion();
						break;
					}
				},
				[&j, &currentBone, &prevRotation](MinecartInfo& cart)
				{
					switch (j)
					{
					case 1:
					case 2:
					case 3:
					case 4:
						short zRot = (short)std::clamp(cart.Velocity, 0, (int)ANGLE(25.0f)) + EulerAngles(prevRotation).z;
						currentBone->ExtraRotation = EulerAngles(0, 0, zRot).ToQuaternion();
						break;
					}
				},
				[&j, &currentBone](RubberBoatInfo& boat)
				{
					if (j == 2)
						currentBone->ExtraRotation = EulerAngles(0, 0, boat.PropellerRotation).ToQuaternion();
				},
				[&j, &currentBone](UPVInfo& upv)
				{
					switch (j)
					{
					case 1:
						currentBone->ExtraRotation = EulerAngles(upv.LeftRudderRotation, 0, 0).ToQuaternion();
						break;

					case 2:
						currentBone->ExtraRotation = EulerAngles(upv.RightRudderRotation, 0, 0).ToQuaternion();
						break;

					case 3:
						currentBone->ExtraRotation = EulerAngles(0, 0, upv.TurbineRotation).ToQuaternion();
						break;
					}
				},
				[&j, &currentBone](BigGunInfo& bigGun)
				{
					if (j == 2)
						currentBone->ExtraRotation = EulerAngles(0, 0, FROM_RAD(bigGun.BarrelRotation)).ToQuaternion();
				},
					[&j, &currentBone, &lastJoint](CreatureInfo& creature)
				{
					auto xRot = Quaternion::Identity;
					auto yRot = Quaternion::Identity;
					auto zRot = Quaternion::Identity;

					if (currentBone->ExtraRotationFlags & ROT_Y)
					{
						yRot = EulerAngles(0, creature.JointRotation[lastJoint], 0).ToQuaternion();
						lastJoint++;
					}

					if (currentBone->ExtraRotationFlags & ROT_X)
					{
						xRot = EulerAngles(creature.JointRotation[lastJoint], 0, 0).ToQuaternion();
						lastJoint++;
					}

					if (currentBone->ExtraRotationFlags & ROT_Z)
					{
						zRot = EulerAngles(0, 0, creature.JointRotation[lastJoint]).ToQuaternion();
						lastJoint++;
					}

					currentBone->ExtraRotation = xRot * yRot * zRot;
				});
		}

		auto frameData = GetFrameInterpData(*nativeItem);
		UpdateAnimation(itemToDraw, moveableObj, frameData, UINT_MAX);

		for (int m = 0; m < obj->nmeshes; m++)
			itemToDraw->AnimationTransforms[m] = itemToDraw->AnimationTransforms[m];
	}

	void Renderer11::UpdateItemAnimations(RenderView& view)
	{
		for (const auto* room : view.RoomsToDraw)
		{
			for (const auto* itemToDraw : room->ItemsToDraw)
			{
				const auto& nativeItem = g_Level.Items[itemToDraw->ItemNumber];

				// Player has its own routine.
				if (nativeItem.ObjectNumber == ID_LARA)
					continue;

				UpdateItemAnimations(itemToDraw->ItemNumber, false);
			}
		}
	}

	void Renderer11::BuildHierarchyRecursive(RendererObject *obj, RendererBone *node, RendererBone *parentNode)
	{
		node->GlobalTransform = node->Transform * parentNode->GlobalTransform;
		obj->BindPoseTransforms[node->Index] = node->GlobalTransform;
		obj->Skeleton->GlobalTranslation = Vector3::Zero;
		node->GlobalTranslation = node->Translation + parentNode->GlobalTranslation;

		for (auto* childNode : node->Children)
			BuildHierarchyRecursive(obj, childNode, node);
	}

	void Renderer11::BuildHierarchy(RendererObject *obj)
	{
		obj->Skeleton->GlobalTransform = obj->Skeleton->Transform;
		obj->BindPoseTransforms[obj->Skeleton->Index] = obj->Skeleton->GlobalTransform;
		obj->Skeleton->GlobalTranslation = Vector3::Zero;

		for (auto* childNode : obj->Skeleton->Children)
			BuildHierarchyRecursive(obj, childNode, obj->Skeleton);
	}

	bool Renderer11::IsFullsScreen() 
	{
		return (!m_windowed);
	}

	void Renderer11::UpdateCameraMatrices(CAMERA_INFO *cam, float roll, float fov, float farView)
	{
		if (farView < MIN_FAR_VIEW)
			farView = DEFAULT_FAR_VIEW;

		m_farView = farView;
		gameCamera = RenderView(cam, roll, fov, 32, farView, g_Configuration.Width, g_Configuration.Height);
	}

	bool Renderer11::SphereBoxIntersection(BoundingBox box, Vector3 sphereCentre, float sphereRadius)
	{
		if (sphereRadius == 0.0f)
		{
			return box.Contains(sphereCentre);
		}
		else
		{
			BoundingSphere sphere = BoundingSphere(sphereCentre, sphereRadius);
			return box.Intersects(sphere);
		}
	}

	void Renderer11::FlipRooms(short roomNumber1, short roomNumber2)
	{
		std::swap(m_rooms[roomNumber1], m_rooms[roomNumber2]);

		m_rooms[roomNumber1].RoomNumber = roomNumber1;
		m_rooms[roomNumber2].RoomNumber = roomNumber2;

		m_invalidateCache = true;
	}

	RendererObject& Renderer11::GetRendererObject(GAME_OBJECT_ID id)
	{
		if (id == GAME_OBJECT_ID::ID_LARA || id == GAME_OBJECT_ID::ID_LARA_SKIN)
		{
			if (m_moveableObjects[GAME_OBJECT_ID::ID_LARA_SKIN].has_value())
				return m_moveableObjects[GAME_OBJECT_ID::ID_LARA_SKIN].value();
			else
				return m_moveableObjects[GAME_OBJECT_ID::ID_LARA].value();
		}
		else
		{
			return m_moveableObjects[id].value();
		}
	}

	RendererMesh* Renderer11::GetMesh(int meshIndex)
	{
		return m_meshes[meshIndex];
	}

	int Renderer11::GetSpheres(short itemNumber, BoundingSphere* spheres, char worldSpace, Matrix local)
	{
		auto* itemToDraw = &m_items[itemNumber];
		auto* nativeItem = &g_Level.Items[itemNumber];

		itemToDraw->ItemNumber = itemNumber;

		if (!nativeItem)
			return 0;

		if (!itemToDraw->DoneAnimations)
		{
			if (itemNumber == Lara.ItemNumber)
				UpdateLaraAnimations(false);
			else
				UpdateItemAnimations(itemNumber, false);
		}

		Matrix world;

		if (worldSpace & SPHERES_SPACE_WORLD)
			world = Matrix::CreateTranslation(nativeItem->Pose.Position.x, nativeItem->Pose.Position.y, nativeItem->Pose.Position.z) * local;
		else
			world = Matrix::Identity * local;

		world = nativeItem->Pose.Orientation.ToRotationMatrix() * world;

		auto& moveable = GetRendererObject(nativeItem->ObjectNumber);

		for (int i = 0; i< moveable.ObjectMeshes.size();i++)
		{
			auto mesh = moveable.ObjectMeshes[i];

			auto pos = (Vector3)mesh->Sphere.Center;
			if (worldSpace & SPHERES_SPACE_BONE_ORIGIN)
				pos += moveable.LinearizedBones[i]->Translation;

			spheres[i].Center = Vector3::Transform(pos, (itemToDraw->AnimationTransforms[i] * world));
			spheres[i].Radius = mesh->Sphere.Radius;

			// Spheres debug
			// auto v1 = Vector3(spheres[i].Center.x - spheres[i].Radius, spheres[i].Center.y, spheres[i].Center.z);
			// auto v2 = Vector3(spheres[i].Center.x + spheres[i].Radius, spheres[i].Center.y, spheres[i].Center.z);
			// AddLine3D(v1, v2, Vector4::One);
		}

		return (int)moveable.ObjectMeshes.size();
	}

	void Renderer11::GetBoneMatrix(short itemNumber, int jointIndex, Matrix* outMatrix)
	{
		if (itemNumber == Lara.ItemNumber)
		{
			auto& object = *m_moveableObjects[ID_LARA];
			*outMatrix = object.AnimationTransforms[jointIndex] * m_LaraWorldMatrix;
		}
		else
		{
			UpdateItemAnimations(itemNumber, true);
			
			auto* rendererItem = &m_items[itemNumber];
			auto* nativeItem = &g_Level.Items[itemNumber];

			auto& obj = *m_moveableObjects[nativeItem->ObjectNumber];
			*outMatrix = obj.AnimationTransforms[jointIndex] * rendererItem->World;
		}
	}

	Vector4 Renderer11::GetPortalRect(Vector4 v, Vector4 vp) 
	{
		auto sp = (v * Vector4(0.5f, 0.5f, 0.5f, 0.5f)
			+ Vector4(0.5f, 0.5f, 0.5f, 0.5f)) 
			* Vector4(vp.z, vp.w, vp.z, vp.w);

		Vector4 s(sp.x + vp.x, sp.y + vp.y, sp.z + vp.x, sp.w + vp.y);

		// expand
		s.x -= 2;
		s.y -= 2;
		s.z += 2;
		s.w += 2;

		// clamp
		s.x = std::max(s.x, vp.x);
		s.y = std::max(s.y, vp.y);
		s.z = std::min(s.z, vp.x + vp.z);
		s.w = std::min(s.w, vp.y + vp.w);

		// convert from bounds to x,y,w,h
		s.z -= s.x;
		s.w -= s.y;

		// Use the viewport rect if one of the dimensions is the same size
		// as the viewport. This may fix clipping bugs while still allowing
		// impossible geometry tricks.
		if (s.z - s.x >= vp.z - vp.x || s.w - s.y >= vp.w - vp.y) 
			return vp;

		return s;
	}

	Vector2i Renderer11::GetScreenResolution() const
	{
		return Vector2i(m_screenWidth, m_screenHeight);
	}

	std::optional<Vector2> Renderer11::Get2DPosition(const Vector3& pos) const
	{
		auto point = Vector4(pos.x, pos.y, pos.z, 1.0f);
		auto cameraPos = Vector4(
			gameCamera.Camera.WorldPosition.x,
			gameCamera.Camera.WorldPosition.y,
			gameCamera.Camera.WorldPosition.z,
			1.0f);
		auto cameraDirection = Vector4(
			gameCamera.Camera.WorldDirection.x,
			gameCamera.Camera.WorldDirection.y,
			gameCamera.Camera.WorldDirection.z,
			1.0f);
		
		// Point is behind camera; return nullopt.
		if ((point - cameraPos).Dot(cameraDirection) < 0.0f)
			return std::nullopt;

		// Calculate clip space coords.
		point = Vector4::Transform(point, gameCamera.Camera.ViewProjection);

		// Calculate NDC.
		point /= point.w;

		// Calculate and return 2D position.
		return TEN::Utils::ConvertNDCTo2DPosition(Vector2(point));
	}

	Vector3 Renderer11::GetAbsEntityBonePosition(int itemNumber, int jointIndex, const Vector3& relOffset)
	{
		auto* rendererItem = &m_items[itemNumber];

		rendererItem->ItemNumber = itemNumber;

		if (!rendererItem)
			return Vector3::Zero;

		if (!rendererItem->DoneAnimations)
		{
			if (itemNumber == Lara.ItemNumber)
				UpdateLaraAnimations(false);
			else
				UpdateItemAnimations(itemNumber, false);
		}

		if (jointIndex >= MAX_BONES)
			jointIndex = 0;

		auto world = rendererItem->AnimationTransforms[jointIndex] * rendererItem->World;
		return Vector3::Transform(relOffset, world);
	}
}
