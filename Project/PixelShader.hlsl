#include "LightingHeader.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
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
float4 main(VertexToPixel input) : SV_TARGET
{
    float4 surfaceColor = colorTint * SurfaceTexture.Sample(BasicSampler, input.texCoord * uvscale + uvoffset);
    float3 normal = normalize(input.normal);
    float3 dirToCamera = normalize(input.worldPosition - cameraPosition);
    float4 c = float4(0, 0, 0, 0);
    
    for (int i = 0; i < numLights; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL: c += directionalLight(input.worldPosition, dirToCamera, normal, lights[i], surfaceColor); break;
            case LIGHT_TYPE_POINT: c += directionalLight(input.worldPosition, dirToCamera, normal, lights[i], surfaceColor); break;
            case LIGHT_TYPE_SPOT: c += directionalLight(input.worldPosition, dirToCamera, normal, lights[i], surfaceColor); break;
        }
    }
    // Ambient
    c += float4(ambientColor, 1) * surfaceColor;
    // No specular constant yet...

    return c;
}

