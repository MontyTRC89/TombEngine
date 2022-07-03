#include "./CameraMatrixBuffer.hlsli"
#include "./AlphaTestBuffer.hlsli"
#include "./VertexInput.hlsli"

cbuffer SpriteBuffer: register(b9)
{
	float4x4 billboardMatrix;
	float4 color;
	bool isBillboard;
}

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float Fog : FOG;
	float4 PositionCopy: TEXCOORD2;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D DepthMap : register(t6);
SamplerState DepthMapSampler : register(s6);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition;

	if (isBillboard) 
	{
		worldPosition = mul(float4(input.Position, 1.0f), billboardMatrix);
		output.Position = mul(mul(float4(input.Position, 1.0f), billboardMatrix), ViewProjection);
	} else 
	{
		worldPosition = float4(input.Position, 1.0f);
		output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	}

	output.PositionCopy = output.Position;
	
	output.Normal = input.Normal;
	output.Color = input.Color * color;
	output.UV = input.UV;

	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV) * input.Color;

	float particleDepth = input.PositionCopy.z / input.PositionCopy.w;
	input.PositionCopy.xy /= input.PositionCopy.w;
	float2 texCoord = 0.5f * (float2(input.PositionCopy.x, -input.PositionCopy.y) + 1);
	float sceneDepth = DepthMap.Sample(DepthMapSampler, texCoord).r;

	if (particleDepth > sceneDepth)
		discard;

	float fade = (sceneDepth - particleDepth) * 300.0F;
	output.w = min(output.w, fade);

	if (FogMaxDistance != 0)
		output.xyz = lerp(output.xyz, FogColor, input.Fog);

	return output;
}