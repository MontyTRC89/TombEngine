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
	float3 Normal : TEXCOORD0;
	float2 TextureCoordinate : TEXCOORD1;
};

float4x4 World;
float4x4 View;
float4x4 Projection;

float4x4 Bones[48];

bool EnableVertexColors;
bool CinematicMode;
bool UseSkinning;

float4 LightDirection;
float4 LightColor;
float LightIntensity;
float4 AmbientLight;

float FadeTimer;

texture ModelTexture;
sampler2D textureSampler = sampler_state {
	Texture = (ModelTexture);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPosition;
	float4 normal;
	float4x4 world;

	if (UseSkinning)
	{
		world = mul(Bones[input.Bone], World);
		worldPosition = mul(float4(input.Position, 1), world);
		normal = mul(float4(input.Normal, 0), world);
	}
	else
	{
		world = World;
		worldPosition = mul(float4(input.Position, 1), world);
		normal = mul(float4(input.Normal, 0), world);
	}

	float4 viewPosition = mul(worldPosition, View);

	output.Position = mul(viewPosition, Projection);
	output.Normal = normalize(normal).xyz;
	output.TextureCoordinate = input.TextureCoordinate;

	return output;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	float4 textureColor = tex2D(textureSampler, input.TextureCoordinate);
	float d = max(dot(-LightDirection, input.Normal), 0.0f);
	float4 light = AmbientLight + d * LightColor * LightIntensity;
	textureColor *= light;
	textureColor.a = 1.0f;

	return textureColor;
}

technique Textured
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}
