
static uint offset = 0;

static uint positionOffset = offset;
offset += 12;

#ifdef INPUT_TEXCCORDS  
static uint textureCoordsOffset = offset;
offset += 8;
#endif

static uint normalOffset = offset;
offset += 12;

static uint tangentOffset = offset;
offset += 12;

static uint bitangentOffset = offset;
offset += 12;

static uint VertexDataSize = offset;

struct VertexData
{
    float3 position;
	
#ifdef INPUT_TEXCCORDS  
     float2 textureCoords;
#endif

#ifdef INPUT_NORMAL  
     float3 normal;
#endif	

#ifdef INPUT_TANGENT  
     float3 tangent;
#endif	

#ifdef INPUT_BITANGENT  
     float3 bitangent;
#endif	
};

ByteAddressBuffer vertIn : register(t0);
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

    float3 pos = vertIn[fieldOffsetOfVertices + tid].position;

    // Wave-level reduction within threadgroup
    float3 waveMin = WaveActiveMin(pos);
    float3 waveMax = WaveActiveMax(pos);

    // First lane in wave writes to shared memory
    if (WaveIsFirstLane())
    {
        sharedMin[GroupThreadID.x] = waveMin;
        sharedMax[GroupThreadID.x] = waveMax;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    if (GroupThreadID.x == 0)
    {
        float3 finalMin = float3(FLT_MAX, FLT_MAX, FLT_MAX);
        float3 finalMax = float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // accounting for int-rounding, making it "round" upwards
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