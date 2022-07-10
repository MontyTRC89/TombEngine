#include "framework.h"
#include "Specific/level.h"

#include <process.h>
#include <zlib.h>
#include "Game/animation.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/control/lot.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/pickup/pickup.h"
#include "Game/savegame.h"
#include "Game/spotcam.h"
#include "Renderer/Renderer11.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/Objects/ScriptInterfaceObjectsHandler.h"
#include "Scripting/Include/ScriptInterfaceGame.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using TEN::Renderer::g_Renderer;
using std::vector;
using std::string;

using namespace TEN::Input;
using namespace TEN::Entities::Doors;

uintptr_t hLoadLevel;
unsigned int ThreadId;
char* LevelDataPtr;
bool IsLevelLoading;
bool LoadedSuccessfully;
vector<int> MoveablesIds;
vector<int> StaticObjectsIds;
ChunkReader* g_levelChunkIO;
LEVEL g_Level;

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

void LoadItems()
{
	g_Level.NumItems = ReadInt32();
	TENLog("Num items: " + std::to_string(g_Level.NumItems), LogLevel::Info);

	if (g_Level.NumItems == 0)
		return;

	g_Level.Items.resize(NUM_ITEMS);

	InitialiseItemArray(NUM_ITEMS);

	if (g_Level.NumItems > 0)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			ItemInfo* item = &g_Level.Items[i];

			item->Data = ITEM_DATA{};
			item->ObjectNumber = from_underlying(ReadInt16());
			item->RoomNumber = ReadInt16();
			item->Pose.Position.x = ReadInt32();
			item->Pose.Position.y = ReadInt32();
			item->Pose.Position.z = ReadInt32();
			item->Pose.Orientation.y = ReadInt16();
			item->Shade = ReadInt16();
			item->TriggerFlags = ReadInt16();
			item->Flags = ReadInt16();

			byte numBytes = ReadInt8();
			char buffer[255];
			ReadBytes(buffer, numBytes);
			item->LuaName = std::string(buffer, buffer + numBytes);

			g_GameScriptEntities->AddName(item->LuaName, i);
			g_GameScriptEntities->TryAddColliding(i);

			memcpy(&item->StartPose, &item->Pose, sizeof(PHD_3DPOS));
		}

		for (int i = 0; i < g_Level.NumItems; i++)
			InitialiseItem(i);
	}
}

