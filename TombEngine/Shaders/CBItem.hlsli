#ifndef CBITEMSHADER
#define CBITEMSHADER

#include "./Math.hlsli"

#define MAX_BONES 32

cbuffer ItemBuffer : register(b1)
{
    float4x4 World;
    float4x4 Bones[MAX_BONES];
    float4 Color;
    float4 AmbientLight;
    int4 BoneLightModes[MAX_BONES / 4];
    ShaderLight ItemLights[MAX_LIGHTS_PER_ITEM];
    int NumItemLights;
};

#endif