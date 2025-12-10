cbuffer GridParams : register(b0)
{
    uint3 gridResolution;   // cells in x, y, z
    uint numVertices;
    uint numCells;
}

StructuredBuffer<float3> vertIn : register(t0);
StructuredBuffer<float3> bbMin : register(t1);
StructuredBuffer<float3> bbMax : register(t2);

RWStructuredBuffer<float3> cellSum : register(u0);   // global cell sums
RWStructuredBuffer<uint> cellCount : register(u1);   // global cell counts
RWStructuredBuffer<uint> vertToCell : register(u2);  // vertex -> cell mapping

uint GetGridHash(uint3 cell, uint3 res)
{
    return cell.x + res.x * (cell.y + res.y * cell.z);
}

#define NUM_THREADS 256
#define MAX_WAVES_PER_LANE 32

groupshared float3 sharedSums[NUM_THREADS];
groupshared uint sharedCounts[NUM_THREADS];

[numthreads(NUM_THREADS, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID)
{
    uint tid = DTid.x;
    if (tid >= numVertices)
        return;

    float3 pos = vertIn[tid];
    float3 rel = (pos - bbMin[0]) / (bbMax[0] - bbMin[0]);
    uint3 cellIndex = min(uint3(rel * gridResolution), gridResolution - 1);
    uint cellHash = GetGridHash(cellIndex, gridResolution);
    vertToCell[tid] = cellIndex;
    
    float3 waveSum = WaveActiveSum(pos);
    uint waveCount = WaveActiveCountBits(true);
    
    if (WaveIsFirstLane())
    {
        sharedSums[GroupThreadID.x] = waveSum;
        sharedCounts[GroupThreadID.x] = waveCount;
    }

}