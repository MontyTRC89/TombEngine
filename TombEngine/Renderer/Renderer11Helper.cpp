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
#include "Renderer/RenderView/RenderView.h"
#include "Renderer/Renderer11.h"
#include "Specific/configuration.h"
#include "Specific/level.h"
#include "Specific/setup.h"

extern GameConfiguration g_Configuration;
extern ScriptInterfaceFlowHandler *g_GameFlow;

namespace TEN::Renderer
{
	using std::pair;
	using std::vector;

	void Renderer11::UpdateAnimation(RendererItem *item, RendererObject& obj, AnimFrame** frmptr, short frac, short rate, int mask, bool useObjectWorldRotation)
	{
		static std::vector<int> boneIndexList;
		boneIndexList.clear();
		
		RendererBone *Bones[MAX_BONES] = {};
		int nextBone = 0;

		Matrix rotation;
		
		auto* transforms = ((item == nullptr) ? obj.AnimationTransforms.data() : &item->AnimationTransforms[0]);

		// Push
		Bones[nextBone++] = obj.Skeleton;

		while (nextBone != 0)
		{
			// Pop the last bone in the stack
			RendererBone *bone = Bones[--nextBone];
			if (!bone)
			{
				return; // Otherwise inventory crashes
			}

			if (frmptr[0]->angles.size() <= bone->Index || (frac && frmptr[1]->angles.size() <= bone->Index))
			{
				TENLog("Attempt to animate object ID " + GetObjectName((GAME_OBJECT_ID)item->ObjectNumber) +
					" with incorrect animation data. Bad set of animations for a slot?", LogLevel::Error);
				return;
			}

			bool calculateMatrix = (mask >> bone->Index) & 1;
			if (calculateMatrix)
			{
				auto p = Vector3(frmptr[0]->offsetX, frmptr[0]->offsetY, frmptr[0]->offsetZ);

				rotation = Matrix::CreateFromQuaternion(frmptr[0]->angles[bone->Index]);
				
				if (frac)
				{
					auto p2 = Vector3(frmptr[1]->offsetX, frmptr[1]->offsetY, frmptr[1]->offsetZ);
					p = Vector3::Lerp(p, p2, frac / ((float)rate));

					Matrix rotation2 = Matrix::CreateFromQuaternion(frmptr[1]->angles[bone->Index]);

					Quaternion q1, q2, q3;

					q1 = Quaternion::CreateFromRotationMatrix(rotation);
					q2 = Quaternion::CreateFromRotationMatrix(rotation2);
					q3 = Quaternion::Slerp(q1, q2, frac / ((float)rate));

					rotation = Matrix::CreateFromQuaternion(q3);
				}

				Matrix translation;
				if (bone == obj.Skeleton)
					translation = Matrix::CreateTranslation(p.x, p.y, p.z);

				Matrix extraRotation = Matrix::CreateFromYawPitchRoll(bone->ExtraRotation.y, bone->ExtraRotation.x, bone->ExtraRotation.z);

				if (useObjectWorldRotation)
				{
					Quaternion invertedQuat;
					auto scale = Vector3{};
					auto translation = Vector3{};
					transforms[bone->Parent->Index].Invert().Decompose(scale, invertedQuat, translation);
					rotation = rotation * extraRotation * Matrix::CreateFromQuaternion(invertedQuat);
				}
				else
					rotation = extraRotation * rotation;

				if (bone != obj.Skeleton)
					transforms[bone->Index] = rotation * bone->Transform;
				else
					transforms[bone->Index] = rotation * translation;

				if (bone != obj.Skeleton)
					transforms[bone->Index] = transforms[bone->Index] * transforms[bone->Parent->Index];
			}

			boneIndexList.push_back(bone->Index);

			for (int i = 0; i < bone->Children.size(); i++)
			{
				// Push
				Bones[nextBone++] = bone->Children[i];
			}
		}

		// Apply mutations on top
		if (item) 
		{
			auto* nativeItem = &g_Level.Items[item->ItemNumber];

			if (nativeItem->Model.Mutator.size() == boneIndexList.size())
			{
				for (int i : boneIndexList)
				{
					auto mutator = nativeItem->Model.Mutator[i];
					if (mutator.IsEmpty())
						continue;

					auto m = Matrix::CreateFromYawPitchRoll(mutator.Rotation.y, mutator.Rotation.x, mutator.Rotation.z);
					auto s = Matrix::CreateScale(mutator.Scale);
					auto t = Matrix::CreateTranslation(mutator.Offset);

					transforms[i] = m * s * t * transforms[i];
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
			currentBone->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);
				
			nativeItem->Data.apply(
				[&j, &currentBone](QuadBikeInfo& quadBike)
				{
					if (j == 3 || j == 4)
						currentBone->ExtraRotation.x = TO_RAD(quadBike.RearRot);
					else if (j == 6 || j == 7)
					{
						currentBone->ExtraRotation.x = TO_RAD(quadBike.FrontRot);
						currentBone->ExtraRotation.y = TO_RAD(quadBike.TurnRate * 2);
					}
				},
				[&j, &currentBone](JeepInfo& jeep)
				{
					switch(j)
					{
					case 9:
						currentBone->ExtraRotation.x = TO_RAD(jeep.FrontRightWheelRotation);
						currentBone->ExtraRotation.y = TO_RAD(jeep.TurnRate * 4);
						break;

					case 10:
						currentBone->ExtraRotation.x = TO_RAD(jeep.FrontLeftWheelRotation);
						currentBone->ExtraRotation.y = TO_RAD(jeep.TurnRate * 4);
						break;

					case 12:
						currentBone->ExtraRotation.x = TO_RAD(jeep.BackRightWheelRotation);
						break;

					case 13:
						currentBone->ExtraRotation.x = TO_RAD(jeep.BackLeftWheelRotation);
						break;
					}
				},
				[&j, &currentBone](MotorbikeInfo& bike)
				{
					switch (j)
					{
					case 2:
						currentBone->ExtraRotation.x = TO_RAD(bike.RightWheelsRotation);
						currentBone->ExtraRotation.y = TO_RAD(bike.TurnRate * 8);
						break;

					case 4:
						currentBone->ExtraRotation.x = TO_RAD(bike.RightWheelsRotation);
						break;

					case 8:
						currentBone->ExtraRotation.x = TO_RAD(bike.LeftWheelRotation);
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
						currentBone->ExtraRotation.z = TO_RAD((short)std::clamp(cart.Velocity, 0, (int)ANGLE(25.0f)) + FROM_RAD(prevRotation.z));
						break;
					}
				},
				[&j, &currentBone](RubberBoatInfo& boat)
				{
					if (j == 2)
						currentBone->ExtraRotation.z = TO_RAD(boat.PropellerRotation);
				},
				[&j, &currentBone](UPVInfo& upv)
				{
					switch (j)
					{
					case 1:
						currentBone->ExtraRotation.x = TO_RAD(upv.LeftRudderRotation);
						break;

					case 2:
						currentBone->ExtraRotation.x = TO_RAD(upv.RightRudderRotation);
						break;

					case 3:
						currentBone->ExtraRotation.z = TO_RAD(upv.TurbineRotation);
						break;
					}
				},
				[&j, &currentBone](BigGunInfo& big_gun)
				{
					if (j == 2)
						currentBone->ExtraRotation.z = big_gun.BarrelRotation;
				},
				[&j, &currentBone, &lastJoint](CreatureInfo& creature)
				{
					if (currentBone->ExtraRotationFlags & ROT_Y)
					{
						currentBone->ExtraRotation.y = TO_RAD(creature.JointRotation[lastJoint]);
						lastJoint++;
					}

					if (currentBone->ExtraRotationFlags & ROT_X)
					{
						currentBone->ExtraRotation.x = TO_RAD(creature.JointRotation[lastJoint]);
						lastJoint++;
					}

					if (currentBone->ExtraRotationFlags & ROT_Z)
					{
						currentBone->ExtraRotation.z = TO_RAD(creature.JointRotation[lastJoint]);
						lastJoint++;
					}
				});
		}

		AnimFrame* framePtr[2];
		int rate;
		int frac = GetFrame(nativeItem, framePtr, rate);

		UpdateAnimation(itemToDraw, moveableObj, framePtr, frac, rate, UINT_MAX);

		for (int m = 0; m < obj->nmeshes; m++)
			itemToDraw->AnimationTransforms[m] = itemToDraw->AnimationTransforms[m];
	}

	void Renderer11::UpdateItemAnimations(RenderView& view)
	{
		Matrix translation;
		Matrix rotation;

		for (auto* room : view.roomsToDraw)
		{
			for (auto* itemToDraw : room->ItemsToDraw)
			{
				auto* nativeItem = &g_Level.Items[itemToDraw->ItemNumber];

				// Lara has her own routine
				if (nativeItem->ObjectNumber == ID_LARA)
					continue;

				UpdateItemAnimations(itemToDraw->ItemNumber, false);
			}
		}
	}

	void Renderer11::BuildHierarchyRecursive(RendererObject *obj, RendererBone *node, RendererBone *parentNode)
	{
		node->GlobalTransform = node->Transform * parentNode->GlobalTransform;
		obj->BindPoseTransforms[node->Index] = node->GlobalTransform;
		obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);
		node->GlobalTranslation = node->Translation + parentNode->GlobalTranslation;

		for (int j = 0; j < node->Children.size(); j++)
			BuildHierarchyRecursive(obj, node->Children[j], node);
	}

