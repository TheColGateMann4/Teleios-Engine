
#ifdef TEXTURE_ANY
    SamplerState sampler_ : register(s0);
#endif

#ifdef TEXTURE_DIFFUSE
    Texture2D texture_ : register(t0);
#endif

cbuffer lightBuffer : register(b0)
{
    float3 lightPositionInCameraSpace;
    float3 diffuseColor;
    float attenuationQuadratic;
    float attenuationLinear;
    float attenuationConstant;
};

float4 PSMain(
#ifdef INPUT_TEXCCORDS  
    float2 textureCoords : TEXCOORDS,
#endif

    float3 positionInCameraSpace : CAMERAPOSITION
) : SV_TARGET
{

#ifdef TEXTURE_DIFFUSE
    float4 sample = texture_.Sample(sampler_, textureCoords);
#else
    float4 sample = float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif

    float distanceFromLight = length(lightPositionInCameraSpace - positionInCameraSpace);

    float attenuation = 1.0f / ((attenuationConstant) + (attenuationLinear * distanceFromLight) + (attenuationQuadratic * distanceFromLight * distanceFromLight));

    float3 diffuse = diffuseColor * attenuation;

    return float4(sample.rgb * diffuse, 1.0f);
}