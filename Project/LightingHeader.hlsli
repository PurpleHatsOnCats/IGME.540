#ifndef __LIGHTING_HEADER__ // Each .hlsli file needs a unique identifier!
#define __LIGHTING_HEADER__
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 texCoord :		TEXCOORD;
    float3 normal :			NORMAL;
    float3 worldPosition :	POSITION;
};
struct V2PTangent
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT;
    float4 shadowMapPos : SHADOW_POSITION;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition :	POSITION; // XYZ position
    float2 texCoord :		TEXCOORD;
    float3 normal :			NORMAL;
    float3 tangent :		TANGENT;
};

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_LIGHTS 30
#define MIN_ROUGHNESS 0.0000001

struct Light
{
    int Type; // Which kind of light? 0, 1 or 2 (see above)
    float3 Direction; // Directional and Spot lights need a direction
    float Range; // Point and Spot lights have a max range for attenuation
    float3 Position; // Point and Spot lights have a position in space
    float Intensity; // All lights need an intensity
    float3 Color; // All lights need a color
    float SpotInnerAngle; // Inner cone angle (in radians) – Inside this, full light!
    float SpotOuterAngle; // Outer cone angle (radians) – Outside this, no light!
    float2 Padding; // Purposefully padding to hit the 16-byte boundary
};

float3 diffuse(float3 dirToLight, float3 normal, Light light)
{
    // Lambert Model
    return saturate(dot(dirToLight, normal) * light.Color * light.Intensity);
}

float3 specular(float3 dirToLight, float3 dirToCamera, float3 normal, Light light)
{
    // Phong Model
    return pow(saturate(dot(reflect(dirToLight, normal), dirToCamera)), 128) * light.Color * light.Intensity;
}
static const float pi = 3.14159265359f;
float NormalDistribution(float3 n, float3 h, float r)
{
    // GGX, or Ground Glass X (Trowbridge-Reitz)
    // Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = r * r; // Remapping roughness
    float a2 = max(a * a, MIN_ROUGHNESS);
    // Denominator to be squared is ((n dot h)^2 * (a^2 - 1) + 1)
    float denomToSquare = NdotH2 * (a2 - 1) + 1;
    return a2 / (pi * denomToSquare * denomToSquare);

}
float GeometricShadowing(float3 n, float3 v, float3 l, float r)
{
    // k = remapped roughness
    float k = pow(r + 1, 2) / 8.0f;
    float ndotv = saturate(dot(n, v));
    float ndotl = saturate(dot(n, l));
    // Removed ndotv and ndotl from nominator to match MicrofacetBRDF()
    return (1 / (ndotv * (1 - k) + k)) * (1 / (ndotl * (1 - k) + k));

}
float3 Fresnel(float3 v, float3 h, float3 f0)
{
    return f0 + (1 - f0) * pow(1 - saturate(dot(v, h)), 5);
}
float3 MicrofacetBRDF(float ND, float GS, float3 F)
{
    // Removed ndotv and ndotl from denominator to match GeometricShadowing()
    return ND * GS * F / 4;
}
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}
float4 directionalLight(float3 normal, float3 dirToCamera, float roughness, float3 f0, float metal, Light light, float4 surfaceColor)
{
    float3 dirToLight = -normalize(light.Direction);
    float3 h = normalize(dirToCamera + dirToLight);
    
    float ND = NormalDistribution(normal, h, roughness);
    float GS = GeometricShadowing(normal, dirToCamera, dirToLight, roughness);
    float3 F = Fresnel(dirToCamera, h, f0);
    
    // Calculate Terms
    float4 diffuseTerm = float4(diffuse(dirToLight, normal, light), 1);
    float4 specularTerm = float4(MicrofacetBRDF(ND, GS, F), 1) * saturate(dot(normal, dirToLight));
    
    // Balance / energy convservation
    diffuseTerm = float4(DiffuseEnergyConserve(diffuseTerm.xyz, F, metal),1) * surfaceColor;
    
    // Combine Terms
    return diffuseTerm + specularTerm * any(diffuseTerm);
}
float4 pointLight(float3 normal, float3 dirToCamera, float roughness, float3 f0, float metal, Light light, float4 surfaceColor, float3 worldPosition)
{
    float3 toLight = light.Position - worldPosition;
    float3 dirToLight = normalize(toLight);    
    float3 h = normalize(dirToCamera + dirToLight);
    
    float ND = NormalDistribution(normal, h, roughness);
    float GS = GeometricShadowing(normal, dirToCamera, dirToLight, roughness);
    float3 F = Fresnel(dirToCamera, h, f0);
    
    // Calculate Terms
    float4 diffuseTerm = float4(diffuse(dirToLight, normal, light), 1);
    float4 specularTerm = float4(MicrofacetBRDF(ND, GS, F), 1);
    
    // Balance / energy convservation
    diffuseTerm = float4(DiffuseEnergyConserve(diffuseTerm.xyz, F, metal), 1) * surfaceColor;
        
    // Dim light based on distance
    float attenuationFactor = pow(max(0, 1.0f - dot(toLight, toLight) / (light.Range * light.Range)), 2);
    
    // Combine terms, adjust specular for edge cases
    return (diffuseTerm + specularTerm * any(diffuseTerm)) * attenuationFactor;
}
float4 spotLight(float3 normal, float3 dirToCamera, float roughness, float3 f0, float metal, Light light, float4 surfaceColor, float3 worldPosition)
{
    float4 pointTerm = pointLight(normal, dirToCamera, roughness, f0, metal, light, surfaceColor, worldPosition);
    
    float pixelAngle = saturate(dot(normalize(worldPosition - light.Position), normalize(light.Direction)));
    float cosInner = cos(radians(light.SpotInnerAngle));
    float cosOuter = cos(radians(light.SpotOuterAngle));
    float falloffRange = cosOuter - cosInner;
    
    return pointTerm * saturate((cosOuter - pixelAngle) / falloffRange);
}
float4 calculateLight(float3 normal, float3 dirToCamera, float roughness, float3 f0, float metal, float3 worldPosition, float4 surfaceColor, int numLights, 
    Light lights[MAX_LIGHTS], float shadowAmount)
{
    float4 c = float4(0, 0, 0, 0);
    float4 lightAmount = float4(0, 0, 0, 0);
    
    for (int i = 0; i < numLights; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                lightAmount = directionalLight(normal, dirToCamera, roughness, f0, metal, lights[i], surfaceColor);
                break;
            case LIGHT_TYPE_POINT:
                lightAmount = pointLight(normal, dirToCamera, roughness, f0, metal, lights[i], surfaceColor, worldPosition);
                break;
            case LIGHT_TYPE_SPOT:
                lightAmount = spotLight(normal, dirToCamera, roughness, f0, metal, lights[i], surfaceColor, worldPosition);
                break;
        }
        if (i == 0)
        {
            lightAmount *= shadowAmount;
        }
        c += lightAmount;
    }
    
    return c;
}
#endif