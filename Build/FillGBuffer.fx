#define MODELTYPE_HORIZON			0
#define MODELTYPE_ROOM				1
#define MODELTYPE_MOVEABLE			2
#define MODELTYPE_STATIC			3
#define MODELTYPE_INVENTORY			4
#define MODELTYPE_PICKUP			5
#define MODELTYPE_LARA				6
#define MODELTYPE_SKY				7
#define MODELTYPE_WATER_SURFACE		8
#define MODELTYPE_ROOM_UNDERWATER	9

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

// Bones used for Lara skinning
float4x4 Bones[24];

// Customize the behaviour of the shader
int ModelType;
int BlendMode;
bool UseSkinning;
bool Underwater;
float4 AmbientLight;

texture TextureAtlas;
sampler2D TextureAtlasSampler = sampler_state {
	Texture = (TextureAtlas);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Wrap;
	AddressV = Wrap;
};

texture2D CausticsMap;
sampler CausticsSampler = sampler_state
{
	Texture = (CausticsMap);
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	Mipfilter = LINEAR;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPosition;
	float4 normal;

	if (UseSkinning)
	{
		worldPosition = mul(float4(input.Position, 1), mul(Bones[input.Bone], World));
		normal = mul(float4(input.Normal, 1), Bones[input.Bone]);
	}
	else
	{
		worldPosition = mul(float4(input.Position, 1), World);
		normal = float4(input.Normal, 1.0f); // mul(float4(input.Normal, 1), World);
	}

	float4 viewPosition = mul(worldPosition, View);

	output.Position = mul(viewPosition, Projection);
	output.Normal = normalize(normal.xyz);
	output.TextureCoordinate = input.TextureCoordinate;
	output.Color = input.Color;
	output.WorldPosition = worldPosition;
	output.PositionCopy = output.Position;
	output.Depth = output.Position.z / output.Position.w;

	return output;
}

PixelShaderOutput PixelShaderFunction(VertexShaderOutput input) 
{
	PixelShaderOutput output;

	float4 vertexColors = float4(1.0f, 1.0f, 1.0f, 1.0f);
	if (ModelType == MODELTYPE_ROOM || ModelType == MODELTYPE_ROOM_UNDERWATER)
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
	//float pixelFlags = ModelType;
	//if (Underwater) pixelFlags += 32.0f;
	//output.Normal.w = pixelFlags / 64.0f;
	//output.Normal.w = 1.0f;

	output.Depth = input.PositionCopy.z / input.PositionCopy.w;  

	if (ModelType == MODELTYPE_ROOM_UNDERWATER)
	{
		float3 position = input.WorldPosition.xyz;
		float3 normal = input.Normal.xyz;

		float fracX = position.x - floor(position.x / 2048.0f) * 2048.0f;
		float fracY = position.y - floor(position.y / 2048.0f) * 2048.0f;
		float fracZ = position.z - floor(position.z / 2048.0f) * 2048.0f;

		float attenuation = saturate(dot(float3(0.0f, -1.0f, 0.0f), normal));

		float3 blending = abs(normal);
		blending = normalize(max(blending, 0.00001f));
		float b = (blending.x + blending.y + blending.z);
		blending /= float3(b, b, b);

		float3 p = float3(fracX, fracY, fracZ) / 2048.0f;
		float3 xaxis = tex2D(CausticsSampler, p.yz).rgb;
		float3 yaxis = tex2D(CausticsSampler, p.xz).rgb;
		float3 zaxis = tex2D(CausticsSampler, p.xy).rgb;

		vertexColors += float4((xaxis * blending.x + yaxis * blending.y + zaxis * blending.z).xyz, 0.0f) * attenuation * 2.0f;
	}

	if (ModelType == MODELTYPE_ROOM || ModelType == MODELTYPE_ROOM_UNDERWATER)
		output.VertexColor = vertexColors;
	else
		output.VertexColor = float4(AmbientLight.rgb, 1.0f);

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