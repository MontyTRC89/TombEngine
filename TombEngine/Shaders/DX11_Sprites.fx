#include "./CameraMatrixBuffer.hlsli"
#include "./Blending.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"

// NOTE: This shader is used for all 3D and alpha blended sprites, because we send aleady transformed vertices to the GPU 
// instead of instances

cbuffer SpriteBuffer : register(b9)
{
	float IsSoftParticle;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float4 PositionCopy: TEXCOORD2;
	float4 Fog : TEXCOORD3;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D DepthMap : register(t6);
SamplerState DepthMapSampler : register(s6);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition = float4(input.Position, 1.0f);

	output.Position = mul(worldPosition, ViewProjection);
	output.PositionCopy = output.Position;	
	output.Normal = input.Normal;
	output.Color = input.Color;
	output.UV = input.UV;

	// Apply fog
	output.Fog = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (FogMaxDistance != 0)
	{
		float d = length(CamPositionWS.xyz - worldPosition);
		float fogFactor = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);
		output.Fog.xyz = FogColor.xyz * fogFactor;
		output.Fog.w = fogFactor;
	}

	output.Fog = float4(0, 0, 0, 0);
	for (int i = 0; i < NumFogBulbs; i++)
	{
		float fogFactor = DoFogBulb(worldPosition, FogBulbs[i]);
		output.Fog.xyz += FogBulbs[i].Color.xyz * fogFactor;
		output.Fog.w += fogFactor;
		if (output.Fog.w >= 1.0f)
		{
			break;
		}
	}

	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV) * input.Color;

	DoAlphaTest(output);

	if (IsSoftParticle == 1)
	{
		float particleDepth = input.PositionCopy.z / input.PositionCopy.w;
		input.PositionCopy.xy /= input.PositionCopy.w;
		float2 texCoord = 0.5f * (float2(input.PositionCopy.x, -input.PositionCopy.y) + 1);
		float sceneDepth = DepthMap.Sample(DepthMapSampler, texCoord).r;

		sceneDepth = LinearizeDepth(sceneDepth, NearPlane, FarPlane);
		particleDepth = LinearizeDepth(particleDepth, NearPlane, FarPlane);

		if (particleDepth - sceneDepth > 0.01f)
			discard;

		float fade = (sceneDepth - particleDepth) * 1024.0f;
		output.w = min(output.w, fade);
	}

	output.xyz -= float3(input.Fog.w, input.Fog.w, input.Fog.w) * 0.5f;
	output.xyz = saturate(output.xyz);
	output.xyz += saturate(input.Fog.xyz);

	return output;
}