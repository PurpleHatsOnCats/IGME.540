TextureCube SkyTexture: register(t0);
SamplerState Sampler : register(s0);

struct VertexToPixelSky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

float4 main(VertexToPixelSky input) : SV_TARGET
{
    //return float4(1, 1, 1, 1);
    float4 color = SkyTexture.Sample(Sampler, input.sampleDir);
    return color;
}