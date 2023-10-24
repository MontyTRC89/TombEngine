#include "./CameraMatrixBuffer.hlsli"
#include "./Blending.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"

// NOTE: This shader is used for all opaque or not sorted transparent sprites, that can be instanced for a faster drawing

#define INSTANCED_SPRITES_BUCKET_SIZE 512

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float4 PositionCopy: TEXCOORD2;
	float4 FogBulbs : TEXCOORD3;
	float DistanceFog : FOG;
	uint InstanceID : SV_InstanceID;
};

struct InstancedSprite
{
	float4x4 World;
	float4 UV[2];
	float4 Color;
	float IsBillboard;
	float IsSoftParticle;
};

cbuffer InstancedSpriteBuffer : register(b13)
{
	InstancedSprite Sprites[INSTANCED_SPRITES_BUCKET_SIZE];
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

PixelShaderInput VS(VertexShaderInput input, uint InstanceID : SV_InstanceID)
{
	PixelShaderInput output;

	float4 worldPosition;

	if (Sprites[InstanceID].IsBillboard == 1)
	{
		worldPosition = mul(float4(input.Position, 1.0f), Sprites[InstanceID].World);
		output.Position = mul(mul(float4(input.Position, 1.0f), Sprites[InstanceID].World), ViewProjection);
	}
	else
	{
		worldPosition = float4(input.Position, 1.0f);
		output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
	}

	output.PositionCopy = output.Position;
	output.Color = Sprites[InstanceID].Color;
	output.UV = float2(Sprites[InstanceID].UV[0][input.PolyIndex], Sprites[InstanceID].UV[1][input.PolyIndex]);
	output.InstanceID  = InstanceID;

	output.FogBulbs = DoFogBulbsForVertex(worldPosition);
	output.DistanceFog = DoDistanceFogForVertex(worldPosition);

	return output;
}

// TODO: From NVIDIA SDK, check if it can be useful instead of linear ramp
float Contrast(float Input, float ContrastPower)
{
#if 1
	//piecewise contrast function
	bool IsAboveHalf = Input > 0.5;
	float ToRaise = saturate(2 * (IsAboveHalf ? 1 - Input : Input));
	float Output = 0.5 * pow(ToRaise, ContrastPower);
	Output = IsAboveHalf ? 1 - Output : Output;
	return Output;
#else
	// another solution to create a kind of contrast function
	return 1.0 - exp2(-2 * pow(2.0 * saturate(Input), ContrastPower));
#endif
}

#ifdef TRANSPARENT
[earlydepthstencil]
float4 PS(PixelShaderInput input) : SV_Target
#else
float4 PS(PixelShaderInput input) : SV_TARGET
#endif
{
	float4 outputColor = Texture.Sample(Sampler, input.UV) * input.Color;

	if (Sprites[input.InstanceID].IsSoftParticle == 1)
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

	return outputColor;

#else

	return outputColor;

#endif
}