#pragma once
#include "Math/Math.h"

enum LIGHT_TYPES
{
	LIGHT_TYPE_SUN = 0,
	LIGHT_TYPE_POINT = 1,
	LIGHT_TYPE_SPOT = 2,
	LIGHT_TYPE_SHADOW = 3,
	LIGHT_TYPE_FOG_BULB = 4
};

enum class BlendMode
{
	Unknown = -1,
	Opaque = 0,
	AlphaTest = 1,
	Additive = 2,
	NoDepthTest = 4,
	Subtractive = 5,
	Wireframe = 6,
	Exclude = 8,
	Screen = 9,
	Lighten = 10,
	AlphaBlend = 11
};

enum CULL_MODES
{
	CULL_MODE_NONE = 0,
	CULL_MODE_CW = 1,
	CULL_MODE_CCW = 2,
	CULL_MODE_UNSET = -1
};

enum class ShadowMode
{
	None,
	Lara,
	All
};

enum class AntialiasingMode
{
	None,
	Low,
	Medium,
	High
};

enum LIGHT_MODES
{
	LIGHT_MODE_DYNAMIC,
	LIGHT_MODE_STATIC
};

enum DEPTH_STATES
{
	DEPTH_STATE_WRITE_ZBUFFER = 0,
	DEPTH_STATE_READ_ONLY_ZBUFFER = 1,
	DEPTH_STATE_NONE = 2,
	DEPTH_STATE_UNSET = -1
};

enum RENDERER_SPRITE_TYPE
{
	SPRITE_TYPE_BILLBOARD,
	SPRITE_TYPE_3D,
	SPRITE_TYPE_BILLBOARD_CUSTOM,
	SPRITE_TYPE_BILLBOARD_LOOKAT
};

enum RENDERER_POLYGON_SHAPE
{
	RENDERER_POLYGON_QUAD,
	RENDERER_POLYGON_TRIANGLE
};

enum RENDERER_FADE_STATUS
{
	NO_FADE,
	FADE_IN,
	FADE_OUT
};

enum class RendererDebugPage
{
	None,
	RendererStats,
	DimensionStats,
	PlayerStats,
	InputStats,
	CollisionStats,
	PathfindingStats,
	WireframeMode,

	Count
};

enum class TransparentFaceType
{
	Room,
	Moveable,
	Static,
	Sprite,
	None
};

enum class TextureRegister
{
	ColorMap = 0,
	NormalMap = 1,
	CausticsMap = 2,
	ShadowMap = 3,
	ReflectionMap = 4,
	Hud = 5,
	DepthMap = 6
};

enum class SamplerStateRegister
{
	None = 0,
	PointWrap = 1,
	LinearWrap = 2,
	LinearClamp = 3,
	AnisotropicWrap = 4,
	AnisotropicClamp = 5,
	ShadowMap = 6
};

enum class ConstantBufferRegister
{
	Camera = 0,
	Item = 1,
	InstancedStatics = 3,
	ShadowLight = 4,
	Room = 5,
	AnimatedTextures = 6,
	PostProcess = 7,
	Static = 8,
	Sprite = 9,
	Hud = 10,
	HudBar = 11,
	Blending = 12,
	InstancedSprites = 13
};

enum class AlphaTestModes
{
	None = 0,
	GreatherThan = 1,
	LessThan = 2
};

enum class PrintStringFlags
{
	Center	= (1 << 0),
	Blink	= (1 << 1),
	Right	= (1 << 2),
	Outline	= (1 << 3)
};

enum class RendererPass
{
	ShadowMap,
	Opaque,
	Transparent,
	CollectSortedFaces
};

enum class SpriteRenderType
{
	Default,
	LaserBarrier
};

constexpr auto TEXTURE_HEIGHT = 256;
constexpr auto TEXTURE_WIDTH = 256;
constexpr auto TEXTURE_PAGE = (TEXTURE_HEIGHT * TEXTURE_WIDTH);

