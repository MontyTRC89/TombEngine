/**
 * Copyright (C) 2013 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2013 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2013 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2013 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2013 Diego Gutierrez (diegog@unizar.es)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. As clarification, there
 * is no requirement that the copyright notice and permission be included in
 * binary distributions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

cbuffer SMAABuffer : register(b13)
{
    /**
     * This is only required for temporal modes (SMAA T2x).
     */
    float4 subsampleIndices;

    /**
     * This is required for blending the results of previous subsample with the
     * output render target; it's used in SMAA S2x and 4x, for other modes just use
     * 1.0 (no blending).
     */
    float blendFactor;

    /**
     * This can be ignored; its purpose is to support interactive custom parameter
     * tweaking.
     */
    float threshld;
    float maxSearchSteps;
    float maxSearchStepsDiag;
    float cornerRounding;
};

// Use a real macro here for maximum performance!
#ifndef SMAA_RT_METRICS // This is just for compilation-time syntax checking.
#define SMAA_RT_METRICS float4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)
#endif

// Set the HLSL version:
#ifndef SMAA_HLSL_4_1
#define SMAA_HLSL_4
#endif

// Set preset defines:
#ifdef SMAA_PRESET_CUSTOM
#define SMAA_THRESHOLD threshld
#define SMAA_MAX_SEARCH_STEPS maxSearchSteps
#define SMAA_MAX_SEARCH_STEPS_DIAG maxSearchStepsDiag
#define SMAA_CORNER_ROUNDING cornerRounding
#endif

// And include our header!
#include "SMAA.hlsli"

// Set pixel shader version accordingly:
#if SMAA_HLSL_4_1
#define PS_VERSION ps_4_1
#else
#define PS_VERSION ps_4_0
#endif


/**
 * DepthStencilState's and company
 */
DepthStencilState DisableDepthStencil {
    DepthEnable = FALSE;
    StencilEnable = FALSE;
};

DepthStencilState DisableDepthReplaceStencil {
    DepthEnable = FALSE;
    StencilEnable = TRUE;
    FrontFaceStencilPass = REPLACE;
};

DepthStencilState DisableDepthUseStencil {
    DepthEnable = FALSE;
    StencilEnable = TRUE;
    FrontFaceStencilFunc = EQUAL;
};

BlendState Blend {
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = BLEND_FACTOR;
    DestBlend = INV_BLEND_FACTOR;
    BlendOp = ADD;
};

BlendState NoBlending {
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};


/**
 * Input textures
 */
Texture2D colorTex : register(t0);
Texture2D colorTexGamma : register(t1);
Texture2D colorTexPrev : register(t2);
Texture2DMS<float4, 2> colorTexMS;
Texture2D depthTex : register(t3);
Texture2D velocityTex : register(t4);

/**
 * Temporal textures
 */
Texture2D edgesTex : register(t5);
Texture2D blendTex : register(t6);

/**
 * Pre-computed area and search textures
 */
Texture2D areaTex : register(t7);
Texture2D searchTex : register(t8);


/**
 * Function wrappers
 */
void DX11_SMAAEdgeDetectionVS(float3 position : POSITION,
    out float4 svPosition : SV_POSITION,
    inout float2 texcoord : TEXCOORD0,
    out float4 offset[3] : TEXCOORD1) {
    svPosition = float4(position, 1.0f);
    SMAAEdgeDetectionVS(texcoord, offset);
}

void DX11_SMAABlendingWeightCalculationVS(float3 position : POSITION,
    out float4 svPosition : SV_POSITION,
    inout float2 texcoord : TEXCOORD0,
    out float2 pixcoord : TEXCOORD1,
    out float4 offset[3] : TEXCOORD2) {
    svPosition = float4(position, 1.0f);
    SMAABlendingWeightCalculationVS(texcoord, pixcoord, offset);
}

void DX11_SMAANeighborhoodBlendingVS(float3 position : POSITION,
    out float4 svPosition : SV_POSITION,
    inout float2 texcoord : TEXCOORD0,
    out float4 offset : TEXCOORD1) {
    svPosition = float4(position, 1.0f);
    SMAANeighborhoodBlendingVS(texcoord, offset);
}

void DX11_SMAAResolveVS(float3 position : POSITION,
    out float4 svPosition : SV_POSITION,
    inout float2 texcoord : TEXCOORD0) {
    svPosition = float4(position, 1.0f);
}

void DX11_SMAASeparateVS(float3 position : POSITION,
    out float4 svPosition : SV_POSITION,
    inout float2 texcoord : TEXCOORD0) {
    svPosition = float4(position, 1.0f);
}

float2 DX11_SMAALumaEdgeDetectionPS(float4 position : SV_POSITION,
    float2 texcoord : TEXCOORD0,
    float4 offset[3] : TEXCOORD1) : SV_TARGET{
#if SMAA_PREDICATION
return SMAALumaEdgeDetectionPS(texcoord, offset, colorTexGamma, depthTex);
#else
return SMAALumaEdgeDetectionPS(texcoord, offset, colorTexGamma);
#endif
}

