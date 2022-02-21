#include "./../VertexInput.hlsli"
#include "./../Math.hlsli"

cbuffer HUDBarBuffer : register(b11)
{
	float Percent;
	int Poisoned;
	int Frame;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};
Texture2D Texture : register(t5);
SamplerState Sampler : register(s5);

half4 glassOverlay(float2 UVs, half4 originalColor) {
	float y = UVs.y;
	y -= 0.15f;
	y = distance(0.1f, y);
	y = 1 - y;
	y = pow(y, 4);
	y = saturate(y);
	half4 color = originalColor;
	return saturate(lerp(color, (color * 1.6f)+half4(0.4f, 0.4f, 0.4f, 0.0f), y));
}

half4 PSColored(PixelShaderInput input) : SV_TARGET
{
	if (input.UV.x > Percent) {
		discard;
	}
    half4 col = input.Color;
	if (Poisoned) {
		float factor = sin(((Frame % 30) / 30.0) * PI2)*0.5 + 0.5;
		col = lerp(col,half4(214 / 512.0, 241 / 512.0, 18 / 512.0, 1),factor);
	}
	return glassOverlay(input.UV,col);
}

half4 PSTextured(PixelShaderInput input) : SV_TARGET
{
	if (input.UV.x > Percent) {
		discard;
	}
	half4 col = Texture.Sample(Sampler, input.UV);
	if (Poisoned) {
		float factor = sin(((Frame % 30) / 30.0) * PI2)*0.5 + 0.5;
		col = lerp(col, half4(214 / 512.0, 241 / 512.0, 18 / 512.0, 1), factor);
	}
	return glassOverlay(input.UV, col);
}