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
#include <process.h>
#include <zlib.h>
using TEN::Renderer::g_Renderer;
using std::vector;
using std::string;

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
			
			item->objectNumber = from_underlying(ReadInt16());
			item->roomNumber = ReadInt16();
			item->pos.xPos = ReadInt32();
			item->pos.yPos = ReadInt32();
			item->pos.zPos = ReadInt32();
			item->pos.yRot = ReadInt16();
			item->shade = ReadInt16();
			item->triggerFlags = ReadInt16();
			item->flags = ReadInt16();

			byte numBytes = ReadInt8();
			char buffer[255];
			ReadBytes(buffer, numBytes);
			item->luaName = std::string(buffer, buffer + numBytes);

			g_GameScript->AddName(item->luaName, i);

			memcpy(&item->startPos, &item->pos, sizeof(PHD_3DPOS));
		}

		for (int i = 0; i < g_Level.NumItems; i++)
			InitialiseItem(i);
	}

	for (auto& r : g_Level.Rooms)
	{
		for (const auto& mesh : r.mesh)
		{
			FLOOR_INFO* floor = &r.floor[((mesh.z - r.z) / 1024) + r.xSize * ((mesh.x - r.x) / 1024)];
			 
			if (floor->box == NO_BOX)
				continue;

			if (!(g_Level.Boxes[floor->box].flags & BLOCKED))
			{
				int fl = floor->floor * 4;
				STATIC_INFO* st = &StaticObjects[mesh.staticNumber];
				if (fl <= mesh.y - st->collisionBox.Y2 + 512 && fl < mesh.y - st->collisionBox.Y1)
				{
					if (st->collisionBox.X1 == 0 || st->collisionBox.X2 == 0 ||
						st->collisionBox.Z1 == 0 || st->collisionBox.Z2 == 0 ||
						((st->collisionBox.X1 < 0) ^ (st->collisionBox.X2 < 0)) &&
						((st->collisionBox.Z1 < 0) ^ (st->collisionBox.Z2 < 0)))
					{
						floor->stopper = true;
					}
				}
			}
		}
	}

	return true;
}

