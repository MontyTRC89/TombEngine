#pragma once
#include "framework.h"
#include "Game/collision/floordata.h"
#include "Math/Math.h"
#include "Specific/newtypes.h"

enum class ReverbType;
struct TriggerVolume;

constexpr auto MAX_FLIPMAP	= 256;
constexpr auto NUM_ROOMS	= 1024;
constexpr auto NO_ROOM		= -1;
constexpr auto OUTSIDE_Z	= 64;
constexpr auto OUTSIDE_SIZE = 1024;

extern bool FlipStatus;
extern bool FlipStats[MAX_FLIPMAP];
extern int  FlipMap[MAX_FLIPMAP];

enum RoomEnvFlags
{
	ENV_FLAG_WATER			  = (1 << 0),
	ENV_FLAG_SWAMP			  = (1 << 2),
	ENV_FLAG_OUTSIDE		  = (1 << 3),
	ENV_FLAG_DYNAMIC_LIT	  = (1 << 4),
	ENV_FLAG_WIND			  = (1 << 5),
	ENV_FLAG_NOT_NEAR_OUTSIDE = (1 << 6),
	ENV_FLAG_NO_LENSFLARE	  = (1 << 7),
	ENV_FLAG_MIST			  = (1 << 8),
	ENV_FLAG_CAUSTICS		  = (1 << 9),
	ENV_FLAG_UNKNOWN3		  = (1 << 10),
	ENV_FLAG_DAMAGE			  = (1 << 11),
	ENV_FLAG_COLD			  = (1 << 12)
};

enum StaticMeshFlags : short
{
	SM_VISIBLE = 1,
	SM_SOLID = 2
};

struct ROOM_VERTEX
{
	Vector3 position;
	Vector3 normal;
	Vector2 textureCoordinates;
	Vector3 color;
	int		effects;
	int		index;
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
	bool Dirty;
};

struct LIGHTINFO
{
	int x;
	int y;
	int z;
	unsigned char Type;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	short nx;
	short ny;
	short nz;
	short Intensity;
	unsigned char Inner;
	unsigned char Outer;
	short FalloffScale;
	short Length;
	short Cutoff;
};

struct ROOM_INFO
{
	int index;
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
	ReverbType reverbType;
	int flipNumber;
	short itemNumber;
	short fxNumber;
	bool boundActive;

	std::string name = {};
	std::vector<std::string> tags = {};

	std::vector<FloorInfo>	   floor		  = {};
	std::vector<ROOM_LIGHT>	   lights		  = {};
	std::vector<MESH_INFO>	   mesh			  = {};
	std::vector<TriggerVolume> triggerVolumes = {};

	std::vector<Vector3>   positions = {};
	std::vector<Vector3>   normals	 = {};
	std::vector<Vector3>   colors	 = {};
	std::vector<Vector3>   effects	 = {};
	std::vector<BUCKET>	   buckets	 = {};
	std::vector<ROOM_DOOR> doors	 = {};

	std::vector<int> neighbors = {};

	bool Active();
};

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
