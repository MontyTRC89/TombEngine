#pragma once
#include <framework.h>

typedef struct tr5_room_layer
{
	unsigned int NumLayerVertices;   // Number of vertices in this layer (4 bytes)
	unsigned short UnknownL1;
	unsigned short NumLayerRectangles; // Number of rectangles in this layer (2 bytes)
	unsigned short NumLayerTriangles;  // Number of triangles in this layer (2 bytes)
	unsigned short UnknownL2;
	unsigned short Filler;             // Always 0
	unsigned short Filler2;            // Always 0
	/// The following 6 floats define the bounding box for the layer
	float LayerBoundingBoxX1;
	float LayerBoundingBoxY1;
	float LayerBoundingBoxZ1;
	float LayerBoundingBoxX2;
	float LayerBoundingBoxY2;
	float LayerBoundingBoxZ2;
	unsigned int Filler3;     // Always 0 (4 bytes)
	void* VerticesOffset;
	void* PolyOffset;
	void* PolyOffset2;
};

typedef struct tr5_vertex
{
	float x;
	float y;
	float z;
};

struct ROOM_VERTEX
{
	Vector3 position;
	Vector3 normal;
	Vector2 textureCoordinates;
	Vector3 color;
	int effects;
};

struct ROOM_DOOR
{
	short room;
	Vector3 normal;
	Vector3 vertices[4];
};

typedef struct tr4_mesh_face3    // 10 bytes
{
	short Vertices[3];
	short Texture;
	short Effects;    // TR4-5 ONLY: alpha blending and environment mapping strength
};

typedef struct tr4_mesh_face4    // 12 bytes
{
	short Vertices[4];
	short Texture;
	short Effects;
};

struct POLYGON
{
	int shape;
	int baseIndex;
	int texture;
};

struct BUCKET 
{
	int texture;
	byte blendMode;
	bool animated;
	std::vector<int> indices;
};

typedef struct tr_room_sector // 8 bytes
{
	unsigned short FDindex;    // Index into FloorData[]
	unsigned short BoxIndex;   // Index into Boxes[] (-1 if none)
	unsigned char RoomBelow;  // 255 is none
	signed char Floor;      // Absolute height of floor
	unsigned char RoomAbove;  // 255 if none
	signed char Ceiling;    // Absolute height of ceiling
};

typedef struct ROOM_LIGHT
{
	float x, y, z;       // Position of light, in world coordinates
	float r, g, b;       // Colour of the light
	float in;            // Cosine of the IN value for light / size of IN value
	float out;           // Cosine of the OUT value for light / size of OUT value
	float radIn;         // (IN radians) * 2
	float radOut;        // (OUT radians) * 2
	float range;         // Range of light
	float dx, dy, dz;    // Direction - used only by sun and spot lights
	byte type;
};

typedef struct MESH_INFO
{
	int x;
	int y;
	int z;
	short yRot;
	short shade;
	short flags;
	short staticNumber;
};

typedef struct LIGHTINFO
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

enum SECTOR_SPLIT_TYPE
{
	ST_NONE = 0,
	ST_SPLIT1 = 1,
	ST_SPLIT2 = 2
};

enum SECTOR_NOCOLLISION_TYPE
{
	NC_NONE = 0,
	NC_TRIANGLE1 = 1,
	NC_TRIANGLE2 = 2
};

struct SECTOR_PLANE
{
	float a;
	float b;
	float c;
};

struct SECTOR_COLLISION_INFO
{
	int split;
	int noCollision;
	SECTOR_PLANE planes[2];
};

typedef struct FLOOR_INFO
{
	int index;
	int box;
	int fx;
	int stopper;
	int pitRoom;
	int floor;
	int skyRoom;
	int ceiling;
	SECTOR_COLLISION_INFO floorCollision;
	SECTOR_COLLISION_INFO ceilingCollision;
};

typedef enum RoomEnumFlag
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

typedef struct ROOM_INFO
{
	int x;
	int y;
	int z;
	int minfloor;
	int maxceiling;
	std::vector<ROOM_VERTEX> vertices;
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
};

typedef struct ANIM_STRUCT
{
	short* framePtr;
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
constexpr auto NO_ROOM = 0xFF;
constexpr auto NO_HEIGHT = (-0x7F00);