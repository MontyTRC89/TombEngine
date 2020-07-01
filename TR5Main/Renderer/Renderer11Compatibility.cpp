#include "framework.h"
#include "Renderer11.h"
#include "level.h"
#include "savegame.h"
#include "setup.h"
#include "control.h"
#include "objects.h"
using std::vector;
using std::stack;
using std::optional;
namespace T5M::Renderer {
	bool Renderer11::PrepareDataForTheRenderer() {
		m_moveableObjects.resize(ID_NUMBER_OBJECTS);
		m_spriteSequences.resize(ID_NUMBER_OBJECTS);
		m_staticObjects.resize(MAX_STATICS);
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
					float x = texture->vertices[k].x;
					float y = texture->vertices[k].y;

					newTexture.UV[k] = Vector2(x, y);
				}

			}
		}

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
		for (int i = 0; i < RoomTextures.size(); i++) {
			TEXTURE* texture = &RoomTextures[i];
			m_roomTextures.push_back(Texture2D(m_device, texture->data.data(), texture->size));
		}

		for (int i = 0; i < MoveablesTextures.size(); i++) {
			TEXTURE* texture = &MoveablesTextures[i];
			m_moveablesTextures.push_back(Texture2D(m_device, texture->data.data(), texture->size));
		}

		for (int i = 0; i < StaticsTextures.size(); i++) {
			TEXTURE* texture = &StaticsTextures[i];
			m_staticsTextures.push_back(Texture2D(m_device, texture->data.data(), texture->size));
		}

		for (int i = 0; i < SpritesTextures.size(); i++) {
			TEXTURE* texture = &SpritesTextures[i];
			m_spritesTextures.push_back(Texture2D(m_device, texture->data.data(), texture->size));
		}

		m_skyTexture = Texture2D(m_device, MiscTextures.data.data(), MiscTextures.size);


		// Step 2: prepare rooms
		vector<RendererVertex> roomVertices;
		vector<int> roomIndices;

		int baseRoomVertex = 0;
		int baseRoomIndex = 0;

		for (int i = 0; i < Rooms.size(); i++)
		{
			ROOM_INFO* room = &Rooms[i];

			m_rooms[i] = RendererRoom();
			RendererRoom& r = m_rooms[i];
			r.RoomNumber = i;
			r.Room = room;
			r.AmbientLight = Vector4(room->ambient.x, room->ambient.y, room->ambient.z, 1.0f);
			//r.LightsToDraw = vector<RendererLight*>(MAX_LIGHTS);
			r.Statics.resize(room->mesh.size());

			if (room->vertices.size() == 0)
				continue;

			ROOM_VERTEX * vertices = room->vertices.data();

			for (int n = 0; n < room->buckets.size(); n++)
			{
				BUCKET* levelBucket = &room->buckets[n];
				RendererBucket* bucket;
				int bucketIndex;

				if (levelBucket->blendMode != 0)
					bucketIndex = RENDERER_BUCKET_TRANSPARENT;
				else
					bucketIndex = RENDERER_BUCKET_SOLID;

				//if (!levelBucket->animated)
				//{
				bucket = &r.Buckets[bucketIndex];
				/*}
				else
				{
					bucket = &r.AnimatedBuckets[bucketIndex];
				}*/

				for (int v = 0; v < levelBucket->indices.size(); v++)
				{
					int index = levelBucket->indices[v];
					ROOM_VERTEX* levelVertex = &vertices[index];

					RendererVertex vertex;

					vertex.Position.x = room->x + levelVertex->position.x;
					vertex.Position.y = room->y + levelVertex->position.y;
					vertex.Position.z = room->z + levelVertex->position.z;

					vertex.Normal.x = levelVertex->normal.x;
					vertex.Normal.y = levelVertex->normal.y;
					vertex.Normal.z = levelVertex->normal.z;

					vertex.UV.x = levelVertex->textureCoordinates.x;
					vertex.UV.y = levelVertex->textureCoordinates.y;

					vertex.Color.x = levelVertex->color.x;
					vertex.Color.y = levelVertex->color.y;
					vertex.Color.z = levelVertex->color.z;
					vertex.Color.w = 1.0f;

					vertex.Bone = 0;

					bucket->Indices.push_back(bucket->NumVertices);
					bucket->NumVertices++;
					bucket->Vertices.push_back(vertex);
				}
			}

			if (room->lights.size() != 0)
			{
				for (int l = 0; l < room->lights.size(); l++)
				{
					RendererLight light;
					ROOM_LIGHT* oldLight = &room->lights[l];

					if (oldLight->type == LIGHT_TYPES::LIGHT_TYPE_SUN)
					{
						light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light.Type = LIGHT_TYPES::LIGHT_TYPE_SUN;
						light.Intensity = 1.0f;

						r.Lights.push_back(light);
					}
					else if (oldLight->type == LIGHT_TYPE_POINT)
					{
						light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
						light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light.Intensity = 1.0f;
						light.In = oldLight->in;
						light.Out = oldLight->out;
						light.Type = LIGHT_TYPE_POINT;

						r.Lights.push_back(light);
					}
					else if (oldLight->type == LIGHT_TYPE_SHADOW)
					{
						light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
						light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light.In = oldLight->in;
						light.Out = oldLight->out;
						light.Type = LIGHT_TYPE_SHADOW;
						light.Intensity = 1.0f;

						r.Lights.push_back(light);
					}
					else if (oldLight->type == LIGHT_TYPE_SPOT)
					{
						light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
						light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
						light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light.Intensity = 1.0f;
						light.In = oldLight->in;
						light.Out = oldLight->out;
						light.Range = oldLight->range;
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
		m_roomsVertexBuffer = VertexBuffer(m_device, roomVertices.size(), roomVertices.data());
		m_roomsIndexBuffer = IndexBuffer(m_device, roomIndices.size(), roomIndices.data());

		m_numHairVertices = 0;
		m_numHairIndices = 0;

		vector<RendererVertex> moveablesVertices;
		vector<int> moveablesIndices;
		int baseMoveablesVertex = 0;
		int baseMoveablesIndex = 0;

		// Step 3: prepare moveables
		for (int i = 0; i < MoveablesIds.size(); i++) {
			int objNum = MoveablesIds[i];
			ObjectInfo* obj = &Objects[objNum];

			if (obj->nmeshes > 0) {
				m_moveableObjects[MoveablesIds[i]] = RendererObject();
				RendererObject& moveable = *m_moveableObjects[MoveablesIds[i]];
				moveable.Id = MoveablesIds[i];

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

				moveable.DoNotDraw = (obj->drawRoutine == NULL);

				for (int j = 0; j < obj->nmeshes; j++) {
					// HACK: mesh pointer 0 is the placeholder for Lara's body parts and is right hand with pistols
					// We need to override the bone index because the engine will take mesh 0 while drawing pistols anim,
					// and vertices have bone index 0 and not 10
					RendererMesh* mesh = getRendererMeshFromTrMesh(&moveable,
																   Meshes[obj->meshIndex + j],
																   j, MoveablesIds[i] == ID_LARA_SKIN_JOINTS,
																   MoveablesIds[i] == ID_LARA_HAIR);
					moveable.ObjectMeshes.push_back(mesh);
				}

				if (objNum == ID_IMP_ROCK || objNum == ID_ENERGY_BUBBLES || objNum == ID_BUBBLES || objNum == ID_BODY_PART) {
					// HACK: these objects must have nmeshes = 0 because engine will use them in a different way while drawing Effects.
					// In Core's code this was done in SETUP.C but we must do it here because we need to create renderer's meshes.
					obj->nmeshes = 0;
				}
				else {
					int* bone = &Bones[obj->boneIndex];

					stack<RendererBone*> stack;

					for (int j = 0; j < obj->nmeshes; j++) {
						moveable.LinearizedBones.push_back(new RendererBone(j));
						moveable.AnimationTransforms.push_back(Matrix::Identity);
						moveable.BindPoseTransforms.push_back(Matrix::Identity);
					}

					RendererBone* currentBone = moveable.LinearizedBones[0];
					RendererBone* stackBone = moveable.LinearizedBones[0];

					for (int mi = 0; mi < obj->nmeshes - 1; mi++) {
						int j = mi + 1;

						int opcode = *(bone++);
						int linkX = *(bone++);
						int linkY = *(bone++);
						int linkZ = *(bone++);

						byte flags = opcode & 0x1C;

						moveable.LinearizedBones[j]->ExtraRotationFlags = flags;

						switch (opcode & 0x03) {
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
							RendererBone* theBone = stack.top();
							stack.pop();

							moveable.LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
							moveable.LinearizedBones[j]->Parent = theBone;
							theBone->Children.push_back(moveable.LinearizedBones[j]);
							currentBone = moveable.LinearizedBones[j];
							stack.push(theBone);

							break;
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
					if (MoveablesIds[i] == ID_LARA_SKIN_JOINTS) {
						int bonesToCheck[2] = { 0,0 };

						RendererObject& objSkin = *m_moveableObjects[ID_LARA_SKIN];

						for (int j = 1; j < obj->nmeshes; j++) {
							RendererMesh* jointMesh = moveable.ObjectMeshes[j];
							RendererBone* jointBone = moveable.LinearizedBones[j];

							bonesToCheck[0] = jointBone->Parent->Index;
							bonesToCheck[1] = j;

							for (int b1 = 0; b1 < NUM_BUCKETS; b1++) {
								RendererBucket* jointBucket = &jointMesh->Buckets[b1];

								for (int v1 = 0; v1 < jointBucket->Vertices.size(); v1++) {
									RendererVertex* jointVertex = &jointBucket->Vertices[v1];

									bool done = false;

									for (int k = 0; k < 2; k++) {
										RendererMesh* skinMesh = objSkin.ObjectMeshes[bonesToCheck[k]];
										RendererBone* skinBone = objSkin.LinearizedBones[bonesToCheck[k]];

										for (int b2 = 0; b2 < NUM_BUCKETS; b2++) {
											RendererBucket* skinBucket = &skinMesh->Buckets[b2];
											for (int v2 = 0; v2 < skinBucket->Vertices.size(); v2++) {
												RendererVertex* skinVertex = &skinBucket->Vertices[v2];

												int x1 = jointBucket->Vertices[v1].Position.x + jointBone->GlobalTranslation.x;
												int y1 = jointBucket->Vertices[v1].Position.y + jointBone->GlobalTranslation.y;
												int z1 = jointBucket->Vertices[v1].Position.z + jointBone->GlobalTranslation.z;

												int x2 = skinBucket->Vertices[v2].Position.x + skinBone->GlobalTranslation.x;
												int y2 = skinBucket->Vertices[v2].Position.y + skinBone->GlobalTranslation.y;
												int z2 = skinBucket->Vertices[v2].Position.z + skinBone->GlobalTranslation.z;

												if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2 && abs(z1 - z2) < 2) {
													jointVertex->Bone = bonesToCheck[k];
													jointVertex->Position.x = skinVertex->Position.x;
													jointVertex->Position.y = skinVertex->Position.y;
													jointVertex->Position.z = skinVertex->Position.z;
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

					if (MoveablesIds[i] == ID_LARA_HAIR) {
						for (int j = 0; j < moveable.ObjectMeshes.size(); j++) {
							RendererMesh* mesh = moveable.ObjectMeshes[j];
							for (int n = 0; n < NUM_BUCKETS; n++) {
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


				// Merge vertices and indices in a single list
				for (int m = 0; m < moveable.ObjectMeshes.size(); m++) {
					RendererMesh* msh = moveable.ObjectMeshes[m];

					for (int j = 0; j < NUM_BUCKETS; j++) {
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
		m_moveablesVertexBuffer = VertexBuffer(m_device, moveablesVertices.size(), moveablesVertices.data());
		m_moveablesIndexBuffer = IndexBuffer(m_device, moveablesIndices.size(), moveablesIndices.data());

		// Step 4: prepare static meshes
		vector<RendererVertex> staticsVertices;
		vector<int> staticsIndices;
		int baseStaticsVertex = 0;
		int baseStaticsIndex = 0;

		for (int i = 0; i < StaticObjectsIds.size(); i++) {
			StaticInfo* obj = &StaticObjects[StaticObjectsIds[i]];
			m_staticObjects[StaticObjectsIds[i]] = RendererObject();
			RendererObject& staticObject = *m_staticObjects[StaticObjectsIds[i]];
			staticObject.Id = StaticObjectsIds[i];

			short* meshPtr = Meshes[obj->meshNumber];
			RendererMesh* mesh = getRendererMeshFromTrMesh(&staticObject, Meshes[obj->meshNumber], 0, false, false);

			staticObject.ObjectMeshes.push_back(mesh);

			m_staticObjects[StaticObjectsIds[i]] = staticObject;

			// Merge vertices and indices in a single list
			RendererMesh* msh = staticObject.ObjectMeshes[0];

			for (int j = 0; j < NUM_BUCKETS; j++) {
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
		m_staticsVertexBuffer = VertexBuffer(m_device, staticsVertices.size(), staticsVertices.data());
		m_staticsIndexBuffer = IndexBuffer(m_device, staticsIndices.size(), staticsIndices.data());

		// Step 5: prepare sprites
		m_sprites.resize(g_NumSprites);

		for (int i = 0; i < g_NumSprites; i++) {
			SPRITE* oldSprite = &Sprites[i];
			m_sprites[i] = RendererSprite();
			RendererSprite& sprite = m_sprites[i];

			sprite.UV[0] = Vector2(oldSprite->x1, oldSprite->y1);
			sprite.UV[1] = Vector2(oldSprite->x2, oldSprite->y2);
			sprite.UV[2] = Vector2(oldSprite->x3, oldSprite->y3);
			sprite.UV[3] = Vector2(oldSprite->x4, oldSprite->y4);
			sprite.Texture = &m_spritesTextures[oldSprite->tile];

			
		}
		
		for (int i = 0; i < MoveablesIds.size(); i++) {
			ObjectInfo* obj = &Objects[MoveablesIds[i]];

			if (obj->nmeshes < 0) {
				short numSprites = abs(obj->nmeshes);
				short baseSprite = obj->meshIndex;
				m_spriteSequences[MoveablesIds[i]] = RendererSpriteSequence(MoveablesIds[i], numSprites);
				RendererSpriteSequence& sequence = m_spriteSequences[MoveablesIds[i]];

				for (int j = baseSprite; j < baseSprite + numSprites; j++) {
					sequence.SpritesList[j - baseSprite] = &m_sprites[j];
				}

				m_spriteSequences[MoveablesIds[i]] = sequence;
			}
		}

		for (int i = 0; i < 6; i++) {
			if (Objects[ID_WATERFALL1 + i].loaded) {
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
}
