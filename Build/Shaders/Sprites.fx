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
	float2 TextureCoordinate : TEXCOORD0;
	float4 Color : TEXCOORD1;
};

float4x4 View;
float4x4 Projection;

texture TextureAtlas;
sampler2D textureSampler = sampler_state {
	Texture = (TextureAtlas);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 viewPosition = mul(float4(input.Position, 1.0f), View);
	output.Position = mul(viewPosition, Projection);
	output.TextureCoordinate = input.TextureCoordinate;
	output.Color = input.Color;

	return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	float4 textureColor = tex2D(textureSampler, input.TextureCoordinate);
	//clip(textureColor.a - 0.6f);

	float a = textureColor.r;
	textureColor *= input.Color;
	textureColor.a = a;

	return textureColor;
}

technique Sprites
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}
