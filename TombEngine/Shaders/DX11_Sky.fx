#include "./CameraMatrixBuffer.hlsli"
#include "./Blending.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"

cbuffer StaticMatrixBuffer : register(b8)
{
	float4x4 World;
	float4 Color;
	float4 AmbientLight;
	int LightType;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float4 FogBulbs : TEXCOORD3;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition = mul(float4(input.Position, 1.0f), World);

	output.Position = mul(worldPosition, ViewProjection);
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;
	output.FogBulbs = DoFogBulbsForSky(worldPosition);

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	
	DoAlphaTest(output);
	
	output.xyz = output.xyz * Color;
	output.xyz -= float3(input.FogBulbs.w, input.FogBulbs.w, input.FogBulbs.w);
	output.xyz = saturate(output.xyz);
	output.xyz += saturate(input.FogBulbs.xyz);

	return output;
}