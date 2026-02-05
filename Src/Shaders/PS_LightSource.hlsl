struct PointLightData
{
    float3 lightPositionInCameraSpace;
    float3 lightDiffuseColor;
    float attenuationQuadratic;
    float attenuationLinear;
    float attenuationConstant;
};

#ifndef NUM_POINTLIGHTS
#define NUM_POINTLIGHTS 10
#endif

cbuffer lightBuffer : register(b0)
{
    PointLightData b_pointlights[NUM_POINTLIGHTS];
};

cbuffer constants : register(b1)
{
    int pointLightIndex;
};

float4 PSMain() : SV_TARGET
{
    return float4(b_pointlights[pointLightIndex].lightDiffuseColor, 1.0f);
}