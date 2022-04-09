#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Specific/configuration.h"
#include "Game/camera.h"
#include "Game/animation.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/collision/sphere.h"
#include "Scripting/GameFlowScript.h"
#include "Renderer\RenderView\RenderView.h"
#include "Objects/TR3/Vehicles/quad.h"
#include "Objects/TR3/Vehicles/rubberboat.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR3/Vehicles/biggun.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"
#include <algorithm>
#include "Game/itemdata/creature_info.h"
#include "Objects/TR3/Vehicles/quad_info.h"
#include "Objects/TR4/Vehicles/jeep_info.h"
#include "Objects/TR4/Vehicles/motorbike_info.h"
#include "Objects/TR3/Vehicles/rubberboat_info.h"
#include "Objects/TR3/Vehicles/upv_info.h"
#include "Objects/TR3/Vehicles/biggun_info.h"
#include "Game/items.h"

extern GameConfiguration g_Configuration;
extern GameFlow *g_GameFlow;

namespace TEN::Renderer
{
	using std::pair;
	using std::vector;

	bool Renderer11::IsRoomUnderwater(short roomNumber)
	{
		return (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER);
	}

	bool Renderer11::IsInRoom(int x, int y, int z, short roomNumber)
	{
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];

		return (x >= r->x && x <= r->x + r->xSize * 1024.0f &&
				y >= r->maxceiling && y <= r->minfloor &&
				z >= r->z && z <= r->z + r->zSize * 1024.0f);
	}

	std::vector<TEN::Renderer::RendererVideoAdapter>* Renderer11::GetAdapters()
{
		return &m_adapters;
	}

	void Renderer11::UpdateEffects(RenderView& view)
	{
		for (auto room : view.roomsToDraw)
		{
			for (auto fx : room->EffectsToDraw)
			{
				Matrix translation = Matrix::CreateTranslation(fx->Effect->pos.xPos, fx->Effect->pos.yPos, fx->Effect->pos.zPos);
				Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(fx->Effect->pos.yRot), TO_RAD(fx->Effect->pos.xRot), TO_RAD(fx->Effect->pos.zRot));
				fx->World = rotation * translation;
			}
		}
	}

	void Renderer11::UpdateAnimation(RendererItem *item, RendererObject& obj, ANIM_FRAME** frmptr, short frac, short rate, int mask, bool useObjectWorldRotation)
	{
		static std::vector<int> boneIndexList;
		boneIndexList.clear();
		
		RendererBone *Bones[32] = {};
		int nextBone = 0;

		Matrix rotation;

		Matrix *transforms = (item == NULL ? obj.AnimationTransforms.data() : &item->AnimationTransforms[0]);

		// Push
		Bones[nextBone++] = obj.Skeleton;

		while (nextBone != 0)
		{
			// Pop the last bone in the stack
			RendererBone *bone = Bones[--nextBone];
			if (!bone) return;//otherwise inventory crashes mm
			bool calculateMatrix = (mask >> bone->Index) & 1;

			if (calculateMatrix)
			{
				Vector3 p = Vector3(frmptr[0]->offsetX, frmptr[0]->offsetY, frmptr[0]->offsetZ);

				rotation = Matrix::CreateFromQuaternion(frmptr[0]->angles[bone->Index]);
				//FromTrAngle(&rotation, frmptr[0], bone->Index);
				
				if (frac)
				{
					Vector3 p2 = Vector3(frmptr[1]->offsetX, frmptr[1]->offsetY, frmptr[1]->offsetZ);
					p = Vector3::Lerp(p, p2, frac / ((float)rate));

					Matrix rotation2 = Matrix::CreateFromQuaternion(frmptr[1]->angles[bone->Index]);
					//FromTrAngle(&rotation2, frmptr[1], bone->Index);

					Quaternion q1, q2, q3;

					q1 = Quaternion::CreateFromRotationMatrix(rotation);
					q2 = Quaternion::CreateFromRotationMatrix(rotation2);
					q3 = Quaternion::Slerp(q1, q2, frac / ((float)rate));

					rotation = Matrix::CreateFromQuaternion(q3);
				}

				Matrix translation;
				if (bone == obj.Skeleton)
					translation = Matrix::CreateTranslation(p.x, p.y, p.z);

				Matrix extraRotation;
				extraRotation = Matrix::CreateFromYawPitchRoll(bone->ExtraRotation.y, bone->ExtraRotation.x, bone->ExtraRotation.z);
					

				if (useObjectWorldRotation)
				{
					Quaternion invertedQuat;
					auto scale = Vector3{};
					auto translation = Vector3{};
					transforms[bone->Parent->Index].Invert().Decompose(scale, invertedQuat, translation);
					rotation = rotation * extraRotation * Matrix::CreateFromQuaternion(invertedQuat);
				}
				else
				{
					rotation = extraRotation * rotation;
				}

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
			ITEM_INFO* nativeItem = &g_Level.Items[item->ItemNumber];

			if (nativeItem->mutator.size() == boneIndexList.size())
			{
				for (int i : boneIndexList)
				{
					auto mutator = nativeItem->mutator[i];
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
		RendererItem* itemToDraw = &m_items[itemNumber];
		ITEM_INFO* nativeItem = &g_Level.Items[itemNumber];

		// TODO: hack for fixing a bug, check again if needed
		itemToDraw->ItemNumber = itemNumber;

		// Lara has her own routine
		if (nativeItem->objectNumber == ID_LARA)
			return;

		// Has been already done?
		if (!force && itemToDraw->DoneAnimations)
			return;

		OBJECT_INFO* obj = &Objects[nativeItem->objectNumber];
		RendererObject& moveableObj = *m_moveableObjects[nativeItem->objectNumber];

		// Update animation matrices
		if (obj->animIndex != -1 /*&& item->objectNumber != ID_HARPOON*/)
		{
			// Apply extra rotations
			int lastJoint = 0;
			for (int j = 0; j < moveableObj.LinearizedBones.size(); j++)
			{
				RendererBone *currentBone = moveableObj.LinearizedBones[j];
				currentBone->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);
				
				nativeItem->data.apply(
					[&j, &currentBone](QUAD_INFO& quad) {
					if(j == 3 || j == 4) {
						currentBone->ExtraRotation.x = TO_RAD(quad.rearRot);
					} else if(j == 6 || j == 7) {
						currentBone->ExtraRotation.x = TO_RAD(quad.frontRot);
					}
				},
				[&j, &currentBone](JEEP_INFO& jeep) {
					switch(j) {
					case 9:
						currentBone->ExtraRotation.x = TO_RAD(jeep.rot1);
						break;
					case 10:
						currentBone->ExtraRotation.x = TO_RAD(jeep.rot2);

						break;
					case 12:
						currentBone->ExtraRotation.x = TO_RAD(jeep.rot3);

						break;
					case 13:
						currentBone->ExtraRotation.x = TO_RAD(jeep.rot4);

						break;
					}
				},
				[&j, &currentBone](MOTORBIKE_INFO& bike) {
				switch(j) {
					case 2:
					case 4:
						currentBone->ExtraRotation.x = TO_RAD(bike.wheelRight);
						break;
					case 10:
						currentBone->ExtraRotation.x = TO_RAD(bike.wheelLeft);
					}
				},
				[&j, &currentBone](RUBBER_BOAT_INFO& boat) {
				if(j == 2)
					currentBone->ExtraRotation.z = TO_RAD(boat.propRot);
				},
				[&j, &currentBone](SUB_INFO& upv) {
				if(j == 3)
					currentBone->ExtraRotation.z = TO_RAD(upv.FanRot);
				},
				[&j, &currentBone](BIGGUNINFO& biggun) {
				if(j == 2)
					currentBone->ExtraRotation.z = biggun.barrelZ;
				},
				[&j, &currentBone, &lastJoint](CREATURE_INFO& creature) {
				if(currentBone->ExtraRotationFlags & ROT_Y) {
					currentBone->ExtraRotation.y = TO_RAD(creature.jointRotation[lastJoint]);
					lastJoint++;
				}

				if(currentBone->ExtraRotationFlags & ROT_X) {
					currentBone->ExtraRotation.x = TO_RAD(creature.jointRotation[lastJoint]);
					lastJoint++;
				}

				if(currentBone->ExtraRotationFlags & ROT_Z) {
					currentBone->ExtraRotation.z = TO_RAD(creature.jointRotation[lastJoint]);
					lastJoint++;
				}
				}
				);
			}

			ANIM_FRAME* framePtr[2];
			int rate;
			int frac = GetFrame(nativeItem, framePtr, &rate);

			UpdateAnimation(itemToDraw, moveableObj, framePtr, frac, rate, 0xFFFFFFFF);

			for (int m = 0; m < obj->nmeshes; m++)
				itemToDraw->AnimationTransforms[m] = itemToDraw->AnimationTransforms[m];
		}

		itemToDraw->DoneAnimations = true;
	}

	void Renderer11::UpdateItemsAnimations(RenderView& view) {
		Matrix translation;
		Matrix rotation;

		for (auto room : view.roomsToDraw)
		{
			for (auto itemToDraw : room->ItemsToDraw)
			{
				ITEM_INFO* nativeItem = &g_Level.Items[itemToDraw->ItemNumber];

				// Lara has her own routine
				if (nativeItem->objectNumber == ID_LARA)
					continue;

				UpdateItemAnimations(itemToDraw->ItemNumber, false);
			}
		}
	}

	void Renderer11::FromTrAngle(Matrix *matrix, short *frameptr, int index) {
		short *ptr = &frameptr[0];

		ptr += 9;
		for (int i = 0; i < index; i++)
		{
			ptr += ((*ptr & 0xc000) == 0 ? 2 : 1);
		}

		int rot0 = *ptr++;
		int frameMode = (rot0 & 0xc000);

		int rot1;
		int rotX;
		int rotY;
		int rotZ;

		switch (frameMode)
		{
		case 0:
			rot1 = *ptr++;
			rotX = ((rot0 & 0x3ff0) >> 4);
			rotY = (((rot1 & 0xfc00) >> 10) | ((rot0 & 0xf) << 6) & 0x3ff);
			rotZ = ((rot1)&0x3ff);

			*matrix = Matrix::CreateFromYawPitchRoll(rotY * (360.0f / 1024.0f) * RADIAN,
													 rotX * (360.0f / 1024.0f) * RADIAN,
													 rotZ * (360.0f / 1024.0f) * RADIAN);
			break;

		case 0x4000:
			*matrix = Matrix::CreateRotationX((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
			break;

		case 0x8000:
			*matrix = Matrix::CreateRotationY((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
			break;

		case 0xc000:
			*matrix = Matrix::CreateRotationZ((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
			break;
		}
	}

	void Renderer11::BuildHierarchyRecursive(RendererObject *obj, RendererBone *node, RendererBone *parentNode) {
		node->GlobalTransform = node->Transform * parentNode->GlobalTransform;
		obj->BindPoseTransforms[node->Index] = node->GlobalTransform;
		obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);
		node->GlobalTranslation = node->Translation + parentNode->GlobalTranslation;

		for (int j = 0; j < node->Children.size(); j++)
		{
			BuildHierarchyRecursive(obj, node->Children[j], node);
		}
	}

	void Renderer11::BuildHierarchy(RendererObject *obj) {
		obj->Skeleton->GlobalTransform = obj->Skeleton->Transform;
		obj->BindPoseTransforms[obj->Skeleton->Index] = obj->Skeleton->GlobalTransform;
		obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);

		for (int j = 0; j < obj->Skeleton->Children.size(); j++)
		{
			BuildHierarchyRecursive(obj, obj->Skeleton->Children[j], obj->Skeleton);
		}
	}

	bool Renderer11::IsFullsScreen() {
		return (!Windowed);
	}

	void Renderer11::UpdateCameraMatrices(CAMERA_INFO *cam, float roll, float fov)
	{
		gameCamera = RenderView(cam, roll, fov, 32, 102400, g_Configuration.Width, g_Configuration.Height);
	}

	bool Renderer11::SphereBoxIntersection(Vector3 boxMin, Vector3 boxMax, Vector3 sphereCentre, float sphereRadius)
	{
		Vector3 centre = (boxMin + boxMax) / 2.0f;
		Vector3 extens = boxMax - centre;
		BoundingBox box = BoundingBox(centre, extens);
		BoundingSphere sphere = BoundingSphere(sphereCentre, sphereRadius);
		return box.Intersects(sphere);
	}

	void Renderer11::GetLaraBonePosition(Vector3 *pos, int bone) {}

	void Renderer11::FlipRooms(short roomNumber1, short roomNumber2)
	{
		std::swap(m_rooms[roomNumber1], m_rooms[roomNumber2]);

		m_rooms[roomNumber1].RoomNumber = roomNumber1;
		m_rooms[roomNumber2].RoomNumber = roomNumber2;
	}

	RendererMesh* Renderer11::GetMesh(int meshIndex)
	{
		return m_meshes[meshIndex];
	}

	void Renderer11::GetLaraAbsBonePosition(Vector3 *pos, int joint)
	{
		Matrix world = m_moveableObjects[ID_LARA]->AnimationTransforms[joint];
		world = world * m_LaraWorldMatrix;
		*pos = Vector3::Transform(*pos, world);
	}

	void Renderer11::GetItemAbsBonePosition(int itemNumber, Vector3 *pos, int joint)
	{
		RendererItem *rendererItem = &m_items[itemNumber];
		ITEM_INFO* nativeItem = &g_Level.Items[itemNumber];

		rendererItem->ItemNumber = itemNumber;

		if (!rendererItem)
			return;

		if (!rendererItem->DoneAnimations)
		{
			if (itemNumber == Lara.itemNumber)
				UpdateLaraAnimations(false);
			else
				UpdateItemAnimations(itemNumber, false);
		}

		Matrix world = rendererItem->AnimationTransforms[joint] * rendererItem->World;
		*pos = Vector3::Transform(*pos, world);
	}

	int Renderer11::getSpheres(short itemNumber, BoundingSphere *spheres, char worldSpace, Matrix local)
	{
		RendererItem* itemToDraw = &m_items[itemNumber];
		ITEM_INFO* nativeItem = &g_Level.Items[itemNumber];

		itemToDraw->ItemNumber = itemNumber;

		if (!nativeItem)
			return 0;

		if (!itemToDraw->DoneAnimations)
		{
			if (itemNumber == Lara.itemNumber)
				UpdateLaraAnimations(false);
			else
				UpdateItemAnimations(itemNumber, false);
		}

		Matrix world;

		if (worldSpace & SPHERES_SPACE_WORLD)
			world = Matrix::CreateTranslation(nativeItem->pos.xPos, nativeItem->pos.yPos, nativeItem->pos.zPos) * local;
		else
			world = Matrix::Identity * local;

		world = Matrix::CreateFromYawPitchRoll(TO_RAD(nativeItem->pos.yRot), TO_RAD(nativeItem->pos.xRot), TO_RAD(nativeItem->pos.zRot)) * world;

		short objNum = nativeItem->objectNumber;
		if (objNum == ID_LARA) objNum = ID_LARA_SKIN;

		RendererObject& moveable = *m_moveableObjects[objNum];

		for (int i = 0; i< moveable.ObjectMeshes.size();i++)
		{
			auto mesh = moveable.ObjectMeshes[i];

			Vector3 pos = mesh->Sphere.Center;
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

	void Renderer11::GetBoneMatrix(short itemNumber, int joint, Matrix *outMatrix)
	{
		if (itemNumber == Lara.itemNumber)
		{
			RendererObject& obj = *m_moveableObjects[ID_LARA];
			*outMatrix = obj.AnimationTransforms[joint] * m_LaraWorldMatrix;
		}
		else
		{
			UpdateItemAnimations(itemNumber, true);
			
			RendererItem* rendererItem = &m_items[itemNumber];
			ITEM_INFO* nativeItem = &g_Level.Items[itemNumber];

			RendererObject& obj = *m_moveableObjects[nativeItem->objectNumber];
			*outMatrix = obj.AnimationTransforms[joint] * rendererItem->World;
		}
	}

	Vector4 Renderer11::GetPortalRect(Vector4 v, Vector4 vp) 
	{
		Vector4 sp = (v * Vector4(0.5f, 0.5f, 0.5f, 0.5f) 
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
		{
			return vp;
		}

		return s;
	}
}
