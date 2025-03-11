#ifndef CBWATERSHADER
#define CBWATERSHADER

cbuffer WaterConstantBuffer : register(b2)
{
    float4x4 WaterReflectionViews[8];
    //--
    float4 WaterLevels[8];
    //--
    float4x4 SkyWorldMatrices[8];
    //--
    float4x4 WaterReflectionView;
    //--
    float3 LightPosition;
    float KSpecular;
	//--
    float3 LightColor;
    float Shininess;
	//--
    float MoveFactor;
    float WaveStrength;
    int WaterLevel;
    int WaterPlaneIndex;
    //--
    float4 SkyColor;
    //--
    float3 WaterFogColor;
    float WaterDepthScale;
    //--
    float3 AbsorptionCoefficient;
    float WaterFogDensity;
};

#endif