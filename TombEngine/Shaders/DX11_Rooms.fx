#include "./CameraMatrixBuffer.hlsli"
#include "./VertexInput.hlsli"
#include "./Math.hlsli"
#include "./ShaderLight.hlsli"
#include "./AlphaTestBuffer.hlsli"

cbuffer LightsBuffer : register(b2)
{
	ShaderLight Lights[MAX_LIGHTS];
	int NumLights;
	float3 Padding;
};

cbuffer MiscBuffer : register(b3)
{
	int Caustics;
};

cbuffer ShadowLightBuffer : register(b4)
{
	ShaderLight Light;
	float4x4 LightViewProjections[6];
	int CastShadows;
	float3 Padding2;
};

cbuffer RoomBuffer : register(b5)
{
	float4 AmbientColor;
	int Water;
};

struct AnimatedFrameUV
{
	float2 topLeft;
	float2 topRight;
	float2 bottomRight;
	float2 bottomLeft;
};

cbuffer AnimatedBuffer : register(b6) {
	AnimatedFrameUV AnimFrames[32];
	uint numAnimFrames;
}

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 WorldPosition: POSITION0;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR;
	float3x3 TBN : TBN;
	float Fog : FOG;
	float4 PositionCopy : TEXCOORD1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);

Texture2D CausticsTexture : register(t2);

Texture2DArray ShadowMap : register(t3);
SamplerComparisonState ShadowMapSampler : register(s3);

float hash(float3 n)
{
	float x = n.x;
	float y = n.y;
	float z = n.z;
	return float((frac(sin(x)) * 7385.6093) + (frac(cos(y)) * 1934.9663) - (frac(sin(z)) * 8349.2791));
}

struct PixelShaderOutput
{
	float4 Color: SV_Target0;
	float4 Depth: SV_Target1;
};

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;
	float3 pos = input.Position;
	float4 col = input.Color;
	
	// Setting effect weight on TE side prevents portal vertices from moving.
	// Here we just read weight and decide if we should apply refraction or movement effect.
	float weight = input.Effects.z;
	
	// Wibble effect returns different value depending on vertex hash and frame number.
	// In theory, hash could be affected by WaterScheme value from room.
	float wibble = sin((((Frame + input.Hash) % 64) / 64.0) * (PI2)); // sin from -1 to 1 with a period of 64 frames
	
	// Glow
	if (input.Effects.x > 0.0f)
	{
		float intensity = input.Effects.x * lerp(-0.5f, 1.0f, wibble * 0.5f + 0.5f);
		col = saturate(col + float4(intensity, intensity, intensity, 0));
	}
	
	// Movement
	if (input.Effects.y > 0.0f)
        pos.y += wibble * input.Effects.y * weight * 128.0f; // 128 units offset to top and bottom (256 total)

	// Refraction
	float4 screenPos = mul(float4(pos, 1.0f), ViewProjection);
	float2 clipPos = screenPos.xy / screenPos.w;
	if (CameraUnderwater != Water)
	{
		float factor = (Frame + clipPos.x * 320);
		float xOffset = (sin(factor * PI / 20.0f)) * (screenPos.z/1024) * 4;
		float yOffset = (cos(factor * PI / 20.0f)) * (screenPos.z/1024) * 4;
		screenPos.x += xOffset * weight;
		screenPos.y += yOffset * weight;
	}
	
	output.Position = screenPos;
	output.Normal = input.Normal;
	output.Color = col;
	output.PositionCopy = screenPos;

#ifdef ANIMATED
	int frame = (Frame / 2) % numAnimFrames;
	switch (input.PolyIndex) {
	case 0:
		output.UV = AnimFrames[frame].topLeft;
		break;
	case 1:
		output.UV = AnimFrames[frame].topRight;
		break;
	case 2:
		output.UV = AnimFrames[frame].bottomRight;
		break;
	case 3:
		output.UV = AnimFrames[frame].bottomLeft;
		break;
	}
