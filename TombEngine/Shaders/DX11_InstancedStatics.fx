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
	float4 PositionCopy: TEXCOORD2;
	float4 FogBulbs : TEXCOORD3;
	float DistanceFog : FOG;
	uint InstanceID : SV_InstanceID;
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

PixelShaderInput VS(VertexShaderInput input, uint InstanceID : SV_InstanceID)
{
	PixelShaderInput output;

	float3 normal = (mul(float4(input.Normal, 0.0f), StaticMeshes[InstanceID].World).xyz);

	float wibble = Wibble(input.Effects.xyz, input.Hash);
	float3 pos = Move(input.Position, input.Effects.xyz, wibble);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, input.Hash);

	float4 worldPosition = (mul(float4(pos, 1.0f), StaticMeshes[InstanceID].World));

	output.Position = mul(worldPosition, ViewProjection);
	output.Normal = normal;
	output.UV = input.UV;
	output.WorldPosition = worldPosition;
	output.Color = float4(col, input.Color.w);
	output.Color *= StaticMeshes[InstanceID].Color;
	output.PositionCopy = output.Position;
	output.Sheen = input.Effects.w;
	output.InstanceID = InstanceID;

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
	float4 tex = Texture.Sample(Sampler, input.UV);
	DoAlphaTest(tex);

	uint mode = StaticMeshes[input.InstanceID].LightInfo.y;
	uint numLights = StaticMeshes[input.InstanceID].LightInfo.x;

	float3 normal = normalize(input.Normal);

	float3 color = (mode == 0) ?
		CombineLights(
			StaticMeshes[input.InstanceID].AmbientLight.xyz,
			input.Color.xyz,
			tex.xyz, 
			input.WorldPosition, 
			normal, 
			input.Sheen,
			StaticMeshes[input.InstanceID].InstancedStaticLights,
			numLights,
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