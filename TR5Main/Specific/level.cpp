#include "framework.h"
#include "level.h"
#include "setup.h"
#include "draw.h"
#include "lot.h"
#include "Lara.h"
#include "savegame.h"
#include "spotcam.h"
#include "camera.h"
#include "control.h"
#include "pickup.h"
#include "door.h"
#include "box.h"
#include "sound.h"
#include "GameFlowScript.h"

using T5M::Renderer::g_Renderer;
using std::vector;
using std::string;

ChunkId* ChunkTriggersList = ChunkId::FromString("Tr5Triggers");
ChunkId* ChunkTrigger = ChunkId::FromString("Tr5Trigger");
ChunkId* ChunkLuaIds = ChunkId::FromString("Tr5LuaIds");
ChunkId* ChunkLuaId = ChunkId::FromString("Tr5LuaId");

FILE* LevelFilePtr;
uintptr_t hLoadLevel;
unsigned int ThreadId;
int IsLevelLoading;
bool g_FirstLevel = true;
vector<int> MoveablesIds;
vector<int> StaticObjectsIds;
char* LevelDataPtr;
ChunkReader* g_levelChunkIO;
LEVEL g_Level;

extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;

short ReadInt8()
{
	byte value = *(byte*)LevelDataPtr;
	LevelDataPtr += 1;
	return value;
}

short ReadInt16()
{
	short value = *(short*)LevelDataPtr;
	LevelDataPtr += 2;
	return value;
}

unsigned short ReadUInt16()
{
	unsigned short value = *(unsigned short*)LevelDataPtr;
	LevelDataPtr += 2;
	return value;
}

int ReadInt32()
{
	int value = *(int*)LevelDataPtr;
	LevelDataPtr += 4;
	return value;
}

float ReadFloat()
{
	float value = *(float*)LevelDataPtr;
	LevelDataPtr += 4;
	return value;
}

Vector2 ReadVector2()
{
	Vector2 value;
	value.x = ReadFloat();
	value.y = ReadFloat();
	return value;
}

Vector3 ReadVector3()
{
	Vector3 value;
	value.x = ReadFloat();
	value.y = ReadFloat();
	value.z = ReadFloat();
	return value;
}

void ReadBytes(void* dest, int count)
{
	memcpy(dest, LevelDataPtr, count);
	LevelDataPtr += count;
}

int LoadItems()
{
	g_Level.NumItems = ReadInt32();
	if (g_Level.NumItems == 0)
		return false;

	g_Level.Items.resize(NUM_ITEMS);

	InitialiseClosedDoors();
	InitialiseItemArray(NUM_ITEMS);

	if (g_Level.NumItems > 0)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			ITEM_INFO* item = &g_Level.Items[i];
			
			item->objectNumber = ReadInt16();
			item->roomNumber = ReadInt16();
			item->pos.xPos = ReadInt32();
			item->pos.yPos = ReadInt32();
			item->pos.zPos = ReadInt32();
			item->pos.yRot = ReadInt16();
			item->shade = ReadInt16();
			item->triggerFlags = ReadInt16();
			item->flags = ReadInt16();
		}

		for (int i = 0; i < g_Level.NumItems; i++)
			InitialiseItem(i);
	}

	for (int r = 0; r < g_Level.Rooms.size(); r++)
	{
		MESH_INFO* mesh = g_Level.Rooms[r].mesh.data();

		for (int m = 0; m < g_Level.Rooms[r].mesh.size(); m++)
		{
			FLOOR_INFO* floor = &g_Level.Rooms[r].floor[((mesh->z - g_Level.Rooms[r].z) >> 10) + g_Level.Rooms[r].xSize * ((mesh->x - g_Level.Rooms[r].x) >> 10)];
			 
			if (!(g_Level.Boxes[floor->box].flags & BLOCKED)
				&& !(CurrentLevel == 5 && (r == 19 || r == 23 || r == 16)))
			{
				int fl = floor->floor << 2;
				StaticInfo* st = &StaticObjects[mesh->staticNumber];
				if (fl <= mesh->y - st->yMaxc + 512 && fl < mesh->y - st->yMinc)
				{
					if (st->xMaxc == 0 || st->xMinc == 0 ||
						st->zMaxc == 0 || st->zMinc == 0 ||
						(st->xMaxc ^ st->xMinc) & 0x8000 &&
						(st->xMaxc ^ st->zMinc) & 0x8000)
					{
						floor->box |= 8; 
					}
				}
			}
		}
	}

	return true;
}

