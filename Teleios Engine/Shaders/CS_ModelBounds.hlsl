StructuredBuffer<float> vertIn : register(t0);
RWStructuredBuffer<uint> outMin : register(u0); // original type is float3
RWStructuredBuffer<uint> outMax : register(u1);

cbuffer Params : register(b0)
{
    uint fieldOffsetOfVertices;
    uint numVertices;
}

uint FloatToOrderedUint(float val)
{
    uint u = asuint(val);
    return (u & 0x80000000u) != 0u ? ~u : (u ^ 0x80000000u);
}

float OrderedUintToFloat(uint u)
{
    uint f = (u & 0x80000000u) != 0u ? (u ^ 0x80000000u) : ~u;
    return asfloat(f);
}

groupshared float3 sharedMin[256];
groupshared float3 sharedMax[256];

[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID)
{
    uint tid = DTid.x;
    uint vInd = tid * 3;
    
    if (tid >= numVertices)
        return;

    float3 pos = float3(
        vertIn[fieldOffsetOfVertices + vInd],
        vertIn[fieldOffsetOfVertices + vInd + 1],
        vertIn[fieldOffsetOfVertices + vInd + 2]
    );

    // Initialize shared memory
    sharedMin[GroupThreadID.x] = pos;
    sharedMax[GroupThreadID.x] = pos;
    GroupMemoryBarrierWithGroupSync();

    // Wave-level reduction within threadgroup
    float3 waveMin = float3(
        WaveActiveMin(pos.x),
        WaveActiveMin(pos.y),
        WaveActiveMin(pos.z)
    );

    float3 waveMax = float3(
        WaveActiveMax(pos.x),
        WaveActiveMax(pos.y),
        WaveActiveMax(pos.z)
    );

    // First lane in wave writes to shared memory
    if (WaveIsFirstLane())
    {
        sharedMin[GroupThreadID.x] = waveMin;
        sharedMax[GroupThreadID.x] = waveMax;
    }
    GroupMemoryBarrierWithGroupSync();

    // Thread 0 of the threadgroup reduces shared memory to a single value
    if (GroupThreadID.x == 0)
    {
        float3 finalMin = float3(1e30, 1e30, 1e30);
        float3 finalMax = float3(-1e30, -1e30, -1e30);

        [unroll]
        for (uint i = 0; i < 256; i++)
        {
            finalMin = min(finalMin, sharedMin[i]);
            finalMax = max(finalMax, sharedMax[i]);
        }

        float tmp;

        InterlockedMin(outMin[0], FloatToOrderedUint(finalMin.x), tmp);
        InterlockedMin(outMin[1], FloatToOrderedUint(finalMin.y), tmp);
        InterlockedMin(outMin[2], FloatToOrderedUint(finalMin.z), tmp);

        InterlockedMax(outMax[0], FloatToOrderedUint(finalMax.x), tmp);
        InterlockedMax(outMax[1], FloatToOrderedUint(finalMax.y), tmp);
        InterlockedMax(outMax[2], FloatToOrderedUint(finalMax.z), tmp);
    }
}