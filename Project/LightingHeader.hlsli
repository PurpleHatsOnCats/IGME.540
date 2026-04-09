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
#define MAX_LIGHTS 128

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
    return saturate(dot(dirToLight, normal) * light.Color * light.Intensity);
}

float3 specular(float3 dirToLight, float3 dirToCamera, float3 normal, Light light)
{
    return pow(saturate(dot(reflect(dirToLight, normal), dirToCamera)), 128) * light.Color * light.Intensity;
}
float4 directionalLight(float3 worldPosition, float3 dirToCamera, float3 normal, Light light, float4 surfaceColor)
{
    float3 dirToLight = -normalize(light.Direction);
    
    float4 diffuseTerm = float4(diffuse(dirToLight, normal, light), 1) * surfaceColor;
    float4 specularTerm = float4(specular(dirToLight, dirToCamera, normal, light), 1);
    return diffuseTerm + specularTerm * any(diffuseTerm);
}
float4 pointLight(float3 worldPosition, float3 dirToCamera, float3 normal, Light light, float4 surfaceColor)
{
    float3 toLight = light.Position - worldPosition;
    float3 dirToLight = normalize(toLight);
    float attenuationFactor = pow(max(0, 1.0f - dot(toLight, toLight) / (light.Range * light.Range)), 2);
    
    float4 diffuseTerm = float4(diffuse(dirToLight, normal, light), 1) * surfaceColor;
    float4 specularTerm = float4(specular(dirToLight, dirToCamera, normal, light), 1);
    return (diffuseTerm + specularTerm * any(diffuseTerm)) * attenuationFactor;
}
float4 spotLight(float3 worldPosition, float3 dirToCamera, float3 normal, Light light, float4 surfaceColor)
{
    float4 pointTerm = pointLight(worldPosition, dirToCamera, normal, light, surfaceColor);
    
    float pixelAngle = saturate(dot(normalize(worldPosition - light.Position), normalize(light.Direction)));
    float cosInner = cos(radians(light.SpotInnerAngle));
    float cosOuter = cos(radians(light.SpotOuterAngle));
    float falloffRange = cosOuter - cosInner;
    
    return pointTerm * saturate((cosOuter - pixelAngle) / falloffRange);
}
float4 calculateLight(float3 worldPosition, float4 surfaceColor, float3 normal, float3 dirToCamera, int numLights, 
                        Light lights[MAX_LIGHTS], float3 ambientColor)
{
    float4 c = float4(0, 0, 0, 0);
    
    for (int i = 0; i < numLights; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                c += directionalLight(worldPosition, dirToCamera, normal, lights[i], surfaceColor);
                break;
            case LIGHT_TYPE_POINT:
                c += pointLight(worldPosition, dirToCamera, normal, lights[i], surfaceColor);
                break;
            case LIGHT_TYPE_SPOT:
                c += spotLight(worldPosition, dirToCamera, normal, lights[i], surfaceColor);
                break;
        }
    }
    
    // Ambient
    c += float4(ambientColor, 1) * surfaceColor;
    // No specular constant yet...
    
    return c;
}
#endif