void LoadObjects()
{
	std::memset(Objects, 0, sizeof(ObjectInfo) * ID_NUMBER_OBJECTS);
	std::memset(StaticObjects, 0, sizeof(STATIC_INFO) * MAX_STATICS);

	int numMeshes = ReadInt32();
	TENLog("Num meshes: " + std::to_string(numMeshes), LogLevel::Info);

	g_Level.Meshes.reserve(numMeshes);
	for (int i = 0; i < numMeshes; i++)
	{
		MESH mesh;

		mesh.LightMode = ReadInt8();

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
				poly.shineStrength = ReadFloat();
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
	TENLog("Num animations: " + std::to_string(numAnimations), LogLevel::Info);

	g_Level.Anims.resize(numAnimations);
	for (int i = 0; i < numAnimations; i++)
	{
		ANIM_STRUCT* anim = &g_Level.Anims[i];

		anim->framePtr = ReadInt32();
		anim->interpolation = ReadInt32();
		anim->ActiveState = ReadInt32();
		anim->velocity = ReadInt32();
		anim->acceleration = ReadInt32();
		anim->Xvelocity = ReadInt32();
		anim->Xacceleration = ReadInt32();
		anim->frameBase = ReadInt32();
		anim->frameEnd = ReadInt32();
		anim->jumpAnimNum = ReadInt32();
		anim->jumpFrameNum = ReadInt32();
		anim->numberChanges = ReadInt32();
		anim->changeIndex = ReadInt32();
		anim->numberCommands = ReadInt32();
		anim->commandIndex = ReadInt32();
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
	TENLog("Num models: " + std::to_string(numModels), LogLevel::Info);

	for (int i = 0; i < numModels; i++)
	{
		int objNum = ReadInt32();
		MoveablesIds.push_back(objNum);

		Objects[objNum].loaded = true;
		Objects[objNum].nmeshes = ReadInt32();
		Objects[objNum].meshIndex = ReadInt32();
		Objects[objNum].boneIndex = ReadInt32();
		Objects[objNum].frameBase = ReadInt32();
		Objects[objNum].animIndex = ReadInt32();

		ReadInt16();

		Objects[objNum].loaded = true;
	}

	TENLog("Initializing objects...", LogLevel::Info);
	InitialiseObjects();

	int numStatics = ReadInt32();
	TENLog("Num statics: " + std::to_string(numStatics), LogLevel::Info);

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
		StaticObjects[meshID].shatterSound = (short)ReadInt16();
	}

	// HACK: to remove after decompiling LoadSprites
	MoveablesIds.push_back(ID_DEFAULT_SPRITES);
}

void LoadCameras()
{
	int numCameras = ReadInt32();
	TENLog("Num cameras: " + std::to_string(numCameras), LogLevel::Info);

	g_Level.Cameras.reserve(numCameras);
	for (int i = 0; i < numCameras; i++)
	{
		auto & camera = g_Level.Cameras.emplace_back();
		camera.x = ReadInt32();
		camera.y = ReadInt32();
		camera.z = ReadInt32();
		camera.roomNumber = ReadInt32();
		camera.flags = ReadInt32();
		camera.speed = ReadInt32();

		byte numBytes = ReadInt8();
		char buffer[255];
		ReadBytes(buffer, numBytes);
		camera.luaName = std::string(buffer, buffer + numBytes);

		g_GameScriptEntities->AddName(camera.luaName, camera);
	}

	NumberSpotcams = ReadInt32();

	if (NumberSpotcams != 0)
	{
		ReadBytes(SpotCam, NumberSpotcams * sizeof(SPOTCAM));
	}

	int numSinks = ReadInt32();
	TENLog("Num sinks: " + std::to_string(numSinks), LogLevel::Info);

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

		g_GameScriptEntities->AddName(sink.luaName, sink);
	}
}

void LoadTextures()
{
	TENLog("Loading textures... ", LogLevel::Info);

	int size;

	int numTextures = ReadInt32();
	TENLog("Num room textures: " + std::to_string(numTextures), LogLevel::Info);

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
	TENLog("Num object textures: " + std::to_string(numTextures), LogLevel::Info);

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
	TENLog("Num static textures: " + std::to_string(numTextures), LogLevel::Info);

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
	TENLog("Num anim textures: " + std::to_string(numTextures), LogLevel::Info);

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
	TENLog("Num sprite textures: " + std::to_string(numTextures), LogLevel::Info);

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

	g_Level.SkyTexture.width = ReadInt32();
	g_Level.SkyTexture.height = ReadInt32();
	size = ReadInt32();
	g_Level.SkyTexture.colorMapData.resize(size);
	ReadBytes(g_Level.SkyTexture.colorMapData.data(), size);
}

