float4x4 World;
float4x4 View;
float4x4 Projection;

texture Texture : register(t0);
sampler TextureSampler : register(s0) = sampler_state
{
	Texture = <Texture>;
};

struct VertexShaderInput
{
	float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 Position : POSITION0;
	float2 TexCoord : TEXCOORD0;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.Position = float4(input.Position, 1.0f);
	output.TexCoord = input.TexCoord;

	return output;
}

float4 PixelShaderFunction(in float2 texCoords : TEXCOORD0) : COLOR0
{
	return theSample = tex2D(TextureSampler, texCoords);
}

technique FXAA
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}
