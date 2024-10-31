Texture2D texture_ : register(t0);
SamplerState sampler_ : register(s0);

cbuffer lightBuffer : register(b0)
{
    float3 lightPositionInCameraSpace;
    float3 diffuseColor;
    float attenuationQuadratic;
    float attenuationLinear;
    float attenuationConstant;
};

cbuffer constBuffer : register(b2)
{
    float texCoordsScale;
};

float4 PSMain(float3 positionInCameraSpace : CAMERAPOSITION, float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    float4 sample = texture_.Sample(sampler_, textureCoords * texCoordsScale);

    float distanceFromLight = length(lightPositionInCameraSpace - positionInCameraSpace);

    float attenuation = 1.0f / ((attenuationConstant) + (attenuationLinear * distanceFromLight) + (attenuationQuadratic * distanceFromLight * distanceFromLight));

    float3 diffuse = diffuseColor * attenuation;

    return float4(sample.rgb * diffuse, sample.a);
}