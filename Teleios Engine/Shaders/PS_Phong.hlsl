
#ifdef TEXTURE_ANY
    SamplerState sampler_ : register(s0);
#endif

#ifdef TEXTURE_DIFFUSE
    Texture2D texture_diffuse : register(t0);
#endif

#ifdef TEXTURE_NORMAL
    Texture2D texture_normal : register(t1);
#endif

#ifdef TEXTURE_SPECULAR
    Texture2D texture_specular : register(t2);
#endif

cbuffer lightBuffer : register(b0)
{
    float3 b_lightPositionInCameraSpace;
    float b_specularConcentration;
    float b_attenuationQuadratic;
    float b_attenuationLinear;
    float b_attenuationConstant;
};

float4 PSMain(
    float3 positionInCameraSpace : CAMERAPOSITION

#ifdef INPUT_TEXCCORDS  
    , float2 textureCoords : TEXCOORDS
#endif

#ifdef INPUT_NORMAL  
    , float3 normal : NORMAL
#endif

#ifdef INPUT_TANGENT  
    , float2 tangent : TANGENT
#endif

#ifdef INPUT_BITANGENT  
    , float3 bitangent : BITANGENT
#endif
) : SV_TARGET
{

#ifdef INPUT_NORMAL  
    #ifdef TEXTURE_NORMAL
        float3x3 tangentRotationMatrix = mul(float3x3(normal, tangent, bitangent), normal);

        normal = texture_normal.Sample(sampler_, textureCoords).rgb;
        normal = normal * 2 - 1.0f;
        normal.z = normal.z * -1.0f;

        normal = mul(tangentRotationMatrix, normal);
    #endif

    normal = normalize(normal);
#endif

#ifdef TEXTURE_DIFFUSE
    float4 diffuseSample = texture_diffuse.Sample(sampler_, textureCoords);
    float3 diffuseColor = diffuseSample.rgb;
    float diffuseAlpha = 1.0f; // make some ignoreAlpha value whether to use diffuseSample.a
#else
    float3 diffuseColor = float3(1.0f, 1.0f, 1.0f);
    float diffuseAlpha = 1.0f;
#endif

    float3 vecDistanceToLight = b_lightPositionInCameraSpace - positionInCameraSpace;
    float fDistanceToLight = length(vecDistanceToLight);
    float3 directionToLight = vecDistanceToLight / fDistanceToLight;

    float attenuation = 1.0f / ((b_attenuationConstant) + (b_attenuationLinear * fDistanceToLight) + (b_attenuationQuadratic * fDistanceToLight * fDistanceToLight));

    float3 diffuse = b_lightDiffuseColor * attenuation * max(0.0f, dot(directionToLight, normal));

#ifdef TEXTURE_SPECULAR
    float4 specularSample = texture_specular.Sample(sampler_, textureCoords);
    float3 specularColor = specularSample.rgb;
    float specularPower = pow(2.0f, specularSample.a * 13.0f);
#else
    float3 specularColor = float3(1.0f, 1.0f, 1.0f);
    float specularPower = 60.1f;
#endif
    const float3 w = normal * dot(vecDistanceToLight, normal);
    const float3 r = w * 2.0f - vecDistanceToLight;

    float3 specular = b_lightSpecularColor  * attenuation * pow(max(0.0f, dot(normalize(-r), normalize(positionInCameraSpace))), specularPower);


    return float4(diffuseColor * diffuse + specularColor * specular, diffuseAlpha);
}