#include "./Math.hlsli"
#include "./CameraMatrixBuffer.hlsli"
#include "./ShaderLight.hlsli"
#include "./VertexEffects.hlsli"
#include "./VertexInput.hlsli"
#include "./AlphaTestBuffer.hlsli"

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
	float3 WorldPosition: POSITION;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float Sheen: SHEEN;
	float Fog: FOG;
	float4 PositionCopy: TEXCOORD2;
};

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
	float4 Depth: SV_TARGET1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition = (mul(float4(input.Position, 1.0f), World));
	float3 normal = (mul(float4(input.Normal, 0.0f), World).xyz);

	output.Normal = normal;
	output.UV = input.UV;
	output.WorldPosition = worldPosition;
	
	float3 pos = Move(input.Position, input.Effects.xyz, input.Hash);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, input.Hash);
	
	output.Position = mul(worldPosition, ViewProjection);
	output.Color = float4(col, input.Color.w);
	output.Color *= Color;

	// Apply distance fog
	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);
	
	output.PositionCopy = output.Position;
    output.Sheen = input.Effects.w;
	return output;
}

PixelShaderOutput PS(PixelShaderInput input)
{
	PixelShaderOutput output;

	float4 tex = Texture.Sample(Sampler, input.UV);
    DoAlphaTest(tex);

	float3 color = (LightType == 0) ?
		CombineLights(AmbientLight.xyz, input.Color.xyz, tex.xyz, input.WorldPosition, normalize(input.Normal), input.Sheen) :
		StaticLight(input.Color.xyz, tex.xyz);

	output.Color = float4(color, tex.w);

	output.Depth = tex.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (FogMaxDistance != 0)
		output.Color.xyz = lerp(output.Color.xyz, FogColor.xyz, input.Fog);

	return output;
}