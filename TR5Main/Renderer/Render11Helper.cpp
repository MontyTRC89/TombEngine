#pragma once
#include "Renderer11.h"
#include "../Specific/configuration.h"
#include "../Game/camera.h"
#include "../Game/draw.h"
#include "../Specific/setup.h"
extern GameConfiguration g_Configuration;
extern GameFlow* g_GameFlow;
bool Renderer11::isRoomUnderwater(short roomNumber)
{
	return (m_rooms[roomNumber].Room->flags & ENV_FLAG_WATER);
}

bool Renderer11::isInRoom(int x, int y, int z, short roomNumber)
{
	RendererRoom& const room = m_rooms[roomNumber];
	ROOM_INFO* r = room.Room;

	return (x >= r->x && x <= r->x + r->xSize * 1024.0f &&
		y >= r->maxceiling && y <= r->minfloor &&
		z >= r->z && z <= r->z + r->ySize * 1024.0f);
}

vector<RendererVideoAdapter>* Renderer11::GetAdapters()
{
	return &m_adapters;
}

void Renderer11::createBillboardMatrix(Matrix* out, Vector3* particlePos, Vector3* cameraPos, float rotation)
{
	Vector3 look = *particlePos;
	look = look - *cameraPos;
	look.Normalize();

	Vector3 cameraUp = Vector3(0.0f, -1.0f, 0.0f);

	Vector3 right;
	right = cameraUp.Cross(look);
	right.Normalize();

	// Rotate right vector
	Matrix rightTransform = Matrix::CreateFromAxisAngle(look, rotation);
	right = Vector3::Transform(right, rightTransform);

	Vector3 up;
	up = look.Cross(right);
	up.Normalize();

	*out = Matrix::Identity;

	out->_11 = right.x;
	out->_12 = right.y;
	out->_13 = right.z;

	out->_21 = up.x;
	out->_22 = up.y;
	out->_23 = up.z;

	out->_31 = look.x;
	out->_32 = look.y;
	out->_33 = look.z;

	out->_41 = particlePos->x;
	out->_42 = particlePos->y;
	out->_43 = particlePos->z;
}

void Renderer11::updateAnimatedTextures()
{
	// Update room's animated textures
	for (int i = 0; i < NumberRooms; i++)
	{
		if (m_rooms.size() <= i) continue;
		RendererRoom & const room = m_rooms[i];

		for (int bucketIndex = 0; bucketIndex < NUM_BUCKETS; bucketIndex++)
		{
			RendererBucket* bucket = &room.AnimatedBuckets[bucketIndex];

			if (bucket->Vertices.size() == 0)
				continue;

			for (int p = 0; p < bucket->Polygons.size(); p++)
			{
				RendererPolygon* polygon = &bucket->Polygons[p];
				RendererAnimatedTextureSet& const set = m_animatedTextureSets[polygon->AnimatedSet];
				int textureIndex = -1;
				for (int j = 0; j < set.NumTextures; j++)
				{
					if (set.Textures[j].Id == polygon->TextureId)
					{
						textureIndex = j;
						break;
					}
				}
				if (textureIndex == -1)
					continue;

				if (textureIndex == set.NumTextures - 1)
					textureIndex = 0;
				else
					textureIndex++;

				polygon->TextureId = set.Textures[textureIndex].Id;

				for (int v = 0; v < (polygon->Shape == SHAPE_RECTANGLE ? 4 : 3); v++)
				{
					bucket->Vertices[polygon->Indices[v]].UV.x = set.Textures[textureIndex].UV[v].x;
					bucket->Vertices[polygon->Indices[v]].UV.y = set.Textures[textureIndex].UV[v].y;
				}
			}
		}
	}

	// Update waterfalls textures
	for (int i = ID_WATERFALL1; i <= ID_WATERFALLSS2; i++)
	{
		OBJECT_INFO* obj = &Objects[i];

		if (obj->loaded)
		{
			RendererObject* waterfall = m_moveableObjects[i];

			for (int m = 0; m < waterfall->ObjectMeshes.size(); m++)
			{
				RendererMesh* mesh = waterfall->ObjectMeshes[m];
				RendererBucket* bucket = &mesh->Buckets[RENDERER_BUCKET_TRANSPARENT_DS];

				for (int v = 0; v < bucket->Vertices.size(); v++)
				{
					RendererVertex* vertex = &bucket->Vertices[v];
					int y = vertex->UV.y * TEXTURE_ATLAS_SIZE + 64;
					y %= 128;
					vertex->UV.y = (float)y / TEXTURE_ATLAS_SIZE;
				}
			}
		}
	}
}

