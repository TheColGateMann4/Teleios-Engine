Texture2D t_depthStencil : register(t1);
RWStructuredBuffer<float> u_middleDepth : register(u0);

#define NUM_THREADS 5
#define FLOOR_HALF_NUM_THREADS (NUM_THREADS / 2)
#define TOTAL_NUM_THREADS NUM_THREADS * NUM_THREADS

groupshared float g_sharedDepthTable[TOTAL_NUM_THREADS];

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void CSMain(uint3 GTid : SV_GroupThreadID)
{
    uint2 textureSize;
    
    t_depthStencil.GetDimensions(textureSize.x, textureSize.y);
    
    uint2 textureMiddle = textureSize / 2;
    int2 pixelOffset = int2(GTid.xy) - int2(FLOOR_HALF_NUM_THREADS, FLOOR_HALF_NUM_THREADS);
    uint2 targetPixelPosition = textureMiddle + pixelOffset;
    
    uint sharedIndex = GTid.x + GTid.y * NUM_THREADS;
    g_sharedDepthTable[sharedIndex] = t_depthStencil.Load(uint3(targetPixelPosition, 0)).r;
    
    GroupMemoryBarrierWithGroupSync();
    
    if (GTid.x == 0 && GTid.y == 0)
    {
        float accumulatedDepth = 0.0f;
        
        for (uint i = 0; i < TOTAL_NUM_THREADS; i++)
            accumulatedDepth += g_sharedDepthTable[i];
        
        u_middleDepth[0] = accumulatedDepth / (TOTAL_NUM_THREADS);
    }
}