#define BLENDMODE_OPAQUE			0
#define BLENDMODE_ALPHATEST			1
#define BLENDMODE_ALPHABLEND		2

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
	float Depth : TEXCOORD1;
};

// Main matrices
float4x4 World;
float4x4 View;
float4x4 Projection;

// Bones used for Lara skinning
float4x4 Bones[32];

// Customize the behaviour of the shader
bool UseSkinning;
int BlendMode;

texture TextureAtlas;
sampler2D TextureAtlasSampler = sampler_state {
	Texture = (TextureAtlas);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPosition;

	if (UseSkinning)
	{
		worldPosition = mul(float4(input.Position, 1), mul(Bones[input.Bone], World));
	}
	else
	{
		worldPosition = mul(float4(input.Position, 1), World);
	}

	float4 viewPosition = mul(worldPosition, View);

	output.Position = mul(viewPosition, Projection);
	output.TextureCoordinate = input.TextureCoordinate;
	output.Depth = output.Position.z / output.Position.w;

	return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR
{
	float4 textureColor = tex2D(TextureAtlasSampler, input.TextureCoordinate);

	// If alpha test, then reject pixel based on alpha
	if (BlendMode == BLENDMODE_ALPHATEST)
		clip(textureColor.a - 0.5f);

	return float4(input.Depth, 0.0f, 0.0f, 1.0f); 
}

technique Depth
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}