void ReadRooms()
{
	int numRooms = ReadInt32();
	TENLog("Num rooms: " + std::to_string(numRooms), LogLevel::Info);

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

		room.zSize = ReadInt32();
		room.xSize = ReadInt32();

		room.floor.reserve(room.zSize * room.xSize);

		for (int j = 0; j < room.zSize * room.xSize; j++)
		{
			FloorInfo floor;

			floor.TriggerIndex = ReadInt32();
			floor.Box = ReadInt32();
			floor.Material = (FLOOR_MATERIAL)ReadInt32();
			floor.Stopper = (bool)ReadInt32();

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
			mesh.pos.Position.x = ReadInt32();
			mesh.pos.Position.y = ReadInt32();
			mesh.pos.Position.z = ReadInt32();
			mesh.pos.Orientation.x = 0;
			mesh.pos.Orientation.y = ReadUInt16();
			mesh.pos.Orientation.z = 0;
			mesh.flags = ReadUInt16();
			Vector3 rgb = ReadVector3();
			float a = ReadFloat();
			mesh.staticNumber = ReadUInt16();
			mesh.color = Vector4(rgb.x, rgb.y, rgb.z, a);
			mesh.HitPoints = ReadInt16();

			byte numBytes = ReadInt8();
			char buffer[255];
			ReadBytes(buffer, numBytes);
			mesh.luaName = std::string(buffer, buffer + numBytes);

			g_GameScriptEntities->AddName(mesh.luaName, mesh);
		}

		int numTriggerVolumes = ReadInt32();
		for (int j = 0; j < numTriggerVolumes; j++)
		{
			TriggerVolume volume;

			volume.Type = (TriggerVolumeType)ReadInt32();

			volume.Position.x = ReadFloat();
			volume.Position.y = ReadFloat();
			volume.Position.z = ReadFloat();

			volume.Rotation.x = ReadFloat();
			volume.Rotation.y = ReadFloat();
			volume.Rotation.z = ReadFloat();
			volume.Rotation.w = ReadFloat();

			volume.Scale.x = ReadFloat();
			volume.Scale.y = ReadFloat();
			volume.Scale.z = ReadFloat();

			volume.Activators = ReadInt32();

			byte numBytes = ReadInt8();
			char buffer[255];
			ReadBytes(buffer, numBytes);
			volume.OnEnter = std::string(buffer, buffer+numBytes);

			numBytes = ReadInt8();
			ReadBytes(buffer, numBytes);
			volume.OnInside = std::string(buffer, buffer+numBytes);

			numBytes = ReadInt8();
			ReadBytes(buffer, numBytes);
			volume.OnLeave = std::string(buffer, buffer+numBytes);

			volume.OneShot = ReadInt8();
			volume.Status = TriggerStatus::Outside;

			volume.Box    = BoundingOrientedBox(volume.Position, volume.Scale, volume.Rotation);
			volume.Sphere = BoundingSphere(volume.Position, volume.Scale.x);

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
	TENLog("Loading rooms... ", LogLevel::Info);
	
	Wibble = 0;

	ReadRooms();
	BuildOutsideRoomsTable();

	int numFloorData = ReadInt32(); 
	g_Level.FloorData.resize(numFloorData);
	ReadBytes(g_Level.FloorData.data(), numFloorData * sizeof(short));
}

void FreeLevel()
{
	static bool firstLevel = true;
	if (firstLevel)
	{
		firstLevel = false;
		return;
	}

	g_Level.RoomTextures.resize(0);
	g_Level.MoveablesTextures.resize(0);
	g_Level.StaticsTextures.resize(0);
	g_Level.AnimatedTextures.resize(0);
	g_Level.SpritesTextures.resize(0);
	g_Level.AnimatedTexturesSequences.resize(0);
	g_Level.Rooms.resize(0);
	g_Level.ObjectTextures.resize(0);
	g_Level.Bones.resize(0);
	g_Level.Meshes.resize(0);
	MoveablesIds.resize(0);
	g_Level.Boxes.resize(0);
	g_Level.Overlaps.resize(0);
	g_Level.Anims.resize(0);
	g_Level.Changes.resize(0);
	g_Level.Ranges.resize(0);
	g_Level.Commands.resize(0);
	g_Level.Frames.resize(0);
	g_Level.Sprites.resize(0);
	g_Level.SoundDetails.resize(0);
	g_Level.SoundMap.resize(0);
	g_Level.FloorData.resize(0);
	g_Level.Cameras.resize(0);
	g_Level.Sinks.resize(0);
	g_Level.SoundSources.resize(0);
	g_Level.AIObjects.resize(0);
	g_Level.LuaFunctionNames.resize(0);
	g_Level.Items.resize(0);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 4; j++)
			g_Level.Zones[j][i].clear();
	}

	g_Renderer.FreeRendererData();
	g_GameScript->FreeLevelScripts();
	g_GameScriptEntities->FreeEntities();

	FreeSamples();
}

size_t ReadFileEx(void* ptr, size_t size, size_t count, FILE* stream)
{
	_lock_file(stream);
	size_t result = fread(ptr, size, count, stream);
	_unlock_file(stream);
	return result;
}

