#include "./Math.hlsli"

#define LT_SUN		0
#define LT_POINT	1
#define LT_SPOT		2
#define LT_SHADOW	3

#define MAX_LIGHTS	48
#define SPEC_FACTOR 64

struct ShaderLight
{
	float3 Position;
	uint Type;
	float3 Color;
	float Intensity;
	float3 Direction;
	float In;
	float Out;
	float InRange;
	float OutRange;

	float padding;
};

cbuffer LightsBuffer : register(b2)
{
	ShaderLight Lights[MAX_LIGHTS];
	int NumLights;
};

float3 DoSpecularPoint(float3 pos, float3 n, ShaderLight light, float strength)
{
    if ((strength <= 0.0))
		return float3(0, 0, 0);
	else
	{
		float3 lightPos = light.Position.xyz;
		float radius = light.Out;

		float dist = distance(lightPos, pos);
		if (dist > radius)
			return float3(0, 0, 0);
		else
		{
			float3 lightDir = normalize(lightPos - pos);
			float3 reflectDir = reflect(lightDir, n);

			float3 color = light.Color.xyz;
			float intensity = saturate(light.Intensity);
			float spec = pow(saturate(dot(CamDirectionWS.xyz, reflectDir)), strength * SPEC_FACTOR);
			float attenuation = (radius - dist) / radius;

			return attenuation * spec * color * intensity;
		}
	}
}

float3 DoSpecularSun(float3 n, ShaderLight light, float strength)
{
    if (strength <= 0.0)
		return float3(0, 0, 0);
	else
	{
		float3 lightDir = normalize(light.Direction);
		float3 reflectDir = reflect(lightDir, n);

		float3 color = light.Color.xyz;
		float intensity = saturate(light.Intensity);
		float spec = pow(saturate(dot(CamDirectionWS.xyz, reflectDir)), strength * SPEC_FACTOR);

		return spec * color * intensity;
	}
}

float3 DoSpecularSpot(float3 pos, float3 n, ShaderLight light, float strength)
{
	if (strength <= 0.0)
		return float3(0, 0, 0);
	else
	{
		float3 lightPos = light.Position.xyz;
		float radius = light.OutRange;

		float dist = distance(lightPos, pos);
		if (dist > radius)
			return float3(0, 0, 0);
		else
		{
			float3 lightDir = normalize(lightPos - pos);
			float3 reflectDir = reflect(lightDir, n);

			float3 color = light.Color.xyz;
			float intensity = saturate(light.Intensity);
			float spec = pow(saturate(dot(CamDirectionWS.xyz, reflectDir)), strength * SPEC_FACTOR);
			float attenuation = (radius - dist) / radius;

			return attenuation * spec * color * intensity;
		}
	}
}

float3 DoPointLight(float3 pos, float3 n, ShaderLight light)
{
	float3 lightPos = light.Position.xyz;
	float3 color = light.Color.xyz;
	float intensity = saturate(light.Intensity);

	float3 lightVec = (lightPos - pos);
	float distance = length(lightVec);

	if (distance > light.Out)
		return float3(0, 0, 0);
	else
	{
		lightVec = normalize(lightVec);
		float d = saturate(dot(n, lightVec));

		float attenuation = 1.0f;
		if (distance > light.In)
			attenuation = 1.0f - saturate((distance - light.In) / (light.Out - light.In));

		return saturate(color * intensity * attenuation * d);
	}
}

float3 DoShadowLight(float3 pos, float3 n, ShaderLight light)
{
	float3 lightPos = light.Position.xyz;
	float3 color = light.Color.xyz;
	float intensity = light.Intensity;

	float3 lightVec = (lightPos - pos);
	float distance = length(lightVec);

	if (distance > light.Out)
		return float3(0, 0, 0);
	else
	{
		lightVec = normalize(lightVec);
		float d = saturate(dot(n, lightVec));

		float attenuation = 1.0f;
		if (distance > light.In)
			attenuation = 1.0f - saturate((distance - light.In) / (light.Out - light.In));

		float absolute = float3(color * intensity * attenuation);
		float directional = absolute * d;

		return ((absolute * 0.33f) + (directional * 0.66f)) * 2.0f;
	}
}

float3 DoSpotLight(float3 pos, float3 n, ShaderLight light)
{
	float3 lightPos = light.Position.xyz;
	float3 color = light.Color.xyz;
	float intensity = saturate(light.Intensity);
	float3 direction = -light.Direction.xyz;
	float innerRange = light.In;
	float outerRange = light.Out;
	float coneIn = light.InRange;
	float coneOut = light.OutRange;

	float3 lightVec = (lightPos - pos);
	float distance = length(lightVec);

	if (distance > outerRange)
		return float3(0, 0, 0);
	else
	{
		lightVec = normalize(lightVec);
		
		float d = saturate(dot(n, lightVec));
		if (d < 0)
			return float3(0, 0, 0);
		else
		{
			float cosine = dot(-lightVec, direction);

			float minCosineIn = cos(coneIn * (PI / 180.0f));
			float attenuationIn = max((cosine - minCosineIn), 0.0f) / (1.0f - minCosineIn);

			float minCosineOut = cos(coneOut * (PI / 180.0f));
			float attenuationOut = max((cosine - minCosineOut), 0.0f) / (1.0f - minCosineOut);

			float attenuation = saturate(attenuationIn * 2.0f + attenuationOut);
			
			if (attenuation > 0.0f)
			{
				float falloff = saturate((outerRange - distance) / (outerRange - innerRange + 1.0f));
				return saturate(color * intensity * attenuation * falloff * d);
			}
			else
				return float3(0, 0, 0);
		}
	}
}

float3 DoDirectionalLight(float3 pos, float3 n, ShaderLight light)
{
	float3 color = light.Color.xyz;
	float3 intensity = saturate(light.Intensity);
	float3 direction = light.Direction.xyz;

	direction = normalize(direction);

	float d = dot(n, direction);

	if (d < 0)
		return float3(0, 0, 0);
	else
		return (color * intensity * d);
}

float3 CombineLights(float3 ambient, float3 vertex, float3 tex, float3 pos, float3 normal, float sheen)
{
	float3 diffuse = 0;
	float3 shadow  = 0;
	float3 spec    = 0;

	for (int i = 0; i < NumLights; i++)
	{
		int lightType = Lights[i].Type;

		if (lightType == LT_POINT)
		{
			diffuse += DoPointLight(pos, normal, Lights[i]);
			spec += DoSpecularPoint(pos, normal, Lights[i], sheen);
		}
		else if (lightType == LT_SHADOW)
		{
			shadow += DoShadowLight(pos, normal, Lights[i]);
		}
		else if (lightType == LT_SUN)
		{
			diffuse += DoDirectionalLight(pos, normal, Lights[i]);
			spec += DoSpecularSun(normal, Lights[i], sheen);
		}
		else if (lightType == LT_SPOT)
		{
			diffuse += DoSpotLight(pos, normal, Lights[i]);
			spec += DoSpecularSpot(pos, normal, Lights[i], sheen);
		}
	}

	shadow = saturate(shadow);
	diffuse.xyz *= tex.xyz;

	float3 ambTex = saturate(ambient - shadow) * tex;
	float3 combined = ambTex + diffuse + spec;

	return (combined * vertex);
}

float3 StaticLight(float3 vertex, float3 tex)
{
	return saturate(tex * vertex);
}