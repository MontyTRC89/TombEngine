struct VertexShaderInput
{
	float3 Position : POSITION0;
	float3 Normal : NORMAL0;
	float2 TextureCoordinate : TEXCOORD0;
	float4 Color : COLOR0;
	float Bone : BLENDINDICES0;
};

struct VertexShaderOutput
{
	float4 Position : POSITION0;
	float4 Color : TEXCOORD0;
};

float4x4 View;
float4x4 Projection;

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 viewPosition = mul(float4(input.Position, 1.0f), View);
	output.Position = mul(viewPosition, Projection);
	output.Color = input.Color;

	return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	return float4(input.Color.rgb, 0.0f);
}

technique Rain
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}
