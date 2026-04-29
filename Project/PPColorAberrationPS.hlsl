struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

cbuffer externalData : register(b0)
{
    float2 redOffset;
    float2 greenOffset;
    float2 blueOffset;
}

float4 main(VertexToPixel input) : SV_TARGET
{
    float4 color = float4(0,0,0,1);
    color.r = Pixels.Sample(ClampSampler, input.uv + redOffset).r;
    color.g = Pixels.Sample(ClampSampler, input.uv + greenOffset).g;
    color.b = Pixels.Sample(ClampSampler, input.uv + blueOffset).b;
    
    return color;
}