void LoadObjects()
{
	std::memset(Objects, 0, sizeof(OBJECT_INFO) * ID_NUMBER_OBJECTS);
	std::memset(StaticObjects, 0, sizeof(STATIC_INFO) * MAX_STATICS);

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

		mesh.effects.resize(numVertices);
		ReadBytes(mesh.effects.data(), 12 * numVertices);

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
				poly.animatedSequence = ReadInt32();
				poly.animatedFrame = ReadInt32();
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
	for (int i = 0; i < numAnimations; i++)
	{
		ANIM_STRUCT* anim = &g_Level.Anims[i];

		anim->framePtr = ReadInt32();
		anim->interpolation = ReadInt16();
		anim->currentAnimState = ReadInt16();
		anim->velocity = ReadInt32();
		anim->acceleration = ReadInt32();
		anim->Xvelocity = ReadInt32();
		anim->Xacceleration = ReadInt32();
		anim->frameBase = ReadInt16();
		anim->frameEnd = ReadInt16();
		anim->jumpAnimNum = ReadInt16();
		anim->jumpFrameNum = ReadInt16();
		anim->numberChanges = ReadInt16();
		anim->changeIndex = ReadInt16();
		anim->numberCommands = ReadInt16();
		anim->commandIndex = ReadInt16();
	}

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
	for (int i = 0; i < numFrames; i++)
	{
		ANIM_FRAME* frame = &g_Level.Frames[i];
		frame->boundingBox.X1 = ReadInt16();
		frame->boundingBox.X2 = ReadInt16();
		frame->boundingBox.Y1 = ReadInt16();
		frame->boundingBox.Y2 = ReadInt16();
		frame->boundingBox.Z1 = ReadInt16();
		frame->boundingBox.Z2 = ReadInt16();
		frame->offsetX = ReadInt16();
		frame->offsetY = ReadInt16();
		frame->offsetZ = ReadInt16();
		int numAngles = ReadInt16();
		frame->angles.resize(numAngles);
		for (int j = 0; j < numAngles; j++)
		{
			Quaternion* q = &frame->angles[j];
			q->x = ReadFloat();
			q->y = ReadFloat();
			q->z = ReadFloat();
			q->w = ReadFloat();
		}
	}
	//ReadBytes(g_Level.Frames.data(), sizeof(ANIM_FRAME) * numFrames);

	int numModels = ReadInt32();
	for (int i = 0; i < numModels; i++)
	{
		int objNum = ReadInt32();
		MoveablesIds.push_back(objNum);

		Objects[objNum].loaded = true;
		Objects[objNum].nmeshes = (short)ReadInt16();
		Objects[objNum].meshIndex = (short)ReadInt16();
		Objects[objNum].boneIndex = ReadInt32();
		Objects[objNum].frameBase = ReadInt32();
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

		StaticObjects[meshID].meshNumber = (short)ReadInt32();

		StaticObjects[meshID].visibilityBox.X1 = ReadInt16();
		StaticObjects[meshID].visibilityBox.X2 = ReadInt16();
		StaticObjects[meshID].visibilityBox.Y1 = ReadInt16();
		StaticObjects[meshID].visibilityBox.Y2 = ReadInt16();
		StaticObjects[meshID].visibilityBox.Z1 = ReadInt16();
		StaticObjects[meshID].visibilityBox.Z2 = ReadInt16();

		StaticObjects[meshID].collisionBox.X1 = ReadInt16();
		StaticObjects[meshID].collisionBox.X2 = ReadInt16();
		StaticObjects[meshID].collisionBox.Y1 = ReadInt16();
		StaticObjects[meshID].collisionBox.Y2 = ReadInt16();
		StaticObjects[meshID].collisionBox.Z1 = ReadInt16();
		StaticObjects[meshID].collisionBox.Z2 = ReadInt16();

		StaticObjects[meshID].flags = (short)ReadInt16();

		StaticObjects[meshID].shatterType = (short)ReadInt16();
		StaticObjects[meshID].shatterDamage = (short)ReadInt16();
		StaticObjects[meshID].shatterSound = (short)ReadInt16();
	}

	// HACK: to remove after decompiling LoadSprites
	MoveablesIds.push_back(ID_DEFAULT_SPRITES);
}

void LoadCameras()
{
	int numCameras = ReadInt32();
	g_Level.Cameras.reserve(numCameras);
	for (int i = 0; i < numCameras; i++)
	{
		auto & camera = g_Level.Cameras.emplace_back();
		camera.x = ReadInt32();
		camera.y = ReadInt32();
		camera.z = ReadInt32();
		camera.roomNumber = ReadInt32();
		camera.flags = ReadInt32();

		byte numBytes = ReadInt8();
		char buffer[255];
		ReadBytes(buffer, numBytes);
		camera.luaName = std::string(buffer, buffer + numBytes);

		g_GameScript->AddName(camera.luaName, camera);
	}

	NumberSpotcams = ReadInt32();

	if (NumberSpotcams != 0)
	{
		ReadBytes(SpotCam, NumberSpotcams * sizeof(SPOTCAM));
	}

	int numSinks = ReadInt32();
	g_Level.Sinks.reserve(numSinks);
	for (int i = 0; i < numSinks; i++)
	{
		auto & sink = g_Level.Sinks.emplace_back();
		sink.x = ReadInt32();
		sink.y = ReadInt32();
		sink.z = ReadInt32();
		sink.strength = ReadInt32();
		sink.boxIndex = ReadInt32();

		byte numBytes = ReadInt8();
		char buffer[255];
		ReadBytes(buffer, numBytes);
		sink.luaName = std::string(buffer, buffer+numBytes);

		g_GameScript->AddName(sink.luaName, sink);
	}
}

