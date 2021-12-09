#include "framework.h"
#include "Renderer11.h"
#include "level.h"
#include "savegame.h"
#include "setup.h"
#include "control/control.h"
#include "objects.h"
#include <lara_struct.h>
#include <tuple>
#include <stack>
using std::optional;
using std::stack;
using std::vector;
namespace TEN::Renderer
{
	bool Renderer11::PrepareDataForTheRenderer()
	{
		m_moveableObjects.resize(ID_NUMBER_OBJECTS);
		m_spriteSequences.resize(ID_NUMBER_OBJECTS);
		m_staticObjects.resize(MAX_STATICS);
		m_rooms.resize(g_Level.Rooms.size());

		m_meshes.clear();

		// Step 0: prepare animated textures
		/*short numSets = *AnimTextureRanges;
		short *animatedPtr = AnimTextureRanges;
		animatedPtr++;

		m_animatedTextureSets = vector<RendererAnimatedTextureSet>(NUM_ANIMATED_SETS);
		m_numAnimatedTextureSets = numSets;

		for (int i = 0; i < numSets; i++)
		{
			m_animatedTextureSets[i] = RendererAnimatedTextureSet();
			RendererAnimatedTextureSet &const set = m_animatedTextureSets[i];
			short numTextures = *animatedPtr + 1;
			animatedPtr++;

			set.Textures = vector<RendererAnimatedTexture>(numTextures);
			set.NumTextures = numTextures;

			for (int j = 0; j < numTextures; j++)
			{
				short textureId = *animatedPtr;
				animatedPtr++;

				OBJECT_TEXTURE *texture = &g_Level.ObjectTextures[textureId];
				int tile = texture->tileAndFlag & 0x7FFF;
				set.Textures[j] = RendererAnimatedTexture();
				RendererAnimatedTexture &const newTexture = set.Textures[j];
				newTexture.Id = textureId;

				for (int k = 0; k < 4; k++)
				{
					float x = texture->vertices[k].x;
					float y = texture->vertices[k].y;

					newTexture.UV[k] = Vector2(x, y);
				}
			}
		}*/

		std::transform(g_Level.AnimatedTexturesSequences.begin(), g_Level.AnimatedTexturesSequences.end(), std::back_inserter(m_animatedTextureSets), [](ANIMATED_TEXTURES_SEQUENCE& sequence) {
			RendererAnimatedTextureSet set{};
			set.NumTextures = sequence.numFrames;
			std::transform(sequence.frames.begin(), sequence.frames.end(), std::back_inserter(set.Textures), [](ANIMATED_TEXTURES_FRAME& frm) {
				RendererAnimatedTexture tex{};
				tex.UV[0].x = frm.x1;
				tex.UV[0].y = frm.y1;
				tex.UV[1].x = frm.x2;
				tex.UV[1].y = frm.y2;
				tex.UV[2].x = frm.x3;
				tex.UV[2].y = frm.y3;
				tex.UV[3].x = frm.x4;
				tex.UV[3].y = frm.y4;
				return tex;
			});
			return set;
		});

		// Step 1: create the texture atlas
		/*byte* buffer = (byte*)malloc(TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);
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
			return false;*/

		//D3DX11SaveTextureToFileA(m_context, m_skyTexture->Texture, D3DX11_IFF_PNG, "H:\\sky.png");

		//free(buffer);

		// Upload textures to GPU memory
		m_roomTextures.resize(g_Level.RoomTextures.size());
		for (int i = 0; i < g_Level.RoomTextures.size(); i++)
		{
			TEXTURE *texture = &g_Level.RoomTextures[i];
			Texture2D normal;
			if (texture->normalMapData.size() < 1) {
				normal = createDefaultNormalTexture();
			} else {
				normal = Texture2D(m_device.Get(), texture->normalMapData.data(), texture->normalMapData.size());
			}
			TexturePair tex = std::make_tuple(Texture2D(m_device.Get(), texture->colorMapData.data(), texture->colorMapData.size()), normal);
			m_roomTextures[i] = tex;

#ifdef DUMP_TEXTURES
			char filename[255];ifdef DUMP_TEXTURES
			sprintf(filename, "dump\\room_%d.png", i);

			std::ofstream outfile(filename, std::ios::out | std::ios::binary);
			outfile.write(reinterpret_cast<const char*>(texture->colorMapData.data()), texture->colorMapData.size());
#endif
		}

		m_animatedTextures.resize(g_Level.AnimatedTextures.size());
		for (int i = 0; i < g_Level.AnimatedTextures.size(); i++)
		{
			TEXTURE *texture = &g_Level.AnimatedTextures[i];
			Texture2D normal;
			if (texture->normalMapData.size() < 1) {
				normal = createDefaultNormalTexture();
			}
			else {
				normal = Texture2D(m_device.Get(), texture->normalMapData.data(), texture->normalMapData.size());
			}
			TexturePair tex = std::make_tuple(Texture2D(m_device.Get(), texture->colorMapData.data(), texture->colorMapData.size()), normal);
			m_animatedTextures[i] = tex;
		}

		m_moveablesTextures.resize(g_Level.MoveablesTextures.size());
		for (int i = 0; i < g_Level.MoveablesTextures.size(); i++)
		{
			TEXTURE *texture = &g_Level.MoveablesTextures[i];
			Texture2D normal;
			if (texture->normalMapData.size() < 1) {
				normal = createDefaultNormalTexture();
			} else {
				normal = Texture2D(m_device.Get(), texture->normalMapData.data(), texture->normalMapData.size());
			}
			TexturePair tex = std::make_tuple(Texture2D(m_device.Get(), texture->colorMapData.data(), texture->colorMapData.size()), normal);
			m_moveablesTextures[i] = tex;

#ifdef DUMP_TEXTURES
			char filename[255];
			sprintf(filename, "dump\\moveable_%d.png", i);

			std::ofstream outfile(filename, std::ios::out | std::ios::binary);
			outfile.write(reinterpret_cast<const char*>(texture->colorMapData.data()), texture->colorMapData.size());
#endif
		}

		m_staticsTextures.resize(g_Level.StaticsTextures.size());
		for (int i = 0; i < g_Level.StaticsTextures.size(); i++)
		{
			TEXTURE *texture = &g_Level.StaticsTextures[i];
			Texture2D normal;
			if (texture->normalMapData.size() < 1) {
				normal = createDefaultNormalTexture();
			} else {
				normal = Texture2D(m_device.Get(), texture->normalMapData.data(), texture->normalMapData.size());
			}
			TexturePair tex = std::make_tuple(Texture2D(m_device.Get(), texture->colorMapData.data(), texture->colorMapData.size()), normal);
			m_staticsTextures[i] = tex;

#ifdef DUMP_TEXTURES
			char filename[255];
			sprintf(filename, "dump\\static_%d.png", i);

			std::ofstream outfile(filename, std::ios::out | std::ios::binary);
			outfile.write(reinterpret_cast<const char*>(texture->colorMapData.data()), texture->colorMapData.size());
#endif
		}

		m_spritesTextures.resize(g_Level.SpritesTextures.size());
		for (int i = 0; i < g_Level.SpritesTextures.size(); i++)
		{
			TEXTURE *texture = &g_Level.SpritesTextures[i];
			m_spritesTextures[i] = Texture2D(m_device.Get(), texture->colorMapData.data(), texture->colorMapData.size());
		}

		m_skyTexture = Texture2D(m_device.Get(), g_Level.SkyTexture.colorMapData.data(), g_Level.SkyTexture.colorMapData.size());

		// Step 2: prepare rooms
		vector<RendererVertex> roomVertices;
		vector<int> roomIndices;

		int totalRoomsVertices = 0;
		int totalRoomsIndices = 0;

		for (int i = 0; i < g_Level.Rooms.size(); i++)
		{
			ROOM_INFO* room = &g_Level.Rooms[i];

			RendererRoom* r = &m_rooms[i];
			r->roomNumber = i;
			r->ambientLight = Vector4(room->ambient.x, room->ambient.y, room->ambient.z, 1.0f);
			  
			if (room->positions.size() == 0)
				continue;

			int baseRoomVertex = 0;
			int baseRoomIndex = 0;
			
			for (int n = 0; n < room->buckets.size(); n++)
			{
				BUCKET* levelBucket = &room->buckets[n];
				RendererBucket bucket{};
				bucket.animated = levelBucket->animated;
				bucket.blendMode = static_cast<BLEND_MODES>(levelBucket->blendMode);
				bucket.texture = levelBucket->texture;
				bucket.Vertices.resize(levelBucket->numQuads * 4 + levelBucket->numTriangles * 3);
				bucket.Indices.resize(levelBucket->numQuads * 6 + levelBucket->numTriangles * 3);

				int lastVertex = 0;
				int lastIndex = 0;

				for (int p = 0; p < levelBucket->polygons.size(); p++)
				{
					POLYGON* poly = &levelBucket->polygons[p];
					RendererPolygon newPoly;

					newPoly.shape = poly->shape;
					newPoly.centre = (
						room->positions[poly->indices[0]] +
						room->positions[poly->indices[1]] +
						room->positions[poly->indices[2]]) / 3.0f;
					
					int baseVertices = lastVertex;

					for (int k = 0; k < poly->indices.size(); k++)
					{
						RendererVertex* vertex = &bucket.Vertices[lastVertex];
						int v = poly->indices[k];

						vertex->Position.x = room->x + room->positions[v].x;
						vertex->Position.y = room->y + room->positions[v].y;
						vertex->Position.z = room->z + room->positions[v].z;

						vertex->Normal = poly->normals[k];
						vertex->UV = poly->textureCoordinates[k];
						vertex->Color = Vector4(room->colors[v].x, room->colors[v].y, room->colors[v].z, 1.0f);
						vertex->Tangent = poly->tangents[k];
						vertex->BiTangent = poly->bitangents[k];
						vertex->IndexInPoly = k;
						vertex->OriginalIndex = v;
						vertex->Effects = room->effects[v];
						unsigned long long primes[]{ 73856093ULL ,19349663ULL ,83492791ULL };

						vertex->hash = std::hash<float>{}((vertex->Position.x)* primes[0]) ^ (std::hash<float>{}(vertex->Position.y)* primes[1]) ^ std::hash<float>{}(vertex->Position.z) * primes[2];
						vertex->Bone = 0;

						lastVertex++;
						totalRoomsVertices++;
					}

					if (poly->shape == 0)
					{
						newPoly.baseIndex = lastIndex;

						bucket.Indices[lastIndex + 0] = baseVertices + 0;
						bucket.Indices[lastIndex + 1] = baseVertices + 1;
						bucket.Indices[lastIndex + 2] = baseVertices + 3;
						bucket.Indices[lastIndex + 3] = baseVertices + 2;
						bucket.Indices[lastIndex + 4] = baseVertices + 3;
						bucket.Indices[lastIndex + 5] = baseVertices + 1;

						newPoly.baseIndex = lastIndex;

						lastIndex += 6;
						totalRoomsIndices += 6;
					}
					else
					{
						newPoly.baseIndex = lastIndex;
 
						bucket.Indices[lastIndex + 0] = baseVertices + 0;
						bucket.Indices[lastIndex + 1] = baseVertices + 1;
						bucket.Indices[lastIndex + 2] = baseVertices + 2;

						lastIndex += 3;
						totalRoomsIndices += 3;
					}

					bucket.Polygons.push_back(newPoly);

				}
				r->buckets.push_back(bucket);
				
			}

			if (room->lights.size() != 0)
			{
				r->lights.resize(room->lights.size());
				for (int l = 0; l < room->lights.size(); l++)
				{
					RendererLight* light = &r->lights[l];
					ROOM_LIGHT* oldLight = &room->lights[l];

					if (oldLight->type == LIGHT_TYPES::LIGHT_TYPE_SUN)
					{
						light->Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light->Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light->Type = LIGHT_TYPES::LIGHT_TYPE_SUN;
						light->Intensity = 1.0f;
					}
					else if (oldLight->type == LIGHT_TYPE_POINT)
					{
						light->Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
						light->Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light->Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light->Intensity = 1.0f;
						light->In = oldLight->in;
						light->Out = oldLight->out;
						light->Type = LIGHT_TYPE_POINT;
					}
					else if (oldLight->type == LIGHT_TYPE_SHADOW)
					{
						light->Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
						light->Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light->In = oldLight->in;
						light->Out = oldLight->out;
						light->Type = LIGHT_TYPE_SHADOW;
						light->Intensity = 1.0f;
					}
					else if (oldLight->type == LIGHT_TYPE_SPOT)
					{
						light->Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
						light->Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light->Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light->Intensity = 1.0f;
						light->In = oldLight->in;
						light->Out = oldLight->out;
						light->Range = oldLight->length;
						light->Type = LIGHT_TYPE_SPOT;
					}

					oldLight++;
				}
			}
		}

		roomVertices.resize(totalRoomsVertices);
		roomIndices.resize(totalRoomsIndices);

		int baseRoomVertex = 0;
		int baseRoomIndex = 0;

		for (int i = 0; i < g_Level.Rooms.size(); i++)
		{
			ROOM_INFO* room = &g_Level.Rooms[i];
			RendererRoom* r = &m_rooms[i];
			// Merge vertices and indices in a single list
			for (auto& bucket : r->buckets) {
				bucket.StartVertex = baseRoomVertex;
				bucket.StartIndex = baseRoomIndex;

				for (int k = 0; k < bucket.Vertices.size(); k++)
					roomVertices[baseRoomVertex + k] = bucket.Vertices[k];

				for (int k = 0; k < bucket.Indices.size(); k++)
					roomIndices[baseRoomIndex + k] = baseRoomVertex + bucket.Indices[k];

				baseRoomVertex += bucket.Vertices.size();
				baseRoomIndex += bucket.Indices.size();
			}
		}

		// Create a single vertex buffer and a single index buffer for all rooms
		// NOTICE: in theory, a 1,000,000 vertices scene should have a VB of 52 MB and an IB of 4 MB
		m_roomsVertexBuffer = VertexBuffer(m_device.Get(), roomVertices.size(), roomVertices.data());
		m_roomsIndexBuffer = IndexBuffer(m_device.Get(), roomIndices.size(), roomIndices.data());

		m_numHairVertices = 0;
		m_numHairIndices = 0;

		vector<RendererVertex> moveablesVertices;
		vector<int> moveablesIndices;
		int baseMoveablesVertex = 0;
		int baseMoveablesIndex = 0;

		bool skinPresent = false;
		bool hairsPresent = false;

		// Step 3: prepare moveables
		for (int i = 0; i < MoveablesIds.size(); i++)
		{
			int objNum = MoveablesIds[i];
			OBJECT_INFO *obj = &Objects[objNum];

			if (obj->nmeshes > 0)
			{
				m_moveableObjects[MoveablesIds[i]] = RendererObject();
				RendererObject &moveable = *m_moveableObjects[MoveablesIds[i]];
				moveable.Id = MoveablesIds[i];
				moveable.DoNotDraw = (obj->drawRoutine == NULL);

				for (int j = 0; j < obj->nmeshes; j++)
				{
					// HACK: mesh pointer 0 is the placeholder for Lara's body parts and is right hand with pistols
					// We need to override the bone index because the engine will take mesh 0 while drawing pistols anim,
					// and vertices have bone index 0 and not 10
					RendererMesh *mesh = getRendererMeshFromTrMesh(&moveable,
																   &g_Level.Meshes[obj->meshIndex + j],
																   j, MoveablesIds[i] == ID_LARA_SKIN_JOINTS,
																   MoveablesIds[i] == ID_LARA_HAIR);
					moveable.ObjectMeshes.push_back(mesh);
				}

				if (objNum == ID_IMP_ROCK || objNum == ID_ENERGY_BUBBLES || objNum == ID_BUBBLES || objNum == ID_BODY_PART)
				{
					// HACK: these objects must have nmeshes = 0 because engine will use them in a different way while drawing Effects.
					// In Core's code this was done in SETUP.C but we must do it here because we need to create renderer's meshes.
					obj->nmeshes = 0;
				}
				else
				{
					for (int j = 0; j < obj->nmeshes; j++)
					{
						moveable.LinearizedBones.push_back(new RendererBone(j));
						moveable.AnimationTransforms.push_back(Matrix::Identity);
						moveable.BindPoseTransforms.push_back(Matrix::Identity);
					}

					if (obj->nmeshes > 1)
					{
						int *bone = &g_Level.Bones[obj->boneIndex];

						stack<RendererBone *> stack;

						RendererBone *currentBone = moveable.LinearizedBones[0];
						RendererBone *stackBone = moveable.LinearizedBones[0];

						for (int mi = 0; mi < obj->nmeshes - 1; mi++)
						{
							int j = mi + 1;

							int opcode = *(bone++);
							int linkX = *(bone++);
							int linkY = *(bone++);
							int linkZ = *(bone++);

							byte flags = opcode & 0x1C;

							moveable.LinearizedBones[j]->ExtraRotationFlags = flags;

							switch (opcode & 0x03)
							{
							case 0:
								moveable.LinearizedBones[j]->Parent = currentBone;
								moveable.LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
								currentBone->Children.push_back(moveable.LinearizedBones[j]);
								currentBone = moveable.LinearizedBones[j];

								break;
							case 1:
								if (stack.empty())
									continue;
								currentBone = stack.top();
								stack.pop();

								moveable.LinearizedBones[j]->Parent = currentBone;
								moveable.LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
								currentBone->Children.push_back(moveable.LinearizedBones[j]);
								currentBone = moveable.LinearizedBones[j];

								break;
							case 2:
								stack.push(currentBone);

								moveable.LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
								moveable.LinearizedBones[j]->Parent = currentBone;
								currentBone->Children.push_back(moveable.LinearizedBones[j]);
								currentBone = moveable.LinearizedBones[j];

								break;
							case 3:
								if (stack.empty())
									continue;
								RendererBone *theBone = stack.top();
								stack.pop();

								moveable.LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
								moveable.LinearizedBones[j]->Parent = theBone;
								theBone->Children.push_back(moveable.LinearizedBones[j]);
								currentBone = moveable.LinearizedBones[j];
								stack.push(theBone);

								break;
							}
						}
					}

					for (int n = 0; n < obj->nmeshes; n++)
						moveable.LinearizedBones[n]->Transform = Matrix::CreateTranslation(
							moveable.LinearizedBones[n]->Translation.x,
							moveable.LinearizedBones[n]->Translation.y,
							moveable.LinearizedBones[n]->Translation.z);

					moveable.Skeleton = moveable.LinearizedBones[0];
					buildHierarchy(&moveable);

					// Fix Lara skin joints and hairs
					if (MoveablesIds[i] == ID_LARA_SKIN_JOINTS)
					{
						skinPresent = true;
						int BonesToCheck[2] = {0, 0};

						RendererObject &objSkin = *m_moveableObjects[ID_LARA_SKIN];

						for (int j = 1; j < obj->nmeshes; j++)
						{
							RendererMesh *jointMesh = moveable.ObjectMeshes[j];
							RendererBone *jointBone = moveable.LinearizedBones[j];

							BonesToCheck[0] = jointBone->Parent->Index;
							BonesToCheck[1] = j;

							for (int b1 = 0; b1 < jointMesh->buckets.size(); b1++)
							{
								RendererBucket *jointBucket = &jointMesh->buckets[b1];

								for (int v1 = 0; v1 < jointBucket->Vertices.size(); v1++)
								{
									RendererVertex *jointVertex = &jointBucket->Vertices[v1];

									bool done = false;

									for (int k = 0; k < 2; k++)
									{
										RendererMesh *skinMesh = objSkin.ObjectMeshes[BonesToCheck[k]];
										RendererBone *skinBone = objSkin.LinearizedBones[BonesToCheck[k]];

										for (int b2 = 0; b2 < skinMesh->buckets.size(); b2++)
										{
											RendererBucket *skinBucket = &skinMesh->buckets[b2];
											for (int v2 = 0; v2 < skinBucket->Vertices.size(); v2++)
											{
												RendererVertex *skinVertex = &skinBucket->Vertices[v2];

												int x1 = jointBucket->Vertices[v1].Position.x + jointBone->GlobalTranslation.x;
												int y1 = jointBucket->Vertices[v1].Position.y + jointBone->GlobalTranslation.y;
												int z1 = jointBucket->Vertices[v1].Position.z + jointBone->GlobalTranslation.z;

												int x2 = skinBucket->Vertices[v2].Position.x + skinBone->GlobalTranslation.x;
												int y2 = skinBucket->Vertices[v2].Position.y + skinBone->GlobalTranslation.y;
												int z2 = skinBucket->Vertices[v2].Position.z + skinBone->GlobalTranslation.z;


												if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2 && abs(z1 - z2) < 2)
												{
													jointVertex->Bone = BonesToCheck[k];
													jointVertex->Position = skinVertex->Position;
													jointVertex->Normal = skinVertex->Normal;

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
					else if (MoveablesIds[i] == ID_LARA_HAIR && skinPresent)
					{
						hairsPresent = true;

						for (int j = 0; j< obj->nmeshes;j++)
						{
							RendererMesh* currentMesh = moveable.ObjectMeshes[j];
							RendererBone* currentBone = moveable.LinearizedBones[j];

							for (int b1 = 0; b1 < currentMesh->buckets.size(); b1++)
							{
								RendererBucket* currentBucket = &currentMesh->buckets[b1];

								for (int v1 = 0; v1 < currentBucket->Vertices.size(); v1++)
								{
									RendererVertex* currentVertex = &currentBucket->Vertices[v1];
									currentVertex->Bone = j + 1;

									if (j == 0)
									{
										// Mesh 0 must be linked with head
										int parentVertices[] = { 37,39,40,38 };
										
										RendererObject& skinObj = *m_moveableObjects[ID_LARA_SKIN];
										RendererMesh* parentMesh = skinObj.ObjectMeshes[LM_HEAD];
										RendererBone* parentBone = skinObj.LinearizedBones[LM_HEAD];

										if (currentVertex->OriginalIndex < 4)
										{
											for (int b2 = 0; b2 < parentMesh->buckets.size(); b2++)
											{
												RendererBucket* parentBucket = &parentMesh->buckets[b2];
												for (int v2 = 0; v2 < parentBucket->Vertices.size(); v2++)
												{
													RendererVertex* parentVertex = &parentBucket->Vertices[v2];

													if (parentVertex->OriginalIndex == parentVertices[currentVertex->OriginalIndex])
													{
														currentVertex->Bone = 0;
														currentVertex->Position = parentVertex->Position;
														currentVertex->Normal = parentVertex->Normal;
													}
												}
											}
										}										
									}
									else
									{
										// Meshes > 0 must be linked with hair parent meshes
										RendererMesh* parentMesh = moveable.ObjectMeshes[j - 1];
										RendererBone* parentBone = moveable.LinearizedBones[j - 1];

										for (int b2 = 0; b2 < parentMesh->buckets.size(); b2++)
										{
											RendererBucket* parentBucket = &parentMesh->buckets[b2];
											for (int v2 = 0; v2 < parentBucket->Vertices.size(); v2++)
											{
												RendererVertex* parentVertex = &parentBucket->Vertices[v2];

												int x1 = currentBucket->Vertices[v1].Position.x + currentBone->GlobalTranslation.x;
												int y1 = currentBucket->Vertices[v1].Position.y + currentBone->GlobalTranslation.y;
												int z1 = currentBucket->Vertices[v1].Position.z + currentBone->GlobalTranslation.z;

												int x2 = parentBucket->Vertices[v2].Position.x + parentBone->GlobalTranslation.x;
												int y2 = parentBucket->Vertices[v2].Position.y + parentBone->GlobalTranslation.y;
												int z2 = parentBucket->Vertices[v2].Position.z + parentBone->GlobalTranslation.z;

												if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2 && abs(z1 - z2) < 2)
												{
													currentVertex->Bone = j;
													currentVertex->Position = parentVertex->Position;
													currentVertex->Normal = parentVertex->Normal;
													currentVertex->BiTangent = parentVertex->BiTangent;
													currentVertex->Tangent = parentVertex->Tangent;
													break;
												}
											}
										}
									}
								}
							}
						}
					}
				}

				// Merge vertices and indices in a single list
				for (int m = 0; m < moveable.ObjectMeshes.size(); m++)
				{
					RendererMesh *msh = moveable.ObjectMeshes[m];

					for (int j = 0; j < msh->buckets.size(); j++)
					{
						RendererBucket *bucket = &msh->buckets[j];

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
		m_moveablesVertexBuffer = VertexBuffer(m_device.Get(), moveablesVertices.size(), moveablesVertices.data());
		m_moveablesIndexBuffer = IndexBuffer(m_device.Get(), moveablesIndices.size(), moveablesIndices.data());

		// Step 4: prepare static meshes
		vector<RendererVertex> staticsVertices;
		vector<int> staticsIndices;
		int baseStaticsVertex = 0;
		int baseStaticsIndex = 0;

		for (int i = 0; i < StaticObjectsIds.size(); i++)
		{
			STATIC_INFO *obj = &StaticObjects[StaticObjectsIds[i]];
			m_staticObjects[StaticObjectsIds[i]] = RendererObject();
			RendererObject &staticObject = *m_staticObjects[StaticObjectsIds[i]];
			staticObject.Id = StaticObjectsIds[i];

			RendererMesh *mesh = getRendererMeshFromTrMesh(&staticObject, &g_Level.Meshes[obj->meshNumber], 0, false, false);

			staticObject.ObjectMeshes.push_back(mesh);

			m_staticObjects[StaticObjectsIds[i]] = staticObject;

			// Merge vertices and indices in a single list
			RendererMesh *msh = staticObject.ObjectMeshes[0];

			for (int j = 0; j < msh->buckets.size(); j++)
			{
				RendererBucket* bucket = &msh->buckets[j];

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
		m_staticsVertexBuffer = VertexBuffer(m_device.Get(), staticsVertices.size(), staticsVertices.data());
		m_staticsIndexBuffer = IndexBuffer(m_device.Get(), staticsIndices.size(), staticsIndices.data());

		// Step 5: prepare sprites
		m_sprites.resize(g_Level.Sprites.size());

		for (int i = 0; i < g_Level.Sprites.size(); i++)
		{
			SPRITE *oldSprite = &g_Level.Sprites[i];
			m_sprites[i] = RendererSprite();
			RendererSprite &sprite = m_sprites[i];

			sprite.UV[0] = Vector2(oldSprite->x1, oldSprite->y1);
			sprite.UV[1] = Vector2(oldSprite->x2, oldSprite->y2);
			sprite.UV[2] = Vector2(oldSprite->x3, oldSprite->y3);
			sprite.UV[3] = Vector2(oldSprite->x4, oldSprite->y4);
			sprite.Texture = &m_spritesTextures[oldSprite->tile];
		}

		for (int i = 0; i < MoveablesIds.size(); i++)
		{
			OBJECT_INFO *obj = &Objects[MoveablesIds[i]];

			if (obj->nmeshes < 0)
			{
				short numSprites = abs(obj->nmeshes);
				short baseSprite = obj->meshIndex;
				m_spriteSequences[MoveablesIds[i]] = RendererSpriteSequence(MoveablesIds[i], numSprites);
				RendererSpriteSequence &sequence = m_spriteSequences[MoveablesIds[i]];

				for (int j = baseSprite; j < baseSprite + numSprites; j++)
				{
					sequence.SpritesList[j - baseSprite] = &m_sprites[j];
				}

				m_spriteSequences[MoveablesIds[i]] = sequence;
			}
		}
		/*
		for (int i = 0; i < 6; i++)
		{
			if (Objects[ID_WATERFALL1 + i].loaded)
			{
				// Get the first textured bucket
				RendererBucket *bucket = NULL;
				for (int j = 0; j < NUM_BUCKETS; j++)
					if (m_moveableObjects[ID_WATERFALL1 + i]->ObjectMeshes[0]->buckets[j].Polygons.size() > 0)
						bucket = &m_moveableObjects[ID_WATERFALL1 + i]->ObjectMeshes[0]->buckets[j];

				if (bucket == NULL)
					continue;

				OBJECT_TEXTURE *texture = &g_Level.ObjectTextures[bucket->Polygons[0].TextureId];
				WaterfallTextures[i] = texture;
				WaterfallY[i] = texture->vertices[0].y;
			}
		}
		*/
		return true;
	}
} // namespace TEN::Renderer
