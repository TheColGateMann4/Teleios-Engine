RWStructuredBuffer<float3> cellSum : register(u0);
RWStructuredBuffer<uint> cellCount : register(u1);
RWStructuredBuffer<float3> vertOut : register(u2);

[numthreads(256,1,1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint cid = DTid.x;
    
    uint length = 0;
    uint stride = 0;
    cellSum.GetDimensions(length, stride);
    
    if (cid >= length)
        return;

    uint count = cellCount[cid];
    if (count == 0) return;

    vertOut[cid] = cellSum[cid] / count;
}