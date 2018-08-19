#define MAX_LIGHTS					16

#define LIGHTTYPE_SUN				0
#define LIGHTTYPE_POINT				1
#define LIGHTTYPE_SPOT				2
#define LIGHTTYPE_SHADOW			3

#define MODELTYPE_HORIZON			0
#define MODELTYPE_ROOM				1
#define MODELTYPE_MOVEABLE			2
#define MODELTYPE_STATIC			3
#define MODELTYPE_INVENTORY			4
#define MODELTYPE_PICKUP			5
#define MODELTYPE_LARA				6

#define BLENDMODE_OPAQUE			0
#define BLENDMODE_ALPHATEST			1
#define BLENDMODE_ALPHABLEND		2

#define DEPTH_BIAS					0.0f
#define TEXEL_SIZE					1.0f / 2048.0f

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
};

// Main matrices
float4x4 World;
float4x4 View;
float4x4 Projection;

float4x4 LightView;
float4x4 LightProjection;

float3 CameraPosition;

// Bones used for Lara skinning
float4x4 Bones[24];

// Customize the behaviour of the shader
int ModelType;
int BlendMode;
bool CinematicMode;
bool UseSkinning;
bool EnableShadows;
float FadeTimer;

// Lighting
float3 AmbientLight;
bool LightActive;
float4 LightPosition;
float4 LightColor;
int LightType;
float LightIn;
float LightOut;
float LightRange;
float4 LightDirection;
bool DynamicLight;

texture TextureAtlas;
sampler2D TextureAtlasSampler = sampler_state {
	Texture = (TextureAtlas);
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = Clamp;
	AddressV = Clamp;
};

texture2D ShadowMap;
sampler2D ShadowMapSampler = sampler_state {
	texture = (ShadowMap);
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = None;
	AddressU = Clamp;
	AddressV = Clamp;
};

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	float4 worldPosition;
	float4 normal;
	float4x4 shadowWorld;

	if (UseSkinning)
	{
		worldPosition = mul(float4(input.Position, 1), mul(Bones[input.Bone], World));
		normal = mul(float4(input.Normal, 1), mul(Bones[input.Bone], World));
		shadowWorld = Bones[input.Bone];
	}
	else
	{
		worldPosition = mul(float4(input.Position, 1), World);
		normal = mul(float4(input.Normal, 1), World);
		shadowWorld = World;
	}

	float4 viewPosition = mul(worldPosition, View);

	output.Position = mul(viewPosition, Projection);
	output.Normal = normalize(normal);
	output.TextureCoordinate = input.TextureCoordinate;
	output.Color = input.Color;
	output.WorldPosition = worldPosition;
	output.PositionCopy = output.Position;

	return output;
}

float ShadowMapLookup(sampler2D shadowMap, float2 texCoord, float depth)
{
	return (tex2D(shadowMap, texCoord).r + DEPTH_BIAS < depth) ? 0.5f : 1.0f;
}

float ShadowMapLookup(sampler2D shadowMap, float2 texCoord, float2 offset, float depth)
{
	return (tex2D(shadowMap, texCoord + offset * TEXEL_SIZE).r + DEPTH_BIAS < depth) ? 0.5f : 1.0f;
}

float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	float4 textureColor = tex2D(TextureAtlasSampler, input.TextureCoordinate);

	// If alpha test, then reject pixel based on alpha
	if (BlendMode == BLENDMODE_ALPHATEST)
		clip(textureColor.a - 0.5f);

	// Horizon has not to be lit 
	if (ModelType == MODELTYPE_HORIZON)
		return textureColor;

	float3 totalLight = AmbientLight * 2.0f;

	// Check if in shadow
	float shadowOcclusion = 1.0f;
	float lightDistance = length(LightPosition.xyz - input.WorldPosition.xyz);

	// Shadows are only for rooms
	if (ModelType == MODELTYPE_ROOM && EnableShadows && lightDistance <= LightOut)
	{
		// Find the position of this pixel in light space
		float4 lightingPosition = mul(input.WorldPosition, mul(LightView, LightProjection));

		// Find the position in the shadow map for this pixel
		float2 shadowTexCoords = 0.5 * lightingPosition.xy / lightingPosition.w + float2(0.5, 0.5);
		shadowTexCoords.y = 1.0f - shadowTexCoords.y;

		if (shadowTexCoords.x >= 0.0f && shadowTexCoords.x <= 1.0f && shadowTexCoords.y >= 0.0f && shadowTexCoords.y <= 1.0f
			&& lightingPosition.w >= 0.0f)
		{
			// Get the depth of the current pixel in the light space
			float ourDepth = (lightingPosition.z / lightingPosition.w); // DepthBias;

																		// Check to see if this pixel is in front or behind the value in the shadow map
																		//if (shadowDepth < ourDepth + DEPTH_BIAS)
																		//{
																		// Calculate the sahdow factor with 3x3 PCF for soft shadows
			shadowOcclusion = 0.0f;

			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(0.0f, 0.0f), ourDepth);
			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(1.0f, 0.0f), ourDepth);
			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(2.0f, 0.0f), ourDepth);

			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(0.0f, 1.0f), ourDepth);
			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(1.0f, 1.0f), ourDepth);
			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(2.0f, 1.0f), ourDepth);

			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(0.0f, 2.0f), ourDepth);
			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(1.0f, 2.0f), ourDepth);
			shadowOcclusion += ShadowMapLookup(ShadowMapSampler, shadowTexCoords, float2(2.0f, 2.0f), ourDepth);

			shadowOcclusion /= 9.0f;

			/*{
			float atten = lightDistance / LightOut;
			shadowOcclusion *= atten;
			}*/
			//}
		}
	}

	// Only rooms use static vertex lighting
	if (ModelType == MODELTYPE_ROOM)
	{
		float3 colorMul = min(input.Color.xyz * shadowOcclusion, 1.0f);
		totalLight = colorMul;
	}

	if ((ModelType == MODELTYPE_LARA || ModelType == MODELTYPE_MOVEABLE) && LightActive)
	{
		if (LightType == LIGHTTYPE_POINT)
		{
			float3 l = LightPosition.xyz - input.WorldPosition.xyz;
			float distance = length(l);
			l = normalize(l);

			float d = dot(l, input.Normal);

			if (distance <= LightOut && d > 0)
			{
				float atten = 1.0f - (distance / LightOut);
				totalLight += atten * LightColor * d;
			}
		}
		else if (LightType == LIGHTTYPE_SPOT)
		{
			float3 p = LightPosition.xyz - input.WorldPosition.xyz;
			float distance = length(p);
			p = normalize(p);

			float3 l = LightDirection.xyz;
			l = normalize(l);

			float c = dot(-l, p);
			float d = dot(-l, input.Normal);

			if (distance < LightRange && c > cos(LightOut / 2.0f) /*&& d > 0*/)
			{
				float atten = (1.0f - c / cos(LightOut / 2.0f));
				totalLight += d * LightColor * atten * 1.0f;
			}
		}
	}

	//if (ModelType == MODELTYPE_LARA)
	//	return  float4(input.Normal.xyz * 0.5 + 0.5, 1);

	textureColor.xyz = saturate(totalLight.xyz) * textureColor.xyz;
	textureColor.a = 1;

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
