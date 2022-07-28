#include "./CameraMatrixBuffer.hlsli"
#include "./AlphaTestBuffer.hlsli"
#include "./VertexInput.hlsli"
#include "./ShaderLight.hlsli"
cbuffer ItemBuffer : register(b1)
{
	float4x4 World;
	float4x4 Bones[32];
	float4 ItemPosition;
	float4 AmbientLight;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float3 WorldPosition : POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
    float Sheen : SHEEN;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	output.Position = mul(mul(float4(input.Position, 1.0f), World), ViewProjection);
	output.Normal = (mul(float4(input.Normal, 0.0f), World).xyz);
	output.Color = input.Color;
	output.UV = input.UV;
	output.WorldPosition = (mul(float4(input.Position, 1.0f), World).xyz);
    output.Sheen = input.Effects.w;
	return output;
}

float4 PS(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);

	DoAlphaTest(output);
    ShaderLight l[2];
    l[0].Color = float3(1.0f, 1.0f, 0.5f) * 0.6f;
    l[0].Type = LT_SUN;
    l[0].Direction = normalize(float3(-1.0f, -0.707f, -0.5f));
    l[1].Color = float3(0.5f, 0.5f, 1.0f) * 0.2f;
    l[1].Type = LT_SUN;
    l[1].Direction = normalize(float3(1.0f, 0.707f, -0.5f));
    for (int i = 0; i < 2; i++)
    {
        output.xyz += DoDirectionalLight(input.WorldPosition, input.Normal, l[i]);
        output.xyz += DoSpecularSun(input.Normal, l[i], input.Sheen);
    }

	return output;
}