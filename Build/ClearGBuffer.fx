struct VertexShaderInput
{
	float3 Position : POSITION0;
	float2 TextureCoordinate : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 Position : POSITION0;
};

struct PixelShaderOutput
{
	float4 Color : COLOR0;
	float4 Normal : COLOR1;
	float4 Depth : COLOR2;
	float4 VertexColor : COLOR3;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.Position = float4(input.Position, 1.0f);

	return output;
}

PixelShaderOutput PixelShaderFunction(VertexShaderOutput input)
{
	PixelShaderOutput output;

	output.Color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.Normal = float4(0.5f, 0.5f, 0.5f, 0.0f);
	output.Depth = 1.0f;
	output.VertexColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	return output;
}

technique ClearGBuffer
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}