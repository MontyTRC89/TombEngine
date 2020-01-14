cbuffer HUDBarBuffer : register(b0)
{
	float Percent;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};
Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

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
	return glassOverlay(input.UV,input.Color);
}

half4 PSTextured(PixelShaderInput input) : SV_TARGET
{
	if (input.UV.x > Percent) {
		discard;
	}
	float4 color = Texture.Sample(Sampler, input.UV);
	return glassOverlay(input.UV, color);
}