#include "./CBCamera.hlsli"
#include "./VertexInput.hlsli"
#include "./VertexEffects.hlsli"
#include "./Blending.hlsli"
#include "./Math.hlsli"
#include "./AnimatedTextures.hlsli"
#include "./Shadows.hlsli"
#include "./ShaderLight.hlsli"

#define ROOM_LIGHT_COEFF 0.7f

cbuffer RoomBuffer : register(b5)
{
	int Water;
	int Caustics;
	int NumRoomLights;
	int Padding;
	float2 CausticsStartUV;
	float2 CausticsScale;
	float4 AmbientColor;
	ShaderLight RoomLights[MAX_LIGHTS_PER_ROOM];
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 WorldPosition: POSITION0;
    //float4 ReflectionPosition : POSITION1;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD0;
	float4 Color: COLOR;
	float Sheen : SHEEN;
	float4 PositionCopy : TEXCOORD1;
	float4 FogBulbs : TEXCOORD2;
	float DistanceFog : FOG;
	float3 Tangent: TANGENT;
	float3 Binormal: BINORMAL;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);
SamplerState NormalTextureSampler : register(s1);

Texture2D CausticsTexture : register(t2);
SamplerState CausticsTextureSampler : register(s2);

Texture2D SSAOTexture : register(t9);
SamplerState SSAOSampler : register(s9);

Texture2D WaterReflectionTexture : register(t10);
SamplerState WaterReflectionSampler : register(s10);

Texture2D<uint> HashTexture : register(t11);

Texture2D WaterNormalTexture : register(t12);
SamplerState WaterNormalTextureSampler : register(s12);

struct PixelShaderOutput
{
	float4 Color: SV_TARGET0;
};

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	// Setting effect weight on TE side prevents portal vertices from moving.
	// Here we just read weight and decide if we should apply refraction or movement effect.
	float weight = input.Effects.z;

	// Calculate vertex effects
	float wibble = Wibble(input.Effects.xyz, input.Hash);
	float3 pos = Move(input.Position, input.Effects.xyz * weight, wibble);
	float3 col = Glow(input.Color.xyz, input.Effects.xyz, wibble);

	// Refraction
	float4 screenPos = mul(float4(pos, 1.0f), ViewProjection);
	float2 clipPos = screenPos.xy / screenPos.w;

	if (CameraUnderwater != Water)
	{
		float factor = (Frame + clipPos.x * 320);
		float xOffset = (sin(factor * PI / 20.0f)) * (screenPos.z / 1024) * 4;
		float yOffset = (cos(factor * PI / 20.0f)) * (screenPos.z / 1024) * 4;
		screenPos.x += xOffset * weight;
		screenPos.y += yOffset * weight;
	}
	
	output.Position = screenPos;
	output.Normal = input.Normal;
	output.Color = float4(col, input.Color.w);
	output.PositionCopy = screenPos;

#ifdef ANIMATED

	if (Type == 0)
		output.UV = GetFrame(input.PolyIndex, input.AnimationFrameOffset);
	else
		output.UV = input.UV; // TODO: true UVRotate in future?
#else
	output.UV = input.UV;
#endif
	
	output.WorldPosition = pos;
	output.Tangent = input.Tangent;
	output.Binormal = input.Binormal;

	output.FogBulbs = DoFogBulbsForVertex(output.WorldPosition);
	output.DistanceFog = DoDistanceFogForVertex(output.WorldPosition);

	return output;
}

float3 UnpackNormalMap(float4 n)
{
	n = n * 2.0f - 1.0f;
	n.z = saturate(1.0f - dot(n.xy, n.xy));
	return n.xyz;
}

float3 PackNormal(float3 n)
{
	n = (n + 1.0f) * 0.5f;
	n.z = 0;
	return n.xyz;
}

float2 Project(float3 worldPos)
{
    float4 clipPos = mul(float4(worldPos, 1.0f), ViewProjection);
    float2 ndc = clipPos.xy / clipPos.w;
    ndc = ndc * 0.5f + 0.5f;
    ndc.y = 1.0f - ndc.y;
    
    return ndc;
}

float ComputeReflectionConfidence(uint2 reflectedPixel, uint2 frameSize)
{
    // Load hash buffer value for the reflected position
    uint hash = HashTexture.Load(uint3(reflectedPixel, 0));

    // If hash is zero, it's an invalid reflection
    float confidence = (hash > 0) ? 1.0 : 0.0;

    // Smooth transition for unreliable reflections
    float2 uv = float2(reflectedPixel) / frameSize;
    float edgeFade = smoothstep(0.05, 0.2, uv) * smoothstep(0.05, 0.2, 1.0 - uv);

    return confidence * edgeFade;
}