void LoadTextures()
{
	printf("LoadTextures\n");

	int size;

	int numTextures = ReadInt32();
	g_Level.RoomTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		texture.width = ReadInt32();
		texture.height = ReadInt32();

		size = ReadInt32();
		texture.colorMapData.resize(size);
		ReadBytes(texture.colorMapData.data(), size);
		
		byte hasNormalMap = ReadInt8();
		if (hasNormalMap)
		{
			size = ReadInt32();
			texture.normalMapData.resize(size);
			ReadBytes(texture.normalMapData.data(), size);
		}

		g_Level.RoomTextures.push_back(texture);
	}

	numTextures = ReadInt32();
	g_Level.MoveablesTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		texture.width = ReadInt32();
		texture.height = ReadInt32();

		size = ReadInt32();
		texture.colorMapData.resize(size);
		ReadBytes(texture.colorMapData.data(), size);

		bool hasNormalMap = ReadInt8();
		if (hasNormalMap)
		{
			size = ReadInt32();
			texture.normalMapData.resize(size);
			ReadBytes(texture.normalMapData.data(), size);
		}

		g_Level.MoveablesTextures.push_back(texture);
	}

	numTextures = ReadInt32();
	g_Level.StaticsTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		texture.width = ReadInt32();
		texture.height = ReadInt32();

		size = ReadInt32();
		texture.colorMapData.resize(size);
		ReadBytes(texture.colorMapData.data(), size);

		bool hasNormalMap = ReadInt8();
		if (hasNormalMap)
		{
			size = ReadInt32();
			texture.normalMapData.resize(size);
			ReadBytes(texture.normalMapData.data(), size);
		}

		g_Level.StaticsTextures.push_back(texture);
	}

	numTextures = ReadInt32();
	g_Level.AnimatedTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		texture.width = ReadInt32();
		texture.height = ReadInt32();

		size = ReadInt32();
		texture.colorMapData.resize(size);
		ReadBytes(texture.colorMapData.data(), size);

		bool hasNormalMap = ReadInt8();
		if (hasNormalMap)
		{
			size = ReadInt32();
			texture.normalMapData.resize(size);
			ReadBytes(texture.normalMapData.data(), size);
		}

		g_Level.AnimatedTextures.push_back(texture);
	}

	numTextures = ReadInt32();
	g_Level.SpritesTextures.reserve(numTextures);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		texture.width = ReadInt32();
		texture.height = ReadInt32();

		size = ReadInt32();
		texture.colorMapData.resize(size);
		ReadBytes(texture.colorMapData.data(), size);

		g_Level.SpritesTextures.push_back(texture);
	}

	g_Level.MiscTextures.width = ReadInt32();
	g_Level.MiscTextures.height = ReadInt32();
	size = ReadInt32();
	g_Level.MiscTextures.colorMapData.resize(size);
	ReadBytes(g_Level.MiscTextures.colorMapData.data(), size);
}

