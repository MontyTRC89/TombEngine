#include "./Math.hlsli"
#include "./CameraMatrixBuffer.hlsli"
#include "./ShaderLight.hlsli"
#include "./VertexEffects.hlsli"
#include "./VertexInput.hlsli"
#include "./Blending.hlsli"
#include "./AnimatedTextures.hlsli"
//#include "./Shadows.hlsli"

#define MAX_BONES 32

cbuffer ItemBuffer : register(b1) 
{
	float4x4 World;
	float4x4 Bones[MAX_BONES];
	float4 Color;
	float4 AmbientLight;
	int4 BoneLightModes[MAX_BONES / 4];
	ShaderLight ItemLights[MAX_LIGHTS_PER_ITEM];
	int NumItemLights;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float3 WorldPosition: POSITION;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float Sheen: SHEEN;
	float4 PositionCopy: TEXCOORD2;
	float4 FogBulbs : TEXCOORD3;
	float DistanceFog : FOG;
	unsigned int Bone: BONE;
};

#ifdef TRANSPARENT

struct PixelAndLinkBufferData
{
	uint PixelColorRG;
	uint PixelColorBA;
	uint PixelDepthAndBlendMode;
	uint NextNode;
};

RWStructuredBuffer<PixelAndLinkBufferData> FLBuffer : register(u2);

RWByteAddressBuffer StartOffsetBuffer : register(u3);

#else

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
	float4 Depth: SV_TARGET1;
};

#endif

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);
SamplerState NormalSampler : register(s1);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4x4 world = mul(Bones[input.Bone], World);

	float3 normal = (mul(float4(input.Normal, 0.0f), world).xyz);
	float3 worldPosition = (mul(float4(input.Position, 1.0f), world).xyz);

	output.Normal = normal;
	output.UV = input.UV;
	output.WorldPosition = worldPosition;

	// Calculate vertex effects
	float wibble = Wibble(input.Effects.xyz, input.Hash);
	float3 pos = Move(input.Position, input.Effects.xyz, wibble);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, wibble);
	
	output.Position = mul(mul(float4(pos, 1.0f), world), ViewProjection);
	output.Color = float4(col, input.Color.w);
	output.Color *= Color;
	output.PositionCopy = output.Position;
    output.Sheen = input.Effects.w;
	output.Bone = input.Bone;

	output.FogBulbs = DoFogBulbsForVertex(worldPosition);
	output.DistanceFog = DoDistanceFogForVertex(worldPosition);

	return output;
}

#ifdef TRANSPARENT
[earlydepthstencil]
float PS(PixelShaderInput input) : SV_Target
#else
PixelShaderOutput PS(PixelShaderInput input)
#endif
{
	if (Type == 1)
		input.UV = CalculateUVRotate(input.UV, 0);

	float4 tex = Texture.Sample(Sampler, input.UV);	
    DoAlphaTest(tex);

	float3 normal = normalize(input.Normal);

	float3 color = (BoneLightModes[input.Bone / 4][input.Bone % 4] == 0) ?
		CombineLights(
			AmbientLight.xyz,
			input.Color.xyz,
			tex.xyz, 
			input.WorldPosition,
			normal, 
			input.Sheen,
			ItemLights, 
			NumItemLights,
			input.FogBulbs.w) :
		StaticLight(input.Color.xyz, tex.xyz, input.FogBulbs.w);

	float4 outputColor = saturate(float4(color, tex.w));
	outputColor = DoFogBulbsForPixel(outputColor, float4(input.FogBulbs.xyz, 1.0f));
	outputColor = DoDistanceFogForPixel(outputColor, FogColor, input.DistanceFog);

	float outputDepth = tex.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);

#ifdef TRANSPARENT

	uint uPixelCount = FLBuffer.IncrementCounter();
	// Exchange offsets in StartOffsetBuffer
	float4 vPos = (input.PositionCopy);
	vPos.xy /= vPos.w;
	float2 pos = 0.5f * (float2(vPos.x, vPos.y) + 1);
	int posX = (int)(pos.x * ViewSize.x);
	int posY = (int)(pos.y * ViewSize.y);

	uint uStartOffsetAddress = 4 * ((ViewSize.x * posY) + posX);
	uint uOldStartOffset;
	StartOffsetBuffer.InterlockedExchange(
		uStartOffsetAddress, uPixelCount, uOldStartOffset);
	// Add new fragment entry in Fragment & Link Buffer
	PixelAndLinkBufferData Element;
	Element.PixelColorRG = ((uint)(outputColor.x * 255.0f) << 16) | ((uint)(outputColor.y * 255.0f) & 0xFFFF);
	Element.PixelColorBA = ((uint)(outputColor.z * 255.0f) << 16) | ((uint)(outputColor.w * 255.0f) & 0xFFFF);
	Element.PixelDepthAndBlendMode = ((BlendMode & 0x0F) << 24) | (uint)(outputDepth * 16777215);
	Element.NextNode = uOldStartOffset;
	FLBuffer[uPixelCount] = Element;

	return 0;

#else

	PixelShaderOutput output;

	output.Depth = outputDepth;
	output.Color = outputColor;

	return output;

#endif
}