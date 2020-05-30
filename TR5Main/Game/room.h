#pragma once

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

typedef struct tr5_room_vertex
{
	tr5_vertex Vertex;		// Vertex is now floating-point
	tr5_vertex Normal;
	DWORD Colour;			// 32-bit colour
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

typedef struct tr_room_portal  // 32 bytes
{
	short AdjoiningRoom; // Which room this portal leads to
	TR_VERTEX Normal;
	TR_VERTEX Vertices[4];
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

typedef struct tr5_room_light
{
	float x, y, z;       // Position of light, in world coordinates
	float r, g, b;       // Colour of the light
	int Separator;		 // Dummy value = 0xCDCDCDCD
	float In;            // Cosine of the IN value for light / size of IN value
	float Out;           // Cosine of the OUT value for light / size of OUT value
	float RadIn;         // (IN radians) * 2
	float RadOut;        // (OUT radians) * 2
	float Range;         // Range of light
	float dx, dy, dz;    // Direction - used only by sun and spot lights
	int x2, y2, z2;		 // Same as position, only in integer.
	int dx2, dy2, dz2;	 // Same as direction, only in integer.
	byte LightType;
	byte Filler[3];      // Dummy values = 3 x 0xCD
};

typedef struct MESH_INFO
{
	int x;
	int y;
	int z;
	short yRot;
	short shade;
	short Flags;
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

typedef struct FLOOR_INFO
{
	unsigned short index;
	unsigned short fx : 4;
	unsigned short box : 11;
	unsigned short stopper : 1;
	unsigned char pitRoom;
	signed char floor;
	unsigned char skyRoom;
	signed char ceiling;
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
	short* data;
	short* door;
	FLOOR_INFO* floor;
	void* something;
	MESH_INFO* mesh;
	int x;
	int y;
	int z;
	int minfloor;
	int maxceiling;
	short xSize;
	short ySize;
	CVECTOR ambient;
	short numLights;
	short numMeshes;
	unsigned char reverbType;
	unsigned char flipNumber;
	byte meshEffect;
	byte boundActive;
	short left;
	short right;
	short top;
	short bottom;
	short testLeft;
	short testRight;
	short testTop;
	short testBottom;
	short itemNumber;
	short fxNumber;
	short flippedRoom;
	unsigned short flags; // ENV_FLAG_enum
	unsigned int Unknown1;
	unsigned int Unknown2;     // Always 0
	unsigned int Unknown3;     // Always 0
	unsigned int Separator;    // 0xCDCDCDCD
	unsigned short Unknown4;
	unsigned short Unknown5;
	float RoomX;
	float RoomY;
	float RoomZ;
	unsigned int Separator1[4]; // Always 0xCDCDCDCD
	unsigned int Separator2;    // 0 for normal rooms and 0xCDCDCDCD for null rooms
	unsigned int Separator3;    // Always 0xCDCDCDCD
	unsigned int NumRoomTriangles;
	unsigned int NumRoomRectangles;
	tr5_room_light* light;     // Always 0
	unsigned int LightDataSize;
	unsigned int NumLights2;    // Always same as NumLights
	unsigned int Unknown6;
	int RoomYTop;
	int RoomYBottom;
	unsigned int NumLayers;
	tr5_room_layer* LayerOffset;
	tr5_room_vertex* VerticesOffset;
	void* PolyOffset;
	void* PolyOffset2;   // Same as PolyOffset
	int NumVertices;
	int Separator5[4];  // Always 0xCDCDCDCD
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