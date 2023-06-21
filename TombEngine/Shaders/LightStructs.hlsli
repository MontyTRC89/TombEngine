#ifndef LIGHT_STRUCTS
#define LIGHT_STRUCTS

#define LT_SUN		0
#define LT_POINT	1
#define LT_SPOT		2
#define LT_SHADOW	3

#define MAX_LIGHTS_PER_ROOM	48
#define MAX_LIGHTS_PER_ITEM	8
#define MAX_FOG_BULBS	32
#define SPEC_FACTOR 64

struct ShaderLight
{
	float3 Position;
	unsigned int Type;
	float3 Color;
	float Intensity;
	float3 Direction;
	float In;
	float Out;
	float InRange;
	float OutRange;
	float Padding;
};

struct ShaderFogBulb
{
	float3 Position;
	float Density;
	float3 Color;
	float SquaredRadius;
	float3 FogBulbToCameraVector;
	float SquaredCameraToFogBulbDistance;
	float4 Padding2;
};

#endif