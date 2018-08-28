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

#define DEPTH_BIAS					0.0f
#define TEXEL_SIZE					1.0f / 2048.0f

texture2D ColorMap;
sampler ColorSampler = sampler_state
{
	Texture = (ColorMap);
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	Mipfilter = LINEAR;
};

texture2D LightMap;
sampler LightSampler = sampler_state
{
	Texture = (LightMap);
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	Mipfilter = LINEAR;
};

texture2D VertexColorMap;
sampler VertexColorSampler = sampler_state
{
	Texture = (VertexColorMap);
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	Mipfilter = LINEAR;
};
 
/*texture2D ShadowMap;
sampler ShadowSampler = sampler_state
{
	Texture = (ShadowMap);
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	Mipfilter = LINEAR;
};*/

texture2D NormalMap;
sampler NormalSampler = sampler_state
{
	Texture = (NormalMap);
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = POINT;
	MinFilter = POINT;
	Mipfilter = POINT;
};

texture2D DepthMap;
sampler DepthSampler = sampler_state
{
	Texture = (DepthMap);
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = POINT;
	MinFilter = POINT;
	Mipfilter = POINT;
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

struct VertexShaderInput
{
	float3 Position : POSITION0;
	float2 TextureCoordinate : TEXCOORD0;
};

struct VertexShaderOutput
{
	float4 Position : POSITION0;
	float2 TextureCoordinate : TEXCOORD0;
	float4 ScreenPosition : TEXCOORD1;
};

float HalfPixelX;
float HalfPixelY;

// We do shadow here
bool CastShadows;
float4x4 ViewProjectionInverse;
float4x4 LightView;
float4x4 LightProjection;
float4 LightPosition;
float LightOut;

float ShadowMapLookup(sampler2D shadowMap, float2 texCoord, float depth)
{
	return (tex2D(shadowMap, texCoord).r + DEPTH_BIAS < depth) ? 0.3f : 1.0f;
}

float ShadowMapLookup(sampler2D shadowMap, float2 texCoord, float2 offset, float depth)
{
	return (tex2D(shadowMap, texCoord + offset * TEXEL_SIZE).r + DEPTH_BIAS < depth) ? 0.5f : 1.0f;
}

VertexShaderOutput VertexShaderFunction(VertexShaderInput input)
{
	VertexShaderOutput output;

	output.Position = float4(input.Position, 1.0f);
	output.TextureCoordinate = input.TextureCoordinate - float2(HalfPixelX, HalfPixelY);
	output.ScreenPosition = float4(input.Position, 1.0f);

	return output;
}
 
float4 PixelShaderFunction(VertexShaderOutput input) : COLOR0
{
	float3 diffuseColor = tex2D(ColorSampler, input.TextureCoordinate).rgb;
	float3 ambientColor = tex2D(VertexColorSampler, input.TextureCoordinate).rgb;
	float4 light = tex2D(LightSampler, input.TextureCoordinate);
	/*int pixelFlags = tex2D(NormalSampler, input.TextureCoordinate).w * 64.0f;
	int modelType = pixelFlags % 32;
	bool underwater = (pixelFlags / 32 == 1);
	int pixelFlags = normalData.w * 64.0f;
	bool underwater = ((pixelFlags / 32) == 1);
	if (pixelFlags >= 32) pixelFlags -= 32;
	int modelType = pixelFlags;*/
	int modelType = round(tex2D(NormalSampler, input.TextureCoordinate).w * 16.0f);
	//if (underwater)
	//	return float4(0, 0, 1, 1);

	float3 diffuseLight = light.rgb;
	float specularLight = 0; // light.a;

	if (modelType == MODELTYPE_HORIZON || modelType == MODELTYPE_SKY)
		return float4(diffuseColor, 1.0f);

	float shadowOcclusion = 1.0f;

	if (modelType == MODELTYPE_ROOM || modelType == MODELTYPE_ROOM_UNDERWATER)
	{
		if (CastShadows)
		{
			input.ScreenPosition.xy /= input.ScreenPosition.w;

			// Get the depth value
			float depthVal = tex2D(DepthSampler, input.TextureCoordinate).r;

			// This light cast shadows? In this case we need also to compute the shadow term

			// Compute screen-space position
			float4 position;
			position.xy = input.ScreenPosition.xy;
			position.z = depthVal;
			position.w = 1.0f;

			// Transform to world space
			position = mul(position, ViewProjectionInverse);
			position /= position.w;

			float shadowOcclusion = 1.0f;
			float3 lightDirection = LightPosition.xyz - position.xyz;
			float lightDistance = length(lightDirection);
			lightDirection = normalize(lightDirection);

			if (lightDistance <= LightOut * 1.5f)
			{
				// Find the position of this pixel in light space
				float4 lightingPosition = mul(position, mul(LightView, LightProjection));

				// Get the depth of the current pixel in the light space
				float ourDepth = (lightingPosition.z / lightingPosition.w);

				// Find the position in the shadow map for this pixel
				float2 shadowTexCoords = 0.5 * lightingPosition.xy / lightingPosition.w + float2(0.5, 0.5);
				shadowTexCoords.y = 1.0f - shadowTexCoords.y;

				if (shadowTexCoords.x >= 0.0f && shadowTexCoords.x <= 1.0f && shadowTexCoords.y >= 0.0f && shadowTexCoords.y <= 1.0f
					&& lightingPosition.w >= 0.0f)
				{
					// Get the depth of the current pixel in the light space
					float ourDepth = (lightingPosition.z / lightingPosition.w);
					
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

					if (shadowOcclusion <= 0.5f)
						return float4(shadowOcclusion * (diffuseColor * (ambientColor + diffuseLight)).xyz, 1.0f);
				}
			}
		}
	}

	return float4((diffuseColor * (ambientColor + diffuseLight) + specularLight), 1);
}

technique CombineFinal
{
	pass Pass1
	{
		VertexShader = compile vs_3_0 VertexShaderFunction();
		PixelShader = compile ps_3_0 PixelShaderFunction();
	}
}