void LoadSoundSources()
{
	int numSoundSources = ReadInt32();
	TENLog("Num sound sources: " + std::to_string(numSoundSources), LogLevel::Info);

	g_Level.SoundSources.reserve(numSoundSources);
	for (int i = 0; i < numSoundSources; i++)
	{
		auto& source = g_Level.SoundSources.emplace_back(SOUND_SOURCE_INFO{});

		source.x = ReadInt32();
		source.y = ReadInt32();
		source.z = ReadInt32();
		source.soundId = ReadInt32();
		source.flags = ReadInt32();

		byte numBytes = ReadInt8();
		char buffer[255];
		ReadBytes(buffer, numBytes);
		source.luaName = std::string(buffer, buffer+numBytes);

		g_GameScriptEntities->AddName(source.luaName, source);
	}
}

void LoadAnimatedTextures()
{
	int numAnimatedTextures = ReadInt32();
	TENLog("Num anim textures: " + std::to_string(numAnimatedTextures), LogLevel::Info);

	for (int i = 0; i < numAnimatedTextures; i++)
	{
		ANIMATED_TEXTURES_SEQUENCE sequence;
		sequence.atlas = ReadInt32();
		sequence.Fps = ReadInt32();
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

	// Unused for now
	int nAnimUVRanges = ReadInt8();
}

void LoadTextureInfos()
{
	ReadInt32(); // TEX/0

	int numObjectTextures = ReadInt32();
	TENLog("Num texinfos: " + std::to_string(numObjectTextures), LogLevel::Info);

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
	TENLog("Num AI objects: " + std::to_string(nAIObjects), LogLevel::Info);

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

		g_GameScriptEntities->AddName(obj.luaName, obj);
	}
}

