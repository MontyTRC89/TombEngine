cbuffer CameraMatrixBuffer : register(b0)
{
	float4x4 ViewProjection;
	float4x4 View;
	float4x4 Projection;
	float4 CamPositionWS;
	float4 CamDirectionWS;
	float2 ViewSize;
	float2 InvViewSize;
	uint Frame;
	uint RoomNumber;
	uint CameraUnderwater;
	float4 FogColor;
	int FogMinDistance;
	int FogMaxDistance;
};