	void Renderer11::BuildHierarchy(RendererObject *obj)
	{
		obj->Skeleton->GlobalTransform = obj->Skeleton->Transform;
		obj->BindPoseTransforms[obj->Skeleton->Index] = obj->Skeleton->GlobalTransform;
		obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);

		for (int j = 0; j < obj->Skeleton->Children.size(); j++)
			BuildHierarchyRecursive(obj, obj->Skeleton->Children[j], obj->Skeleton);
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

		return moveable.ObjectMeshes.size();
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

	Vector2 Renderer11::GetScreenSpacePosition(const Vector3& pos) const
	{
		auto point = Vector4(pos.x, pos.y, pos.z, 1.0f);
		auto cameraPos = Vector4(
			gameCamera.camera.WorldPosition.x,
			gameCamera.camera.WorldPosition.y,
			gameCamera.camera.WorldPosition.z,
			1.0f);
		auto cameraDirection = Vector4(
			gameCamera.camera.WorldDirection.x,
			gameCamera.camera.WorldDirection.y,
			gameCamera.camera.WorldDirection.z,
			1.0f);
		
		// If point is behind camera, return invalid screen space position.
		if ((point - cameraPos).Dot(cameraDirection) < 0.0f)
			return INVALID_2D_POSITION;

		// Calculate clip space coords.
		point = Vector4::Transform(point, gameCamera.camera.ViewProjection);

		// Calculate normalized device coords.
		point /= point.w;

		// Calculate and return screen space position.
		return Vector2(
			((point.x + 1.0f) * SCREEN_SPACE_RES.x) / 2,
			((1.0f - point.y) * SCREEN_SPACE_RES.y) / 2);
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
