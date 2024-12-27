Texture2D t_texture : register(t0);
RWTexture2D<float4> outBuffer : register(u0);

SamplerState s_sampler : register(s0);

cbuffer data : register(b0)
{
    uint b_LevelOfDetail;
}

[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    uint bufferWidth, bufferHeight;
    outBuffer.GetDimensions(bufferWidth, bufferHeight);

    float4 firstSample = t_texture.SampleLevel(s_sampler, uint2(bufferWidth, bufferHeight), 0);

    for(uint y = 0; y < bufferHeight; y++)
    {
        for(uint x = 0; x < bufferWidth; x++)
        {
            outBuffer[uint2(x,y)] = firstSample;
        }
    }
}