void Renderer11::updateEffects()
{
	for (int i = 0; i < m_effectsToDraw.size(); i++)
	{
		RendererEffect* fx = m_effectsToDraw[i];

		Matrix translation = Matrix::CreateTranslation(fx->Effect->pos.xPos, fx->Effect->pos.yPos, fx->Effect->pos.zPos);
		Matrix rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(fx->Effect->pos.yRot), TR_ANGLE_TO_RAD(fx->Effect->pos.xRot), TR_ANGLE_TO_RAD(fx->Effect->pos.zRot));
		m_effectsToDraw[i]->World = rotation * translation;
	}
}

void Renderer11::updateAnimation(RendererItem* item, RendererObject* obj, short** frmptr, short frac, short rate, int mask, bool useObjectWorldRotation)
{
	RendererBone* bones[32];
	int nextBone = 0;

	Matrix rotation;

	Matrix* transforms = (item == NULL ? obj->AnimationTransforms.data() : &item->AnimationTransforms[0]);

	// Push
	bones[nextBone++] = obj->Skeleton;

	while (nextBone != 0)
	{
		// Pop the last bone in the stack
		RendererBone* bone = bones[--nextBone];

		bool calculateMatrix = (mask >> bone->Index) & 1;

		if (calculateMatrix)
		{
			Vector3 p = Vector3((int) * (frmptr[0] + 6), (int) * (frmptr[0] + 7), (int) * (frmptr[0] + 8));

			fromTrAngle(&rotation, frmptr[0], bone->Index);

			if (frac)
			{
				Vector3 p2 = Vector3((int) * (frmptr[1] + 6), (int) * (frmptr[1] + 7), (int) * (frmptr[1] + 8));
				p = Vector3::Lerp(p, p2, frac / ((float)rate));

				Matrix rotation2;
				fromTrAngle(&rotation2, frmptr[1], bone->Index);

				Quaternion q1, q2, q3;

				q1 = Quaternion::CreateFromRotationMatrix(rotation);
				q2 = Quaternion::CreateFromRotationMatrix(rotation2);
				q3 = Quaternion::Slerp(q1, q2, frac / ((float)rate));

				rotation = Matrix::CreateFromQuaternion(q3);
			}

			Matrix translation;
			if (bone == obj->Skeleton)
				translation = Matrix::CreateTranslation(p.x, p.y, p.z);

			Matrix extraRotation;
			extraRotation = Matrix::CreateFromYawPitchRoll(bone->ExtraRotation.y, bone->ExtraRotation.x, bone->ExtraRotation.z);
			if (useObjectWorldRotation) {
				Quaternion invertedQuat;
				transforms[bone->Parent->Index].Invert().Decompose(Vector3(), invertedQuat, Vector3());
				rotation = extraRotation * rotation * Matrix::CreateFromQuaternion(invertedQuat);
			}
			else {
				rotation = extraRotation * rotation;

			}

			if (bone != obj->Skeleton)
				transforms[bone->Index] = rotation * bone->Transform;
			else
				transforms[bone->Index] = rotation * translation;

			if (bone != obj->Skeleton)

				transforms[bone->Index] = transforms[bone->Index] * transforms[bone->Parent->Index];
		}

		for (int i = 0; i < bone->Children.size(); i++)
		{
			// Push
			bones[nextBone++] = bone->Children[i];
		}
	}
}
int Renderer11::getFrame(short animation, short frame, short** framePtr, int* rate)
{
	ITEM_INFO item;
	item.animNumber = animation;
	item.frameNumber = frame;

	return GetFrame_D2(&item, framePtr, rate);
}

bool Renderer11::updateConstantBuffer(ID3D11Buffer* buffer, void* data, int size)
{
	HRESULT res;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	res = m_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(res))
		return false;

	// Get a pointer to the data in the constant buffer.
	char* dataPtr = reinterpret_cast<char*>(mappedResource.pData);
	memcpy(dataPtr, data, size);

	// Unlock the constant buffer.
	m_context->Unmap(buffer, 0);

	return true;
}

