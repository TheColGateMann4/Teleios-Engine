SamplerState s_sampler : register(s0);
Texture2D t_renderTarget : register(t0);
Texture2D t_depthStencil : register(t1);
StructuredBuffer<float> t_depthBuffer : register(t2);

cbuffer CameraData : register(b0)
{
    float b_nearPlane;
    float b_farPlane;
};

cbuffer DepthOfFieldData : register(b1)
{
    float b_focusDistance;
    float b_focusRange;
    float b_maxBlur;
};

static const float2 poisson[8] =
{
    float2(-0.326, -0.406),
    float2(-0.840, -0.074),
    float2(-0.696, 0.457),
    float2(-0.203, 0.621),
    float2(0.962, -0.195),
    float2(0.473, -0.480),
    float2(0.519, 0.767),
    float2(0.185, -0.893)
};

float GeViewSpaceDepth(float depth)
{
    return (b_nearPlane * b_farPlane) / (b_farPlane - depth * (b_farPlane - b_nearPlane));
}

float ComputeCoC(float linearFocusedDepth, float linearDepth)
{
    float coc = abs(linearDepth - linearFocusedDepth) / b_focusRange;
    return saturate(coc);
}

float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    float depth = t_depthStencil.Sample(s_sampler, textureCoords).r;
    float viewDepth = GeViewSpaceDepth(depth);
    float focusedViewDepth = GeViewSpaceDepth(t_depthBuffer[0]);

    float coc = ComputeCoC(focusedViewDepth, viewDepth);
    float blurRadius = coc * b_maxBlur;
    
    uint displaySizeX, displaySizeY;
    
    t_renderTarget.GetDimensions(displaySizeX, displaySizeY);
    
    float2 texelSize = 1.0 / float2(displaySizeX, displaySizeY);
    
    float3 color = t_renderTarget.Sample(s_sampler, textureCoords).rgb;
    float totalWeight = 1.0;

    for (int i = 0; i < 8; i++)
    {
        float2 offset = poisson[i] * blurRadius * texelSize;
        float3 sampleColor = t_renderTarget.Sample(s_sampler, textureCoords + offset).rgb;
        color += sampleColor;
        totalWeight += 1.0;
    }

    color /= totalWeight;
    
    return float4(color, 1.0f);
}