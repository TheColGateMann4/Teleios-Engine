
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
    float3 b_lightDiffuseColor;
    float3 b_lightSpecularColor;
    float b_attenuationQuadratic;
    float b_attenuationLinear;
    float b_attenuationConstant;
};

cbuffer modelBuffer : register(b1)
{
    float3 b_ambient;
    float3 b_defaultDiffuseColor;
    float3 b_defaultSpecularColor;

    bool b_ignoreDiffseAlpha;
    bool b_specularMapOnlyOneChannel;

	float b_specularShinnynes;
	float b_specularPower;
}

float4 PSMain(
    float3 positionInCameraSpace : CAMERAPOSITION

#ifdef INPUT_TEXCCORDS  
    , float2 textureCoords : TEXCOORDS
#endif

#ifdef INPUT_NORMAL  
    , float3 normal : NORMAL
#endif

#ifdef INPUT_TANGENT  
    , float3 tangent : TANGENT
#endif

#ifdef INPUT_BITANGENT  
    , float3 bitangent : BITANGENT
#endif
) : SV_TARGET
{

#ifdef INPUT_NORMAL  
    #ifdef TEXTURE_NORMAL
        // transpose() since matrices on gpu have to be transposed
        const float3x3 tangentRotationMatrix = transpose(float3x3(
            tangent,
            bitangent,
            normal
       ));

        float3 normalMapSample = texture_normal.Sample(sampler_, textureCoords).rgb;
        normalMapSample = normalMapSample * 2.0f - 1.0f;

        normal = mul(tangentRotationMatrix, normalMapSample);
    #endif

    normal = normalize(normal);
#endif

#ifdef TEXTURE_DIFFUSE
    float4 diffuseSample = texture_diffuse.Sample(sampler_, textureCoords);
    float3 diffuseColor = diffuseSample.rgb;
    float diffuseAlpha = b_ignoreDiffseAlpha ? 1.0f : diffuseSample.a;
#else
    float3 diffuseColor = b_defaultDiffuseColor;
    float diffuseAlpha = 1.0f;
#endif

    float3 vecDistanceToLight = b_lightPositionInCameraSpace - positionInCameraSpace;
    float fDistanceToLight = length(vecDistanceToLight);
    float3 directionToLight = vecDistanceToLight / fDistanceToLight;

    float attenuation = 1.0f / ((b_attenuationConstant) + (b_attenuationLinear * fDistanceToLight) + (b_attenuationQuadratic * fDistanceToLight * fDistanceToLight));

    float3 diffuse = b_lightDiffuseColor * attenuation * max(0.0f, dot(directionToLight, normal));

    float3 specularColor;
    float specularPower;

#ifdef TEXTURE_SPECULAR
    float4 specularSample = texture_specular.Sample(sampler_, textureCoords);

    if(b_specularMapOnlyOneChannel)
    {
        specularColor = b_defaultSpecularColor;
        specularPower = pow(2.0f, specularSample.r * 13.0f);
    }
    else
    {
        specularColor = specularSample.rgb;
        specularPower = pow(2.0f, specularSample.a * 13.0f);
    }
#else
    specularColor = b_defaultSpecularColor;
    specularPower = b_specularPower;
#endif

    const float3 w = normal * dot(vecDistanceToLight, normal);
    const float3 r = w * 2.0f - vecDistanceToLight;

    float3 specular = (b_lightSpecularColor * b_specularShinnynes) * attenuation * pow(max(0.0f, dot(normalize(-r), normalize(positionInCameraSpace))), specularPower);


    return float4(diffuseColor * saturate(diffuse + b_ambient) + specularColor * specular, diffuseAlpha);
}