float3 BlurReflection(float2 uv, float strength)
{
    float3 color = 0.0;
    float weightSum = 0.0;

    // Larger kernel for more noticeable blur
    float2 offsets[9] =
    {
        float2(-1, -1), float2(0, -1), float2(1, -1),
        float2(-1, 0), float2(0, 0), float2(1, 0),
        float2(-1, 1), float2(0, 1), float2(1, 1)
    };
    
    float weights[9] =
    {
        0.05, 0.1, 0.05,
        0.1, 0.4, 0.1,
        0.05, 0.1, 0.05
    };

    for (int i = 0; i < 9; i++)
    {
        float2 sampleUV = uv + offsets[i] * strength;
        float3 sampleColor = WaterReflectionTexture.Sample(
        WaterReflectionSampler, sampleUV).
        rgb;
        color += sampleColor * weights[i];
        weightSum += weights[i];
    }

    return color / weightSum;
}

float ComputeReflectionFade(float3 normalWS, float3 viewDirWS)
{
    float fresnelFactor = saturate(dot(normalWS, viewDirWS)); // Dot product for angle effect
    return pow(fresnelFactor, 2.0); // Adjust exponent for smooth fading
}

float ComputeReflectionValidity(uint2 reflectedPixel, uint2 frameSize)
{
    // Load hash buffer value for the reflected position
    uint hash = HashTexture.Load(uint3(reflectedPixel, 0));

    // If hash is zero, it's an invalid reflection
    return (hash > 0) ? 1.0 : 0.0;
}

float ComputeProximityFade(uint2 reflectedPixel, uint2 frameSize)
{
    float fade = 1.0;
    int searchRadius = 16; // Tune for smoothness

    // Loop over nearby pixels to detect missing data
    for (int y = -searchRadius; y <= searchRadius; y++)
    {
        for (int x = -searchRadius; x <= searchRadius; x++)
        {
            uint2 neighborPixel = reflectedPixel + uint2(x, y);

            // Clamp to screen size
            neighborPixel = clamp(neighborPixel, uint2(0, 0), frameSize - 1);

            // Check if the neighbor is missing
            if (ComputeReflectionValidity(neighborPixel, frameSize) == 0.0)
            {
                float dist = length(float2(x, y)) / searchRadius;
                fade = min(fade, smoothstep(0.5, 1.0, dist));
            }
        }
    }

    return fade;
}

float2 ComputeScrollingUV(float2 baseUV, float time, float2 speed)
{
    return baseUV + time * speed;
}

float2 DistortUV(float2 reflectedUV, float3 normal, float strength)
{
    return reflectedUV + normal.xy * strength;
}

float GetFresnel(float3 eyeVector, float3 normalVector)
{
    float fangle = 1 + dot(eyeVector, normalVector);
    fangle = pow(fangle, 5);
    float fresnelTerm = 1 / fangle;
	
    return fresnelTerm;
}

float2 Noise2(float2 uv, float time)
{
    float n = sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453;
    return float2(frac(n), frac(n * sin(time)));
}

float3 BlurSSR(float2 uv, float blurRadius)
{
    float3 color = float3(0, 0, 0);
    float weightSum = 0.0;

    for (int x = -5; x <= 5; x++)
    {
        for (int y = -5; y <= 5; y++)
        {
            float2 offset = float2(x, y) * blurRadius;
            float weight = exp(-dot(offset, offset) / (2.0 * blurRadius * blurRadius));
            color += WaterReflectionTexture.Sample(WaterReflectionSampler, uv + offset).rgb * weight;
            weightSum += weight;
        }
    }
    return color / weightSum;
}

