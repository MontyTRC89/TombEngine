#include "./Math.hlsli"

cbuffer CameraMatrixBuffer : register(b0)
{
	float4x4 ViewProjection;
	float4x4 View;
	float4x4 Projection;
	float4x4 InverseProjection;
	float4 CamPositionWS;
	float4 CamDirectionWS;
	float2 ViewSize;
	float2 InvViewSize;
	unsigned int Frame;
	unsigned int RoomNumber;
	unsigned int CameraUnderwater;
	float4 FogColor;
	int FogMinDistance;
	int FogMaxDistance;
	float NearPlane;
	float FarPlane;		
	int NumFogBulbs;
	ShaderFogBulb FogBulbs[MAX_FOG_BULBS];
};