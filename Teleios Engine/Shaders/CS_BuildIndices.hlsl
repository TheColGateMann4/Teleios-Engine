StructuredBuffer<uint> indIn : register(t0);
StructuredBuffer<uint> vertToCell : register(t1);
RWStructuredBuffer<uint> indOut : register(u0);

[numthreads(256,1,1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    uint tid = DTid.x;
    
    uint length = 0;
    uint stride = 0;
    indIn.GetDimensions(length, stride);
    
    uint triCount = length / 3;
    if (tid >= triCount) return;

    uint i0 = indIn[tid*3 + 0];
    uint i1 = indIn[tid*3 + 1];
    uint i2 = indIn[tid*3 + 2];

    uint c0 = vertToCell[i0];
    uint c1 = vertToCell[i1];
    uint c2 = vertToCell[i2];

    // Skip degenerate triangles (all vertices in same cell)
    if (c0 == c1 || c0 == c2 || c1 == c2) return;

    uint outIdx = tid*3;
    indOut[outIdx+0] = c0;
    indOut[outIdx+1] = c1;
    indOut[outIdx+2] = c2;
}