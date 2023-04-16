struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

cbuffer HUDBarBuffer : register(b11)
{
	float2 BarStartUV;
	float2 BarScale;
};

Texture2D Texture : register(t5);
SamplerState Sampler : register(s5);

half4 PSColored(PixelShaderInput input) : SV_TARGET
{
	return input.Color;
}

half4 PSTextured(PixelShaderInput input) : SV_TARGET
{
	float2 uv = float2((input.UV.x * BarScale.x) + BarStartUV.x, (input.UV.y * BarScale.y) + BarStartUV.y);
	float4 output = Texture.Sample(Sampler, uv);
	return output;
}