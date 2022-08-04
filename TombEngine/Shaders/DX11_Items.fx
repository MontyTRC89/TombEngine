#include "./Math.hlsli"
#include "./CameraMatrixBuffer.hlsli"
#include "./ShaderLight.hlsli"
#include "./VertexEffects.hlsli"
#include "./VertexInput.hlsli"
#include "./AlphaTestBuffer.hlsli"
#include "./AnimatedTextures.hlsli"
#include "./Shadows.hlsli"

#define MAX_BONES 32

cbuffer ItemBuffer : register(b1) 
{
	float4x4 World;
	float4x4 Bones[MAX_BONES];
	float4 Color;
	float4 AmbientLight;
	int4 BoneLightModes[MAX_BONES / 4];
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float3 WorldPosition: POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float Sheen: SHEEN;
	float3x3 TBN: TBN;
	float Fog: FOG;
	float4 PositionCopy: TEXCOORD2;
	uint Bone: BONE;
};

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
	float4 Depth: SV_TARGET1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);

//TextureCube Reflection : register (t4);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4x4 world = mul(Bones[input.Bone], World);

	float3 normal = (mul(float4(input.Normal, 0.0f), world).xyz);
	float3 worldPosition = (mul(float4(input.Position, 1.0f), world).xyz);

	output.Normal = normal;
	output.UV = input.UV;
	output.WorldPosition = worldPosition;
	
	float3 Tangent = mul(float4(input.Tangent, 0), world).xyz;
    float3 Bitangent = cross(normal, Tangent);
	float3x3 TBN = float3x3(Tangent, Bitangent, normal);

	output.TBN = transpose(TBN);

	// Calculate vertex effects
	float wibble = Wibble(input.Effects.xyz, input.Hash);
	float3 pos = Move(input.Position, input.Effects.xyz, wibble);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, wibble);
	
	output.Position = mul(mul(float4(pos, 1.0f), world), ViewProjection);
	output.Color = float4(col, input.Color.w);
	output.Color *= Color;

	// Apply distance fog
	float d = distance(CamPositionWS.xyz, worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);
	
	output.PositionCopy = output.Position;
    output.Sheen = input.Effects.w;
	output.Bone = input.Bone;
	return output;
}

PixelShaderOutput PS(PixelShaderInput input)
{
	PixelShaderOutput output;

	if (Type == 1)
		input.UV = CalculateUVRotate(input.UV, 0);

	float4 tex = Texture.Sample(Sampler, input.UV);	
    DoAlphaTest(tex);

	float3 normal = NormalTexture.Sample(Sampler, input.UV).rgb;
	normal = normal * 2 - 1;
	normal = normalize(mul(input.TBN, normal));

	float3 color = (BoneLightModes[input.Bone / 4][input.Bone % 4] == 0) ?
		CombineLights(AmbientLight.xyz, input.Color.xyz, tex.xyz, input.WorldPosition, normal, input.Sheen) :
		StaticLight(input.Color.xyz, tex.xyz);

	output.Color = saturate(float4(color, tex.w));

	output.Depth = tex.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	if (FogMaxDistance != 0)
		output.Color.xyz = lerp(output.Color.xyz, FogColor.xyz, input.Fog);
	
	return output;
}