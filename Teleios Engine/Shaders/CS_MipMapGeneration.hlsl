Texture2D t_texture : register(t0);
RWTexture2D<float4> outBuffer : register(u0);

SamplerState s_sampler : register(s0);

// handle mip maps that have odd number of pixels
[numthreads(16, 16, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    uint outBufferWidth, outBufferHeight;
    outBuffer.GetDimensions(outBufferWidth, outBufferHeight);

    if(DTid.x > outBufferWidth || DTid.y > outBufferHeight)
        return;

    float2 pixelRatio = float2(1.0f / outBufferWidth, 1.0f / outBufferHeight);

    float2 textureCoords = pixelRatio * DTid.xy + pixelRatio / 2;
    outBuffer[uint2(DTid.x, DTid.y)] = t_texture.SampleLevel(s_sampler, textureCoords, 0);
}