#include "LightingHeader.hlsli"

Texture2D AlbedoTexture : register(t0); // "t" registers for textures
Texture2D RoughnessTexture : register(t1); // "t" registers for textures
Texture2D MetalTexture : register(t2); // "t" registers for textures

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
    float4 albedoColor = colorTint * pow(AlbedoTexture.Sample(BasicSampler, input.texCoord * uvscale + uvoffset), 2.2);
    float3 dirToCamera = normalize(input.worldPosition - cameraPosition);
    
    
    float3 normal = normalize(input.normal);
    float roughness = RoughnessTexture.Sample(BasicSampler, input.texCoord).r;
    float metal = MetalTexture.Sample(BasicSampler, input.texCoord).r;
    
    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(0.04f, albedoColor.rgb, metal);
 
    return pow(calculateLight(normal, dirToCamera, roughness, specularColor, metal, input.worldPosition, albedoColor, numLights, lights, ambientColor), 1.0 / 2.2);
}

