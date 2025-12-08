
#ifdef TEXTURE_ANY
    SamplerState s_sampler : register(s0);
#endif

#ifdef TEXTURE_DIFFUSE
    Texture2D t_diffuse : register(t0);
#endif

#ifdef TEXTURE_NORMAL
    Texture2D t_normal : register(t1);
#endif

#ifdef TEXTURE_SPECULAR
    Texture2D t_specular : register(t2);
#endif

#ifdef METALNESS_ROUGHNESS_ONE_TEXTURE
    Texture2D t_metalnessRoughness : register(t3);
#else
    #ifdef TEXTURE_METALNESS
        Texture2D t_metalness : register(t3);
    #endif
    
    #ifdef TEXTURE_ROUGHNESS
        Texture2D t_roughness : register(t4);
    #endif
#endif

#ifdef TEXTURE_REFLECTIVITY
    Texture2D t_reflectivity : register(t5);
#endif

#ifdef TEXTURE_AMBIENT
    Texture2D t_ambient : register(t6);
#endif

#ifdef TEXTURE_OPACITY
    Texture2D t_opacity : register(t7);
#endif

#ifdef TEXTURE_ENVIRONMENT_MAP
    Texture2D t_environmentMap : register(t8);
#endif

cbuffer lightBuffer : register(b0)
{
    float3 b_lightPositionInCameraSpace;
    float3 b_lightDiffuseColor;
    float b_attenuationQuadratic;
    float b_attenuationLinear;
    float b_attenuationConstant;
};

cbuffer modelBuffer : register(b1)
{
    float3 b_ambient;
    float3 b_diffuse;

#ifdef METALNESS_PIPELINE
    float3 b_reflectivity;

    float b_metalness;
    float b_roughness;
#else
    float3 b_specularColor;
    bool b_specularMapOnlyOneChannel;

	float b_specular;
	float b_glosiness;
#endif
    
    float b_opacity;
}

float NormalDistribution(const float alpha, const float NdotH)
{
    const float alpha2 = alpha * alpha;
    const float denominator = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (3.14159265 * denominator * denominator);
}

float GeometryShadowingFunction(const float alpha, const float NdotX)
{
    const float k = alpha / 2.0f;
    const float numerator = NdotX;
    const float denominator = max(NdotX * (1.0f - k) + k, 0.000001f);

    return numerator / denominator;
}

float GeometricShadowing(const float alpha, const float NdotV, const float NdotL)
{
    return GeometryShadowingFunction(alpha, NdotV) * GeometryShadowingFunction(alpha, NdotL);
}

float3 FrenselSchlick(const float3 F0, const float NdotL)
{
    return F0 + (1.0f - F0) * pow(1.0f - NdotL, 5.0f);
}

float GetAttenuation(const float distanceToLight)
{
    return 1.0f / ((b_attenuationConstant) + (b_attenuationLinear * distanceToLight) + (b_attenuationQuadratic * distanceToLight * distanceToLight));
}

static const float _pi = 3.14159265358979f;

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
    
#ifdef TEXTURE_NORMAL
    {
        // transpose() since matrices on gpu have to be transposed
        const float3x3 tangentRotationMatrix = transpose(float3x3(
            tangent,
            bitangent,
            normal
        ));
        
        const float3 normalMapSample = t_normal.Sample(s_sampler, textureCoords).rgb * 2.0f - 1.0f;
        
        normal = normalize(mul(tangentRotationMatrix, normalMapSample));
    }
#endif
    
    
#ifdef TEXTURE_DIFFUSE
    float4 diffuseSample = t_diffuse.Sample(s_sampler, textureCoords);
    float3 diffuse = diffuseSample.rgb;

    #ifndef IGNORE_DIFFUSE_ALPHA
        float textureOpacity = diffuseSample.a;
    #else
        float textureOpacity = b_opacity;
    #endif
#else
    float3 diffuse = b_diffuse;
    float textureOpacity = b_opacity;
#endif
    
#ifdef METALNESS_ROUGHNESS_ONE_TEXTURE
    float metalness = t_metalnessRoughness.Sample(s_sampler, textureCoords).b; // r
#else
    #ifdef TEXTURE_METALNESS
        float metalness = t_metalness.Sample(s_sampler, textureCoords).r;
    #else
        float metalness = b_metalness;
    #endif
#endif    
    
    
#ifdef METALNESS_ROUGHNESS_ONE_TEXTURE
    float roughness = t_metalnessRoughness.Sample(s_sampler, textureCoords).g;
#else
    #ifdef TEXTURE_ROUGHNESS
        float roughness = t_roughness.Sample(s_sampler, textureCoords).r;
    #else
        float roughness = b_roughness;
    #endif
#endif


    
    
#ifdef TEXTURE_REFLECTIVITY
    float3 reflectivity = t_reflectivity.Sample(s_sampler, textureCoords).rgb;
#else
    float3 reflectivity = b_reflectivity;
#endif
    
    
#ifdef TEXTURE_AMBIENT
    float3 ambient = t_ambient.Sample(s_sampler, textureCoords).rgb;
#else
    float3 ambient = b_ambient;
#endif
    
    
#ifdef TEXTURE_OPACITY
    float opacity = t_opacity.Sample(s_sampler, textureCoords).r;
#else
    float opacity = b_opacity;
#endif
    
    const float3 N = normalize(normal); // normal
    const float3 V = normalize(-positionInCameraSpace); // view
    const float3 L = normalize(b_lightPositionInCameraSpace - positionInCameraSpace); // light
    const float3 H = normalize(V + L); // halfway vector
    
    
    const float NdotL = max(dot(N, L), 0.0f);
    const float NdotV = max(dot(N, V), 0.0f);
    const float NdotH = max(dot(N, H), 0.0f);
    const float LdotN = max(dot(L, N), 0.0f);
    const float VdotN = max(dot(V, N), 0.0f);
    const float VdotH = max(dot(V, H), 0.0f);
    
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), diffuse, metalness);
    const float alpha = roughness * roughness;
    const float3 Ks = FrenselSchlick(F0, VdotH);
    const float3 Kd = (1.0f - Ks) * (1.0f - metalness);
    


    const float3 lambert = diffuse / _pi;
    
    const float3 cookTorranceNumerator = NormalDistribution(alpha, NdotH) * GeometricShadowing(alpha, NdotV, NdotL) * Ks;
    const float cookTorranceDenominator = max(4.0f * VdotN * LdotN, 0.000001f);
    const float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;
    
    const float3 DiffuseBRDF = Kd * lambert * opacity;
    const float3 SpecularBRDF = cookTorrance;
    
    const float lengthOfDistanceToLight = length(b_lightPositionInCameraSpace - positionInCameraSpace);
    const float attenuation = GetAttenuation(lengthOfDistanceToLight);
    const float3 lightIntensity = b_lightDiffuseColor;
    
    float3 outgoingLight = (DiffuseBRDF + SpecularBRDF) * LdotN * attenuation * lightIntensity + ambient;

#ifdef TEXTURE_ENVIRONMENT_MAP
    float3 skyboxSampleVector = normalize(reflect(-worldPosition, worldNormal));
    outgoingLight += (t_environmentMap.Sample(s_sampler, skyboxSampleVector).xyz * metalness) * reflectivity;
#else

#endif

    return float4(saturate(outgoingLight), textureOpacity);
}