#else
	output.UV = input.UV;

#endif
	
	output.WorldPosition = input.Position.xyz;

	float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);
	output.TBN = TBN;

	// Apply distance fog
	float4 d = length(CamPositionWS - output.WorldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);

	return output;
}

float2 texOffset(int u, int v) {
	return float2(u * 1.0f / SHADOW_MAP_SIZE, v * 1.0f / SHADOW_MAP_SIZE);
}
//https://gist.github.com/JuanDiegoMontoya/d8788148dcb9780848ce8bf50f89b7bb
int GetCubeFaceIndex(float3 dir)
{
    float x = abs(dir.x);
    float y = abs(dir.y);
    float z = abs(dir.z);
    if (x > y && x > z)
        return 0 + (dir.x > 0 ? 0 : 1);
    else if (y > z)
        return 2 + (dir.y > 0 ? 0 : 1);
    return 4 + (dir.z > 0 ? 0 : 1);
}

float2 GetCubeUVFromDir(int faceIndex, float3 dir)
{
    float2 uv;
    switch (faceIndex)
    {
        case 0:
            uv = float2(-dir.z, dir.y);
            break; // +X
        case 1:
            uv = float2(dir.z, dir.y);
            break; // -X
        case 2:
            uv = float2(dir.x, dir.z);
            break; // +Y
        case 3:
            uv = float2(dir.x, -dir.z);
            break; // -Y
        case 4:
            uv = float2(dir.x, dir.y);
            break; // +Z
        default:
            uv = float2(-dir.x, dir.y);
            break; // -Z
    }
    return uv * .5 + .5;
}

void doPointLightShadow(float3 worldPos, inout float3 lighting)
{
    float shadowFactor = 1.0f;
    for (int i = 0; i < 6; i++)
    {
        float3 dir = normalize(worldPos - Light.Position);
        int face = GetCubeFaceIndex(dir);
		//debug coloring
		/*
        switch (face)
        {
			case 0:
                lighting += float3(0.2, 0, 0);
                break;
			case 1:
                lighting += float3(0.1, 0, 0);

                break;
			case 2:
                lighting += float3(0, 0.2, 0);

                break;
			case 3:
                lighting += float3(0, 0.1, 0);

                break;
			case 4:
                lighting += float3(0, 0, 0.2);

                break;
			default:
                lighting += float3(0, 0, 0.1);

            break;
        }
		*/
        float2 uv = GetCubeUVFromDir(face, dir);
        float4 lightClipSpace = mul(float4(worldPos, 1.0f), LightViewProjections[i]);
        lightClipSpace.xyz /= lightClipSpace.w;
        if (lightClipSpace.x >= -1.0f && lightClipSpace.x <= 1.0f &&
			lightClipSpace.y >= -1.0f && lightClipSpace.y <= 1.0f &&
			lightClipSpace.z >= 0.0f && lightClipSpace.z <= 1.0f)
        {
            lightClipSpace.x = lightClipSpace.x / 2 + 0.5;
            lightClipSpace.y = lightClipSpace.y / -2 + 0.5;
            float sum = 0;
            float x, y;
			//perform PCF filtering on a 4 x 4 texel neighborhood
			// what about borders of cubemap?
            for (y = -1.5; y <= 1.5; y += 1.0)
            {
                for (x = -1.5; x <= 1.5; x += 1.0)
                {
                    sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(lightClipSpace.xy + texOffset(x, y), i), lightClipSpace.z);
                }
            }

            shadowFactor = sum / 16.0;

        }
    }
    float distanceFactor = saturate(((distance(worldPos, Light.Position) ) / (Light.Out)));
    lighting *= saturate((shadowFactor + 0.75) + (pow(distanceFactor,3) * 0.25));
}

