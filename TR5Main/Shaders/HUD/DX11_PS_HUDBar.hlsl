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

half4 PSColored(PixelShaderInput input) : SV_TARGET
{
	if (input.UV.x > Percent) {
		discard;
	}
	return input.Color;
}

half4 PSTextured(PixelShaderInput input) : SV_TARGET
{
	if (input.UV.x > Percent) {
		discard;
	}
	float4 output = Texture.Sample(Sampler, input.UV);
	return output;
}