#include "Renderer11.h"
#include "level.h"
#include <stack>
#include "savegame.h"
#include "setup.h"
#include "control.h"
#include "objects.h"

bool Renderer11::PrepareDataForTheRenderer()
{
	m_moveableObjects = (RendererObject * *)malloc(sizeof(RendererObject*) * ID_NUMBER_OBJECTS);
	ZeroMemory(m_moveableObjects, sizeof(RendererObject*) * ID_NUMBER_OBJECTS);

	m_spriteSequences = vector<RendererSpriteSequence>(ID_NUMBER_OBJECTS);

	m_staticObjects = (RendererObject * *)malloc(sizeof(RendererObject*) * NUM_STATICS);
	ZeroMemory(m_staticObjects, sizeof(RendererObject*) * NUM_STATICS);

	m_rooms = vector<RendererRoom>(NUM_ROOMS);

	m_meshes.clear();

	// Step 0: prepare animated textures
	short numSets = *AnimTextureRanges;
	short* animatedPtr = AnimTextureRanges;
	animatedPtr++;

	m_animatedTextureSets = vector<RendererAnimatedTextureSet>(NUM_ANIMATED_SETS);
	m_numAnimatedTextureSets = numSets;

	for (int i = 0; i < numSets; i++)
	{
		m_animatedTextureSets[i] = RendererAnimatedTextureSet();
		RendererAnimatedTextureSet& const set = m_animatedTextureSets[i];
		short numTextures = *animatedPtr + 1;
		animatedPtr++;

		set.Textures = vector<RendererAnimatedTexture>(numTextures);
		set.NumTextures = numTextures;

		for (int j = 0; j < numTextures; j++)
		{
			short textureId = *animatedPtr;
			animatedPtr++;

			OBJECT_TEXTURE* texture = &ObjectTextures[textureId];
			int tile = texture->tileAndFlag & 0x7FFF;
			set.Textures[j] = RendererAnimatedTexture();
			RendererAnimatedTexture& const newTexture = set.Textures[j];
			newTexture.Id = textureId;

			for (int k = 0; k < 4; k++)
			{
				float x = (texture->vertices[k].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
				float y = (texture->vertices[k].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

				newTexture.UV[k] = Vector2(x, y);
			}

		}
	}

	// Step 1: create the texture atlas
	byte* buffer = (byte*)malloc(TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);
	ZeroMemory(buffer, TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);

	int blockX = 0;
	int blockY = 0;

	for (int p = 0; p < NumTexturePages; p++)
	{
		for (int y = 0; y < 256; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				int pixelIndex = blockY * TEXTURE_PAGE_SIZE * NUM_TEXTURE_PAGES_PER_ROW + y * 256 * NUM_TEXTURE_PAGES_PER_ROW * 4 + blockX * 256 * 4 + x * 4;
				int oldPixelIndex = p * TEXTURE_PAGE_SIZE + y * 256 * 4 + x * 4;

				byte r = Texture32[oldPixelIndex];
				byte g = Texture32[oldPixelIndex + 1];
				byte b = Texture32[oldPixelIndex + 2];
				byte a = Texture32[oldPixelIndex + 3];

				buffer[pixelIndex + 0] = b;
				buffer[pixelIndex + 1] = g;
				buffer[pixelIndex + 2] = r;
				buffer[pixelIndex + 3] = a;
			}
		}

		blockX++;
		if (blockX == NUM_TEXTURE_PAGES_PER_ROW)
		{
			blockX = 0;
			blockY++;
		}
	}

	if (m_textureAtlas != NULL)
		delete m_textureAtlas;

	m_textureAtlas = Texture2D::LoadFromByteArray(m_device, TEXTURE_ATLAS_SIZE, TEXTURE_ATLAS_SIZE, &buffer[0]);
	if (m_textureAtlas == NULL)
		return false;

	free(buffer);

	buffer = (byte*)malloc(256 * 256 * 4);
	memcpy(buffer, MiscTextures + 256 * 512 * 4, 256 * 256 * 4);

	m_skyTexture = Texture2D::LoadFromByteArray(m_device, 256, 256, &buffer[0]);
	if (m_skyTexture == NULL)
		return false;

	//D3DX11SaveTextureToFileA(m_context, m_skyTexture->Texture, D3DX11_IFF_PNG, "H:\\sky.png");

	free(buffer);

	// Step 2: prepare rooms
	vector<RendererVertex> roomVertices;
	vector<int> roomIndices;

	int baseRoomVertex = 0;
	int baseRoomIndex = 0;

	for (int i = 0; i < NumberRooms; i++)
	{
		ROOM_INFO* room = &Rooms[i];


		m_rooms[i] = RendererRoom();
		RendererRoom& r = m_rooms[i];
		r.RoomNumber = i;
		r.Room = room;
		r.AmbientLight = Vector4(room->ambient.b / 255.0f, room->ambient.g / 255.0f, room->ambient.r / 255.0f, 1.0f);
		r.LightsToDraw = vector<RendererLight*>(MAX_LIGHTS);
		r.Statics.resize(room->numMeshes);

		if (room->NumVertices == 0)
			continue;

		int lastRectangle = 0;
		int lastTriangle = 0;

		tr5_room_layer * layers = (tr5_room_layer*)room->LayerOffset;

		for (int l = 0; l < room->NumLayers; l++)
		{
			tr5_room_layer* layer = &layers[l];
			if (layer->NumLayerVertices == 0)
				continue;

			byte * polygons = (byte*)layer->PolyOffset;
			tr5_room_vertex * vertices = (tr5_room_vertex*)layer->VerticesOffset;

			if (layer->NumLayerRectangles > 0)
			{
				for (int n = 0; n < layer->NumLayerRectangles; n++)
				{
					tr4_mesh_face4* poly = (tr4_mesh_face4*)polygons;

					// Get the real texture index and if double sided
					short textureIndex = poly->Texture & 0x3FFF;
					bool doubleSided = (poly->Texture & 0x8000) >> 15;

					// Get the object texture
					OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
					int tile = texture->tileAndFlag & 0x7FFF;

					// Create vertices
					RendererBucket* bucket;

					int animatedSetIndex = getAnimatedTextureInfo(textureIndex);
					int bucketIndex = RENDERER_BUCKET_SOLID;

					if (!doubleSided)
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT;
						else
							bucketIndex = RENDERER_BUCKET_SOLID;
					}
					else
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
						else
							bucketIndex = RENDERER_BUCKET_SOLID_DS;
					}

					if (animatedSetIndex == -1)
					{
						bucket = &r.Buckets[bucketIndex];
					}
					else
					{
						bucket = &r.AnimatedBuckets[bucketIndex];
					}

					// Calculate face normal
					Vector3 p0 = Vector3(vertices[poly->Vertices[0]].Vertex.x,
						vertices[poly->Vertices[0]].Vertex.y,
						vertices[poly->Vertices[0]].Vertex.z);
					Vector3 p1 = Vector3(vertices[poly->Vertices[1]].Vertex.x,
						vertices[poly->Vertices[1]].Vertex.y,
						vertices[poly->Vertices[1]].Vertex.z);
					Vector3 p2 = Vector3(vertices[poly->Vertices[2]].Vertex.x,
						vertices[poly->Vertices[2]].Vertex.y,
						vertices[poly->Vertices[2]].Vertex.z);
					Vector3 e1 = p1 - p0;
					Vector3 e2 = p1 - p2;
					Vector3 normal = e1.Cross(e2);
					normal.Normalize();

					int baseVertices = bucket->NumVertices;
					for (int v = 0; v < 4; v++)
					{
						RendererVertex vertex;

						vertex.Position.x = room->x + vertices[poly->Vertices[v]].Vertex.x;
						vertex.Position.y = room->y + vertices[poly->Vertices[v]].Vertex.y;
						vertex.Position.z = room->z + vertices[poly->Vertices[v]].Vertex.z;

						vertex.Normal.x = vertices[poly->Vertices[v]].Normal.x;
						vertex.Normal.y = vertices[poly->Vertices[v]].Normal.y;
						vertex.Normal.z = vertices[poly->Vertices[v]].Normal.z;

						vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.Color.x = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.Color.y = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.Color.z = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.Color.w = 1.0f;

						vertex.Bone = 0;

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
					newPolygon.AnimatedSet = animatedSetIndex;
					newPolygon.TextureId = textureIndex;
					newPolygon.Indices[0] = baseVertices;
					newPolygon.Indices[1] = baseVertices + 1;
					newPolygon.Indices[2] = baseVertices + 2;
					newPolygon.Indices[3] = baseVertices + 3;
					bucket->Polygons.push_back(newPolygon);

					polygons += sizeof(tr4_mesh_face4);
				}
			}

			if (layer->NumLayerTriangles > 0)
			{
				for (int n = 0; n < layer->NumLayerTriangles; n++)
				{
					tr4_mesh_face3* poly = (tr4_mesh_face3*)polygons;

					// Get the real texture index and if double sided
					short textureIndex = poly->Texture & 0x3FFF;
					bool doubleSided = (poly->Texture & 0x8000) >> 15;

					// Get the object texture
					OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
					int tile = texture->tileAndFlag & 0x7FFF;

					// Create vertices
					RendererBucket* bucket;

					int animatedSetIndex = getAnimatedTextureInfo(textureIndex);
					int bucketIndex = RENDERER_BUCKET_SOLID;

					if (!doubleSided)
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT;
						else
							bucketIndex = RENDERER_BUCKET_SOLID;
					}
					else
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
						else
							bucketIndex = RENDERER_BUCKET_SOLID_DS;
					}

					if (animatedSetIndex == -1)
					{
						bucket = &r.Buckets[bucketIndex];
					}
					else
					{
						bucket = &r.AnimatedBuckets[bucketIndex];
					}

					// Calculate face normal
					Vector3 p0 = Vector3(vertices[poly->Vertices[0]].Vertex.x,
						vertices[poly->Vertices[0]].Vertex.y,
						vertices[poly->Vertices[0]].Vertex.z);
					Vector3 p1 = Vector3(vertices[poly->Vertices[1]].Vertex.x,
						vertices[poly->Vertices[1]].Vertex.y,
						vertices[poly->Vertices[1]].Vertex.z);
					Vector3 p2 = Vector3(vertices[poly->Vertices[2]].Vertex.x,
						vertices[poly->Vertices[2]].Vertex.y,
						vertices[poly->Vertices[2]].Vertex.z);
					Vector3 e1 = p1 - p0;
					Vector3 e2 = p1 - p2;
					Vector3 normal = e1.Cross(e2);
					normal.Normalize();

					int baseVertices = bucket->NumVertices;
					for (int v = 0; v < 3; v++)
					{
						RendererVertex vertex;

						vertex.Position.x = room->x + vertices[poly->Vertices[v]].Vertex.x;
						vertex.Position.y = room->y + vertices[poly->Vertices[v]].Vertex.y;
						vertex.Position.z = room->z + vertices[poly->Vertices[v]].Vertex.z;

						vertex.Normal.x = vertices[poly->Vertices[v]].Normal.x;
						vertex.Normal.y = vertices[poly->Vertices[v]].Normal.y;
						vertex.Normal.z = vertices[poly->Vertices[v]].Normal.z;

						vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.Color.x = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.Color.y = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.Color.z = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.Color.w = 1.0f;

						vertex.Bone = 0;

						bucket->NumVertices++;
						bucket->Vertices.push_back(vertex);
					}

					bucket->Indices.push_back(baseVertices);
					bucket->Indices.push_back(baseVertices + 1);
					bucket->Indices.push_back(baseVertices + 2);
					bucket->NumIndices += 3;

					RendererPolygon newPolygon;
					newPolygon.Shape = SHAPE_TRIANGLE;
					newPolygon.AnimatedSet = animatedSetIndex;
					newPolygon.TextureId = textureIndex;
					newPolygon.Indices[0] = baseVertices;
					newPolygon.Indices[1] = baseVertices + 1;
					newPolygon.Indices[2] = baseVertices + 2;
					bucket->Polygons.push_back(newPolygon);

					polygons += sizeof(tr4_mesh_face3);
				}
			}
		}

		if (room->numLights != 0)
		{
			tr5_room_light* oldLight = room->light;

			for (int l = 0; l < room->numLights; l++)
			{
				RendererLight light;

				if (oldLight->LightType == LIGHT_TYPES::LIGHT_TYPE_SUN)
				{
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Type = LIGHT_TYPES::LIGHT_TYPE_SUN;
					light.Intensity = 1.0f;

					r.Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_POINT)
				{
					light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Intensity = 1.0f;
					light.In = oldLight->In;
					light.Out = oldLight->Out;
					light.Type = LIGHT_TYPE_POINT;

					r.Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_SHADOW)
				{
					light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.In = oldLight->In;
					light.Out = oldLight->Out;
					light.Type = LIGHT_TYPE_SHADOW;
					light.Intensity = 1.0f;

					r.Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_SPOT)
				{
					light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Intensity = 1.0f;
					light.In = oldLight->In;
					light.Out = oldLight->Out;
					light.Range = oldLight->Range;
					light.Type = LIGHT_TYPE_SPOT;

					r.Lights.push_back(light);
				}

				oldLight++;
			}
		}

		// Merge vertices and indices in a single list
		for (int j = 0; j < NUM_BUCKETS; j++)
		{
			RendererBucket* bucket = &r.Buckets[j];

			bucket->StartVertex = baseRoomVertex;
			bucket->StartIndex = baseRoomIndex;

			for (int k = 0; k < bucket->Vertices.size(); k++)
				roomVertices.push_back(bucket->Vertices[k]);

			for (int k = 0; k < bucket->Indices.size(); k++)
				roomIndices.push_back(baseRoomVertex + bucket->Indices[k]);

			baseRoomVertex += bucket->Vertices.size();
			baseRoomIndex += bucket->Indices.size();
		}
	}

	// Create a single vertex buffer and a single index buffer for all rooms
	// NOTICE: in theory, a 1,000,000 vertices scene should have a VB of 52 MB and an IB of 4 MB
	m_roomsVertexBuffer = VertexBuffer::Create(m_device, roomVertices.size(), roomVertices.data());
	m_roomsIndexBuffer = IndexBuffer::Create(m_device, roomIndices.size(), roomIndices.data());

	m_numHairVertices = 0;
	m_numHairIndices = 0;

	vector<RendererVertex> moveablesVertices;
	vector<int> moveablesIndices;
	int baseMoveablesVertex = 0;
	int baseMoveablesIndex = 0;

	// Step 3: prepare moveables
	for (int i = 0; i < MoveablesIds.size(); i++)
	{
		int objNum = MoveablesIds[i];
		ObjectInfo* obj = &Objects[objNum];

		if (obj->nmeshes > 0)
		{
			RendererObject* moveable = new RendererObject();
			moveable->Id = MoveablesIds[i];

			// Assign the draw routine
			/*if (objNum == ID_FLAME || objNum == ID_FLAME_EMITTER || objNum == ID_FLAME_EMITTER2 || objNum == ID_FLAME_EMITTER3 ||
				objNum == ID_TRIGGER_TRIGGERER || objNum == ID_TIGHT_ROPE || objNum == ID_AI_AMBUSH ||
				objNum == ID_AI_FOLLOW || objNum == ID_AI_GUARD || objNum == ID_AI_MODIFY ||
				objNum == ID_AI_PATROL1 || objNum == ID_AI_PATROL2 || objNum == ID_AI_X1 ||
				objNum == ID_AI_X2 || objNum == ID_DART_EMITTER || objNum == ID_HOMING_DART_EMITTER ||
				objNum == ID_ROPE || objNum == ID_KILL_ALL_TRIGGERS || objNum == ID_EARTHQUAKE ||
				objNum == ID_CAMERA_TARGET || objNum == ID_WATERFALLMIST || objNum == ID_SMOKE_EMITTER_BLACK ||
				objNum == ID_SMOKE_EMITTER_WHITE)
			{
				moveable->DoNotDraw = true;
			}
			else
			{
				moveable->DoNotDraw = false;
			}*/

			moveable->DoNotDraw = (obj->drawRoutine == NULL);

			for (int j = 0; j < obj->nmeshes; j++)
			{
				// HACK: mesh pointer 0 is the placeholder for Lara's body parts and is right hand with pistols
				// We need to override the bone index because the engine will take mesh 0 while drawing pistols anim,
				// and vertices have bone index 0 and not 10
				RendererMesh * mesh = getRendererMeshFromTrMesh(moveable,
					Meshes[obj->meshIndex + j],
					j, MoveablesIds[i] == ID_LARA_SKIN_JOINTS,
					MoveablesIds[i] == ID_LARA_HAIR);
				moveable->ObjectMeshes.push_back(mesh);
			}

			if (objNum == ID_IMP_ROCK || objNum == ID_ENERGY_BUBBLES || objNum == ID_BUBBLES || objNum == ID_BODY_PART)
			{
				// HACK: these objects must have nmeshes = 0 because engine will use them in a different way while drawing Effects.
				// In Core's code this was done in SETUP.C but we must do it here because we need to create renderer's meshes.
				obj->nmeshes = 0;
			}
			else
			{
				int* bone = &Bones[obj->boneIndex];

				stack<RendererBone*> stack;

				for (int j = 0; j < obj->nmeshes; j++)
				{
					moveable->LinearizedBones.push_back(new RendererBone(j));
					moveable->AnimationTransforms.push_back(Matrix::Identity);
					moveable->BindPoseTransforms.push_back(Matrix::Identity);
				}

				RendererBone* currentBone = moveable->LinearizedBones[0];
				RendererBone* stackBone = moveable->LinearizedBones[0];

				for (int mi = 0; mi < obj->nmeshes - 1; mi++)
				{
					int j = mi + 1;

					int opcode = *(bone++);
					int linkX = *(bone++);
					int linkY = *(bone++);
					int linkZ = *(bone++);

					byte flags = opcode & 0x1C;

					moveable->LinearizedBones[j]->ExtraRotationFlags = flags;

					switch (opcode & 0x03)
					{
					case 0:
						moveable->LinearizedBones[j]->Parent = currentBone;
						moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
						currentBone->Children.push_back(moveable->LinearizedBones[j]);
						currentBone = moveable->LinearizedBones[j];

						break;
					case 1:
						if (stack.empty())
							continue;
						currentBone = stack.top();
						stack.pop();

						moveable->LinearizedBones[j]->Parent = currentBone;
						moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
						currentBone->Children.push_back(moveable->LinearizedBones[j]);
						currentBone = moveable->LinearizedBones[j];

						break;
					case 2:
						stack.push(currentBone);

						moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
						moveable->LinearizedBones[j]->Parent = currentBone;
						currentBone->Children.push_back(moveable->LinearizedBones[j]);
						currentBone = moveable->LinearizedBones[j];

						break;
					case 3:
						if (stack.empty())
							continue;
						RendererBone* theBone = stack.top();
						stack.pop();

						moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
						moveable->LinearizedBones[j]->Parent = theBone;
						theBone->Children.push_back(moveable->LinearizedBones[j]);
						currentBone = moveable->LinearizedBones[j];
						stack.push(theBone);

						break;
					}
				}

				for (int n = 0; n < obj->nmeshes; n++)
					moveable->LinearizedBones[n]->Transform = Matrix::CreateTranslation(
						moveable->LinearizedBones[n]->Translation.x,
						moveable->LinearizedBones[n]->Translation.y,
						moveable->LinearizedBones[n]->Translation.z);

				moveable->Skeleton = moveable->LinearizedBones[0];
				buildHierarchy(moveable);

				// Fix Lara skin joints and hairs
				if (MoveablesIds[i] == ID_LARA_SKIN_JOINTS)
				{
					int bonesToCheck[2] = { 0,0 };

					RendererObject* objSkin = m_moveableObjects[ID_LARA_SKIN];

					for (int j = 1; j < obj->nmeshes; j++)
					{
						RendererMesh* jointMesh = moveable->ObjectMeshes[j];
						RendererBone* jointBone = moveable->LinearizedBones[j];

						bonesToCheck[0] = jointBone->Parent->Index;
						bonesToCheck[1] = j;

						for (int b1 = 0; b1 < NUM_BUCKETS; b1++)
						{
							RendererBucket* jointBucket = &jointMesh->Buckets[b1];

							for (int v1 = 0; v1 < jointBucket->Vertices.size(); v1++)
							{
								RendererVertex* jointVertex = &jointBucket->Vertices[v1];

								bool done = false;

								for (int k = 0; k < 2; k++)
								{
									RendererMesh* skinMesh = objSkin->ObjectMeshes[bonesToCheck[k]];
									RendererBone* skinBone = objSkin->LinearizedBones[bonesToCheck[k]];

									for (int b2 = 0; b2 < NUM_BUCKETS; b2++)
									{
										RendererBucket* skinBucket = &skinMesh->Buckets[b2];
										for (int v2 = 0; v2 < skinBucket->Vertices.size(); v2++)
										{
											RendererVertex* skinVertex = &skinBucket->Vertices[v2];

											int x1 = jointBucket->Vertices[v1].Position.x + jointBone->GlobalTranslation.x;
											int y1 = jointBucket->Vertices[v1].Position.y + jointBone->GlobalTranslation.y;
											int z1 = jointBucket->Vertices[v1].Position.z + jointBone->GlobalTranslation.z;

											int x2 = skinBucket->Vertices[v2].Position.x + skinBone->GlobalTranslation.x;
											int y2 = skinBucket->Vertices[v2].Position.y + skinBone->GlobalTranslation.y;
											int z2 = skinBucket->Vertices[v2].Position.z + skinBone->GlobalTranslation.z;

											if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2 && abs(z1 - z2) < 2)
											{
												jointVertex->Bone = bonesToCheck[k];
												jointVertex->Position.x = skinVertex->Position.x;
												jointVertex->Position.y = skinVertex->Position.y;
												jointVertex->Position.z = skinVertex->Position.z;
												done = true;
												break;
											}
										}

										if (done)
											break;
									}

									if (done)
										break;
								}
							}
						}
					}
				}

				if (MoveablesIds[i] == ID_LARA_HAIR)
				{
					for (int j = 0; j < moveable->ObjectMeshes.size(); j++)
					{
						RendererMesh* mesh = moveable->ObjectMeshes[j];
						for (int n = 0; n < NUM_BUCKETS; n++)
						{
							m_numHairVertices += mesh->Buckets[n].NumVertices;
							m_numHairIndices += mesh->Buckets[n].NumIndices;
						}
					}

					m_hairVertices.clear();
					m_hairIndices.clear();

					RendererVertex vertex;
					for (int m = 0; m < m_numHairVertices * 2; m++)
						m_hairVertices.push_back(vertex);
					for (int m = 0; m < m_numHairIndices * 2; m++)
						m_hairIndices.push_back(0);
				}
			}

			m_moveableObjects[MoveablesIds[i]] = moveable;

			// Merge vertices and indices in a single list
			for (int m = 0; m < moveable->ObjectMeshes.size(); m++)
			{
				RendererMesh* msh = moveable->ObjectMeshes[m];

				for (int j = 0; j < NUM_BUCKETS; j++)
				{
					RendererBucket* bucket = &msh->Buckets[j];

					bucket->StartVertex = baseMoveablesVertex;
					bucket->StartIndex = baseMoveablesIndex;

					for (int k = 0; k < bucket->Vertices.size(); k++)
						moveablesVertices.push_back(bucket->Vertices[k]);

					for (int k = 0; k < bucket->Indices.size(); k++)
						moveablesIndices.push_back(baseMoveablesVertex + bucket->Indices[k]);

					baseMoveablesVertex += bucket->Vertices.size();
					baseMoveablesIndex += bucket->Indices.size();
				}
			}
		}
	}

	// Create a single vertex buffer and a single index buffer for all moveables
	m_moveablesVertexBuffer = VertexBuffer::Create(m_device, moveablesVertices.size(), moveablesVertices.data());
	m_moveablesIndexBuffer = IndexBuffer::Create(m_device, moveablesIndices.size(), moveablesIndices.data());

	// Step 4: prepare static meshes
	vector<RendererVertex> staticsVertices;
	vector<int> staticsIndices;
	int baseStaticsVertex = 0;
	int baseStaticsIndex = 0;

	for (int i = 0; i < StaticObjectsIds.size(); i++)
	{
		STATIC_INFO* obj = &StaticObjects[StaticObjectsIds[i]];
		RendererObject* staticObject = new RendererObject();
		staticObject->Id = StaticObjectsIds[i];

		short* meshPtr = Meshes[obj->meshNumber];
		RendererMesh* mesh = getRendererMeshFromTrMesh(staticObject, Meshes[obj->meshNumber], 0, false, false);

		staticObject->ObjectMeshes.push_back(mesh);

		m_staticObjects[StaticObjectsIds[i]] = staticObject;

		// Merge vertices and indices in a single list
		RendererMesh* msh = staticObject->ObjectMeshes[0];

		for (int j = 0; j < NUM_BUCKETS; j++)
		{
			RendererBucket* bucket = &msh->Buckets[j];

			bucket->StartVertex = baseStaticsVertex;
			bucket->StartIndex = baseStaticsIndex;

			for (int k = 0; k < bucket->Vertices.size(); k++)
				staticsVertices.push_back(bucket->Vertices[k]);

			for (int k = 0; k < bucket->Indices.size(); k++)
				staticsIndices.push_back(baseStaticsVertex + bucket->Indices[k]);

			baseStaticsVertex += bucket->Vertices.size();
			baseStaticsIndex += bucket->Indices.size();
		}
	}

	// Create a single vertex buffer and a single index buffer for all statics
	m_staticsVertexBuffer = VertexBuffer::Create(m_device, staticsVertices.size(), staticsVertices.data());
	m_staticsIndexBuffer = IndexBuffer::Create(m_device, staticsIndices.size(), staticsIndices.data());

	// Step 5: prepare sprites
	m_sprites = (RendererSprite * *)malloc(sizeof(RendererSprite*) * g_NumSprites);
	ZeroMemory(m_sprites, sizeof(RendererSprite*) * g_NumSprites);

	for (int i = 0; i < g_NumSprites; i++)
	{
		SPRITE* oldSprite = &Sprites[i];

		RendererSprite* sprite = new RendererSprite();

		sprite->Width = (oldSprite->right - oldSprite->left) * 256.0f;
		sprite->Height = (oldSprite->bottom - oldSprite->top) * 256.0f;

		float left = (oldSprite->left * 256.0f + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float top = (oldSprite->top * 256.0f + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));
		float right = (oldSprite->right * 256.0f + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float bottom = (oldSprite->bottom * 256.0f + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));

		sprite->UV[0] = Vector2(left / (float)TEXTURE_ATLAS_SIZE, top / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[1] = Vector2(right / (float)TEXTURE_ATLAS_SIZE, top / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[2] = Vector2(right / (float)TEXTURE_ATLAS_SIZE, bottom / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[3] = Vector2(left / (float)TEXTURE_ATLAS_SIZE, bottom / (float)TEXTURE_ATLAS_SIZE);

		m_sprites[i] = sprite;
	}

	for (int i = 0; i < MoveablesIds.size(); i++)
	{
		ObjectInfo* obj = &Objects[MoveablesIds[i]];

		if (obj->nmeshes < 0)
		{
			short numSprites = abs(obj->nmeshes);
			short baseSprite = obj->meshIndex;
			m_spriteSequences[MoveablesIds[i]] = RendererSpriteSequence(MoveablesIds[i], numSprites);
			RendererSpriteSequence& sequence = m_spriteSequences[MoveablesIds[i]];

			for (int j = baseSprite; j < baseSprite + numSprites; j++)
			{
				sequence.SpritesList[j - baseSprite] = m_sprites[j];
			}

			m_spriteSequences[MoveablesIds[i]] = sequence;
		}
	}

	for (int i = 0; i < 6; i++)
	{
		if (Objects[ID_WATERFALL1 + i].loaded)
		{
			// Get the first textured bucket
			RendererBucket* bucket = NULL;
			for (int j = 0; j < NUM_BUCKETS; j++)
				if (m_moveableObjects[ID_WATERFALL1 + i]->ObjectMeshes[0]->Buckets[j].Polygons.size() > 0)
					bucket = &m_moveableObjects[ID_WATERFALL1 + i]->ObjectMeshes[0]->Buckets[j];

			if (bucket == NULL)
				continue;

			OBJECT_TEXTURE * texture = &ObjectTextures[bucket->Polygons[0].TextureId];
			WaterfallTextures[i] = texture;
			WaterfallY[i] = texture->vertices[0].y;
		}
	}

	return true;
}
