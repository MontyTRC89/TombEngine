#include "./Math.hlsli"

cbuffer CBPostProcess : register(b7)
{
    float CinematicBarsHeight;
    float ScreenFadeFactor;
    int ViewportWidth;
    int ViewportHeight;
    //--
    float4 SSAOKernel[64];
};