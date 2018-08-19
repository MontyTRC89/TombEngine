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
};

float4x4 World;
float4x4 View;
float4x4 Projection;

float4x4 Bones[48];

bool UseSkinning;

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPosition;
	float4 normal;

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

	return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

technique ReconstructZBuffer
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}
