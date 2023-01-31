#include "./Math.hlsli"
#include "./CameraMatrixBuffer.hlsli"
#include "./ShaderLight.hlsli"
#include "./VertexEffects.hlsli"
#include "./VertexInput.hlsli"
#include "./Blending.hlsli"

#define INSTANCED_STATIC_MESH_BUCKET_SIZE 100

struct InstancedStaticMesh
{
	float4x4 World;
	float4 Color;
	float4 AmbientLight;
	ShaderLight InstancedStaticLights[MAX_LIGHTS_PER_ITEM];
	uint4 LightInfo;
};

cbuffer InstancedStaticMeshBuffer : register(b3)
{
	InstancedStaticMesh StaticMeshes[INSTANCED_STATIC_MESH_BUCKET_SIZE];
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float3 WorldPosition: POSITION;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float Sheen : SHEEN;
	float Fog : FOG;
	float4 PositionCopy: TEXCOORD2;
	uint InstanceID : SV_InstanceID;
};

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
	float4 Depth: SV_TARGET1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input, uint InstanceID : SV_InstanceID)
{
	PixelShaderInput output;

	float4 worldPosition = (mul(float4(input.Position, 1.0f), StaticMeshes[InstanceID].World));
	float3 normal = (mul(float4(input.Normal, 0.0f), StaticMeshes[InstanceID].World).xyz);

	output.Normal = normal;
	output.UV = input.UV;
	output.WorldPosition = worldPosition;

	float3 pos = Move(input.Position, input.Effects.xyz, input.Hash);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, input.Hash);

	output.Position = mul(worldPosition, ViewProjection);
	output.Color = float4(col, input.Color.w);
	output.Color *= StaticMeshes[InstanceID].Color;

	// Apply distance fog
	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	output.PositionCopy = output.Position;
	output.Sheen = input.Effects.w;
	output.InstanceID = InstanceID;

	return output;
}

PixelShaderOutput PS(PixelShaderInput input)
{
	PixelShaderOutput output;

	float4 tex = Texture.Sample(Sampler, input.UV);
	DoAlphaTest(tex);

	uint mode = StaticMeshes[input.InstanceID].LightInfo.y;
	uint numLights = StaticMeshes[input.InstanceID].LightInfo.x;

	float3 color = (mode == 0) ?
		CombineLights(
			StaticMeshes[input.InstanceID].AmbientLight.xyz,
			input.Color.xyz,
			tex.xyz, 
			input.WorldPosition, 
			normalize(input.Normal), 
			input.Sheen,
			StaticMeshes[input.InstanceID].InstancedStaticLights,
			numLights) :
		StaticLight(input.Color.xyz, tex.xyz);

	output.Color = float4(color, tex.w);

	output.Depth = tex.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);

	output.Color = DoFog(output.Color, FogColor, input.Fog);

	return output;
}