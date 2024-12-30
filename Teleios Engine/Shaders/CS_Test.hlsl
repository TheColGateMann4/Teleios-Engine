Texture2D t_texture : register(t0);
RWTexture2D<float4> outBuffer : register(u0);

SamplerState s_sampler : register(s0);

cbuffer data : register(b0)
{
    uint b_LevelOfDetail;
}

// handle mip maps that have odd number of pixels
// translate SRGB color space to normal one
[numthreads(1, 1, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    uint outBufferWidth, outBufferHeight;
    outBuffer.GetDimensions(outBufferWidth, outBufferHeight);

    float2 pixelRatio = float2(1.0f / outBufferWidth, 1.0f / outBufferHeight);

    for(uint y = 0; y < outBufferWidth; y++)
    {
        for(uint x = 0; x < outBufferHeight; x++)
        {
            outBuffer[uint2(x,y)] = t_texture.SampleLevel(s_sampler, float2(pixelRatio.x * x + pixelRatio.x / 2, pixelRatio.y * y + pixelRatio.y / 2), b_LevelOfDetail);
        }
    }
}