void LoadObjects()
{
	memset(Objects, 0, sizeof(OBJECT_INFO) * ID_NUMBER_OBJECTS);
	memset(StaticObjects, 0, sizeof(StaticInfo) * MAX_STATICS);

	int numMeshes = ReadInt32();
	g_Level.Meshes.reserve(numMeshes);
	for (int i = 0; i < numMeshes; i++)
	{
		MESH mesh;

		mesh.sphere.Center.x = ReadFloat();
		mesh.sphere.Center.y = ReadFloat();
		mesh.sphere.Center.z = ReadFloat();
		mesh.sphere.Radius = ReadFloat();

		int numVertices = ReadInt32();

		mesh.positions.resize(numVertices);
		ReadBytes(mesh.positions.data(), 12 * numVertices);

		mesh.colors.resize(numVertices);
		ReadBytes(mesh.colors.data(), 12 * numVertices);

		mesh.bones.resize(numVertices);
		ReadBytes(mesh.bones.data(), 4 * numVertices);
		
		int numBuckets = ReadInt32();
		mesh.buckets.reserve(numBuckets);
		for (int j = 0; j < numBuckets; j++)
		{
			BUCKET bucket;

			bucket.texture = ReadInt32();
			bucket.blendMode = ReadInt8();
			bucket.animated = ReadInt8();
			bucket.numQuads = 0;
			bucket.numTriangles = 0;

			int numPolygons = ReadInt32();
			bucket.polygons.reserve(numPolygons);
			for (int k = 0; k < numPolygons; k++)
			{
				POLYGON poly;

				poly.shape = ReadInt32();
				int count = (poly.shape == 0 ? 4 : 3);
				poly.indices.resize(count);
				poly.textureCoordinates.resize(count);
				poly.normals.resize(count);
				poly.tangents.resize(count);
				poly.bitangents.resize(count);
				
				for (int n = 0; n < count; n++)
					poly.indices[n] = ReadInt32();
				for (int n = 0; n < count; n++)
					poly.textureCoordinates[n] = ReadVector2();
				for (int n = 0; n < count; n++)
					poly.normals[n] = ReadVector3();
				for (int n = 0; n < count; n++)
					poly.tangents[n] = ReadVector3();
				for (int n = 0; n < count; n++)
					poly.bitangents[n] = ReadVector3();

				bucket.polygons.push_back(poly);

				if (poly.shape == 0)
					bucket.numQuads++;
				else
					bucket.numTriangles++;
			}

			mesh.buckets.push_back(bucket);
		}

		g_Level.Meshes.push_back(mesh);
	}

	int numAnimations = ReadInt32();
	g_Level.Anims.resize(numAnimations);
	ReadBytes(g_Level.Anims.data(), sizeof(ANIM_STRUCT) * numAnimations);

	int numChanges = ReadInt32();
	g_Level.Changes.resize(numChanges);
	ReadBytes(g_Level.Changes.data(), sizeof(CHANGE_STRUCT) * numChanges);

	int numRanges = ReadInt32();
	g_Level.Ranges.resize(numRanges);
	ReadBytes(g_Level.Ranges.data(), sizeof(RANGE_STRUCT) * numRanges);

	int numCommands = ReadInt32();
	g_Level.Commands.resize(numCommands);
	ReadBytes(g_Level.Commands.data(), sizeof(short) * numCommands);

	int numBones = ReadInt32();
	g_Level.Bones.resize(numBones);
	ReadBytes(g_Level.Bones.data(), 4 * numBones);

	int numFrames = ReadInt32();
	g_Level.Frames.resize(numFrames);
	ReadBytes(g_Level.Frames.data(), sizeof(short) * numFrames);

	for (int i = 0; i < g_Level.Anims.size(); i++)
		AddPtr(g_Level.Anims[i].framePtr, short, g_Level.Frames.data());

	int numModels = ReadInt32();
	for (int i = 0; i < numModels; i++)
	{
		int objNum = ReadInt32();
		MoveablesIds.push_back(objNum);

		Objects[objNum].loaded = true;
		Objects[objNum].nmeshes = (short)ReadInt16();
		Objects[objNum].meshIndex = (short)ReadInt16();
		Objects[objNum].boneIndex = ReadInt32();
		Objects[objNum].frameBase = (short*)(ReadInt32() + (int)g_Level.Frames.data());
		Objects[objNum].animIndex = (short)ReadInt16();

		ReadInt16();

		Objects[objNum].loaded = true;
	}

	InitialiseObjects();
	InitialiseClosedDoors();

	int numStatics = ReadInt32();
	for (int i = 0; i < numStatics; i++)
	{
		int meshID = ReadInt32();
		StaticObjectsIds.push_back(meshID);

		StaticObjects[meshID].meshNumber = (short)ReadInt16();

		StaticObjects[meshID].yMinp = ReadInt16();
		StaticObjects[meshID].xMaxp = ReadInt16();
		StaticObjects[meshID].yMinp = ReadInt16();
		StaticObjects[meshID].yMaxp = ReadInt16();
		StaticObjects[meshID].zMinp = ReadInt16();
		StaticObjects[meshID].zMaxp = ReadInt16();

		StaticObjects[meshID].xMinc = ReadInt16();
		StaticObjects[meshID].xMaxc = ReadInt16();
		StaticObjects[meshID].yMinc = ReadInt16();
		StaticObjects[meshID].yMaxc = ReadInt16();
		StaticObjects[meshID].zMinc = ReadInt16();
		StaticObjects[meshID].zMaxc = ReadInt16();

		StaticObjects[meshID].flags = (short)ReadInt16();
	}

	// HACK: to remove after decompiling LoadSprites
	MoveablesIds.push_back(ID_DEFAULT_SPRITES);
}