void Renderer11::UpdateItemAnimations(int itemNumber)
{
	RendererItem* itemToDraw = &m_items[itemNumber];
	itemToDraw->Id = itemNumber;
	itemToDraw->Item = &Items[itemNumber];

	ITEM_INFO* item = itemToDraw->Item;
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	// Lara has her own routine
	if (item->objectNumber == ID_LARA)
		return;

	// Has been already done?
	/*if (itemToDraw->DoneAnimations)
		return;*/

	OBJECT_INFO* obj = &Objects[item->objectNumber];
	RendererObject* moveableObj = m_moveableObjects[item->objectNumber];

	// Update animation matrices
	if (obj->animIndex != -1 /*&& item->objectNumber != ID_HARPOON*/)
	{
		// Apply extra rotations
		int lastJoint = 0;
		for (int j = 0; j < moveableObj->LinearizedBones.size(); j++)
		{
			RendererBone* currentBone = moveableObj->LinearizedBones[j];
			currentBone->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);

			if (creature)
			{
				if (currentBone->ExtraRotationFlags & ROT_Y)
				{
					currentBone->ExtraRotation.y = TR_ANGLE_TO_RAD(creature->jointRotation[lastJoint]);
					lastJoint++;
				}

				if (currentBone->ExtraRotationFlags & ROT_X)
				{
					currentBone->ExtraRotation.x = TR_ANGLE_TO_RAD(creature->jointRotation[lastJoint]);
					lastJoint++;
				}

				if (currentBone->ExtraRotationFlags & ROT_Z)
				{
					currentBone->ExtraRotation.z = TR_ANGLE_TO_RAD(creature->jointRotation[lastJoint]);
					lastJoint++;
				}
			}
		}

		short* framePtr[2];
		int rate;
		int frac = GetFrame_D2(item, framePtr, &rate);

		updateAnimation(itemToDraw, moveableObj, framePtr, frac, rate, 0xFFFFFFFF);

		for (int m = 0; m < itemToDraw->NumMeshes; m++)
			itemToDraw->AnimationTransforms[m] = itemToDraw->AnimationTransforms[m];
	}

	itemToDraw->DoneAnimations = true;
}

void Renderer11::updateItemsAnimations()
{
	Matrix translation;
	Matrix rotation;

	int numItems = m_itemsToDraw.size();

	for (int i = 0; i < numItems; i++)
	{
		RendererItem* itemToDraw = m_itemsToDraw[i];
		ITEM_INFO* item = itemToDraw->Item;
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		// Lara has her own routine
		if (item->objectNumber == ID_LARA)
			continue;

		UpdateItemAnimations(itemToDraw->Id);
	}
}

void Renderer11::fromTrAngle(Matrix* matrix, short* frameptr, int index)
{
	short* ptr = &frameptr[0];

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
		rotZ = ((rot1) & 0x3ff);

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

void Renderer11::buildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode)
{
	node->GlobalTransform = node->Transform * parentNode->GlobalTransform;
	obj->BindPoseTransforms[node->Index] = node->GlobalTransform;
	obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);
	node->GlobalTranslation = node->Translation + parentNode->GlobalTranslation;

	for (int j = 0; j < node->Children.size(); j++)
	{
		buildHierarchyRecursive(obj, node->Children[j], node);
	}
}

void Renderer11::buildHierarchy(RendererObject* obj)
{
	obj->Skeleton->GlobalTransform = obj->Skeleton->Transform;
	obj->BindPoseTransforms[obj->Skeleton->Index] = obj->Skeleton->GlobalTransform;
	obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);

	for (int j = 0; j < obj->Skeleton->Children.size(); j++)
	{
		buildHierarchyRecursive(obj, obj->Skeleton->Children[j], obj->Skeleton);
	}
}

