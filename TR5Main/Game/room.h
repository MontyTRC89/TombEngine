#pragma once
#include <framework.h>
#include <newtypes.h>
#include "floordata.h"
#include "Specific\phd_global.h"

struct ANIM_FRAME;

struct ROOM_VERTEX
{
	Vector3 position;
	Vector3 normal;
	Vector2 textureCoordinates;
	Vector3 color;
	int effects;
	int index;
};

struct ROOM_DOOR
{
	short room;
	Vector3 normal;
	Vector3 vertices[4];
};

struct ROOM_LIGHT
{
	int x, y, z;       // Position of light, in world coordinates
	float r, g, b;       // Colour of the light
	float intensity;
	float in;            // Cosine of the IN value for light / size of IN value
	float out;           // Cosine of the OUT value for light / size of OUT value
	float length;         // Range of light
	float cutoff;         // Range of light
	float dx, dy, dz;    // Direction - used only by sun and spot lights
	byte type;
	bool castShadows;
};

struct MESH_INFO
{
	PHD_3DPOS pos;
	short staticNumber;
	short flags;
	Vector4 color;
	short hitPoints;
	std::string luaName;
};

struct LIGHTINFO
{
	int x; // size=0, offset=0
	int y; // size=0, offset=4
	int z; // size=0, offset=8
	unsigned char Type; // size=0, offset=12
	unsigned char r; // size=0, offset=13
	unsigned char g; // size=0, offset=14
	unsigned char b; // size=0, offset=15
	short nx; // size=0, offset=16
	short ny; // size=0, offset=18
	short nz; // size=0, offset=20
	short Intensity; // size=0, offset=22
	unsigned char Inner; // size=0, offset=24
	unsigned char Outer; // size=0, offset=25
	short FalloffScale; // size=0, offset=26
	short Length; // size=0, offset=28
	short Cutoff; // size=0, offset=30
};

enum RoomEnumFlag
{
	ENV_FLAG_WATER = 0x0001,
	ENV_FLAG_SWAMP = 0x0004,
	ENV_FLAG_OUTSIDE = 0x0008,
	ENV_FLAG_DYNAMIC_LIT = 0x0010,
	ENV_FLAG_WIND = 0x0020,
	ENV_FLAG_NOT_NEAR_OUTSIDE = 0x0040,
	ENV_FLAG_NO_LENSFLARE = 0x0080, // Was quicksand in TR3.
	ENV_FLAG_MIST = 0x0100,
	ENV_FLAG_CAUSTICS = 0x0200,
	ENV_FLAG_UNKNOWN3 = 0x0400
};

enum StaticMeshFlags : short
{
	SM_VISIBLE = 1,
	SM_SOLID = 2
};

enum TriggerStatus 
{
	TS_OUTSIDE = 0,
	TS_ENTERING = 1,
	TS_INSIDE = 2,
	TS_LEAVING = 3
};

enum TriggerVolumeType
{
	VOLUME_BOX = 0,
	VOLUME_SPHERE = 1,
	VOLUME_PRISM = 2 // Unsupported as of now
};

enum TriggerVolumeActivators
{
	PLAYER = 1,
	NPC = 2,
	MOVEABLES = 4,
	STATICS = 8,
	FLYBYS = 16,
	PHYSICALOBJECTS = 32 // Future-proofness for Bullet
};

struct TRIGGER_VOLUME
{
	TriggerVolumeType type;

	Vector3 position;
	Quaternion rotation;
	Vector3 scale; // X used as radius if type is VOLUME_SPHERE

	std::string onEnter;
	std::string onInside;
	std::string onLeave;

	int activators;
	bool oneShot;

	TriggerStatus status;
	BoundingOrientedBox box;
	BoundingSphere sphere;
};

struct ROOM_INFO
{
	int x;
	int y;
	int z;
	int minfloor;
	int maxceiling;
	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector3> colors;
	std::vector<Vector3> effects;
	std::vector<BUCKET> buckets;
	std::vector<ROOM_DOOR> doors;
	int xSize;
	int ySize;
	std::vector<FLOOR_INFO> floor;
	Vector3 ambient;
	std::vector<ROOM_LIGHT> lights;
	std::vector<MESH_INFO> mesh;
	int flippedRoom;
	int flags;
	int meshEffect;
	int reverbType;
	int flipNumber;
	short itemNumber;
	short fxNumber;
	bool boundActive;
	std::vector<TRIGGER_VOLUME> triggerVolumes;
};

struct ANIM_STRUCT
{
	int framePtr;
	short interpolation;
	short currentAnimState;
	int velocity;
	int acceleration;
	int Xvelocity;
	int Xacceleration;
	short frameBase;
	short frameEnd;
	short jumpAnimNum;
	short jumpFrameNum;
	short numberChanges;
	short changeIndex;
	short numberCommands;
	short commandIndex;
};

constexpr auto NUM_ROOMS = 1024;
constexpr auto NO_ROOM = -1;