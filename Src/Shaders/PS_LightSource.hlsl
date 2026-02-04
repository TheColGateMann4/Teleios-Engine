struct PointLightData
{
    float3 lightPositionInCameraSpace;
    float3 lightDiffuseColor;
    float attenuationQuadratic;
    float attenuationLinear;
    float attenuationConstant;
};

#ifndef NUM_POINTLIGHTS
#define NUM_POINTLIGHTS 1 
#endif

cbuffer lightBuffer : register(b0)
{
    PointLightData b_pointlights[NUM_POINTLIGHTS];
};

float4 PSMain() : SV_TARGET
{
    return float4(b_pointlights[0].lightDiffuseColor, 1.0f);
}