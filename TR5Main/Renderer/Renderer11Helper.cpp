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

	bool Renderer11::isInRoom(int x, int y, int z, short roomNumber)
	{
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];

		return (x >= r->x && x <= r->x + r->xSize * 1024.0f &&
				y >= r->maxceiling && y <= r->minfloor &&
				z >= r->z && z <= r->z + r->zSize * 1024.0f);
	}

	std::vector<TEN::Renderer::RendererVideoAdapter>* Renderer11::getAdapters()
{
		return &m_adapters;
	}

	void Renderer11::UpdateEffects(RenderView& view)
	{
		for (auto room : view.roomsToDraw)
		{
			for (auto fx : room->EffectsToDraw)
			{
				Matrix translation = Matrix::CreateTranslation(fx->Effect->pos.Position.x, fx->Effect->pos.Position.y, fx->Effect->pos.Position.z);
				Matrix rotation = Matrix::CreateFromYawPitchRoll(fx->Effect->pos.Orientation.y, fx->Effect->pos.Orientation.x, fx->Effect->pos.Orientation.z);
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

			if (nativeItem->Animation.Mutator.size() == boneIndexList.size())
			{
				for (int i : boneIndexList)
				{
					auto mutator = nativeItem->Animation.Mutator[i];
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

	void Renderer11::updateItemAnimations(int itemNumber, bool force)
	{
		RendererItem* itemToDraw = &m_items[itemNumber];
		ITEM_INFO* nativeItem = &g_Level.Items[itemNumber];

		// TODO: hack for fixing a bug, check again if needed
		itemToDraw->ItemNumber = itemNumber;

		// Lara has her own routine
		if (nativeItem->ObjectNumber == ID_LARA)
			return;

		// Has been already done?
		if (!force && itemToDraw->DoneAnimations)
			return;

		OBJECT_INFO* obj = &Objects[nativeItem->ObjectNumber];
		RendererObject& moveableObj = *m_moveableObjects[nativeItem->ObjectNumber];

		// Update animation matrices
		if (obj->animIndex != -1 /*&& item->objectNumber != ID_HARPOON*/)
		{
			// Apply extra rotations
			int lastJoint = 0;
			for (int j = 0; j < moveableObj.LinearizedBones.size(); j++)
			{
				RendererBone *currentBone = moveableObj.LinearizedBones[j];
				currentBone->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);
				
				nativeItem->Data.apply(
					[&j, &currentBone](QuadInfo& quad)
				{
					if (j == 3 || j == 4)
						currentBone->ExtraRotation.x = quad.RearRot;
					else if (j == 6 || j == 7)
						currentBone->ExtraRotation.x = quad.FrontRot;
				},
				[&j, &currentBone](JeepInfo& jeep)
				{
					switch(j)
					{
					case 9:
						currentBone->ExtraRotation.x = jeep.rot1;
						break;
					case 10:
						currentBone->ExtraRotation.x = jeep.rot2;

						break;
					case 12:
						currentBone->ExtraRotation.x = jeep.rot3;

						break;
					case 13:
						currentBone->ExtraRotation.x = jeep.rot4;

						break;
					}
				},
				[&j, &currentBone](MotorbikeInfo& bike)
				{
				switch (j)
				{
					case 2:
					case 4:
						currentBone->ExtraRotation.x = bike.wheelRight;
						break;
					case 10:
						currentBone->ExtraRotation.x = bike.wheelLeft;
				}
				},
				[&j, &currentBone](RubberBoatInfo& boat)
				{
				if (j == 2)
					currentBone->ExtraRotation.z = boat.PropellerRotation;
				},
				[&j, &currentBone](UPVInfo& upv)
				{
				if (j == 3)
					currentBone->ExtraRotation.z = upv.FanRot;
				},
				[&j, &currentBone](BigGunInfo& biggun)
				{
				if (j == 2)
					currentBone->ExtraRotation.z = biggun.BarrelZRotation;
				},
				[&j, &currentBone, &lastJoint](CreatureInfo& creature)
				{
				if (currentBone->ExtraRotationFlags & ROT_Y)
				{
					currentBone->ExtraRotation.y = creature.JointRotation[lastJoint];
					lastJoint++;
				}

				if (currentBone->ExtraRotationFlags & ROT_X)
				{
					currentBone->ExtraRotation.x = creature.JointRotation[lastJoint];
					lastJoint++;
				}

				if (currentBone->ExtraRotationFlags & ROT_Z)
				{
					currentBone->ExtraRotation.z = creature.JointRotation[lastJoint];
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

	void Renderer11::UpdateItemsAnimations(RenderView& view)
	{
		Matrix translation;
		Matrix rotation;

		for (auto room : view.roomsToDraw)
		{
			for (auto itemToDraw : room->ItemsToDraw)
			{
				ITEM_INFO* nativeItem = &g_Level.Items[itemToDraw->ItemNumber];

				// Lara has her own routine
				if (nativeItem->ObjectNumber == ID_LARA)
					continue;

				updateItemAnimations(itemToDraw->ItemNumber, false);
			}
		}
	}

	void Renderer11::FromTrAngle(Matrix *matrix, short *frameptr, int index)
	{
		short *ptr = &frameptr[0];

		ptr += 9;
		for (int i = 0; i < index; i++)
			ptr += ((*ptr & 0xc000) == 0 ? 2 : 1);

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

	bool Renderer11::isFullsScreen() {
		return (!Windowed);
	}
	bool Renderer11::isFading()
	{
		return false;
		return (m_fadeStatus != RENDERER_FADE_STATUS::NO_FADE);
	}

	void Renderer11::UpdateCameraMatrices(CAMERA_INFO *cam, float roll, float fov)
	{
		gameCamera = RenderView(cam, roll, fov, 32, 102400, g_Configuration.Width, g_Configuration.Height);
	}

	void Renderer11::EnumerateVideoModes()
{
		HRESULT res;

		IDXGIFactory *dxgiFactory = NULL;
		Utils::throwIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&dxgiFactory));

		IDXGIAdapter *dxgiAdapter = NULL;

		for (int i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC adapterDesc;
			UINT stringLength;
			char videoCardDescription[128];

			dxgiAdapter->GetDesc(&adapterDesc);
			int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);

			RendererVideoAdapter adapter;

			adapter.Index = i;
			adapter.Name = videoCardDescription;

			TENLog("Adapter " + std::to_string(i), LogLevel::Info);
			TENLog("Device Name: " + adapter.Name, LogLevel::Info);

			ComPtr<IDXGIOutput> output;
			if(FAILED(dxgiAdapter->EnumOutputs(0, output.GetAddressOf())))
				continue;

			UINT numModes = 0;
			std::vector<DXGI_MODE_DESC> displayModes;
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

			// Get the number of elements
			Utils::throwIfFailed(output->GetDisplayModeList(format, 0, &numModes, NULL));

			// Get the list
			displayModes.resize(numModes);
			Utils::throwIfFailed(output->GetDisplayModeList(format, 0, &numModes, displayModes.data()));

			for (int j = 0; j < numModes; j++)
			{
				DXGI_MODE_DESC *mode = &displayModes[j];

				RendererDisplayMode newMode;

				// discard lower resolutions
				if (mode->Width < 1024 || mode->Height < 768)
					continue;

				newMode.Width = mode->Width;
				newMode.Height = mode->Height;
				newMode.RefreshRate = mode->RefreshRate.Numerator / mode->RefreshRate.Denominator;

				bool found = false;
				for (int k = 0; k < adapter.DisplayModes.size(); k++)
				{
					RendererDisplayMode *currentMode = &adapter.DisplayModes[k];
					if (currentMode->Width == newMode.Width && currentMode->Height == newMode.Height &&
						currentMode->RefreshRate == newMode.RefreshRate)
					{
						found = true;
						break;
					}
				}
				if (found)
					continue;

				adapter.DisplayModes.push_back(newMode);
			}

			m_adapters.push_back(adapter);

		}

		dxgiFactory->Release();
	}

	void Renderer11::GetVisibleObjects(int from, int to, RenderView& renderView, bool onlyRooms)
	{
		// Avoid allocations, 1024 should be fine
		RendererRoomNode nodes[512];
		int nextNode = 0;

		// Avoid reallocations, 1024 should be fine
		RendererRoomNode *stack[512];
		int stackDepth = 0;

		RendererRoomNode *node = &nodes[nextNode++];
		node->To = to;
		node->From = -1;

		// Push
		stack[stackDepth++] = node;

		while (stackDepth > 0)
		{
			// Pop
			node = stack[--stackDepth];

			if (m_rooms[node->To].Visited)
				continue;

			ROOM_INFO *room = &g_Level.Rooms[node->To];

			Vector3 roomCentre = Vector3(room->x + room->xSize * WALL_SIZE / 2.0f,
										 (room->minfloor + room->maxceiling) / 2.0f,
										 room->z + room->zSize * WALL_SIZE / 2.0f);
			Vector3 laraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

			m_rooms[node->To].Distance = (roomCentre - laraPosition).Length();
			m_rooms[node->To].Visited = true;
			renderView.roomsToDraw.push_back(&m_rooms[node->To]);
			g_Level.Rooms[node->To].boundActive = true;

			if (!onlyRooms)
			{
				CollectLightsForRoom(node->To, renderView);
				CollectItems(node->To, renderView);
				CollectStatics(node->To, renderView);
				CollectEffects(node->To, renderView);
			}

			Vector4 clipPort;

			for (int i = 0; i < room->doors.size(); i++)
			{
				short adjoiningRoom = room->doors[i].room;

				if (node->From != adjoiningRoom && CheckPortal(node->To, &room->doors[i], renderView.camera.ViewProjection))
				{
					RendererRoomNode *childNode = &nodes[nextNode++];
					childNode->From = node->To;
					childNode->To = adjoiningRoom;

					// Push
					stack[stackDepth++] = childNode;
				}
			}
		}
	}

	bool Renderer11::CheckPortal(short roomIndex, ROOM_DOOR* portal, const Matrix& viewProjection)
	{
		ROOM_INFO *room = &g_Level.Rooms[roomIndex];

		Vector3 n = portal->normal;
		Vector3 v = Vector3(
			Camera.pos.x - (room->x + portal->vertices[0].x),
			Camera.pos.y - (room->y + portal->vertices[0].y),
			Camera.pos.z - (room->z + portal->vertices[0].z));

		// Test camera and normal positions and decide if process door or not
		if (n.Dot(v) <= 0.0f)
			return false;

		int zClip = 0;
		Vector4 p[4];
		Vector4 clipPort{ FLT_MAX ,FLT_MAX ,FLT_MIN,FLT_MIN };

		// Project all portal's corners in screen space
		for (int i = 0; i < 4; i++)
		{
			Vector4 tmp = Vector4(portal->vertices[i].x + room->x, portal->vertices[i].y + room->y, portal->vertices[i].z + room->z, 1.0f);

			// Project corner on screen
			Vector4::Transform(tmp, viewProjection, p[i]);

			if (p[i].w > 0.0f)
			{
				// The corner is in front of camera
				p[i].x *= (1.0f / p[i].w);
				p[i].y *= (1.0f / p[i].w);
				p[i].z *= (1.0f / p[i].w);

				clipPort.x = std::min(clipPort.x, p[i].x);
				clipPort.y = std::min(clipPort.y, p[i].y);
				clipPort.z = std::max(clipPort.z, p[i].x);
				clipPort.w = std::max(clipPort.w, p[i].y);
			}
			else
				// The corner is behind camera
				zClip++;
		}

		// If all points are behind camera then door is not visible
		if (zClip == 4)
			return false;

		// If door crosses camera plane...
		if (zClip > 0)
		{
			for (int i = 0; i < 4; i++)
			{
				Vector4 a = p[i];
				Vector4 b = p[(i + 1) % 4];

				if ((a.w > 0.0f) ^ (b.w > 0.0f))
				{

					if (a.x < 0.0f && b.x < 0.0f)
						clipPort.x = -1.0f;
					else if (a.x > 0.0f && b.x > 0.0f)
						clipPort.z = 1.0f;
					else
					{
						clipPort.x = -1.0f;
						clipPort.z = 1.0f;
					}

					if (a.y < 0.0f && b.y < 0.0f)
						clipPort.y = -1.0f;
					else if (a.y > 0.0f && b.y > 0.0f)
						clipPort.w = 1.0f;
					else
					{
						clipPort.y = -1.0f;
						clipPort.w = 1.0f;
					}
				}
			}
		}
		Vector4 vp = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
		clipPort.x = std::max(clipPort.x, vp.x);
		clipPort.y = std::max(clipPort.y, vp.y);
		clipPort.z = std::min(clipPort.z, vp.z);
		clipPort.w = std::min(clipPort.w, vp.w);

		if (clipPort.x > vp.z || clipPort.y > vp.w || clipPort.z < vp.x || clipPort.w < vp.y)
			return false;

		return true;
	}

	bool Renderer11::SphereBoxIntersection(Vector3 boxMin, Vector3 boxMax, Vector3 sphereCentre, float sphereRadius)
	{
		Vector3 centre = (boxMin + boxMax) / 2.0f;
		Vector3 extens = boxMax - centre;
		BoundingBox box = BoundingBox(centre, extens);
		BoundingSphere sphere = BoundingSphere(sphereCentre, sphereRadius);
		return box.Intersects(sphere);
	}

	void Renderer11::getLaraBonePosition(Vector3 *pos, int bone) {}

	void Renderer11::flipRooms(short roomNumber1, short roomNumber2)
	{
		std::swap(m_rooms[roomNumber1], m_rooms[roomNumber2]);

		m_rooms[roomNumber1].RoomNumber = roomNumber1;
		m_rooms[roomNumber2].RoomNumber = roomNumber2;
	}

	RendererMesh* Renderer11::GetMesh(int meshIndex)
	{
		return m_meshes[meshIndex];
	}

	void Renderer11::getLaraAbsBonePosition(Vector3 *pos, int joint)
	{
		Matrix world = m_moveableObjects[ID_LARA]->AnimationTransforms[joint];
		world = world * m_LaraWorldMatrix;
		*pos = Vector3::Transform(*pos, world);
	}

	void Renderer11::getItemAbsBonePosition(int itemNumber, Vector3 *pos, int joint)
	{
		RendererItem *rendererItem = &m_items[itemNumber];
		ITEM_INFO* nativeItem = &g_Level.Items[itemNumber];

		rendererItem->ItemNumber = itemNumber;

		if (!rendererItem)
			return;

		if (!rendererItem->DoneAnimations)
		{
			if (itemNumber == Lara.ItemNumber)
				updateLaraAnimations(false);
			else
				updateItemAnimations(itemNumber, false);
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
			if (itemNumber == Lara.ItemNumber)
				updateLaraAnimations(false);
			else
				updateItemAnimations(itemNumber, false);
		}

		Matrix world;

		if (worldSpace & SPHERES_SPACE_WORLD)
			world = Matrix::CreateTranslation(nativeItem->Pose.Position.x, nativeItem->Pose.Position.y, nativeItem->Pose.Position.z) * local;
		else
			world = Matrix::Identity * local;

		world = Matrix::CreateFromYawPitchRoll(nativeItem->Orientation.y, nativeItem->Orientation.x, nativeItem->Orientation.z) * world;

		short objNum = nativeItem->ObjectNumber;
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
			// addLine3D(v1, v2, Vector4::One);
		}

		return moveable.ObjectMeshes.size();
	}

	void Renderer11::getBoneMatrix(short itemNumber, int joint, Matrix *outMatrix)
	{
		if (itemNumber == Lara.ItemNumber)
		{
			RendererObject& obj = *m_moveableObjects[ID_LARA];
			*outMatrix = obj.AnimationTransforms[joint] * m_LaraWorldMatrix;
		}
		else
		{
			updateItemAnimations(itemNumber, true);
			
			RendererItem* rendererItem = &m_items[itemNumber];
			ITEM_INFO* nativeItem = &g_Level.Items[itemNumber];

			RendererObject& obj = *m_moveableObjects[nativeItem->ObjectNumber];
			*outMatrix = obj.AnimationTransforms[joint] * rendererItem->World;
		}
	}
}
