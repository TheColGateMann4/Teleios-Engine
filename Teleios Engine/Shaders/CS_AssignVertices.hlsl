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

// Each threadgroup will process a block of vertices
groupshared float3 sharedSum[256];  // per-threadgroup cell sums
groupshared uint sharedCount[256];  // per-threadgroup cell counts

uint GridHash(uint3 cell, uint3 res)
{
    return cell.x + res.x * (cell.y + res.y * cell.z);
}

void InterlockedAddFloat3(uint cellIndex, float3 value)
{
    float orig, prev, newVal;
    
    //x
    {
        InterlockedCompareExchangeFloatBitwise(cellSum[cellIndex].x, cellSum[cellIndex].x, cellSum[cellIndex].x, orig);
        
        do
        {    
            newVal = orig + value.x;
            InterlockedCompareExchangeFloatBitwise(cellSum[cellIndex].x, orig, newVal, prev);
            if (prev == orig)
                break;
            orig = prev;
            newVal = orig + value.x;
        } while (true);
    }
    
    //y
    {
        InterlockedCompareExchangeFloatBitwise(cellSum[cellIndex].y, cellSum[cellIndex].y, cellSum[cellIndex].y, orig);
        
        do
        {
            newVal = orig + value.y;
            InterlockedCompareExchangeFloatBitwise(cellSum[cellIndex].y, orig, newVal, prev);
            if (prev == orig)
                break;
            orig = prev;
            newVal = orig + value.y;
        } while (true);
    }
    
    //x
    {
        InterlockedCompareExchangeFloatBitwise(cellSum[cellIndex].z, cellSum[cellIndex].z, cellSum[cellIndex].z, orig);
        
        do
        {
            newVal = orig + value.z;
            InterlockedCompareExchangeFloatBitwise(cellSum[cellIndex].z, orig, newVal, prev);
            if (prev == orig)
                break;
            orig = prev;
            newVal = orig + value.z;
        } while (true);
    }
}

[numthreads(256,1,1)]
void CSMain(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID)
{
    uint tid = DTid.x;
    if (tid >= numVertices)
        return;

    float3 pos = vertIn[tid];
    float3 rel = (pos - bbMin[0]) / (bbMax[0] - bbMin[0]);
    uint3 cell;
    cell.x = min(uint(rel.x * gridResolution.x), gridResolution.x - 1);
    cell.y = min(uint(rel.y * gridResolution.y), gridResolution.y - 1);
    cell.z = min(uint(rel.z * gridResolution.z), gridResolution.z - 1);

    uint cellIndex = GridHash(cell, gridResolution);
    vertToCell[tid] = cellIndex;

    uint lid = GTid.x;
    sharedSum[lid] = float3(0, 0, 0);
    sharedCount[lid] = 0;
    GroupMemoryBarrierWithGroupSync();

    // Accumulate within group
    sharedSum[lid] += pos;
    sharedCount[lid] += 1;
    GroupMemoryBarrierWithGroupSync();

    // Single thread per group writes to global UAV
    if (lid == 0)
    {
        float3 groupSum = 0;
        uint groupCount = 0;
        for (uint i = 0; i < 256; ++i)
        {
            groupSum += sharedSum[i];
            groupCount += sharedCount[i];
        }

        // Write to global atomically (use integer reinterpret for safety)
        InterlockedAddFloat3(cellIndex, groupSum);
        InterlockedAdd(cellCount[cellIndex], groupCount);
    }
}