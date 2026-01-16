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

// rt0 RGB: diffuse.rgb, A: -
// rt1 R: metalness, G: roughness, 
// rt2 RGB: normal.xyz

struct PSOut
{
    float4 rt0 : SV_TARGET0;
    float4 rt1 : SV_TARGET1;
    float4 rt2 : SV_TARGET2;
};

PSOut PSMain(
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
    , bool isBackFace : SV_IsFrontFace

)
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
    metalness = saturate(metalness);
    metalness = (metalness < 0.05f) ? 0.0f : metalness;
    
#ifdef METALNESS_ROUGHNESS_ONE_TEXTURE
    float roughness = t_metalnessRoughness.Sample(s_sampler, textureCoords).g;
#else
#ifdef TEXTURE_ROUGHNESS
        float roughness = t_roughness.Sample(s_sampler, textureCoords).r;
#else
    float roughness = b_roughness;
#endif
#endif
    roughness = clamp(roughness, 0.04f, 1.0f);


    
    
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
    
    if (!isBackFace)
        normal = -normal;
    
    
    PSOut psout;
    
    psout.rt0 = float4(diffuse, 0.0f);
    psout.rt1 = float4(metalness, roughness, 0.0f, 0.0f);    
    psout.rt2 = float4(normal, 0.0f);
    
    return psout;
}