void LoadCameras()
{
	int numCameras = ReadInt32();
	FixedCameras.resize(numCameras);
	ReadBytes(FixedCameras.data(), numCameras * sizeof(OBJECT_VECTOR));

	NumberSpotcams = ReadInt32();

	if (NumberSpotcams != 0)
	{
		ReadBytes(SpotCam, NumberSpotcams * sizeof(SPOTCAM));
	}
}

void LoadTextures()
{
	printf("LoadTextures\n");

	int size;

	int numTextures;
	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	g_Level.RoomTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);

		ReadFileEx(&size, 1, 4, LevelFilePtr);
		texture.colorMapData.resize(size);
		ReadFileEx(texture.colorMapData.data(), 1, size, LevelFilePtr);
		
		bool hasNormalMap;
		ReadFileEx(&hasNormalMap, 1, 1, LevelFilePtr);
		if (hasNormalMap)
		{
			ReadFileEx(&size, 1, 4, LevelFilePtr);
			texture.normalMapData.resize(size);
			ReadFileEx(texture.normalMapData.data(), 1, size, LevelFilePtr);
		}

		g_Level.RoomTextures.push_back(texture);
	}

	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	g_Level.MoveablesTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);

		ReadFileEx(&size, 1, 4, LevelFilePtr);
		texture.colorMapData.resize(size);
		ReadFileEx(texture.colorMapData.data(), 1, size, LevelFilePtr);

		bool hasNormalMap;
		ReadFileEx(&hasNormalMap, 1, 1, LevelFilePtr);
		if (hasNormalMap)
		{
			ReadFileEx(&size, 1, 4, LevelFilePtr);
			texture.normalMapData.resize(size);
			ReadFileEx(texture.normalMapData.data(), 1, size, LevelFilePtr);
		}

		g_Level.MoveablesTextures.push_back(texture);
	}

	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	g_Level.StaticsTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);
		
		ReadFileEx(&size, 1, 4, LevelFilePtr);
		texture.colorMapData.resize(size);
		ReadFileEx(texture.colorMapData.data(), 1, size, LevelFilePtr);

		bool hasNormalMap;
		ReadFileEx(&hasNormalMap, 1, 1, LevelFilePtr);
		if (hasNormalMap)
		{
			ReadFileEx(&size, 1, 4, LevelFilePtr);
			texture.normalMapData.resize(size);
			ReadFileEx(texture.normalMapData.data(), 1, size, LevelFilePtr);
		}

		g_Level.StaticsTextures.push_back(texture);
	}

	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	g_Level.SpritesTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);
		
		ReadFileEx(&size, 1, 4, LevelFilePtr);
		texture.colorMapData.resize(size);
		ReadFileEx(texture.colorMapData.data(), 1, size, LevelFilePtr);

		g_Level.SpritesTextures.push_back(texture);
	}

	ReadFileEx(&g_Level.MiscTextures.width, 1, 4, LevelFilePtr);
	ReadFileEx(&g_Level.MiscTextures.height, 1, 4, LevelFilePtr);
	ReadFileEx(&size, 1, 4, LevelFilePtr);
	g_Level.MiscTextures.colorMapData.resize(size);
	ReadFileEx(g_Level.MiscTextures.colorMapData.data(), 1, size, LevelFilePtr);
}

