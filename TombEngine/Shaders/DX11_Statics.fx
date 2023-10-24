#include "./Math.hlsli"
#include "./CameraMatrixBuffer.hlsli"
#include "./ShaderLight.hlsli"
#include "./VertexEffects.hlsli"
#include "./VertexInput.hlsli"
#include "./Blending.hlsli"

cbuffer StaticMatrixBuffer : register(b8)
{
	float4x4 World;
	float4 Color;
	float4 AmbientLight;
	ShaderLight StaticLights[MAX_LIGHTS_PER_ITEM];
	int NumStaticLights;
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

#else

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
	float4 Depth: SV_TARGET1;
};

#endif

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float3 normal = (mul(float4(input.Normal, 0.0f), World).xyz);

	float wibble = Wibble(input.Effects.xyz, input.Hash);
	float3 pos = Move(input.Position, input.Effects.xyz, wibble);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, input.Hash);
	
	float4 worldPosition = (mul(float4(pos, 1.0f), World));

	output.Position = mul(worldPosition, ViewProjection);
	output.Normal = normal;
	output.UV = input.UV;
	output.WorldPosition = worldPosition;
	output.Color = float4(col, input.Color.w);
	output.Color *= Color;

	output.FogBulbs = DoFogBulbsForVertex(worldPosition);
	output.DistanceFog = DoDistanceFogForVertex(worldPosition);

	output.PositionCopy = output.Position;
    output.Sheen = input.Effects.w;

	return output;
}

#ifdef TRANSPARENT
[earlydepthstencil]
float PS(PixelShaderInput input) : SV_Target
#else
PixelShaderOutput PS(PixelShaderInput input)
#endif
{
	float4 tex = Texture.Sample(Sampler, input.UV);
    DoAlphaTest(tex);

	float3 normal = normalize(input.Normal);

	float3 color = (LightType == 0) ?
		CombineLights(
			AmbientLight.xyz, 
			input.Color.xyz, 
			tex.xyz, 
			input.WorldPosition, 
			normal, 
			input.Sheen, 
			StaticLights, 
			NumStaticLights,
			input.FogBulbs.w) :
		StaticLight(input.Color.xyz, tex.xyz, input.FogBulbs.w);

	float4 outputColor = float4(color, tex.w);
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
	outputDepth = 0;
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