#include "./Math.hlsli"
#include "./CameraMatrixBuffer.hlsli"
#include "./ShaderLight.hlsli"
#include "./VertexInput.hlsli"
#include "./AlphaTestBuffer.hlsli"

cbuffer StaticMatrixBuffer : register(b8)
{
	float4x4 World;
	float4 Position;
	float4 Color;
};

cbuffer LightsBuffer : register(b2)
{
	ShaderLight Lights[MAX_LIGHTS];
	int NumLights;
	float3 CameraPosition;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float2 UV: TEXCOORD1;
	float4 Color: COLOR;
	float Sheen : SHEEN;
	float Fog : FOG;
	float4 PositionCopy: TEXCOORD2;
};

struct PixelShaderOutput
{
	float4 Color: SV_Target0;
	float4 Depth: SV_Target1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 worldPosition = (mul(float4(input.Position, 1.0f), World));

	output.Normal = input.Normal;
	output.UV = input.UV;
	
	float3 pos = input.Position;
	float4 col = input.Color;
	
	// Setting effect weight on TE side prevents portal vertices from moving.
	// Here we just read weight and decide if we should apply refraction or movement effect.
	float weight = input.Effects.z;
	
	// Wibble effect returns different value depending on vertex hash and frame number.
	// In theory, hash could be affected by WaterScheme value from room.
	float wibble = sin((((Frame + input.Hash) % 256) / 256.0) * (PI2)); // sin from -1 to 1 with a period of 64 frames
	
	// Glow
	if (input.Effects.x > 0.0f)
	{
		float intensity = input.Effects.x * lerp(-0.5f, 1.0f, wibble * 0.5f + 0.5f);
		col = saturate(col + float4(intensity, intensity, intensity, 0));
	}

	// Movement
	if (input.Effects.y > 0.0f)
        pos.y += wibble * input.Effects.y * weight * 128.0f; // 128 units offset to top and bottom (256 total)
	
	output.Position = mul(worldPosition, ViewProjection);
	output.Color = col;

	// Apply distance fog
	float4 d = length(CamPositionWS - worldPosition);
	if (FogMaxDistance == 0)
		output.Fog = 1;
	else
		output.Fog = clamp((d - FogMinDistance * 1024) / (FogMaxDistance * 1024 - FogMinDistance * 1024), 0, 1);
	
	output.PositionCopy = output.Position;
    output.Sheen = input.Effects.w;
	return output;
}

PixelShaderOutput PS(PixelShaderInput input) : SV_TARGET
{
	PixelShaderOutput output;
	float4 tex = Texture.Sample(Sampler, input.UV);
	
    DoAlphaTest(tex);
    float3 ambient = Color.xyz * tex.xyz;
	
	float4 worldPosition = (mul(input.Position, World));
	
	float3 diffuse = 0;
    float3 spec = 0;	
	
	for (int i = 0; i < NumLights; i++)
	{
		int lightType = Lights[i].Type;

		if (lightType == LT_POINT || lightType == LT_SHADOW)
		{
            diffuse += DoPointLight(worldPosition.xyz, input.Normal, Lights[i]);
            spec += DoSpecularPoint(worldPosition.xyz, input.Normal, Lights[i], input.Sheen);

        }
		else if (lightType == LT_SUN)
		{
            diffuse += DoDirectionalLight(worldPosition.xyz, input.Normal, Lights[i]);
            spec += DoSpecularSun(input.Normal, Lights[i], input.Sheen);

		}
		else if (lightType == LT_SPOT)
		{
            diffuse += DoSpotLight(worldPosition.xyz, input.Normal, Lights[i]);
            spec += DoSpecularSpot(worldPosition.xyz, input.Normal, Lights[i], input.Sheen);
		}
	}
	
    diffuse.xyz *= tex.xyz;	
	output.Depth = tex.w > 0.0f ?
		float4(input.PositionCopy.z / input.PositionCopy.w, 0.0f, 0.0f, 1.0f) :
		float4(0.0f, 0.0f, 0.0f, 0.0f);
		
    output.Color = float4(ambient + diffuse + spec, tex.w);
	
	float3 colorMul = min(input.Color.xyz, 1.0f); 
	output.Color.xyz *= colorMul.xyz;

	if (FogMaxDistance != 0)
		output.Color.xyz = lerp(output.Color.xyz, FogColor.xyz, input.Fog);

	return output;
}