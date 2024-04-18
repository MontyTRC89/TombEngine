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
    float intensity = 1.5;
	float2 main = uv-pos;
	float2 uvd = uv*(length(uv));
	
	float dist=length(main); dist = pow(dist,.1);
	
	
	float f1 = max(0.01-pow(length(uv+1.2*pos),1.9),.0)*7.0;

	float f2 = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),.0)*00.1;
	float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*00.08;
	float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),.0)*00.06;
	
	float2 uvx = lerp(uv,uvd,-0.5);
	
	float f4 = max(0.01-pow(length(uvx+0.4*pos),2.4),.0)*6.0;
	float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)*5.0;
	float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),.0)*3.0;
	
	uvx = lerp(uv,uvd,-.4);
	
	float f5 = max(0.01-pow(length(uvx+0.2*pos),5.5),.0)*2.0;
	float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),.0)*2.0;
	float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),.0)*2.0;
	
	uvx = lerp(uv,uvd,-0.5);
	
	float f6 = max(0.01-pow(length(uvx-0.3*pos),1.6),.0)*6.0;
	float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),.0)*3.0;
	float f63 = max(0.01-pow(length(uvx-0.35*pos),1.6),.0)*5.0;
	
	float3 c = float3(0,0,0);
	
	c.r+=f2+f4+f5+f6; c.g+=f22+f42+f52+f62; c.b+=f23+f43+f53+f63;
	c = c*1.3 - float3(length(uvd)*.05,length(uvd)*.05,length(uvd)*.05);
	
	return c * intensity;
}

float3 LensFlareColorCorrection(float3 color, float factor,float factor2) // color modifier
{
	float w = color.x+color.y+color.z;
	return lerp(color,float3(w,w,w)*factor,w*factor2);
}

float4 PSLensFlare(PixelShaderInput input) : SV_Target
{
	float4 color = ColorTexture.Sample(ColorSampler, input.UV);
	
	float4 position = input.PositionCopy;

	     position.xyz /= position.w;
        position.xyz = position.xyz * 0.5f + 0.5f; 
        position.y = 1.0f - position.y;

	float4 offset = float4(0,-5000,0, 1.0);
        offset = mul(mul(offset, View), Projection); 
		offset.xyz /= offset.w;

        //offset.xyz /= offset.w;
       // offset.xyz = offset.xyz * 0.5f + 0.5f; 
       // offset.y = 1.0f - offset.y;

		position = input.PositionCopy;
		 

	color.xyz += max(float3(0,0,0), float3(1.5,1.2,1.2)*float3(3,3,3)*LensFlare(position.xy,  offset.xy));
	//color.xyz = LensFlareColorCorrection(color,.5,.1);
	
	return color; // float4(output, color.a);
}