void ReadRooms()
{
	ReadInt32();

	int numRooms = ReadInt32();
	printf("NumRooms: %d\n", numRooms);
	
	for (int i = 0; i < numRooms; i++)
	{
		ROOM_INFO room;

		room.x = ReadInt32();
		room.y = 0;
		room.z = ReadInt32();
		room.minfloor = ReadInt32();
		room.maxceiling = ReadInt32();

		int numVertices = ReadInt32();
		room.positions.reserve(numVertices);
		for (int j = 0; j < numVertices; j++)
			room.positions.push_back(ReadVector3());
		room.colors.reserve(numVertices);
		for (int j = 0; j < numVertices; j++)
			room.colors.push_back(ReadVector3());

		int numBuckets = ReadInt32();
		room.buckets.reserve(numBuckets);
		for (int j = 0; j < numBuckets; j++)
		{
			BUCKET bucket;

			bucket.texture = ReadInt32();
			bucket.blendMode = ReadInt8();
			bucket.animated = ReadInt8();
			bucket.numQuads = 0;
			bucket.numTriangles = 0;

			int numPolygons = ReadInt32();
			bucket.polygons.reserve(numPolygons);
			for (int k = 0; k < numPolygons; k++)
			{
				POLYGON poly;

				poly.shape = ReadInt32();
				int count = (poly.shape == 0 ? 4 : 3);
				poly.indices.resize(count);
				poly.textureCoordinates.resize(count);
				poly.normals.resize(count);
				poly.tangents.resize(count);
				poly.bitangents.resize(count);

				for (int n = 0; n < count; n++)
					poly.indices[n] = ReadInt32();
				for (int n = 0; n < count; n++)
					poly.textureCoordinates[n] = ReadVector2();
				for (int n = 0; n < count; n++)
					poly.normals[n] = ReadVector3();
				for (int n = 0; n < count; n++)
					poly.tangents[n] = ReadVector3();
				for (int n = 0; n < count; n++)
					poly.bitangents[n] = ReadVector3();

				bucket.polygons.push_back(poly);

				if (poly.shape == 0)
					bucket.numQuads++;
				else
					bucket.numTriangles++;
			}

			room.buckets.push_back(bucket);
		}

		int numPortals = ReadInt32();
		for (int j = 0; j < numPortals; j++)
		{
			ROOM_DOOR door;

			door.room = ReadInt16();
			door.normal.x = ReadInt16();
			door.normal.y = ReadInt16();
			door.normal.z = ReadInt16();
			for (int k = 0; k < 4; k++)
			{
				door.vertices[k].x = ReadInt16();
				door.vertices[k].y = ReadInt16();
				door.vertices[k].z = ReadInt16();
			}

			room.doors.push_back(door);
		}

		room.xSize = ReadInt32();
		room.ySize = ReadInt32();
		room.floor.reserve(room.xSize * room.ySize);
		for (int j = 0; j < room.xSize * room.ySize; j++)
		{
			FLOOR_INFO floor;

			floor.index = ReadInt32();
			floor.box = ReadInt32();
			floor.fx = ReadInt32();
			floor.stopper = ReadInt32();
			floor.pitRoom = ReadInt32();
			floor.floor = ReadInt32();
			floor.skyRoom = ReadInt32();
			floor.ceiling = ReadInt32();
			floor.floorCollision.split = ReadInt32();
			floor.floorCollision.noCollision = ReadInt32();
			floor.floorCollision.planes[0].a = ReadFloat();
			floor.floorCollision.planes[0].b = ReadFloat();
			floor.floorCollision.planes[0].c = ReadFloat();
			floor.floorCollision.planes[1].a = ReadFloat();
			floor.floorCollision.planes[1].b = ReadFloat();
			floor.floorCollision.planes[1].c = ReadFloat();
			floor.ceilingCollision.split = ReadInt32();
			floor.ceilingCollision.noCollision = ReadInt32();
			floor.ceilingCollision.planes[0].a = ReadFloat();
			floor.ceilingCollision.planes[0].b = ReadFloat();
			floor.ceilingCollision.planes[0].c = ReadFloat();
			floor.ceilingCollision.planes[1].a = ReadFloat();
			floor.ceilingCollision.planes[1].b = ReadFloat();
			floor.ceilingCollision.planes[1].c = ReadFloat();

			room.floor.push_back(floor);
		}

		room.ambient.x = ReadFloat();
		room.ambient.y = ReadFloat();
		room.ambient.z = ReadFloat();

		int numLights = ReadInt32();
		room.lights.reserve(numLights);
		for (int j = 0; j < numLights; j++)
		{
			ROOM_LIGHT light;

			light.x = ReadFloat();
			light.y = ReadFloat();
			light.z = ReadFloat();
			light.r = ReadFloat();
			light.g = ReadFloat();
			light.b = ReadFloat();
			light.in = ReadFloat();
			light.out = ReadFloat();
			light.radIn = ReadFloat();
			light.radOut = ReadFloat();
			light.range = ReadFloat();
			light.dx = ReadFloat();
			light.dy = ReadFloat();
			light.dz = ReadFloat();
			light.type = ReadInt8();

			room.lights.push_back(light);
		}
		
		int numStatics = ReadInt32();
		room.mesh.reserve(numStatics);
		for (int j = 0; j < numStatics; j++)
		{
			MESH_INFO mesh;

			mesh.x = ReadInt32();
			mesh.y = ReadInt32();
			mesh.z = ReadInt32();
			mesh.yRot = ReadUInt16();
			mesh.shade = ReadUInt16();
			mesh.flags = ReadUInt16();
			mesh.staticNumber = ReadUInt16();

			room.mesh.push_back(mesh);
		}

		room.flippedRoom = ReadInt32();
		room.flags = ReadInt32();
		room.meshEffect = ReadInt32();
		room.reverbType = ReadInt32();
		room.flipNumber = ReadInt32();

		room.itemNumber = NO_ITEM;
		room.fxNumber = NO_ITEM;

		g_Level.Rooms.push_back(room);
	}
}