void ReadRooms()
{
	int numRooms = ReadInt32();
	printf("NumRooms: %d\n", numRooms);

	for (int i = 0; i < numRooms; i++)
	{
		auto & room = g_Level.Rooms.emplace_back();
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

		room.effects.reserve(numVertices);
		for (int j = 0; j < numVertices; j++)
			room.effects.push_back(ReadVector3());

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
				poly.animatedSequence = ReadInt32();
				poly.animatedFrame = ReadInt32();
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

			floor.FloorCollision.SplitAngle = ReadFloat();
			floor.FloorCollision.Portals[0] = ReadInt32();
			floor.FloorCollision.Portals[1] = ReadInt32();
			floor.FloorCollision.Planes[0].x = ReadFloat();
			floor.FloorCollision.Planes[0].y = ReadFloat();
			floor.FloorCollision.Planes[0].z = ReadFloat();
			floor.FloorCollision.Planes[1].x = ReadFloat();
			floor.FloorCollision.Planes[1].y = ReadFloat();
			floor.FloorCollision.Planes[1].z = ReadFloat();
			floor.CeilingCollision.SplitAngle = ReadFloat();
			floor.CeilingCollision.Portals[0] = ReadInt32();
			floor.CeilingCollision.Portals[1] = ReadInt32();
			floor.CeilingCollision.Planes[0].x = ReadFloat();
			floor.CeilingCollision.Planes[0].y = ReadFloat();
			floor.CeilingCollision.Planes[0].z = ReadFloat();
			floor.CeilingCollision.Planes[1].x = ReadFloat();
			floor.CeilingCollision.Planes[1].y = ReadFloat();
			floor.CeilingCollision.Planes[1].z = ReadFloat();
			floor.WallPortal = ReadInt32();

			floor.Flags.Death = ReadInt8();
			floor.Flags.Monkeyswing = ReadInt8();
			floor.Flags.ClimbNorth = ReadInt8();
			floor.Flags.ClimbSouth = ReadInt8();
			floor.Flags.ClimbEast = ReadInt8();
			floor.Flags.ClimbWest = ReadInt8();
			floor.Flags.MarkTriggerer = ReadInt8();
			floor.Flags.MarkTriggererActive = 0; // TODO: IT NEEDS TO BE WRITTEN/READ FROM SAVEGAMES!
			floor.Flags.MarkBeetle = ReadInt8();

			floor.Room = i;

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

			light.x = ReadInt32();
			light.y = ReadInt32();
			light.z = ReadInt32();
			light.dx = ReadFloat();
			light.dy = ReadFloat();
			light.dz = ReadFloat();
			light.r = ReadFloat();
			light.g = ReadFloat();
			light.b = ReadFloat();
			light.intensity = ReadFloat();
			light.in = ReadFloat();
			light.out = ReadFloat();
			light.length = ReadFloat();
			light.cutoff = ReadFloat();
			light.type = ReadInt8();
			light.castShadows = ReadInt8();

			room.lights.push_back(light);
		}
		
		int numStatics = ReadInt32();
		room.mesh.reserve(numStatics);
		for (int j = 0; j < numStatics; j++)
		{
			auto & mesh = room.mesh.emplace_back();
			mesh.x = ReadInt32();
			mesh.y = ReadInt32();
			mesh.z = ReadInt32();
			mesh.yRot = ReadUInt16();
			mesh.flags = ReadUInt16();
			Vector3 rgb = ReadVector3();
			float a = ReadFloat();
			mesh.staticNumber = ReadUInt16();
			mesh.color = Vector4(rgb.x, rgb.y, rgb.z, a);
			mesh.hitPoints = ReadInt16();

			byte numBytes = ReadInt8();
			char buffer[255];
			ReadBytes(buffer, numBytes);
			mesh.luaName = std::string(buffer, buffer + numBytes);

			g_GameScript->AddName(mesh.luaName, mesh);
		}

		int numTriggerVolumes = ReadInt32();
		for (int j = 0; j < numTriggerVolumes; j++)
		{
			TRIGGER_VOLUME volume;

			volume.type = (TriggerVolumeType)ReadInt32();

			volume.position.x = ReadFloat();
			volume.position.y = ReadFloat();
			volume.position.z = ReadFloat();

			volume.rotation.x = ReadFloat();
			volume.rotation.y = ReadFloat();
			volume.rotation.z = ReadFloat();
			volume.rotation.w = ReadFloat();

			volume.scale.x = ReadFloat();
			volume.scale.y = ReadFloat();
			volume.scale.z = ReadFloat();

			volume.activators = ReadInt32();

			byte numBytes = ReadInt8();
			char buffer[255];
			ReadBytes(buffer, numBytes);
			volume.onEnter = std::string(buffer, buffer+numBytes);

			numBytes = ReadInt8();
			ReadBytes(buffer, numBytes);
			volume.onInside = std::string(buffer, buffer+numBytes);

			numBytes = ReadInt8();
			ReadBytes(buffer, numBytes);
			volume.onLeave = std::string(buffer, buffer+numBytes);

			volume.oneShot = ReadInt8();
			volume.status = TS_OUTSIDE;

			volume.box    = BoundingOrientedBox(volume.position, volume.scale, volume.rotation);
			volume.sphere = BoundingSphere(volume.position, volume.scale.x);

			room.triggerVolumes.push_back(volume);
		}

		room.flippedRoom = ReadInt32();
		room.flags = ReadInt32();
		room.meshEffect = ReadInt32();
		room.reverbType = ReadInt32();
		room.flipNumber = ReadInt32();

		room.itemNumber = NO_ITEM;
		room.fxNumber = NO_ITEM;
	}
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
	g_Level.AnimatedTextures.clear();
	g_Level.SpritesTextures.clear();
	g_Level.AnimatedTexturesSequences.clear();
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
	g_Level.Cameras.clear();
	g_Level.Sinks.clear();
	g_Level.SoundSources.clear();
	g_Level.AIObjects.clear();

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			g_Level.Zones[j][i].clear();
		}
	}
	g_Renderer.freeRendererData();
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
	int numSoundSources = ReadInt32();
	g_Level.SoundSources.reserve(numSoundSources);
	for (int i = 0; i < numSoundSources; i++)
	{
		auto & source = g_Level.SoundSources.emplace_back(SOUND_SOURCE_INFO{});

		source.x = ReadInt32();
		source.y = ReadInt32();
		source.z = ReadInt32();
		source.soundId = ReadInt32();
		source.flags = ReadInt32();

		byte numBytes = ReadInt8();
		char buffer[255];
		ReadBytes(buffer, numBytes);
		source.luaName = std::string(buffer, buffer+numBytes);

		g_GameScript->AddName(source.luaName, source);
	}
}

