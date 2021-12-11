
#define LT_SUN		0
#define LT_POINT	1
#define LT_SPOT		2
#define LT_SHADOW	3

#define MAX_LIGHTS	48

struct ShaderLight {
	float3 Position;
	int Type;
	float3 Color;
	float LocalIntensity;
	float3 Direction;
	float Distance;
	float Intensity;
	float In;
	float Out;
	float Range;
};
