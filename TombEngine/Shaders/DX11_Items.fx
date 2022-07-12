#include "./Math.hlsli"
#include "./CameraMatrixBuffer.hlsli"
#include "./ShaderLight.hlsli"
#include "./VertexInput.hlsli"
#include "./AlphaTestBuffer.hlsli"

cbuffer ItemBuffer : register(b1) 
{
	float4x4 World;
	float4x4 Bones[32];
	float4 ItemPosition;
	float4 AmbientLight;
};

cbuffer LightsBuffer : register(b2)
{
	ShaderLight Lights[MAX_LIGHTS];
	int NumLights;
	float3 CameraPosition;
};

cbuffer MiscBuffer : register(b3)
{
	int Caustics;
};

struct PixelShaderInput
{
	float4 Position: SV_POSITION;
	float3 Normal: NORMAL;
	float3 WorldPosition : POSITION;
	float2 UV: TEXCOORD;
	float4 Color: COLOR;
	float Sheen : SHEEN;
	float3x3 TBN : TBN;
	float Fog : FOG;
	float4 PositionCopy : TEXCOORD2;
};

struct PixelShaderOutput
{
	float4 Color: SV_Target0;
	float4 Depth: SV_Target1;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

Texture2D NormalTexture : register(t1);

//TextureCube Reflection : register (t4);

PixelShaderInput VS(VertexShaderInput input)
{
	PixelShaderInput output;

	float4x4 world = mul(Bones[input.Bone], World);

	float3 Normal = (mul(float4(input.Normal, 0.0f), world).xyz);
	float3 WorldPosition = (mul(float4(input.Position, 1.0f), world));

	output.Normal = Normal;
	output.UV = input.UV;
	output.WorldPosition = WorldPosition;
	
	float3 Tangent = mul(float4(input.Tangent, 0), world).xyz;
	float3 Bitangent = mul(float4(input.Bitangent, 0), world).xyz;
	float3x3 TBN = float3x3(Tangent, Bitangent, Normal);

	output.TBN = transpose(TBN);
	
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
	
	output.Position = mul(mul(float4(pos, 1.0f), world), ViewProjection);
	output.Color = col;

	// Apply distance fog
	float d = distance(CamPositionWS.xyz, WorldPosition);
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
    float3 ambient = AmbientLight.xyz * tex.xyz;

	float3 normal = NormalTexture.Sample(Sampler, input.UV).rgb;
	normal = normal * 2 - 1;
	normal = normalize(mul(input.TBN, normal));

	float3 diffuse = 0;
    float3 spec = 0;
	
	for (int i = 0; i < NumLights; i++)
	{
		int lightType = Lights[i].Type;

		if (lightType == LT_POINT || lightType == LT_SHADOW)
		{
            diffuse += DoPointLight(input.WorldPosition, normal, Lights[i]);
            spec += DoSpecularPoint(input.WorldPosition, normal, Lights[i], input.Sheen);

        }
		else if (lightType == LT_SUN)
		{
            diffuse += DoDirectionalLight(input.WorldPosition, normal, Lights[i]);
            spec += DoSpecularSun(normal, Lights[i], input.Sheen);

		}
		else if (lightType == LT_SPOT)
		{
            diffuse += DoSpotLight(input.WorldPosition, normal, Lights[i]);
            spec += DoSpecularSpot(input.WorldPosition, normal, Lights[i], input.Sheen);

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