#include "./../VertexInput.hlsli"

cbuffer HUDBuffer : register(b10)
{
	float4x4 View;
	float4x4 Projection;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;
	output.Position = mul(mul(float4(input.Position, 1.0f), View),Projection); 
	output.Color = input.Color;
	output.UV = input.UV;
	return output;
}