void BuildOutsideRoomsTable() 
{
	/*long max_slots = 0;
	AllocT(OutsideRoomOffsets, short, 27 * 27);
	AllocT(OutsideRoomTable, char, 27 * 27 * OUTSIDE_Z);
	memset(OutsideRoomTable, -1, 27 * 27 * OUTSIDE_Z);

	char flipped[256];
	memset(flipped, 0, 255);

	for (int i = 0; i < number_rooms; i++)
	{
		if (room[i].flipped_room != -1)
			flipped[i] = true;
	}

	for (int y = 0; y < 108; y += 4)
	{
		for (int x = 0; x < 108; x += 4)
		{
			for (int i = 0; i < number_rooms; i++)
			{
				const auto r = &room[i];

				if (!flipped[i])
				{
					const int rx = (r->z >> SECTOR(1)) + 1;
					const int ry = (r->x >> SECTOR(1)) + 1;

					int j = 0;

					for (int yl = 0; yl < 4; yl++)
					{
						for (int xl = 0; xl < 4; xl++)
						{
							if ((x + xl) >= rx && (x + xl) < (rx + r->x_size - 2) &&
								(y + yl) >= ry && (y + yl) < (ry + r->y_size - 2))
							{
								j = 1;
								break;
							}
						}
					}

					if (!j)
						continue;

					if (i == 255)
					{
						S_Warn("ERROR : Room 255 fuckeroony - go tell Chris\n");
					}

					char* d = &OutsideRoomTable[OUTSIDE_Z * (x >> 2) + OUTSIDE_Z * (y >> 2) * 27];

					for (int j = 0; j < OUTSIDE_Z; j++)
					{
						if (d[j] == -1)
						{
							d[j] = i;

							if (j > max_slots)
								max_slots = j;

							break;
						}
					}

					if (j == OUTSIDE_Z)
					{
						S_Warn("ERROR : Buffer shittage - go tell Chris\n");
					}
				}
			}
		}
	}
	// todo it's a bit incorrect
	char* s = OutsideRoomTable;

	for (int y = 0; y < 27; y++)
	{
		for (int x = 0; x < 27; x++)
		{
			int z = 0;

			char* d = &OutsideRoomTable[OUTSIDE_Z * x + OUTSIDE_Z * y * 27];

			const int i = 27 * y + x;

			while (d[z] != -1)
				z++;

			if (z == 0)
			{
				OutsideRoomOffsets[i] = -1;
			}
			else if (z == 1)
			{
				OutsideRoomOffsets[i] = *d | 0x8000;
			}
			else
			{
				char* p = OutsideRoomTable;

				while (p < s)
				{
					if (memcmp(p, d, z) == 0)
					{
						OutsideRoomOffsets[i] = p - OutsideRoomTable;
						break;
					}
					else
					{
						int z2 = 0;

						while (p[z2] != -1)
							z2++;

						p += z2 + 1;
					}
				}

				if (p >= s)
				{
					OutsideRoomOffsets[i] = s - OutsideRoomTable;

					while (z-- > 0)
						* s++ = *d++;

					*s++ = -1;
				}
			}
		}
	}*/
}
 
void LoadRooms()
{
	printf("LoadRooms\n");
	
	Wibble = 0;
	//RoomLightsCount = 0;
	//Unk_007E7FE8 = 0;

	ReadRooms();
	BuildOutsideRoomsTable();

	int numFloorData = ReadInt32(); 
	g_Level.FloorData.resize(numFloorData);
	ReadBytes(g_Level.FloorData.data(), numFloorData * sizeof(short));
}

void FreeLevel()
{
	malloc_ptr = malloc_buffer;
	malloc_free = malloc_size;
	g_Level.RoomTextures.clear();
	g_Level.MoveablesTextures.clear();
	g_Level.StaticsTextures.clear();
	g_Level.SpritesTextures.clear();
	g_Level.Rooms.clear(); 
	g_Level.ObjectTextures.clear();
	g_Level.Bones.clear();
	g_Level.Meshes.clear();
	MoveablesIds.clear();
	g_Level.Boxes.clear();
	g_Level.Overlaps.clear();
	g_Level.Anims.clear();
	g_Level.Changes.clear();
	g_Level.Ranges.clear();
	g_Level.Commands.clear();
	g_Level.Frames.clear();
	g_Level.Sprites.clear();
	g_Level.SoundDetails.clear();
	g_Level.SoundMap.clear();
	g_Level.FloorData.clear();
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			g_Level.Zones[j][i].clear();
		}
	}
	g_Renderer.FreeRendererData();
	g_GameScript->FreeLevelScripts();
}

size_t ReadFileEx(void* ptr, size_t size, size_t count, FILE* stream)
{
	_lock_file(stream);
	size_t result = fread(ptr, size, count, stream);
	_unlock_file(stream);
	return result;
}