RendererMesh* Renderer11::getRendererMeshFromTrMesh(RendererObject* obj, short* meshPtr, short boneIndex, int isJoints, int isHairs)
{
	RendererMesh* mesh = new RendererMesh();

	short* basePtr = meshPtr;

	short cx = *meshPtr++;
	short cy = *meshPtr++;
	short cz = *meshPtr++;
	short r1 = *meshPtr++;
	short r2 = *meshPtr++;

	mesh->Sphere = BoundingSphere(Vector3(cx, cy, cz), r1);

	short numVertices = *meshPtr++;

	VECTOR* vertices = (VECTOR*)malloc(sizeof(VECTOR) * numVertices);
	for (int v = 0; v < numVertices; v++)
	{
		short x = *meshPtr++;
		short y = *meshPtr++;
		short z = *meshPtr++;

		vertices[v].vx = x;
		vertices[v].vy = y;
		vertices[v].vz = z;

		mesh->Positions.push_back(Vector3(x, y, z));
	}

	short numNormals = *meshPtr++;
	VECTOR* normals = NULL;
	short* colors = NULL;
	if (numNormals > 0)
	{
		normals = (VECTOR*)malloc(sizeof(VECTOR) * numNormals);
		for (int v = 0; v < numNormals; v++)
		{
			short x = *meshPtr++;
			short y = *meshPtr++;
			short z = *meshPtr++;

			normals[v].vx = x;
			normals[v].vy = y;
			normals[v].vz = z;
		}
	}
	else
	{
		short numLights = -numNormals;
		colors = (short*)malloc(sizeof(short) * numLights);
		for (int v = 0; v < numLights; v++)
		{
			colors[v] = *meshPtr++;
		}
	}

	short numRectangles = *meshPtr++;

	for (int r = 0; r < numRectangles; r++)
	{
		short v1 = *meshPtr++;
		short v2 = *meshPtr++;
		short v3 = *meshPtr++;
		short v4 = *meshPtr++;
		short textureId = *meshPtr++;
		short effects = *meshPtr++;

		short indices[4] = { v1,v2,v3,v4 };

		short textureIndex = textureId & 0x7FFF;
		bool doubleSided = (textureId & 0x8000) >> 15;

		// Get the object texture
		OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
		int tile = texture->tileAndFlag & 0x7FFF;

		// Create vertices
		RendererBucket* bucket;
		int bucketIndex = RENDERER_BUCKET_SOLID;
		if (!doubleSided)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT;
			else
				bucketIndex = RENDERER_BUCKET_SOLID;
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
			else
				bucketIndex = RENDERER_BUCKET_SOLID_DS;
		}

		// ColAddHorizon special handling
		if (obj != NULL && obj->Id == ID_HORIZON && g_GameFlow->GetLevel(CurrentLevel)->ColAddHorizon)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT;
			else
				bucketIndex = RENDERER_BUCKET_SOLID;
		}

		bucket = &mesh->Buckets[bucketIndex];
		if (obj != NULL) obj->HasDataInBucket[bucketIndex] = true;

		int baseVertices = bucket->NumVertices;
		for (int v = 0; v < 4; v++)
		{
			RendererVertex vertex;

			vertex.Position.x = vertices[indices[v]].vx;
			vertex.Position.y = vertices[indices[v]].vy;
			vertex.Position.z = vertices[indices[v]].vz;

			if (numNormals > 0)
			{
				vertex.Normal.x = normals[indices[v]].vx / 16300.0f;
				vertex.Normal.y = normals[indices[v]].vy / 16300.0f;
				vertex.Normal.z = normals[indices[v]].vz / 16300.0f;
			}

			vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

			vertex.Bone = boneIndex;
			if (isHairs)
				vertex.Bone = indices[v];

			if (colors == NULL)
			{
				vertex.Color = Vector4::One * 0.5f;
			}
			else
			{
				short shade = colors[indices[v]];
				shade = (255 - shade * 255 / 8191) & 0xFF;
				vertex.Color = Vector4(shade / 255.0f, shade / 255.0f, shade / 255.0f, 1.0f);
			}

			bucket->NumVertices++;
			bucket->Vertices.push_back(vertex);
		}

		bucket->Indices.push_back(baseVertices);
		bucket->Indices.push_back(baseVertices + 1);
		bucket->Indices.push_back(baseVertices + 3);
		bucket->Indices.push_back(baseVertices + 2);
		bucket->Indices.push_back(baseVertices + 3);
		bucket->Indices.push_back(baseVertices + 1);
		bucket->NumIndices += 6;

		RendererPolygon newPolygon;
		newPolygon.Shape = SHAPE_RECTANGLE;
		newPolygon.TextureId = textureId;
		newPolygon.Indices[0] = baseVertices;
		newPolygon.Indices[1] = baseVertices + 1;
		newPolygon.Indices[2] = baseVertices + 2;
		newPolygon.Indices[3] = baseVertices + 3;
		bucket->Polygons.push_back(newPolygon);
	}

	short numTriangles = *meshPtr++;

	for (int r = 0; r < numTriangles; r++)
	{
		short v1 = *meshPtr++;
		short v2 = *meshPtr++;
		short v3 = *meshPtr++;
		short textureId = *meshPtr++;
		short effects = *meshPtr++;

		short indices[3] = { v1,v2,v3 };

		short textureIndex = textureId & 0x7FFF;
		bool doubleSided = (textureId & 0x8000) >> 15;

		// Get the object texture
		OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
		int tile = texture->tileAndFlag & 0x7FFF;

		// Create vertices
		RendererBucket* bucket;
		int bucketIndex = RENDERER_BUCKET_SOLID;
		if (!doubleSided)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT;
			else
				bucketIndex = RENDERER_BUCKET_SOLID;
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
			else
				bucketIndex = RENDERER_BUCKET_SOLID_DS;
		}
		bucket = &mesh->Buckets[bucketIndex];
		if (obj != NULL) obj->HasDataInBucket[bucketIndex] = true;

		int baseVertices = bucket->NumVertices;
		for (int v = 0; v < 3; v++)
		{
			RendererVertex vertex;

			vertex.Position.x = vertices[indices[v]].vx;
			vertex.Position.y = vertices[indices[v]].vy;
			vertex.Position.z = vertices[indices[v]].vz;

			if (numNormals > 0)
			{
				vertex.Normal.x = normals[indices[v]].vx / 16300.0f;
				vertex.Normal.y = normals[indices[v]].vy / 16300.0f;
				vertex.Normal.z = normals[indices[v]].vz / 16300.0f;
			}

			vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

			vertex.Bone = boneIndex;
			if (isHairs)
				vertex.Bone = indices[v];

			if (colors == NULL)
			{
				vertex.Color = Vector4::One * 0.5f;
			}
			else
			{
				short shade = colors[indices[v]];
				shade = (255 - shade * 255 / 8191) & 0xFF;
				vertex.Color = Vector4(shade / 255.0f, shade / 255.0f, shade / 255.0f, 1.0f);
			}

			bucket->NumVertices++;
			bucket->Vertices.push_back(vertex);
		}

		bucket->Indices.push_back(baseVertices);
		bucket->Indices.push_back(baseVertices + 1);
		bucket->Indices.push_back(baseVertices + 2);
		bucket->NumIndices += 3;

		RendererPolygon newPolygon;
		newPolygon.Shape = SHAPE_TRIANGLE;
		newPolygon.TextureId = textureId;
		newPolygon.Indices[0] = baseVertices;
		newPolygon.Indices[1] = baseVertices + 1;
		newPolygon.Indices[2] = baseVertices + 2;
		bucket->Polygons.push_back(newPolygon);
	}

	free(vertices);
	if (normals != NULL) free(normals);
	if (colors != NULL) free(colors);

	unsigned int castedMeshPtr = reinterpret_cast<unsigned int>(basePtr);

	if (m_meshPointersToMesh.find(castedMeshPtr) == m_meshPointersToMesh.end())
	{
		m_meshPointersToMesh.insert(pair<unsigned int, RendererMesh*>(castedMeshPtr, mesh));
	}
	/*else if (m_meshPointersToMesh[castedMeshPtr] == NULL)
	{
		m_meshPointersToMesh[castedMeshPtr] = mesh;
	}*/

	m_meshes.push_back(mesh);

	return mesh;
}

