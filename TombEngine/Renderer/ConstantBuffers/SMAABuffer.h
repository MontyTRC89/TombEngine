#pragma once

struct alignas(16) CSMAABuffer
{
    /**
     * This is only required for temporal modes (SMAA T2x).
     */
    Vector4 SubsampleIndices;

    /**
     * This is required for blending the results of previous subsample with the
     * output render target; it's used in SMAA S2x and 4x, for other modes just use
     * 1.0 (no blending).
     */
    float BlendFactor;

    /**
     * This can be ignored; its purpose is to support interactive custom parameter
     * tweaking.
     */
    float Threshld;
    float MaxSearchSteps;
    float MaxSearchStepsDiag;
    float CornerRounding;
};