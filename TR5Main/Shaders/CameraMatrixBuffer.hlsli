cbuffer CameraMatrixBuffer : register(b0)
{
	float4x4 ViewProjection;
	float4x4 View;
	float4x4 Projection;
	float3 CamPositionWS;
	float3 CamDirectionWS;
	float2 ViewSize;
	float2 InvViewSize;
	unsigned int Frame;
	int CameraUnderwater;
	int RoomNumber;
};