int Renderer11::getAnimatedTextureInfo(short textureId)
{
	for (int i = 0; i < m_numAnimatedTextureSets; i++)
	{
		RendererAnimatedTextureSet& const set = m_animatedTextureSets[i];
		for (int j = 0; j < set.NumTextures; j++)
		{
			if (set.Textures[j].Id == textureId)
				return i;
		}
	}

	return -1;
}
bool Renderer11::IsFullsScreen()
{
	return (!Windowed);
}
bool Renderer11::IsFading()
{
	return (m_fadeStatus != FADEMODE_NONE);
}

void Renderer11::UpdateCameraMatrices(float posX, float posY, float posZ, float targetX, float targetY, float targetZ, float roll, float fov)
{
	g_Configuration.MaxDrawDistance = 200;

	Vector3 up = -Vector3::UnitY;
	Matrix upRotation = Matrix::CreateFromYawPitchRoll(0.0f, 0.0f, roll);
	up = Vector3::Transform(up, upRotation);

	int zNear = 20;
	int zFar = g_Configuration.MaxDrawDistance * 1024;

	FieldOfView = fov;
	View = Matrix::CreateLookAt(Vector3(posX, posY, posZ), Vector3(targetX, targetY, targetZ), up);
	Projection = Matrix::CreatePerspectiveFieldOfView(fov, ScreenWidth / (float)ScreenHeight, zNear, zFar);
	ViewProjection = View * Projection;
	// Setup legacy variables
	PhdZNear = zNear << W2V_SHIFT;
	PhdZFar = zFar << W2V_SHIFT;
}

