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
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float Fog : FOG;
	float4 PositionCopy: TEXCOORD2;
};

struct PixelShaderOutput
{
	float4 Color: SV_Target0;
	float4 Depth: SV_Target1;
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
	output.PositionCopy = output.Position;

	// Apply distance fog
	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

PixelShaderOutput PS(PixelShaderInput input) : SV_TARGET
{
	PixelShaderOutput output;

	output.Color = Texture.Sample(Sampler, input.UV);
	
	DoAlphaTest(output.Color);

	float3 colorMul = min(input.Color.xyz, 1.0f);
	output.Color.xyz = output.Color.xyz * colorMul.xyz;

	output.Depth = output.Color.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (FogMaxDistance != 0)
		output.Color.xyz = lerp(output.Color.xyz, FogColor, input.Fog);

	return output;
}