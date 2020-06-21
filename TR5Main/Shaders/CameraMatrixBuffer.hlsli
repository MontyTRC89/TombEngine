cbuffer CameraMatrixBuffer : register(b0)
{
	float4x4 ViewProjection;
	float4x4 View;
	float4x4 Projection;
	float3 WorldPosition;
	float3 WorldDirection;
	float2 ViewSize;
	float2 InvViewSize;
	unsigned int Frame;
	int CameraUnderwater;
};