bool Renderer11::EnumerateVideoModes()
{
	HRESULT res;

	IDXGIFactory* dxgiFactory = NULL;
	res = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)& dxgiFactory);
	if (FAILED(res))
		return false;

	IDXGIAdapter* dxgiAdapter = NULL;

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

		printf("Adapter %d\n", i);
		printf("\t Device Name: %s\n", videoCardDescription);

		IDXGIOutput* output = NULL;
		res = dxgiAdapter->EnumOutputs(0, &output);
		if (FAILED(res))
			return false;

		UINT numModes = 0;
		DXGI_MODE_DESC* displayModes = NULL;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Get the number of elements
		res = output->GetDisplayModeList(format, 0, &numModes, NULL);
		if (FAILED(res))
			return false;

		// Get the list
		displayModes = new DXGI_MODE_DESC[numModes];
		res = output->GetDisplayModeList(format, 0, &numModes, displayModes);
		if (FAILED(res))
		{
			delete displayModes;
			return false;
		}

		for (int j = 0; j < numModes; j++)
		{
			DXGI_MODE_DESC* mode = &displayModes[j];

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
				RendererDisplayMode* currentMode = &adapter.DisplayModes[k];
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

			printf("\t\t %d x %d %d Hz\n", newMode.Width, newMode.Height, newMode.RefreshRate);
		}

		m_adapters.push_back(adapter);

		delete displayModes;
	}

	dxgiFactory->Release();

	return true;
}

int SortLightsFunction(RendererLight* a, RendererLight* b)
{
	if (a->Dynamic > b->Dynamic)
		return -1;
	return 0;
}

bool SortRoomsFunction(RendererRoom* a, RendererRoom* b)
{
	return (a->Distance < b->Distance);
}

int SortRoomsFunctionNonStd(RendererRoom* a, RendererRoom* b)
{
	return (a->Distance - b->Distance);
}

void Renderer11::getVisibleRooms(int from, int to, Vector4* viewPort, bool water, int count)
{
	// Avoid allocations, 1024 should be fine
	RendererRoomNode nodes[1024];
	int nextNode = 0;

	// Avoid reallocations, 1024 should be fine
	RendererRoomNode* stack[1024];
	int stackDepth = 0;

	RendererRoomNode* node = &nodes[nextNode++];
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

		ROOM_INFO* room = &Rooms[node->To];

		Vector3 roomCentre = Vector3(room->x + room->xSize * WALL_SIZE / 2.0f,
			(room->minfloor + room->maxceiling) / 2.0f,
			room->z + room->ySize * WALL_SIZE / 2.0f);
		Vector3 laraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		m_rooms[node->To].Distance = (roomCentre - laraPosition).Length();
		m_rooms[node->To].Visited = true;
		m_roomsToDraw.push_back(&m_rooms[node->To]);
		Rooms[node->To].boundActive = true;

		collectLightsForRoom(node->To);
		collectItems(node->To);
		collectStatics(node->To);
		collectEffects(node->To);

		Vector4 clipPort;

		if (room->door != NULL)
		{
			short numDoors = *(room->door);
			if (numDoors)
			{
				short* door = room->door + 1;
				for (int i = 0; i < numDoors; i++) {
					short adjoiningRoom = *(door);

					if (node->From != adjoiningRoom && checkPortal(node->To, door, viewPort, &node->ClipPort))
					{
						RendererRoomNode* childNode = &nodes[nextNode++];
						childNode->From = node->To;
						childNode->To = adjoiningRoom;

						// Push
						stack[stackDepth++] = childNode;
					}

					door += 16;
				}
			}
		}
	}
}

