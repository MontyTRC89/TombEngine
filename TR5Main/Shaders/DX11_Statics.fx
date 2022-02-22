#include "./CameraMatrixBuffer.hlsli"
#include "./VertexInput.hlsli"
#include "./AlphaTestBuffer.hlsli"

cbuffer StaticMatrixBuffer : register(b8)
{
	float4x4 World;
	float4 Position;
	float4 Color;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float Fog : FOG;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition = mul(float4(input.Position, 1.0f), World);

	output.Position = mul(worldPosition, ViewProjection); 
	output.Normal = input.Normal;
	output.Color = input.Color * Color;
	output.UV = input.UV;

	// Apply distance fog
	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	
	DoAlphaTest(output);

	float3 colorMul = min(input.Color.xyz, 1.0f);
	output.xyz = output.xyz * colorMul.xyz;

	if (FogMaxDistance != 0)
		output.xyz = lerp(output.xyz, FogColor, input.Fog);

	return output;
}