PixelShaderOutput PS(PixelShaderInput input)
{
	PixelShaderOutput output;
	
	/*if (WaterReflections)
    {
        clip(WaterHeight - input.WorldPosition.y);
    }*/

	output.Color = Texture.Sample(Sampler, input.UV);

	DoAlphaTest(output.Color);

	float3x3 TBN = float3x3(input.Tangent, input.Binormal, input.Normal);
	float3 normal = UnpackNormalMap(NormalTexture.Sample(NormalTextureSampler, input.UV));
	normal = normalize(mul(normal, TBN));

	float3 lighting = input.Color.xyz;
	bool doLights = true;

	float occlusion = 1.0f;
	if (AmbientOcclusion == 1)
	{
		float2 samplePosition;
		samplePosition = input.PositionCopy.xy / input.PositionCopy.w; // Perspective divide
		samplePosition = samplePosition * 0.5f + 0.5f; // transform to range 0.0 - 1.0  
		samplePosition.y = 1.0f - samplePosition.y;
		occlusion = pow(SSAOTexture.Sample(SSAOSampler, samplePosition).x, AmbientOcclusionExponent);
	}

	lighting = DoShadow(input.WorldPosition, normal, lighting, -2.5f);
	lighting = DoBlobShadows(input.WorldPosition, lighting);

	bool onlyPointLights = (NumRoomLights & ~LT_MASK) == LT_MASK_POINT;
	int numLights = NumRoomLights & LT_MASK;

	for (int i = 0; i < numLights; i++)
	{
		if (onlyPointLights)
		{
			lighting += DoPointLight(input.WorldPosition, normal, RoomLights[i]) * ROOM_LIGHT_COEFF;
		}
		else
		{
			// Room dynamic lights can only be spot or point, so we use simplified function for that.

			float isPoint = step(0.5f, RoomLights[i].Type == LT_POINT);
			float isSpot  = step(0.5f, RoomLights[i].Type == LT_SPOT);

			float3 pointLight = float3(0.0f, 0.0f, 0.0f);
			float3 spotLight  = float3(0.0f, 0.0f, 0.0f);
			DoPointAndSpotLight(input.WorldPosition, normal, RoomLights[i], pointLight, spotLight);
			
			lighting += pointLight * isPoint * ROOM_LIGHT_COEFF + spotLight  * isSpot * ROOM_LIGHT_COEFF;
		}
	}

	if (Caustics)
	{
		float attenuation = saturate(dot(float3(0.0f, -1.0f, 0.0f), normal));

		float3 blending = abs(normal);
		blending = normalize(max(blending, 0.00001f));
		float b = (blending.x + blending.y + blending.z);
		blending /= float3(b, b, b);

		float3 p = frac(input.WorldPosition.xyz / 2048.0f); 
		
		float3 xaxis = CausticsTexture.SampleLevel(CausticsTextureSampler, float2(p.z, p.y), 0).xyz;
		float3 yaxis = CausticsTexture.SampleLevel(CausticsTextureSampler, float2(p.z, p.x), 0).xyz;
		float3 zaxis = CausticsTexture.SampleLevel(CausticsTextureSampler, float2(p.y, p.x), 0).xyz;

		float3 xc = xaxis * blending.x;
		float3 yc = yaxis * blending.y;
		float3 zc = zaxis * blending.z;

		float3 caustics = xc + yc + zc;

		lighting += (caustics * attenuation * 2.0f);
	}

	lighting -= float3(input.FogBulbs.w, input.FogBulbs.w, input.FogBulbs.w);
	output.Color.xyz = output.Color.xyz * lighting * occlusion;
	output.Color.xyz = saturate(output.Color.xyz);

	output.Color = DoFogBulbsForPixel(output.Color, float4(input.FogBulbs.xyz, 1.0f));
	output.Color = DoDistanceFogForPixel(output.Color, FogColor, input.DistanceFog);
	
    /*if (BlendMode == BLENDMODE_DYNAMIC_WATER_SURFACE)
    {
        float3 eyeVector = normalize(CamPositionWS.xyz - input.WorldPosition.xyz);
       
		// Get reflected pixel coordinates
        float2 reflectedUV = Project(input.WorldPosition);
        uint2 reflectedPixel = reflectedUV * ViewSize;
		
		// Sample the waves normal map
        float2 wavesSamplePosition = frac(input.WorldPosition.xyz / 16384.0f).xz;
        wavesSamplePosition += float2(0.0f, 1.0f) * Frame / 1000.0f;
		float3 wavesNormal = WaterNormalTexture.Sample(WaterNormalTextureSampler, wavesSamplePosition);
        wavesNormal = UnpackNormalMap(float4(wavesNormal, 1.0f));
        wavesNormal = normalize(float3(wavesNormal.x, -wavesNormal.z, wavesNormal.y));
		
		// Sample the reflection texture
    
        float2 perturbatedUV = reflectedUV + wavesNormal.xz * 0.008f;
        float3 reflectedColor =
			//BlurSSR(reflectedUV, 0.01f);
			WaterReflectionTexture.Sample(WaterReflectionSampler, perturbatedUV).xyz;
	
		// Calculate specular highlights
        float3 lightDir = -normalize(float3(0.0f, 1.0f, 0.0f));
        float3 reflectDir = reflect(lightDir, wavesNormal);
        float3 specularColor = float3(1.0f, 1.0f, 1.0f);
		float specularIntensity = 1.0f;
        float specularFactor = pow(saturate(dot(CamDirectionWS.xyz, reflectDir)), 1.0f * SPEC_FACTOR);
        float3 specularTerm = specularFactor * specularColor * specularIntensity;

        //float3 halfVector = normalize(eyeVector + lightDir); // Vettore metà tra view e light
        //specularTerm = pow(saturate(dot(wavesNormal, halfVector)), 1.0f) * specularIntensity;
    
		// Compute the Fresnel term
        float3 waterNormal = float3(0.0f, -1.0f, 0.0f);
        float fresnelTerm = GetFresnel(eyeVector, waterNormal);
		
		// Calculate the final color
        output.Color.xyz = lerp(output.Color.xyz, reflectedColor.xyz, fresnelTerm) + specularTerm.xyz;
    }*/

	return output;
}
