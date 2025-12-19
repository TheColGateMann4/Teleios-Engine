RWTexture2D<float4> outBuffer : register(u0);

float4 ConvertToLinearRGB(float4 nonLinearSRGBVal)
{
    float4 result;
    const float srgbCheckVal = 0.04045;
    
    if (nonLinearSRGBVal.x <= srgbCheckVal &&
        nonLinearSRGBVal.y <= srgbCheckVal &&
        nonLinearSRGBVal.z <= srgbCheckVal)
    {
        result.xyz = nonLinearSRGBVal.xyz / 12.92f;
    }
    else
    {
        result.xyz = pow((nonLinearSRGBVal.xyz + 0.055f) / 1.055f, 2.4f);
    }
    
    result.a = nonLinearSRGBVal.a;
    
    return result;
}

[numthreads(16, 16, 1)]
void CSMain( uint3 DTid : SV_DispatchThreadID )
{
    uint outBufferWidth, outBufferHeight;
    outBuffer.GetDimensions(outBufferWidth, outBufferHeight);

    if(DTid.x > outBufferWidth || DTid.y > outBufferHeight)
        return;

    outBuffer[uint2(DTid.x, DTid.y)] = ConvertToLinearRGB(outBuffer[uint2(DTid.x, DTid.y)]);
}