void doSpotLightShadow(float3 worldPos,inout float3 lighting)
{
    float4 lightClipSpace = mul(float4(worldPos, 1.0f), LightViewProjections[0]);
    lightClipSpace.xyz /= lightClipSpace.w;
    float shadowFactor = 1.0f;
    if (lightClipSpace.x >= -1.0f && lightClipSpace.x <= 1.0f &&
			lightClipSpace.y >= -1.0f && lightClipSpace.y <= 1.0f &&
			lightClipSpace.z >= 0.0f && lightClipSpace.z <= 1.0f)
    {
        lightClipSpace.x = lightClipSpace.x / 2 + 0.5;
        lightClipSpace.y = lightClipSpace.y / -2 + 0.5;
        float sum = 0;
        float x, y;
			//perform PCF filtering on a 4 x 4 texel neighborhood
        for (y = -1.5; y <= 1.5; y += 1.0)
        {
            for (x = -1.5; x <= 1.5; x += 1.0)
            {
                sum += ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(lightClipSpace.xy + texOffset(x, y),0), lightClipSpace.z);
            }
        }

        shadowFactor = sum / 16.0;
    }
    lighting *= saturate(shadowFactor + 0.75);
}
PixelShaderOutput PS(PixelShaderInput input) : SV_TARGET
{
	PixelShaderOutput output;

	output.Color = Texture.Sample(Sampler, input.UV);
	
	DoAlphaTest(output.Color);

	float3 Normal = NormalTexture.Sample(Sampler,input.UV).rgb;
	Normal = Normal * 2 - 1;
	Normal = normalize(mul(Normal,input.TBN));

	float3 lighting = input.Color.xyz;
	bool doLights = true;

	if (CastShadows)
	{
        if (Light.Type == LT_POINT)
        {
            doPointLightShadow(input.WorldPosition,lighting);

        }
        else if (Light.Type == LT_SPOT)
        {
            doSpotLightShadow(input.WorldPosition,lighting);

        }
	}

	if (doLights)
	{
		for (uint i = 0; i < NumLights; i++)
		{
			float3 lightPos = Lights[i].Position.xyz;
			float3 color = Lights[i].Color.xyz;
			float radius = Lights[i].Out;

			float3 lightVec = (lightPos - input.WorldPosition);
			float distance = length(lightVec);
			if (distance > radius)
				continue;

			lightVec = normalize(lightVec);
			float d = saturate(dot(Normal, -lightVec ));
			if (d < 0)
				continue;
			
			float attenuation = pow(((radius - distance) / radius), 2);

			lighting += color * attenuation * d;
		}
	}

	if (Caustics)
	{
		float3 position = input.WorldPosition.xyz;
		float3 normal = Normal;

		float fracX = position.x - floor(position.x / 2048.0f) * 2048.0f;
		float fracY = position.y - floor(position.y / 2048.0f) * 2048.0f;
		float fracZ = position.z - floor(position.z / 2048.0f) * 2048.0f;

		float attenuation = saturate(dot(float3(0.0f, 1.0f, 0.0f), normal));

		float3 blending = abs(normal);
		blending = normalize(max(blending, 0.00001f));
		float b = (blending.x + blending.y + blending.z);
		blending /= float3(b, b, b);

		float3 p = float3(fracX, fracY, fracZ) / 2048.0f;
		float3 xaxis = CausticsTexture.Sample(Sampler, p.yz).xyz; 
		float3 yaxis = CausticsTexture.Sample(Sampler, p.xz).xyz;  
		float3 zaxis = CausticsTexture.Sample(Sampler, p.xy).xyz;  

		lighting += float4((xaxis * blending.x + yaxis * blending.y + zaxis * blending.z).xyz, 0.0f) * attenuation * 2.0f;
	}
	
	output.Color.xyz = output.Color.xyz * lighting;

	output.Depth = output.Color.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (FogMaxDistance != 0)
		output.Color.xyz = lerp(output.Color.xyz, FogColor, input.Fog);

	return output;
}
