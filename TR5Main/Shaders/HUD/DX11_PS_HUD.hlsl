struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
};

half4 PS(PixelShaderInput input) : SV_TARGET
{
	return input.Color;
}