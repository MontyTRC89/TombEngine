bool CinematicBars;
float FadeFactor;

texture Texture;
sampler2D TextureSampler = sampler_state {
	Texture = (Texture);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexShaderInput
{
	float3 Position : POSITION0;
	float2 TextureCoordinate : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 Position : POSITION0;
	float2 TextureCoordinate : TEXCOORD0;
	float4 PositionCopy : TEXCOORD1;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.Position = float4(input.Position, 1.0f);
	output.TextureCoordinate = input.TextureCoordinate;
	output.PositionCopy = output.Position;

	return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	if (CinematicBars && (input.PositionCopy.y < -0.8f || input.PositionCopy.y >= 0.8f))
		return float4(0.0f, 0.0f, 0.0f, 1.0f);

	return float4(tex2D(TextureSampler, input.TextureCoordinate).rgb * FadeFactor, 1.0f);
}

technique FullScreenQuad
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}
