#include "LightingHeader.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D NormalTexture : register(t1); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float2 uvscale;
    float2 uvoffset;
    float3 cameraPosition;
    float time;
    float3 ambientColor;
    int numLights;
    Light lights[MAX_LIGHTS];
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(V2PTangent input) : SV_TARGET
{
    float4 surfaceColor = colorTint * SurfaceTexture.Sample(BasicSampler, input.texCoord * uvscale + uvoffset);
    float3 dirToCamera = normalize(input.worldPosition - cameraPosition);
    
    float3 unpackedNormal = (NormalTexture.Sample(BasicSampler, input.texCoord * uvscale + uvoffset) * 2 - 1);
    
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent - dot(input.tangent,N) * N);
    float3 B = cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N);
    float3 normal = mul(unpackedNormal, TBN);
 
    return calculateLight(input.worldPosition, surfaceColor, normal, dirToCamera, numLights, lights, ambientColor);
}

