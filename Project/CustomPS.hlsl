
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
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
};
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float time;
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
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
    float2 positions[10];
    float timeClamped;
    float modTime;
    for (int i = 0; i < 10; i++)
    {
        timeClamped = (time + i*4.5f/10.0f) % 4.5;
        modTime = timeClamped % 2;
        positions[i].x = timeClamped / 4.5f;
        positions[i].y = (modTime * (1 - floor(modTime) * 2) + floor(modTime) * 2) * 0.9f + 0.05f; // Triangle function
        if (distance(input.texCoord, positions[i]) <= 0.1f)
        {   
            return float4(colorTint);
        }
    }
    return float4(0, 0, 0, 1);
}