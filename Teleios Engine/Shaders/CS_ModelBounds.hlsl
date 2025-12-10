StructuredBuffer<float3> vertIn : register(t0);
RWStructuredBuffer<float3> outMin : register(u0);
RWStructuredBuffer<float3> outMax : register(u1);

cbuffer Params : register(b0)
{
    uint fieldOffsetOfVertices;
    uint numVertices;
}

#define FLT_MAX 3.402823466e+38F

#define NUM_THREADS 256
#define MAX_WAVES_PER_LANE 32

groupshared float3 sharedMin[NUM_THREADS / MAX_WAVES_PER_LANE];
groupshared float3 sharedMax[NUM_THREADS / MAX_WAVES_PER_LANE];

[numthreads(NUM_THREADS, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID, uint3 GroupThreadID : SV_GroupThreadID)
{
    uint tid = DTid.x;
    
    if (tid >= numVertices)
        return;

    float3 pos = vertIn[fieldOffsetOfVertices + tid];

    // Wave-level reduction within threadgroup
    float3 waveMin = WaveActiveMin(pos);
    float3 waveMax = WaveActiveMax(pos);

    // First lane in wave writes to shared memory
    if (WaveIsFirstLane())
    {
        uint waveIndex = GroupThreadID.x / WaveGetLaneCount();
        sharedMin[waveIndex] = waveMin;
        sharedMax[waveIndex] = waveMax;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (GroupThreadID.x == 0)
    {
        float3 finalMin = float3(FLT_MAX, FLT_MAX, FLT_MAX);
        float3 finalMax = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // accounting for rounding error, like 257 / 32 would round it to correct number
        uint waveCount = (NUM_THREADS + WaveGetLaneCount() - 1) / WaveGetLaneCount();
        
        for (int i = 0; i < waveCount; i++)
        {
            finalMin = min(finalMin, sharedMin[i]);
            finalMax = min(finalMax, sharedMax[i]);
        }

        outMin[tid / NUM_THREADS] = finalMin;
        outMax[tid / NUM_THREADS] = finalMax;
    }
}