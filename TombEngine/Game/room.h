#pragma once
#include "framework.h"
#include "Game/collision/floordata.h"
#include "Specific/newtypes.h"
#include "Math/Math.h"

struct TriggerVolume;

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
	Pose pos;
	int roomNumber;
	float scale;
	short staticNumber;
	short flags;
	Vector4 color;
	short HitPoints;
	std::string Name;
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

enum RoomEnvFlags
{
	ENV_FLAG_WATER			  = (1 << 0),
	ENV_FLAG_SWAMP			  = (1 << 2),
	ENV_FLAG_OUTSIDE		  = (1 << 3),
	ENV_FLAG_DYNAMIC_LIT	  = (1 << 4),
	ENV_FLAG_WIND			  = (1 << 5),
	ENV_FLAG_NOT_NEAR_OUTSIDE = (1 << 6),
	ENV_FLAG_NO_LENSFLARE	  = (1 << 7), // NOTE: Was quicksand in TR3.
	ENV_FLAG_MIST			  = (1 << 8),
	ENV_FLAG_CAUSTICS		  = (1 << 9),
	ENV_FLAG_UNKNOWN3		  = (1 << 10),
	ENV_FLAG_COLD			  = (1 << 12)
};

enum StaticMeshFlags : short
{
	SM_VISIBLE = 1,
	SM_SOLID = 2
};

struct ROOM_INFO
{
	int x;
	int y;
	int z;
	int minfloor;
	int maxceiling;
	int xSize;
	int zSize;
	Vector3 ambient;
	int flippedRoom;
	int flags;
	int meshEffect;
	int reverbType;
	int flipNumber;
	short itemNumber;
	short fxNumber;
	bool boundActive;

	std::string name;
	std::vector<std::string> tags;

	std::vector<FloorInfo> floor;
	std::vector<ROOM_LIGHT> lights;
	std::vector<MESH_INFO> mesh;
	std::vector<TriggerVolume> triggerVolumes;

	std::vector<Vector3> positions;
	std::vector<Vector3> normals;
	std::vector<Vector3> colors;
	std::vector<Vector3> effects;
	std::vector<BUCKET> buckets;
	std::vector<ROOM_DOOR> doors;

	std::vector<int> neighbors; // TODO: Move to level struct

	bool Active();
};

constexpr auto MAX_FLIPMAP = 256;
constexpr auto NUM_ROOMS = 1024;
constexpr auto NO_ROOM = -1;
constexpr auto OUTSIDE_Z = 64;
constexpr auto OUTSIDE_SIZE = 1024;

extern byte FlipStatus;
extern int FlipStats[MAX_FLIPMAP];
extern int FlipMap[MAX_FLIPMAP];

void DoFlipMap(short group);
void AddRoomFlipItems(ROOM_INFO* room);
void RemoveRoomFlipItems(ROOM_INFO* room);
bool IsObjectInRoom(short roomNumber, short objectNumber);
bool IsPointInRoom(Vector3i pos, int roomNumber);
int FindRoomNumber(Vector3i pos, int startRoom = NO_ROOM);
Vector3i GetRoomCenter(int roomNumber);
int IsRoomOutside(int x, int y, int z);
std::set<int> GetRoomList(int roomNumber);
void InitializeNeighborRoomList();

GameBoundingBox& GetBoundsAccurate(const MESH_INFO& mesh, bool visibility);
FloorInfo* GetSector(ROOM_INFO* room, int x, int z);
