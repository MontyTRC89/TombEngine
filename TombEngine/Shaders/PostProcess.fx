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

float4 PSMonochrome(PixelShaderInput input) : SV_Target
{
    float4 color = ColorTexture.Sample(ColorSampler, input.UV);

    float luma = Luma(color.rgb);
    float3 output = lerp(color.rgb, float3(luma, luma, luma), EffectStrength);

    return float4(output, color.a);
}

float4 PSNegative(PixelShaderInput input) : SV_Target
{
	float4 color = ColorTexture.Sample(ColorSampler, input.UV);

	float luma = Luma(1.0f - color);
	float3 output = lerp(color.rgb, float3(luma, luma, luma), EffectStrength);

	return float4(output, color.a);
}

float4 PSExclusion(PixelShaderInput input) : SV_Target
{
	float4 color = ColorTexture.Sample(ColorSampler, input.UV);

	float3 exColor = color.xyz + (1.0f - color.xyz) - 2.0f * color.xyz * (1.0f - color.xyz);
	float3 output = lerp(color.rgb, exColor, EffectStrength);

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

	output.xyz = output.xyz * Tint;

    return output;
}

float3 LensFlare(float2 uv, float2 pos)
{
	float intensity = 1.5f;
	float2 main = uv - pos;
	float2 uvd = uv * (length(uv));

	float dist = length(main);
	dist = pow(dist, 0.1f);

	float f1 = max(0.01f - pow(length(uv + 1.2f * pos), 1.9f), 0.0f) * 7.0f;

	float f2 = max(1.0f / (1.0f + 32.0f * pow(length(uvd + 0.8f * pos), 2.0f)), 0.0f) * 0.1f;
	float f22 = max(1.0f / (1.0f + 32.0f * pow(length(uvd + 0.85f * pos), 2.0f)), 0.0f) * 0.08f;
	float f23 = max(1.0f / (1.0f + 32.0f * pow(length(uvd + 0.9f * pos), 2.0f)), 0.0f) * 0.06f;

	float2 uvx = lerp(uv, uvd, -0.5f);

	float f4 = max(0.01f - pow(length(uvx + 0.4f * pos), 2.4f), 0.0f) * 6.0f;
	float f42 = max(0.01f - pow(length(uvx + 0.45f * pos), 2.4f), 0.0f) * 5.0f;
	float f43 = max(0.01f - pow(length(uvx + 0.5f * pos), 2.4f), 0.0f) * 3.0f;

	uvx = lerp(uv, uvd, -0.4f);

	float f5 = max(0.01f - pow(length(uvx + 0.2f * pos), 5.5f), 0.0f) * 2.0f;
	float f52 = max(0.01f - pow(length(uvx + 0.4f * pos), 5.5f), 0.0f) * 2.0f;
	float f53 = max(0.01f - pow(length(uvx + 0.6f * pos), 5.5f), 0.0f) * 2.0f;

	uvx = lerp(uv, uvd, -0.5f);

	float f6 = max(0.01f - pow(length(uvx - 0.3f * pos), 1.6f), 0.0f) * 6.0f;
	float f62 = max(0.01f - pow(length(uvx - 0.325f * pos), 1.6f), 0.0f) * 3.0f;
	float f63 = max(0.01f - pow(length(uvx - 0.35f * pos), 1.6f), 0.0f) * 5.0f;

	float3 c = float3(0.0f, 0.0f, 0.0f);

	c.r += f2 + f4 + f5 + f6; 
	c.g += f22 + f42 + f52 + f62; 
	c.b += f23 + f43 + f53 + f63;
	
	c = c * 1.3f - float3(length(uvd) * 0.05f, length(uvd) * 0.05f, length(uvd) * 0.05f);

	return c * intensity;
}

float3 LensFlareColorCorrection(float3 color, float factor,float factor2) 
{
	float w = color.x + color.y + color.z;
	return lerp(color, float3(w, w, w) * factor, w * factor2);
}

float4 PSLensFlare(PixelShaderInput input) : SV_Target
{
	float4 color = ColorTexture.Sample(ColorSampler, input.UV);
	
	float4 position = input.PositionCopy;
	float3 totalLensFlareColor = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < NumLensFlares; i++)
	{
		float4 lensFlarePosition = float4(LensFlares[i].Position, 1.0f);
        lensFlarePosition = mul(mul(lensFlarePosition, View), Projection); 
		lensFlarePosition.xyz /= lensFlarePosition.w;

		float3 lensFlareColor = max(float3(0.0f, 0.0f, 0.0f),
			LensFlares[i].Color *
			float3(4.5f, 3.6f, 3.6f) * 
			LensFlare(position.xy, lensFlarePosition.xy));
		lensFlareColor = LensFlareColorCorrection(lensFlareColor, 0.5f, 0.1f);
		totalLensFlareColor += lensFlareColor;
	}

	color.xyz += totalLensFlareColor;

	return color;
}