float2 DX11_SMAAColorEdgeDetectionPS(float4 position : SV_POSITION,
    float2 texcoord : TEXCOORD0,
    float4 offset[3] : TEXCOORD1) : SV_TARGET{
#if SMAA_PREDICATION
return SMAAColorEdgeDetectionPS(texcoord, offset, colorTexGamma, depthTex);
#else
return SMAAColorEdgeDetectionPS(texcoord, offset, colorTexGamma);
#endif
}

float2 DX11_SMAADepthEdgeDetectionPS(float4 position : SV_POSITION,
    float2 texcoord : TEXCOORD0,
    float4 offset[3] : TEXCOORD1) : SV_TARGET{
return SMAADepthEdgeDetectionPS(texcoord, offset, depthTex);
}

float4 DX11_SMAABlendingWeightCalculationPS(float4 position : SV_POSITION,
    float2 texcoord : TEXCOORD0,
    float2 pixcoord : TEXCOORD1,
    float4 offset[3] : TEXCOORD2) : SV_TARGET{
return SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, edgesTex, areaTex, searchTex, subsampleIndices);
}

float4 DX11_SMAANeighborhoodBlendingPS(float4 position : SV_POSITION,
    float2 texcoord : TEXCOORD0,
    float4 offset : TEXCOORD1) : SV_TARGET{
#if SMAA_REPROJECTION
return SMAANeighborhoodBlendingPS(texcoord, offset, colorTex, blendTex, velocityTex);
#else
return SMAANeighborhoodBlendingPS(texcoord, offset, colorTex, blendTex);
#endif
}

float4 DX11_SMAAResolvePS(float4 position : SV_POSITION,
    float2 texcoord : TEXCOORD0) : SV_TARGET{
#if SMAA_REPROJECTION
return SMAAResolvePS(texcoord, colorTex, colorTexPrev, velocityTex);
#else
return SMAAResolvePS(texcoord, colorTex, colorTexPrev);
#endif
}

void DX11_SMAASeparatePS(float4 position : SV_POSITION,
    float2 texcoord : TEXCOORD0,
    out float4 target0 : SV_TARGET0,
    out float4 target1 : SV_TARGET1) {
    SMAASeparatePS(position, texcoord, target0, target1, colorTexMS);
}

/**
 * Edge detection techniques
 */
technique10 LumaEdgeDetection {
    pass LumaEdgeDetection {
        SetVertexShader(CompileShader(vs_4_0, DX11_SMAAEdgeDetectionVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(PS_VERSION, DX11_SMAALumaEdgeDetectionPS()));

        SetDepthStencilState(DisableDepthReplaceStencil, 1);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique10 ColorEdgeDetection {
    pass ColorEdgeDetection {
        SetVertexShader(CompileShader(vs_4_0, DX11_SMAAEdgeDetectionVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(PS_VERSION, DX11_SMAAColorEdgeDetectionPS()));

        SetDepthStencilState(DisableDepthReplaceStencil, 1);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

technique10 DepthEdgeDetection {
    pass DepthEdgeDetection {
        SetVertexShader(CompileShader(vs_4_0, DX11_SMAAEdgeDetectionVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(PS_VERSION, DX11_SMAADepthEdgeDetectionPS()));

        SetDepthStencilState(DisableDepthReplaceStencil, 1);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

/**
 * Blending weight calculation technique
 */
technique10 BlendingWeightCalculation {
    pass BlendingWeightCalculation {
        SetVertexShader(CompileShader(vs_4_0, DX11_SMAABlendingWeightCalculationVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(PS_VERSION, DX11_SMAABlendingWeightCalculationPS()));

        SetDepthStencilState(DisableDepthUseStencil, 1);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

/**
 * Neighborhood blending technique
 */
technique10 NeighborhoodBlending {
    pass NeighborhoodBlending {
        SetVertexShader(CompileShader(vs_4_0, DX11_SMAANeighborhoodBlendingVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(PS_VERSION, DX11_SMAANeighborhoodBlendingPS()));

        SetDepthStencilState(DisableDepthStencil, 0);
        SetBlendState(Blend, float4(blendFactor, blendFactor, blendFactor, blendFactor), 0xFFFFFFFF);
        // For SMAA 1x, just use NoBlending!
    }
}

/**
 * Temporal resolve technique
 */
technique10 Resolve {
    pass Resolve {
        SetVertexShader(CompileShader(vs_4_0, DX11_SMAAResolveVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(PS_VERSION, DX11_SMAAResolvePS()));

        SetDepthStencilState(DisableDepthStencil, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}

/**
 * 2x multisampled buffer conversion into two regular buffers
 */
technique10 Separate {
    pass Separate {
        SetVertexShader(CompileShader(vs_4_0, DX11_SMAASeparateVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(PS_VERSION, DX11_SMAASeparatePS()));

        SetDepthStencilState(DisableDepthStencil, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
