#include "./CameraMatrixBuffer.hlsli"
#include "./VertexInput.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 WorldPosition: POSITION0;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR;
	float4 PositionCopy : TEXCOORD1;
	float4 FogBulbs : TEXCOORD2;
	float DistanceFog : FOG;
	float3 Tangent: TANGENT;
	float3 Binormal: BINORMAL;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);
SamplerState NormalTextureSampler : register(s1);

struct PixelShaderOutput
{
	float4 Normals: SV_TARGET0;
	float Depth: SV_TARGET1;
};

float3 UnpackNormalMap(float4 n)
{
	n = n * 2.0f - 1.0f;
	n.z = saturate(1.0f - dot(n.xy, n.xy));
	return n.xyz;
}

float3 PackNormal(float3 n)
{
	n = (n + 1.0f) * 0.5f;
	return n.xyz;
}

PixelShaderOutput PS(PixelShaderInput input)
{
	PixelShaderOutput output;

	float4 color = Texture.Sample(Sampler, input.UV);

	DoAlphaTest(color);

	float3x3 TBN = float3x3(input.Tangent, input.Binormal, input.Normal);
	float3 normal = UnpackNormalMap(NormalTexture.Sample(NormalTextureSampler, input.UV));
	normal = PackNormal(normalize(mul(normal, TBN)));

	output.Normals.xyz = normal;
	output.Normals.w = 1.0f;
	output.Depth = color.w > 0.0f ? input.PositionCopy.z / input.PositionCopy.w : 0.0f;

	return output;
}