bool Renderer11::checkPortal(short roomIndex, short* portal, Vector4* viewPort, Vector4* clipPort)
{
	ROOM_INFO* room = &Rooms[roomIndex];

	portal++;

	Vector3 n = Vector3(portal[0], portal[1], portal[2]);
	Vector3 v = Vector3(
		Camera.pos.x - (room->x + portal[3]),
		Camera.pos.y - (room->y + portal[4]),
		Camera.pos.z - (room->z + portal[5]));

	// Test camera and normal positions and decide if process door or not
	if (n.Dot(v) <= 0.0f)
		return false;

	int  zClip = 0;
	Vector4 p[4];

	clipPort->x = FLT_MAX;
	clipPort->y = FLT_MAX;
	clipPort->z = FLT_MIN;
	clipPort->w = FLT_MIN;

	portal += 3;

	// Project all portal's corners in screen space
	for (int i = 0; i < 4; i++, portal += 3) 
	{
		Vector4 tmp = Vector4(portal[0] + room->x, portal[1] + room->y, portal[2] + room->z, 1.0f);

		// Project corner on screen
		Vector4::Transform(tmp, ViewProjection, p[i]);

		if (p[i].w > 0.0f) 
		{
			// The corner is in front of camera
			p[i].x *= (1.0f / p[i].w);
			p[i].y *= (1.0f / p[i].w);
			p[i].z *= (1.0f / p[i].w);

			clipPort->x = min(clipPort->x, p[i].x);
			clipPort->y = min(clipPort->y, p[i].y);
			clipPort->z = max(clipPort->z, p[i].x);
			clipPort->w = max(clipPort->w, p[i].y);
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

			if ((a.w > 0.0f) ^ (b.w > 0.0f)) {

				if (a.x < 0.0f && b.x < 0.0f)
					clipPort->x = -1.0f;
				else
					if (a.x > 0.0f && b.x > 0.0f)
						clipPort->z = 1.0f;
					else 
					{
						clipPort->x = -1.0f;
						clipPort->z = 1.0f;
					}

				if (a.y < 0.0f && b.y < 0.0f)
					clipPort->y = -1.0f;
				else
					if (a.y > 0.0f && b.y > 0.0f)
						clipPort->w = 1.0f;
					else 
					{
						clipPort->y = -1.0f;
						clipPort->w = 1.0f;
					}

			}
		}
	}

	clipPort->x = max(clipPort->x, viewPort->x);
	clipPort->y = max(clipPort->y, viewPort->y);
	clipPort->z = min(clipPort->z, viewPort->z);
	clipPort->w = min(clipPort->w, viewPort->w);

	if (clipPort->x > viewPort->z || clipPort->y > viewPort->w || clipPort->z < viewPort->x || clipPort->w < viewPort->y)
		return false;

	return true;
}

bool Renderer11::sphereBoxIntersection(Vector3 boxMin, Vector3 boxMax, Vector3 sphereCentre, float sphereRadius)
{
	Vector3 centre = (boxMin + boxMax) / 2.0f;
	Vector3 extens = boxMax - centre;
	BoundingBox box = BoundingBox(centre, extens);
	BoundingSphere sphere = BoundingSphere(sphereCentre, sphereRadius);
	return box.Intersects(sphere);
}

void Renderer11::GetLaraBonePosition(Vector3* pos, int bone)
{

}

void Renderer11::FlipRooms(short roomNumber1, short roomNumber2)
{
	RendererRoom temporary;

	temporary = m_rooms[roomNumber1];
	m_rooms[roomNumber1] = m_rooms[roomNumber2];
	m_rooms[roomNumber2] = temporary;
	m_rooms[roomNumber1].Room = &Rooms[roomNumber1];
	m_rooms[roomNumber2].Room = &Rooms[roomNumber2];
}

RendererMesh* Renderer11::getMeshFromMeshPtr(unsigned int meshp)
{
	return m_meshPointersToMesh[meshp];
}
