#define MODELTYPE_HORIZON			0
#define MODELTYPE_ROOM				1
#define MODELTYPE_MOVEABLE			2
#define MODELTYPE_STATIC			3
#define MODELTYPE_INVENTORY			4
#define MODELTYPE_PICKUP			5
#define MODELTYPE_LARA				6
#define MODELTYPE_SKY				7

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
	float3 Normal : TEXCOORD0;
	float2 TextureCoordinate : TEXCOORD1;
	float4 Color : TEXCOORD2;
	float4 WorldPosition : TEXCOORD3;
	float4 PositionCopy : TEXCOORD4;
	float Depth : TEXCOORD5;
};

struct PixelShaderOutput
{
	float4 Color : COLOR0;
	float4 Normal : COLOR1;
	float4 Depth : COLOR2;
	float4 VertexColor : COLOR3;
};

// Main matrices
float4x4 World;
float4x4 View;
float4x4 Projection;

float3 CameraPosition;
float HalfPixelX;
float HalfPixelY;
float4 Color;
int SkyTimer;

// Bones used for Lara skinning
float4x4 Bones[24];

// Customize the behaviour of the shader
int ModelType;
int BlendMode;
bool UseSkinning;

texture TextureAtlas;
sampler2D TextureAtlasSampler = sampler_state {
	Texture = (TextureAtlas);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPosition;
	float4 normal;

	if (UseSkinning)
	{
		worldPosition = mul(float4(input.Position, 1), mul(Bones[input.Bone], World));
		normal = mul(float4(input.Normal, 1), mul(Bones[input.Bone], World));
	}
	else
	{
		worldPosition = mul(float4(input.Position, 1), World);
		normal = mul(float4(input.Normal, 1), World);
	}

	float4 viewPosition = mul(worldPosition, View);

	output.Position = mul(viewPosition, Projection);
	output.Normal = normalize(input.Normal);
	output.TextureCoordinate = input.TextureCoordinate;
	output.Color = input.Color;
	output.WorldPosition = worldPosition;
	output.PositionCopy = output.Position;
	output.Depth = output.Position.z / output.Position.w;

	if (ModelType == MODELTYPE_SKY)
		output.TextureCoordinate.y += 1.0f / SkyTimer;

	return output;
}

PixelShaderOutput PixelShaderFunction(VertexShaderOutput input) 
{
	PixelShaderOutput output;

	float4 vertexColors = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (ModelType == MODELTYPE_ROOM)
	{
		float3 colorMul = min(input.Color.xyz * 2.0f, 1.0f);
		vertexColors = float4(colorMul, 1.0f);
	}

	float4 textureColor = tex2D(TextureAtlasSampler, input.TextureCoordinate);

	if (BlendMode == BLENDMODE_ALPHATEST)
		clip(textureColor.a - 0.5f);

	output.Color.rgb = textureColor.rgb * Color.rgb;
	output.Color.a = 0.1f;

	output.Normal.xyz = 0.5f * (input.Normal.xyz + 1.0f);
	output.Normal.w = ModelType / 16.0f;  
	//output.Normal.w = 1.0f;

	output.Depth = input.PositionCopy.z / input.PositionCopy.w; // , 0.0f, 0.0f, 1.0f); //   float4(d, 0.0f, 0.0f, 1.0f);

	if (ModelType == MODELTYPE_ROOM)
		output.VertexColor = vertexColors;
	else
		output.VertexColor = float4(0.5f, 0.5f, 0.5f, 1.0f);

	return output;
}

technique Textured
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}