void LoadAnimatedTextures()
{
	NumAnimatedTextures = ReadInt32();
	for (int i = 0; i < NumAnimatedTextures; i++)
	{
		ANIMATED_TEXTURES_SEQUENCE sequence;
		sequence.atlas = ReadInt32();
		sequence.numFrames = ReadInt32();
		for (int j = 0; j < sequence.numFrames; j++)
		{
			ANIMATED_TEXTURES_FRAME frame;
			frame.x1 = ReadFloat();
			frame.y1 = ReadFloat();
			frame.x2 = ReadFloat();
			frame.y2 = ReadFloat();
			frame.x3 = ReadFloat();
			frame.y3 = ReadFloat();
			frame.x4 = ReadFloat();
			frame.y4 = ReadFloat();
			sequence.frames.push_back(frame);
		}
		g_Level.AnimatedTexturesSequences.push_back(sequence);
	}
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
	g_Level.AIObjects.reserve(nAIObjects);
	for (int i = 0; i < nAIObjects; i++)
	{
		auto & obj = g_Level.AIObjects.emplace_back();

		obj.objectNumber = (GAME_OBJECT_ID)ReadInt16();
		obj.roomNumber = ReadInt16();
		obj.x = ReadInt32();
		obj.y = ReadInt32();
		obj.z = ReadInt32();
		obj.triggerFlags = ReadInt16();
		obj.flags = ReadInt16();
		obj.yRot = ReadInt16();
		obj.boxNumber = ReadInt16();

		byte numBytes = ReadInt8();
		char buffer[255];
		ReadBytes(buffer, numBytes);
		obj.luaName = std::string(buffer, buffer+numBytes);

		g_GameScript->AddName(obj.luaName, obj);
	}
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

bool Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize)
{
	z_stream strm;
	ZeroMemory(&strm, sizeof(z_stream));
	strm.avail_in = compressedSize;
	strm.avail_out = uncompressedSize;
	strm.next_out = (BYTE*)dest;
	strm.next_in = (BYTE*)src;

	inflateInit(&strm);
	inflate(&strm, Z_FULL_FLUSH);

	if (strm.total_out == uncompressedSize)
	{
		inflateEnd(&strm);
		return true;
	}
	else
	{
		return false;
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
	char* baseLevelDataPtr = NULL;

	g_Renderer.updateProgress(0);

	LevelFilePtr = FileOpen(filename);
	if (LevelFilePtr)
	{
		int version;
		int uncompressedSize;
		int compressedSize;
		char* compressedBuffer;

		// Read file header
		ReadFileEx(&version, 1, 4, LevelFilePtr);
		ReadFileEx(&uncompressedSize, 1, 4, LevelFilePtr);
		ReadFileEx(&compressedSize, 1, 4, LevelFilePtr);

		// The entire level is ZLIB compressed
		compressedBuffer = (char*)malloc(compressedSize);
		LevelDataPtr = (char*)malloc(uncompressedSize);
		baseLevelDataPtr = LevelDataPtr;

		ReadFileEx(compressedBuffer, compressedSize, 1, LevelFilePtr);
		Decompress((byte*)LevelDataPtr, (byte*)compressedBuffer, compressedSize, uncompressedSize);

		// Now the entire level is decompressed
		free(compressedBuffer);

		LoadTextures();

		g_Renderer.updateProgress(20);

		WeatherType = ReadInt8();
		LaraDrawType = ReadInt8();

		LoadRooms();
		g_Renderer.updateProgress(40);

		LoadObjects();
		g_Renderer.updateProgress(50);

		LoadSprites();
		LoadCameras();
		LoadSoundEffects();
		g_Renderer.updateProgress(60);

		LoadBoxes();

		InitialiseLOTarray(true);

		LoadAnimatedTextures();
		LoadTextureInfos();
		g_Renderer.updateProgress(70);

		LoadItems();
		LoadAIObjects();
		LoadSamples();
		g_Renderer.updateProgress(80);

		free(baseLevelDataPtr);
		LevelDataPtr = NULL;
		FileClose(LevelFilePtr);
	}
	else
	{
		return false;
	}

	g_Renderer.updateProgress(90);
	g_Renderer.PrepareDataForTheRenderer();
	
	// Initialise the game
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	Wibble = 0;
	TorchRoom = -1;
	InitialiseGameFlags();
	InitialiseLara(!(InitialiseGame || CurrentLevel == 1));
	GetCarriedItems();
	GetAIPickups();
	Lara.Vehicle = -1;
	g_GameScript->AssignItemsAndLara();

	// Level loaded
	IsLevelLoading = false;
	g_Renderer.updateProgress(100);

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

		int numSamples = ReadInt32();
		if (numSamples <= 0)
			return;

		int uncompressedSize;
		int compressedSize;
		char* buffer = (char*)malloc(2 * 1048576);

		for (int i = 0; i < numSamples; i++)
		{
			uncompressedSize = ReadInt32();
			compressedSize = ReadInt32();
			ReadBytes(buffer, compressedSize);
			Sound_LoadSample(buffer, compressedSize, uncompressedSize, i);
		}

		free(buffer);
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
		for (int j = 0; j < MAX_ZONES - 1; j++)
		{
			g_Level.Zones[j][i].resize(numBoxes * sizeof(int));
			ReadBytes(g_Level.Zones[j][i].data(), numBoxes * sizeof(int));
		}

		// Fly zone
		g_Level.Zones[MAX_ZONES - 1][i].resize(numBoxes * sizeof(int));
		ReadBytes(g_Level.Zones[MAX_ZONES - 1][i].data(), numBoxes * sizeof(int));
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
	g_Renderer.renderLoadingScreen(loadScreenFile);

	while (IsLevelLoading);

	return true;
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
	AI_OBJECT* object;

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

void BuildOutsideRoomsTable()
{
	for (int x = 0; x < OUTSIDE_SIZE; x++)
		for (int z = 0; z < OUTSIDE_SIZE; z++)
			OutsideRoomTable[x][z].clear();

	for (int x = 0; x < OUTSIDE_SIZE; x++)
	{
		for (int z = 0; z < OUTSIDE_SIZE; z++)
		{
			for (int i = 0; i < g_Level.Rooms.size(); i++)
			{
				ROOM_INFO* r = &g_Level.Rooms[i];

				int rx = (r->x / 1024);
				int rz = (r->z / 1024);

				if (x >= rx + 1 && z >= rz + 1 && x <= (rx + r->ySize - 2) && z <= (rz + r->xSize - 2))
					OutsideRoomTable[x][z].push_back(i);
			}
		}
	}
}