void LoadSoundEffects()
{
	NumberSoundSources = ReadInt32();
	if (NumberSoundSources)
	{
		SoundSources = game_malloc<OBJECT_VECTOR>(NumberSoundSources);
		ReadBytes(SoundSources, NumberSoundSources * sizeof(OBJECT_VECTOR));
	}
}

void LoadAnimatedTextures()
{
	NumAnimatedTextures = ReadInt32();
	
	AnimTextureRanges = game_malloc<short>(NumAnimatedTextures);
	ReadBytes(AnimTextureRanges, NumAnimatedTextures * sizeof(short));
	
	nAnimUVRanges = ReadInt8();
}

void LoadTextureInfos()
{
	ReadInt32(); // TEX/0

	int numObjectTextures = ReadInt32();
	for (int i = 0; i < numObjectTextures; i++)
	{
		OBJECT_TEXTURE texture;
		texture.attribute = ReadInt32();
		texture.tileAndFlag = ReadInt32();
		texture.newFlags = ReadInt32();
		for (int j = 0; j < 4; j++)
		{
			texture.vertices[j].x = ReadFloat();
			texture.vertices[j].y = ReadFloat();
		}
		texture.destination = ReadInt32();
		g_Level.ObjectTextures.push_back(texture);
	}
}

void LoadAIObjects()
{
	int nAIObjects = ReadInt32();
	g_Level.AIObjects.resize(nAIObjects);
	ReadBytes(g_Level.AIObjects.data(), nAIObjects * sizeof(AIOBJECT));
}

FILE* FileOpen(const char* fileName)
{
	FILE* ptr = fopen(fileName, "rb");
	return ptr;
}

void FileClose(FILE* ptr)
{
	fclose(ptr);
}

void Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize)
{
	int z_result = uncompress(

		dest,       // destination for the uncompressed
								// data.  This should be the size of
								// the original data, which you should
								// already know.

		&uncompressedSize,  // length of destination (uncompressed)
								// buffer

		src,   // source buffer - the compressed data

		compressedSize);   // length of compressed data in bytes

	switch (z_result)
	{
	case Z_OK:
		printf("***** SUCCESS! *****\n");
		break;

	case Z_MEM_ERROR:
		printf("out of memory\n");
		exit(1);    // quit.
		break;

	case Z_BUF_ERROR:
		printf("output buffer wasn't large enough!\n");
		exit(1);    // quit.
		break;
	}
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

unsigned CALLBACK LoadLevel(void* data)
{
	printf("LoadLevel\n");

	char* filename = (char*)data;

	LevelDataPtr = NULL;
	LevelFilePtr = NULL;

	g_Renderer.UpdateProgress(0);

	LevelFilePtr = FileOpen(filename);
	if (LevelFilePtr)
	{
		int version;
		short numRoomTextureTiles;
		short numObjectsTextureTiles;
		short numBumpTextureTiles;

		ReadFileEx(&version, 1, 4, LevelFilePtr);
		
		LoadTextures();

		g_Renderer.UpdateProgress(20);

		short buffer[32];
		ReadFileEx(&buffer, 2, 16, LevelFilePtr);
		WeatherType = buffer[0];
		LaraDrawType = buffer[1];

		int uncompressedSize;
		int compressedSize;

		ReadFileEx(&uncompressedSize, 1, 4, LevelFilePtr);
		ReadFileEx(&compressedSize, 1, 4, LevelFilePtr);

		LevelDataPtr = (char*)malloc(uncompressedSize);
		ReadFileEx(LevelDataPtr, uncompressedSize, 1, LevelFilePtr);

		LoadRooms();
		g_Renderer.UpdateProgress(40);

		LoadObjects();
		g_Renderer.UpdateProgress(50);

		LoadSprites();
		LoadCameras();
		LoadSoundEffects();
		g_Renderer.UpdateProgress(60);

		LoadBoxes();

		InitialiseLOTarray(true);

		LoadAnimatedTextures();
		LoadTextureInfos();
		g_Renderer.UpdateProgress(70);

		LoadItems();
		LoadAIObjects();
		//LoadDemoData();
		LoadSamples();
		g_Renderer.UpdateProgress(80);

		int extraSize = 0;
		ReadFileEx(&extraSize, 1, 4, LevelFilePtr);
		if (extraSize > 0)
		{
			ReadFileEx(&extraSize, 1, 4, LevelFilePtr);
			LevelDataPtr = (char*)malloc(extraSize);
			ReadFileEx(LevelDataPtr, extraSize, 1, LevelFilePtr);

			LoadNewData(extraSize);
		}

		LevelDataPtr = NULL;
		FileClose(LevelFilePtr);
	}
	else
	{
		return false;
	}
	/*
	// Load also the new file format
	string t5mFileName = string(filename);
	std::transform(t5mFileName.begin(), t5mFileName.end(), t5mFileName.begin(),
		[](unsigned char c) { return std::tolower(c); });
	replace(t5mFileName, ".trc", ".t5m");
	LevelLoader* loader = new LevelLoader(t5mFileName);
	loader->Load();
	delete loader;
	*/
	g_Renderer.UpdateProgress(90);
	g_Renderer.PrepareDataForTheRenderer();
	
	// Initialise the game
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	SeedRandomDraw(0xD371F947);
	SeedRandomControl(0xD371F947);
	Wibble = 0;
	TorchRoom = -1;
	InitialiseGameFlags();
	InitialiseLara(!(InitialiseGame || CurrentLevel == 1 || level->ResetHub));
	GetCarriedItems();
	GetAIPickups();
	SeedRandomDraw(0xD371F947);
	SeedRandomControl(0xD371F947);
	Lara.Vehicle = -1;
	g_GameScript->AssignItemsAndLara();

	// Level loaded
	IsLevelLoading = false;
	g_Renderer.UpdateProgress(100);

	_endthreadex(1);

	return true;
}

void LoadSamples()
{
	int SoundMapSize = ReadInt16();
	g_Level.SoundMap.resize(SoundMapSize);
	ReadBytes(g_Level.SoundMap.data(), SoundMapSize * sizeof(short));

	int numSamplesInfos = ReadInt32();
	if (numSamplesInfos)
	{
		g_Level.SoundDetails.resize(numSamplesInfos);
		ReadBytes(g_Level.SoundDetails.data(), numSamplesInfos * sizeof(SAMPLE_INFO));

		int numSampleIndices = ReadInt32();
		if (numSampleIndices)
		{
			int numSamples = 0;
			ReadFileEx(&numSamples, 1, 4, LevelFilePtr);
			//if (feof(LevelFilePtr))
			//	return;

			if (numSamples <= 0)
				return;

			int uncompressedSize;
			int compressedSize;
			char* buffer = (char*)malloc(1048576);

			for (int i = 0; i < numSamples; i++)
			{
				ReadFileEx(&uncompressedSize, 4, 1, LevelFilePtr);
				ReadFileEx(&compressedSize, 4, 1, LevelFilePtr);
				ReadFileEx(buffer, 1, compressedSize, LevelFilePtr);
				Sound_LoadSample(buffer, compressedSize, uncompressedSize, i);
			}

			free(buffer);
		}
	}
	else
	{
		//Log(1, aNog_Level.SoundDetailss);
	}
}

void LoadBoxes()
{
	// Read boxes
	int numBoxes = ReadInt32();
	g_Level.Boxes.resize(numBoxes);
	ReadBytes(g_Level.Boxes.data(), numBoxes * sizeof(BOX_INFO));

	// Read overlaps
	int numOverlaps = ReadInt32();
	g_Level.Overlaps.resize(numOverlaps);
	ReadBytes(g_Level.Overlaps.data(), numOverlaps * sizeof(OVERLAP));

	// Read zones
	for (int i = 0; i < 2; i++)
	{
		// Ground zones
		for (int j = 0; j < 4; j++)
		{
			g_Level.Zones[j][i].resize(numBoxes * sizeof(int));
			ReadBytes(g_Level.Zones[j][i].data(), numBoxes * sizeof(int));
		}

		// Fly zone
		g_Level.Zones[4][i].resize(numBoxes * sizeof(int));
		ReadBytes(g_Level.Zones[4][i].data(), numBoxes * sizeof(int));
	}

	// By default all blockable boxes are blocked
	for (int i = 0; i < numBoxes; i++)
	{
		if (g_Level.Boxes[i].flags & BLOCKABLE)
		{
			g_Level.Boxes[i].flags |= BLOCKED;
		}
	}
}

int S_LoadLevelFile(int levelIndex)
{
	//DB_Log(2, "S_LoadLevelFile - DLL");
	printf("S_LoadLevelFile\n");
	 
	SOUND_Stop();
	Sound_FreeSamples();
	if (!g_FirstLevel)
		FreeLevel();
	g_FirstLevel = false;
	
	char filename[80];
	GameScriptLevel* level = g_GameFlow->Levels[levelIndex];
	strcpy_s(filename, level->FileName.c_str());
	
	// Loading level is done is two threads, one for loading level and one for drawing loading screen
	IsLevelLoading = true;
	hLoadLevel = _beginthreadex(0, 0, LoadLevel, filename, 0, &ThreadId);

	// This function loops until progress is 100%. Not very thread safe, but behaviour should be predictable.
	wchar_t loadscreenFileName[80];
	std::mbstowcs(loadscreenFileName, level->LoadScreenFileName.c_str(),80);
	std::wstring loadScreenFile = std::wstring(loadscreenFileName);
	g_Renderer.DrawLoadingScreen(loadScreenFile);

	while (IsLevelLoading);

	return true;
}

bool ReadLuaIds(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(ChunkLuaId))
	{
		int luaId = 0;
		g_levelChunkIO->GetRawStream()->ReadInt32(&luaId);
		
		int itemId = 0;
		g_levelChunkIO->GetRawStream()->ReadInt32(&itemId);

		g_GameScript->AddLuaId(luaId, itemId);

		return true;
	}
	else
		return false;
}

