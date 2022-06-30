
#define LT_SUN		0
#define LT_POINT	1
#define LT_SPOT		2
#define LT_SHADOW	3

#define MAX_LIGHTS	48

struct ShaderLight 
{
	float3 Position;
	int Type;
	float3 Color;
	float LocalIntensity;
	float3 Direction;
	float Distance;
	float Intensity;
	float In;
	float Out;
	float Range;
};


float3 DoSpecularPoint(float3 pos, float3 n, ShaderLight light, float strength)
{
    if (!(strength > 0.0))
    {
        return float3(0, 0, 0);
    }
    float3 lightPos = light.Position.xyz;
    float radius = light.Out;

    float dist = distance(lightPos, pos);
    if (dist > radius)
    {
        return float3(0, 0, 0);
    }
    float3 lightDir = normalize(lightPos - pos);
    float3 reflectDir = reflect(lightDir, n);

    float3 color = light.Color.xyz;
    float spec = pow(saturate(dot(CamDirectionWS.xyz, reflectDir)), strength * 64);
    float attenuation = (radius - dist) / radius;
    return attenuation * spec * color;

}

float3 DoSpecularSun(float3 n, ShaderLight light, float strength)
{
    if (!(strength > 0.0))
    {
        return float3(0, 0, 0);
    }
    float3 lightDir = normalize(light.Direction);
    float3 reflectDir = reflect(lightDir, n);
    float3 color = light.Color.xyz;
    float spec = pow(saturate(dot(CamDirectionWS.xyz, reflectDir)), strength * 64);
    return spec * color;
}

float3 DoSpecularSpot(float3 pos, float3 n, ShaderLight light, float strength)
{
    if (!(strength > 0.0))
    {
        return float3(0, 0, 0);
    }
    float3 lightPos = light.Position.xyz;
    float radius = light.Range;

    float dist = distance(lightPos, pos);
    if (dist > radius)
    {
        return float3(0, 0, 0);
    }
    float3 lightDir = normalize(lightPos - pos);
    float3 reflectDir = reflect(lightDir, n);

    float3 color = light.Color.xyz;
    float spec = pow(saturate(dot(CamDirectionWS.xyz, reflectDir)), strength * 64);
    float attenuation = (radius - dist) / radius;
    return attenuation * spec * color;

}

float3 DoPointLight(float3 pos, float3 n, ShaderLight light)
{
	float3 lightPos = light.Position.xyz;
	float3 color = light.Color.xyz;
	float radius = light.Out;

	float3 lightVec = (lightPos - pos);
	float distance = length(lightVec);

	if (distance > radius)
	{
		return float3(0, 0, 0);
	}

	lightVec = normalize(lightVec);
	float d = saturate(dot(n, lightVec));

	float attenuation = ((radius - distance) / radius);

	return (color * attenuation * d);
}

float3 DoSpotLight(float3 pos, float3 n, ShaderLight light)
{
	float3 lightPos = light.Position.xyz;
	float3 color = light.Color.xyz;
	float3 direction = -light.Direction.xyz;
	float range = light.Range;
	float inAngle = light.In;
	float outAngle = light.Out;

	float3 lightVec = (lightPos - pos);
	float distance = length(lightVec);

	if (distance > range)
	{
		return float3(0, 0, 0);
	}

	lightVec = normalize(lightVec);
    float inCone = acos(dot(lightVec, direction));
	
	if (inCone < outAngle)
	{
		return float3(0, 0, 0);
	}

	float attenuation = 1;

	float d = saturate(dot(n, lightVec));
	if (d < 0)
	{
		return float3(0, 0, 0);
	}

	return (color * attenuation * d);
}

float3 DoDirectionalLight(float3 pos, float3 n, ShaderLight light)
{
	float3 color = light.Color.xyz;
	float3 direction = light.Direction.xyz;

	direction = normalize(direction);

	float d = dot(n, direction);
	if (d < 0)
	{
		return float3(0, 0, 0);
	}

	return (color * d);
}
