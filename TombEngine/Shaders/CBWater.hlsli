#ifndef CBWATERSHADER
#define CBWATERSHADER

#define MAX_WAVES 1024

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
    //--
#ifdef NEW_RIPPLES
    float4 RipplesPosSize[MAX_WAVES];
    //--
    float4 RipplesParameters[MAX_WAVES];
    //--
    int RipplesCount;
    //--
#endif
    float4 WaterDistortionMapUvCoordinates;
    float4 WaterNormalMapUvCoordinates;
    float4 WaterFoamMapUvCoordinates;
};

#endif