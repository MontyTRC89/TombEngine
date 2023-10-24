#include "./CameraMatrixBuffer.hlsli"
#include "./Blending.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"

// NOTE: This shader is used for all 3D and alpha blended sprites, because we send aleady transformed vertices to the GPU 
// instead of instances

#define FADE_FACTOR .789f

cbuffer SpriteBuffer : register(b9)
{
	float IsSoftParticle;
	int RenderType;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float4 PositionCopy: TEXCOORD2;
	float4 FogBulbs : TEXCOORD3;
	float DistanceFog : FOG;
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

#endif

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

	output.FogBulbs = DoFogBulbsForVertex(worldPosition);
	output.DistanceFog = DoDistanceFogForVertex(worldPosition);

	return output;
}

#ifdef TRANSPARENT
[earlydepthstencil]
float PS(PixelShaderInput input) : SV_Target
#else
float4 PS(PixelShaderInput input) : SV_TARGET
#endif
{
	float4 outputColor = Texture.Sample(Sampler, input.UV) * input.Color;

	DoAlphaTest(outputColor);

	if (IsSoftParticle == 1)
	{
		float particleDepth = input.PositionCopy.z / input.PositionCopy.w;
		float2 position = input.PositionCopy.xy / input.PositionCopy.w;
		float2 texCoord = 0.5f * (float2(position.x, -position.y) + 1);
		float sceneDepth = DepthMap.Sample(DepthMapSampler, texCoord).r;

		sceneDepth = LinearizeDepth(sceneDepth, NearPlane, FarPlane);
		particleDepth = LinearizeDepth(particleDepth, NearPlane, FarPlane);

		if (particleDepth - sceneDepth > 0.01f)
			discard;

		float fade = (sceneDepth - particleDepth) * 1024.0f;
		outputColor.w = min(outputColor.w, fade);
	}

	if (RenderType == 1)
	{
		outputColor = DoLaserBarrierEffect(input.Position, outputColor, input.UV, FADE_FACTOR, Frame);
	}

	outputColor.xyz -= float3(input.FogBulbs.w, input.FogBulbs.w, input.FogBulbs.w);
	outputColor.xyz = saturate(outputColor.xyz);
	
	outputColor = DoDistanceFogForPixel(outputColor, float4(0.0f, 0.0f, 0.0f, 0.0f), input.DistanceFog);

	float outputDepth = outputColor.w > 0.0f ?
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

	return outputColor;

#endif
}
