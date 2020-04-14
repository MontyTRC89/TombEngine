#include "./CameraMatrixBuffer.hlsli"

struct VertexShaderInput
{
	float3 Position: POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float Bone : BLENDINDICES;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float4 Color: COLOR;
};

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	output.Color = input.Color;

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	return input.Color;
}