#include "LightingHeader.hlsli"

cbuffer ExternalData : register(b0)
{
    float4x4 view;
    float4x4 projection;
}

struct VertexToPixelSky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

VertexToPixelSky main(VertexShaderInput input)
{
    VertexToPixelSky output;
    float4x4 viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    matrix vp = mul(projection, viewNoTranslation);
    output.position = mul(vp, float4(input.localPosition, 1));
    output.position.z = output.position.w;

    output.sampleDir = input.localPosition;

	return output;
}