void LoadLuaFunctionNames()
{
	TENLog("Parsing Lua function names... ", LogLevel::Info);

	int luaFunctionsCount = ReadInt32();
	for (int i = 0; i < luaFunctionsCount; i++)
	{
		byte numBytes = ReadInt8();
		char buffer[255];
		ReadBytes(buffer, numBytes);
		auto luaFunctionName = std::string(buffer, buffer + numBytes);
		g_Level.LuaFunctionNames.push_back(luaFunctionName);
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
		return false;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

unsigned int _stdcall LoadLevel(void* data)
{
	const int levelIndex = reinterpret_cast<int>(data);

	char filename[80];
	ScriptInterfaceLevel* level = g_GameFlow->GetLevel(levelIndex);
	strcpy_s(filename, level->FileName.c_str());

	TENLog("Loading level file: " + std::string(filename), LogLevel::Info);

	LevelDataPtr = nullptr;
	FILE* filePtr = nullptr;
	char* dataPtr = nullptr;

	wchar_t loadscreenFileName[80];
	std::mbstowcs(loadscreenFileName, level->LoadScreenFileName.c_str(), 80);
	std::wstring loadScreenFile = std::wstring(loadscreenFileName);
	g_Renderer.SetLoadingScreen(loadScreenFile);

	SetScreenFadeIn(FADE_SCREEN_SPEED);
	g_Renderer.UpdateProgress(0);

	try
	{
		filePtr = FileOpen(filename);

		if (!filePtr)
			throw std::exception((std::string("Unable to read level file: ") + filename).c_str());

		char header[4];
		byte version[4];
		int compressedSize;
		int uncompressedSize;
		int systemHash;

		// Read file header
		ReadFileEx(&header, 1, 4, filePtr);
		ReadFileEx(&version, 1, 4, filePtr);
		ReadFileEx(&systemHash, 1, 4, filePtr); // Reserved: for future quick start feature! Check builder system 

		// Check file header
		if (std::string(header) != "TEN")
			throw std::invalid_argument("Level file header is not valid! Must be TEN. Probably old level version?");
		else
			TENLog("Tomb Editor compiler version: " + std::to_string(version[0]) + "." + std::to_string(version[1]) + "." + std::to_string(version[2]), LogLevel::Info);

		// Check system name hash and reset it if it's valid (because we use build & play feature only once)
		if (SystemNameHash != 0 && SystemNameHash != systemHash)
			throw std::exception("An attempt was made to use level debug feature on a different system.");
		else
			SystemNameHash = 0;

		// Read data sizes
		ReadFileEx(&uncompressedSize, 1, 4, filePtr);
		ReadFileEx(&compressedSize, 1, 4, filePtr);

		// The entire level is ZLIB compressed
		auto compressedBuffer = (char*)malloc(compressedSize);
		dataPtr = (char*)malloc(uncompressedSize);
		LevelDataPtr = dataPtr;

		ReadFileEx(compressedBuffer, compressedSize, 1, filePtr);
		Decompress((byte*)LevelDataPtr, (byte*)compressedBuffer, compressedSize, uncompressedSize);

		// Now the entire level is decompressed, we can close it
		free(compressedBuffer);
		FileClose(filePtr);
		filePtr = nullptr;

		LoadTextures();

		g_Renderer.UpdateProgress(20);

		LoadRooms();
		g_Renderer.UpdateProgress(40);

		LoadObjects();
		g_Renderer.UpdateProgress(50);

		LoadSprites();
		LoadCameras();
		LoadSoundSources();
		g_Renderer.UpdateProgress(60);

		LoadBoxes();

		//InitialiseLOTarray(true);

		LoadAnimatedTextures();
		LoadTextureInfos();
		g_Renderer.UpdateProgress(70);

		LoadItems();
		LoadAIObjects();

		LoadLuaFunctionNames();

		LoadSamples();
		g_Renderer.UpdateProgress(80);

		TENLog("Preparing renderer...", LogLevel::Info);
		
		g_Renderer.UpdateProgress(90);
		g_Renderer.PrepareDataForTheRenderer();

		TENLog("Initializing level...", LogLevel::Info);

		// Initialise the game
		InitialiseGameFlags();
		InitialiseLara(!(InitialiseGame || CurrentLevel == 1));
		GetCarriedItems();
		GetAIPickups();
		Lara.Vehicle = -1;
		g_GameScriptEntities->AssignLara();

		TENLog("Level loading complete.", LogLevel::Info);

		SetScreenFadeOut(FADE_SCREEN_SPEED);
		g_Renderer.UpdateProgress(100);

		LoadedSuccessfully = true;
	}
	catch (std::exception& ex)
	{
		if (filePtr)
		{
			FileClose(filePtr);
			filePtr = nullptr;
		}

		TENLog("Error while loading level: " + std::string(ex.what()), LogLevel::Error);
		LoadedSuccessfully = false;
	}

	if (dataPtr)
	{
		free(dataPtr);
		dataPtr = LevelDataPtr = nullptr;
	}

	// Level loaded
	IsLevelLoading = false;
	_endthreadex(1);
	return LoadedSuccessfully;
}

void LoadSamples()
{
	TENLog("Loading samples... ", LogLevel::Info);

	int SoundMapSize = ReadInt16();
	g_Level.SoundMap.resize(SoundMapSize);
	ReadBytes(g_Level.SoundMap.data(), SoundMapSize * sizeof(short));

	TENLog("Sound map size: " + std::to_string(SoundMapSize), LogLevel::Info);

	int numSamplesInfos = ReadInt32();

	if (!numSamplesInfos)
	{
		TENLog("No samples were found and loaded.", LogLevel::Warning);
		return;
	}

	g_Level.SoundDetails.resize(numSamplesInfos);
	ReadBytes(g_Level.SoundDetails.data(), numSamplesInfos * sizeof(SampleInfo));

	int numSamples = ReadInt32();
	if (numSamples <= 0)
		return;

	int uncompressedSize;
	int compressedSize;
	char* buffer = (char*)malloc(2 * 1024 * 1024);

	for (int i = 0; i < numSamples; i++)
	{
		uncompressedSize = ReadInt32();
		compressedSize = ReadInt32();
		ReadBytes(buffer, compressedSize);
		LoadSample(buffer, compressedSize, uncompressedSize, i);
	}

	free(buffer);
}

void LoadBoxes()
{
	// Read boxes
	int numBoxes = ReadInt32();
	g_Level.Boxes.resize(numBoxes);
	ReadBytes(g_Level.Boxes.data(), numBoxes * sizeof(BOX_INFO));

	TENLog("Num boxes: " + std::to_string(numBoxes), LogLevel::Info);

	// Read overlaps
	int numOverlaps = ReadInt32();
	g_Level.Overlaps.resize(numOverlaps);
	ReadBytes(g_Level.Overlaps.data(), numOverlaps * sizeof(OVERLAP));

	TENLog("Num overlaps: " + std::to_string(numOverlaps), LogLevel::Info);

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
			g_Level.Boxes[i].flags |= BLOCKED;
	}
}

int LoadLevelFile(int levelIndex)
{
	TENLog("Loading level file...", LogLevel::Info);

	CleanUp();
	FreeLevel();
	
	// Loading level is done is two threads, one for loading level and one for drawing loading screen
	IsLevelLoading = true;
	hLoadLevel = _beginthreadex(
		nullptr,
		0, 
		LoadLevel, 
		reinterpret_cast<void*>(levelIndex), 
		0, 
		&ThreadId);

	while (IsLevelLoading);

	return LoadedSuccessfully;
}

void LoadSprites()
{
	ReadInt32(); // SPR\0

	int numSprites = ReadInt32();
	g_Level.Sprites.resize(numSprites);

	TENLog("Num sprites: " + std::to_string(numSprites), LogLevel::Info);

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

	TENLog("Num sprite sequences: " + std::to_string(g_Level.NumSpritesSequences), LogLevel::Info);

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
	for (int i = 0; i < g_Level.NumItems; ++i)
		g_Level.Items[i].CarriedItem = NO_ITEM;

	for (int i = 0; i < g_Level.NumItems; ++i)
	{
		auto* item = &g_Level.Items[i];
		if (Objects[item->ObjectNumber].intelligent || item->ObjectNumber >= ID_SEARCH_OBJECT1 && item->ObjectNumber <= ID_SEARCH_OBJECT3)
		{
			for (short linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
			{
				auto* item2 = &g_Level.Items[linkNumber];

				if (abs(item2->Pose.Position.x - item->Pose.Position.x) < CLICK(2) &&
					abs(item2->Pose.Position.z - item->Pose.Position.z) < CLICK(2) &&
					abs(item2->Pose.Position.y - item->Pose.Position.y) < CLICK(1) &&
					Objects[item2->ObjectNumber].isPickup)
				{
					item2->CarriedItem = item->CarriedItem;
					item->CarriedItem = linkNumber;
					RemoveDrawnItem(linkNumber);
					item2->RoomNumber = NO_ROOM;
				}
			}
		}
	}
}

void GetAIPickups()
{
	for (int i = 0; i < g_Level.NumItems; ++i)
	{
		auto* item = &g_Level.Items[i];
		if (Objects[item->ObjectNumber].intelligent)
		{
			item->AIBits = 0;

			for (int number = 0; number < g_Level.AIObjects.size(); ++number)
			{
				auto* object = &g_Level.AIObjects[number];

				if (abs(object->x - item->Pose.Position.x) < CLICK(2) &&
					abs(object->z - item->Pose.Position.z) < CLICK(2) &&
					object->roomNumber == item->RoomNumber &&
					object->objectNumber < ID_AI_PATROL2)
				{
					item->AIBits = (1 << object->objectNumber - ID_AI_GUARD) & 0x1F;
					item->ItemFlags[3] = object->triggerFlags;

					if (object->objectNumber != ID_AI_GUARD)
						object->roomNumber = NO_ROOM;
				}
			}

			if (item->Data.is<CreatureInfo>())
			{
				auto* creature = GetCreatureInfo(item);
				creature->Tosspad |= item->AIBits << 8 | (char)item->ItemFlags[3];
			}
		}
	}
}

void BuildOutsideRoomsTable()
{
	for (int x = 0; x < OUTSIDE_SIZE; x++)
	{
		for (int z = 0; z < OUTSIDE_SIZE; z++)
			OutsideRoomTable[x][z].clear();
	}

	for (int x = 0; x < OUTSIDE_SIZE; x++)
	{
		for (int z = 0; z < OUTSIDE_SIZE; z++)
		{
			for (int i = 0; i < g_Level.Rooms.size(); i++)
			{
				auto* room = &g_Level.Rooms[i];

				int rx = (room->x / SECTOR(1));
				int rz = (room->z / SECTOR(1));

				if (x >= (rx + 1) && z >= (rz + 1) &&
					x <= (rx + room->xSize - 2) && z <= (rz + room->zSize - 2))
				{
					OutsideRoomTable[x][z].push_back(i);
				}
			}
		}
	}
}