#define TEXTURE_ATLAS_SIZE 4096
#define TEXTURE_PAGE_SIZE 262144
#define NUM_TEXTURE_PAGES_PER_ROW 16
#define MAX_SHADOW_MAPS 8
#define GET_ATLAS_PAGE_X(p) ((p) % NUM_TEXTURE_PAGES_PER_ROW) * 256.0f
#define GET_ATLAS_PAGE_Y(p) floor((p) / NUM_TEXTURE_PAGES_PER_ROW) * 256.0f
#define SHAPE_RECTANGLE 0
#define SHAPE_TRIANGLE	1
#define MAX_VERTICES 200000
#define MAX_INDICES 400000
#define MAX_RECTS_2D 32
#define MAX_LINES_2D 256
#define MAX_LINES_3D 16384
#define NUM_BUCKETS 4
#define AMBIENT_CUBE_MAP_SIZE 64
#define NUM_SPRITES_PER_BUCKET 4096
#define NUM_LINES_PER_BUCKET 4096
#define NUM_CAUSTICS_TEXTURES	16
#define PRINTSTRING_COLOR_ORANGE D3DCOLOR_ARGB(255, 216, 117, 49)
#define PRINTSTRING_COLOR_WHITE D3DCOLOR_ARGB(255, 255, 255, 255)
#define PRINTSTRING_COLOR_BLACK D3DCOLOR_ARGB(255, 0, 0, 0)
#define PRINTSTRING_COLOR_YELLOW D3DCOLOR_ARGB(255, 240, 220, 32)

constexpr auto FADE_FRAMES_COUNT = 16;
constexpr auto FADE_FACTOR = 0.0625f;

constexpr auto NUM_LIGHTS_PER_BUFFER = 48;
constexpr auto MAX_LIGHTS_PER_ITEM = 8;
constexpr auto MAX_LIGHTS_PER_ROOM = 48;
constexpr auto MAX_LIGHTS = 100;
constexpr auto AMBIENT_LIGHT_INTERPOLATION_STEP = 1.0f / 10.0f;
constexpr auto MAX_DYNAMIC_SHADOWS = 1;
constexpr auto MAX_DYNAMIC_LIGHTS = 1024;
constexpr auto ITEM_LIGHT_COLLECTION_RADIUS = BLOCK(1);
constexpr auto CAMERA_LIGHT_COLLECTION_RADIUS = BLOCK(4);

constexpr auto MAX_TRANSPARENT_FACES = 16384;
constexpr auto MAX_TRANSPARENT_VERTICES = (MAX_TRANSPARENT_FACES * 6);
constexpr auto MAX_TRANSPARENT_FACES_PER_ROOM = 16384;
constexpr auto TRANSPARENT_BUCKET_SIZE = (3840 * 16);
constexpr auto ALPHA_TEST_THRESHOLD = 0.5f;
constexpr auto FAST_ALPHA_BLEND_THRESHOLD = 0.5f;

constexpr auto MAX_BONES = 32;

constexpr auto SCREEN_SPACE_RES	   = Vector2(800.0f, 600.0f);
constexpr auto REFERENCE_FONT_SIZE = 35.0f;
constexpr auto HUD_ZERO_Y		   = -SCREEN_SPACE_RES.y;

constexpr auto UNDERWATER_FOG_MIN_DISTANCE = 4;
constexpr auto UNDERWATER_FOG_MAX_DISTANCE = 30;
constexpr auto MAX_ROOM_BOUNDS = 256;

constexpr auto MIN_FAR_VIEW = 3200.0f;
constexpr auto DEFAULT_FAR_VIEW = 102400.0f;

constexpr auto INSTANCED_SPRITES_BUCKET_SIZE = 512;

constexpr auto SKY_TILES_COUNT = 20;
constexpr auto SKY_SIZE = 10240.0f;
constexpr auto SKY_VERTICES_COUNT = 4 * SKY_TILES_COUNT * SKY_TILES_COUNT;
constexpr auto SKY_INDICES_COUNT = 6 * SKY_TILES_COUNT * SKY_TILES_COUNT;
constexpr auto SKY_TRIANGLES_COUNT = 2 * SKY_TILES_COUNT * SKY_TILES_COUNT;

constexpr auto MAX_ROOMS_DRAW = 256;
constexpr auto MAX_STATICS_DRAW = 128;
constexpr auto MAX_EFFECTS_DRAW = 16;
constexpr auto MAX_ITEMS_DRAW = 128;
constexpr auto MAX_LIGHTS_DRAW = 48;
constexpr auto MAX_FOG_BULBS_DRAW = 32;
constexpr auto MAX_SPRITES_DRAW = 512;

// FUTURE
/*
#define CBUFFER_CAMERA 0
#define CBUFFER_BLENDING 1
#define CBUFFER_ANIMATED_TEXTURES 2

#define CBUFFER_LIGHTS 7
#define CBUFFER_ITEM 8
#define CBUFFER_STATIC 8
#define CBUFFER_INSTANCED_STATICS 8

#define CBUFFER_ROOM 7
#define CBUFFER_SHADOW_LIGHT 8

#define CBUFFER_SPRITE 7
#define CBUFFER_INSTANCED_SPRITES 8

#define CBUFFER_HUD 7
#define CBUFFER_HUD_BAR 8

#define CBUFFER_POSTPROCESS 7
*/