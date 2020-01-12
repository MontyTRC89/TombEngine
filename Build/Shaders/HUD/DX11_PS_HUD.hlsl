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
	return input.Color;
}

half4 PSTextured(PixelShaderInput input) : SV_TARGET
{
	float4 output = Texture.Sample(Sampler, input.UV);
	return output;
}