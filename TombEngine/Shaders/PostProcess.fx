#include "./CBPostProcess.hlsli"
#include "./CBCamera.hlsli"
#include "./Math.hlsli"

struct VertexShaderInput
{
    float3 Position: POSITION0;
    float2 UV: TEXCOORD0;
    float4 Color: COLOR0;
};

struct PixelShaderInput
{
    float4 Position: SV_POSITION;
    float2 UV: TEXCOORD0;
    float4 Color: COLOR0;
    float4 PositionCopy: TEXCOORD1;
};

Texture2D ColorTexture : register(t0);
SamplerState ColorSampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
    PixelShaderInput output;

    output.Position = float4(input.Position, 1.0f);
    output.UV = input.UV;
    output.Color = input.Color;
    output.PositionCopy = output.Position;

    return output;
}

float4 PSCopy(PixelShaderInput input) : SV_Target
{
    return ColorTexture.Sample(ColorSampler, input.UV);
}

float4 PSSepia(PixelShaderInput input) : SV_Target
{
    float4 color = ColorTexture.Sample(ColorSampler, input.UV);

    float3 red = float3(0.393f, 0.769f, 0.189f);
    float3 green = float3(0.349f, 0.686f, 0.168f);
    float3 blue = float3(0.272f, 0.534f, 0.131f);

    float3 output;
    output.r = dot(color.rgb, red);
    output.g = dot(color.rgb, green);
    output.b = dot(color.rgb, blue);

    return float4(output, color.a);
}

float4 PSMonochrome(PixelShaderInput input) : SV_Target
{
    float4 color = ColorTexture.Sample(ColorSampler, input.UV);

    float3 grayscale = float3(0.2125f, 0.7154f, 0.0721f);
    float3 output = dot(color.rgb, grayscale);

    return float4(output, color.a);
}

float4 PSFinalPass(PixelShaderInput input) : SV_TARGET
{
    float4 output = ColorTexture.Sample(ColorSampler, input.UV);

    float3 colorMul = min(input.Color.xyz, 1.0f);

    float y = input.Position.y / ViewportHeight;

    if (y > 1.0f - CinematicBarsHeight ||
        y < 0.0f + CinematicBarsHeight)
    {
        output = float4(0, 0, 0, 1);
    }
    else
    {
        output.xyz = output.xyz * colorMul.xyz * ScreenFadeFactor;
        output.w = 1.0f;
    }

    return output;
}