bool ReadLuaTriggers(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(ChunkTrigger))
	{
		char* functionName = NULL;
		g_levelChunkIO->GetRawStream()->ReadString(&functionName);
		
		char* functionCode = NULL;
		g_levelChunkIO->GetRawStream()->ReadString(&functionCode);

		LuaFunction* function = new LuaFunction();
		function->Name = string(functionName);
		function->Code = string(functionCode);
		function->Executed = false;

		g_GameScript->AddTrigger(function);

		delete functionName;
		delete functionCode;

		return true;
	}
	else
		return false;
}

bool ReadNewDataChunks(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(ChunkTriggersList))
		return g_levelChunkIO->ReadChunks(ReadLuaTriggers, 0);
	else if (chunkId->EqualsTo(ChunkLuaIds))
		return g_levelChunkIO->ReadChunks(ReadLuaIds, 0);
	return false;
}

void LoadNewData(int size)
{
	// Free old level scripts
	MemoryStream stream(LevelDataPtr, size);
	g_levelChunkIO = new ChunkReader(0x4D355254, &stream);
	if (!g_levelChunkIO->IsValid())
		return;

	g_levelChunkIO->ReadChunks(ReadNewDataChunks, 0);
}

void LoadSprites()
{
	//DB_Log(2, "LoadSprites");

	ReadInt32(); // SPR\0

	int numSprites = ReadInt32();
	g_Level.Sprites.resize(numSprites);
	for (int i = 0; i < numSprites; i++)
	{
		SPRITE* spr = &g_Level.Sprites[i];
		spr->tile = ReadInt32();
		spr->x1 = ReadFloat();
		spr->y1 = ReadFloat();
		spr->x2 = ReadFloat();
		spr->y2 = ReadFloat();
		spr->x3 = ReadFloat();
		spr->y3 = ReadFloat();
		spr->x4 = ReadFloat();
		spr->y4 = ReadFloat();
	}

	g_Level.NumSpritesSequences = ReadInt32();
	for (int i = 0; i < g_Level.NumSpritesSequences; i++)
	{
		int spriteID = ReadInt32();
		short negLength = ReadInt16();
		short offset = ReadInt16();
		if (spriteID >= ID_NUMBER_OBJECTS)
		{
			StaticObjects[spriteID - ID_NUMBER_OBJECTS].meshNumber = offset;
		}
		else
		{
			Objects[spriteID].nmeshes = negLength;
			Objects[spriteID].meshIndex = offset;
			Objects[spriteID].loaded = true;
		}
	}
}

void GetCarriedItems()
{
	int i;
	ITEM_INFO* item, *item2;
	short linknum;

	for (i = 0; i < g_Level.NumItems; ++i)
		g_Level.Items[i].carriedItem = NO_ITEM;

	for (i = 0; i < g_Level.NumItems; ++i)
	{
		item = &g_Level.Items[i];
		if (Objects[item->objectNumber].intelligent || item->objectNumber >= ID_SEARCH_OBJECT1 && item->objectNumber <= ID_SEARCH_OBJECT3)
		{
			for (linknum = g_Level.Rooms[item->roomNumber].itemNumber; linknum != NO_ITEM; linknum = g_Level.Items[linknum].nextItem)
			{
				item2 = &g_Level.Items[linknum];
				if (abs(item2->pos.xPos - item->pos.xPos) < 512
					&& abs(item2->pos.zPos - item->pos.zPos) < 512
					&& abs(item2->pos.yPos - item->pos.yPos) < 256
					&& Objects[item2->objectNumber].isPickup)
				{
					item2->carriedItem = item->carriedItem;
					item->carriedItem = linknum;
					RemoveDrawnItem(linknum);
					item2->roomNumber = NO_ROOM;
				}
			}
		}
	}
}

void GetAIPickups()
{
	int i, num;
	ITEM_INFO* item;
	AIOBJECT* object;

	for (i = 0; i < g_Level.NumItems; ++i)
	{
		item = &g_Level.Items[i];
		if (Objects[item->objectNumber].intelligent)
		{
			item->aiBits = 0;
			for (num = 0; num < g_Level.AIObjects.size(); ++num)
			{
				object = &g_Level.AIObjects[num];
				if (abs(object->x - item->pos.xPos) < 512
					&& abs(object->z - item->pos.zPos) < 512
					&& object->roomNumber == item->roomNumber
					&& object->objectNumber < ID_AI_PATROL2)
				{
					item->aiBits = (1 << object->objectNumber - ID_AI_GUARD) & 0x1F;
					item->itemFlags[3] = object->triggerFlags;
					if (object->objectNumber != ID_AI_GUARD)
						object->roomNumber = NO_ROOM;
				}
			}
			item->TOSSPAD |= item->aiBits << 8 | (char) item->itemFlags[3];
		}
	}
}