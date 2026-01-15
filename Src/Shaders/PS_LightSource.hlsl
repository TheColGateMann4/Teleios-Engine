cbuffer lightBuffer : register(b0)
{
    float3 b_lightPositionInCameraSpace;
    float3 b_lightDiffuseColor;
    float b_attenuationQuadratic;
    float b_attenuationLinear;
    float b_attenuationConstant;
};

float4 PSMain() : SV_TARGET
{
    return float4(b_lightDiffuseColor, 1.0f);
}