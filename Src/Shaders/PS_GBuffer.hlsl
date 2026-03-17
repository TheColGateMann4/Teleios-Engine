#ifdef TEXTURE_ANY
    Texture2D t_textures[] : register(t0);
    SamplerState s_sampler : register(s0);
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

#ifdef TEXTURE_ANY
    float4 SampleTexture(uint id, float2 tc)
    {
        return t_textures[id].Sample(s_sampler, tc);
    }

    cbuffer textureIds : register(b2)
    {
        #ifdef TEXTURE_DIFFUSE
            uint b_diffuseTextureID;
        #endif
        
        #ifdef TEXTURE_NORMAL
            uint b_normalTextureID;
        #endif
        
#if defined(METALNESS_PIPELINE)
        #ifdef METALNESS_ROUGHNESS_ONE_TEXTURE
            uint b_metalnessRoughnessTextureID;
        #else
            #ifdef TEXTURE_METALNESS
                    uint b_metalnessTextureID;
            #endif
                
            #ifdef TEXTURE_ROUGHNESS
                    uint b_roughnessTextureID;
            #endif
        #endif

        #ifdef TEXTURE_REFLECTIVITY
            uint b_reflectivityTextureID;
        #endif
      

#elif defined(SPECULAR_PIPELINE)
        #ifdef TEXTURE_SPECULAR
            uint b_specularTextureID;
        #endif
        
        #ifdef TEXTURE_GLOSINESS
            uint b_glosinessTextureID;
        #endif
#endif
         
        #ifdef TEXTURE_AMBIENT
            uint b_ambientTextureID;
        #endif
        
        #ifdef TEXTURE_OPACITY
            uint b_opacityTextureID;
        #endif
        
        #ifdef TEXTURE_ENVIRONMENT_MAP
            uint b_environmentMapID;
        #endif
    }
#endif

#define FLAG_WORKFLOW_METALNESS (1 << 0)
#define FLAG_WORKFLOW_SPECULAR (1 << 1)

float GetFlagData(uint flags)
{
    return flags / 255.0f;
}

// rt0 RGB: diffuse, A: roughness
// rt1 RGB: reflectivity, A: metalness
// rt2 RGB: normal A: flags

struct PSOut
{
    float4 rt0 : SV_TARGET0;
    float4 rt1 : SV_TARGET1;
    float4 rt2 : SV_TARGET2;
};

PSOut PSMain(
#ifdef INPUT_TEXCCORDS  
    float2 textureCoords : TEXCOORDS,
#endif

#ifdef INPUT_NORMAL  
    float3 normal : NORMAL,
#endif

#ifdef INPUT_TANGENT  
    float3 tangent : TANGENT,
#endif

#ifdef INPUT_BITANGENT  
    float3 bitangent : BITANGENT,
#endif

    bool isFrontFace : SV_IsFrontFace
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
        
        const float3 normalMapSample = SampleTexture(b_normalTextureID, textureCoords).rgb * 2.0f - 1.0f;
        
        normal = normalize(mul(tangentRotationMatrix, normalMapSample));
    
        normal = (normal + 1.0f) / 2.0f;
    }
#endif
    
#ifdef TEXTURE_DIFFUSE
    float4 diffuseSample = SampleTexture(b_diffuseTextureID, textureCoords);
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
    
    
#if defined(METALNESS_PIPELINE)
    #ifdef METALNESS_ROUGHNESS_ONE_TEXTURE
        float3 metalnessRoughnessSample = SampleTexture(b_metalnessRoughnessTextureID, textureCoords).rgb;
        
        float metalness = metalnessRoughnessSample.b;
        float roughness = metalnessRoughnessSample.g;
    #else
        #ifdef TEXTURE_METALNESS
            float metalness = SampleTexture(b_metalnessTextureID, textureCoords).r;
        #else
            float metalness = b_metalness;
        #endif
            
        
        #ifdef TEXTURE_ROUGHNESS
            float roughness = SampleTexture(b_roughnessTextureID, textureCoords).r;
        #else
            float roughness = b_roughness;
        #endif
    #endif
        metalness = saturate(metalness);
        metalness = (metalness < 0.05f) ? 0.0f : metalness;
        
        roughness = clamp(roughness, 0.04f, 1.0f);   
        
    #ifdef TEXTURE_REFLECTIVITY
        float3 reflectivity = SampleTexture(b_reflectivityTextureID, textureCoords).rgb;
    #else
        float3 reflectivity = b_reflectivity;
    #endif
#elif defined (SPECULAR_PIPELINE)
    #ifdef TEXTURE_SPECULAR
        float3 specular = float3(0.0f, 0.0f, 0.0f);
        if(b_specularMapOnlyOneChannel)
        {
            const float specularSample = SampleTexture(b_specularTextureID, textureCoords).r;
            specular = float3(specularSample, specularSample, specularSample);
        }
        else
            specular = SampleTexture(b_specularTextureID, textureCoords).rgb; 
    #else
        float3 specular = b_specularColor;
    #endif
    
    #ifdef TEXTURE_GLOSINESS
        float glosiness = SampleTexture(b_glosinessTextureID, textureCoords).r;
    #else
        float glosiness = b_glosiness;
    #endif
#endif

    
    
#ifdef TEXTURE_AMBIENT
    float3 ambient = SampleTexture(b_ambientTextureID, textureCoords).rgb;
#else
    float3 ambient = b_ambient;
#endif
    
    
#ifdef TEXTURE_OPACITY
    float opacity = SampleTexture(b_opacityTextureID, textureCoords).r;
#else
    float opacity = b_opacity;
#endif
    
    if (!isFrontFace)
        normal = -normal;
    
    uint flags = 0;
#if defined(METALNESS_PIPELINE)
    flags |= FLAG_WORKFLOW_METALNESS;
#elif defined (SPECULAR_PIPELINE)
    flags |= FLAG_WORKFLOW_SPECULAR;
#endif    
    
    PSOut psout;
    
    
    
#if defined(METALNESS_PIPELINE)
    psout.rt0 = float4(diffuse, roughness);
    psout.rt1 = float4(reflectivity, metalness);
#elif defined (SPECULAR_PIPELINE)
    psout.rt0 = float4(diffuse, glosiness);
    psout.rt1 = float4(specular, 0.0f);
#else
    psout.rt0 = float4(diffuse, 0.0f);
    psout.rt1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
#endif
    
    psout.rt2 = float4(normal, GetFlagData(flags));
    
    return psout;
}