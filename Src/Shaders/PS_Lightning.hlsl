SamplerState s_sampler : register(s0);

Texture2D t_rt0 : register(t0);
Texture2D t_rt1 : register(t1);
Texture2D t_rt2 : register(t2);

Texture2D t_depth : register(t3);

TextureCube t_shadowMap : register(t4);

struct PointLightData
{
    float3 lightPositionInCameraSpace;
    float3 lightDiffuseColor;
    float attenuationQuadratic;
    float attenuationLinear;
    float attenuationConstant;
    float nearZ;
    float farZ;
};

#ifndef NUM_POINTLIGHTS
#define NUM_POINTLIGHTS 1 
#endif

cbuffer lightBuffer : register(b0)
{
    PointLightData b_pointlights[NUM_POINTLIGHTS];
};

cbuffer transforms : register(b2)
{
    matrix b_inverseProjection;
    matrix b_inverseView;
};

float NormalDistribution(float alpha, float NdotH)
{
    const float alpha2 = alpha * alpha;
    const float denominator = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (3.14159265 * denominator * denominator);
}

float GeometrySchlickGGX(float NdotX, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return NdotX / (NdotX * (1.0f - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    return GeometrySchlickGGX(NdotV, roughness) *
           GeometrySchlickGGX(NdotL, roughness);
}

float3 FrenselSchlick(float3 F0, float VdotH)
{
    return F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);
}

float GetAttenuation(PointLightData pointlightData, float distanceToLight)
{
    return 1.0f / ((pointlightData.attenuationConstant) + (pointlightData.attenuationLinear * distanceToLight) + (pointlightData.attenuationQuadratic * distanceToLight * distanceToLight));
}

float4 GetClipPosition(float2 texcoords, float depth)
{
    float2 ndc;
    ndc.x = texcoords.x * 2.0f - 1.0f;
    ndc.y = 1.0f - texcoords.y * 2.0f;
  
    return float4(ndc, depth, 1.0f);
}

float3 GetCameraViewPosition(float4 clipPosition)
{  
    const float4 view = mul(b_inverseProjection, clipPosition);

    return view.xyz / view.w;
}

float3 GetWorldPosition(float3 viewPosition)
{  
    const float4 worldPos = mul(b_inverseView, float4(viewPosition.xyz, 1.0f));

    return worldPos.xyz;
}

float CalculateLightDepth(PointLightData pointlight, float3 positionInLightSpace)
{
    const float3 absSampleVec = abs(positionInLightSpace);
    const float viewZ = max(absSampleVec.x, max(absSampleVec.y, absSampleVec.z)); // selecting dominant axis
    const float farZ = pointlight.farZ;
    const float nearZ = pointlight.nearZ;
    
    return (farZ / (farZ - nearZ)) - (farZ * nearZ) / (viewZ * (farZ - nearZ));
}

#define FLT_EPSILON 0.00001f

float SampleShadowMap(float3 sampleDir)
{
    return t_shadowMap.Sample(s_sampler, normalize(sampleDir)).r;
}

bool GetOcclusion(PointLightData pointlightData, float3 worldPos)
{
    const float3 positionInLightSpace = worldPos - GetWorldPosition(pointlightData.lightPositionInCameraSpace);
    
    const float sampledDepth = SampleShadowMap(positionInLightSpace);
    const float calculatedDepth = CalculateLightDepth(pointlightData, positionInLightSpace);

    return sampledDepth < (calculatedDepth - FLT_EPSILON);
}

static const float _pi = 3.14159265358979f;

#define FLAG_WORKFLOW_METALNESS (1 << 0)
#define FLAG_WORKFLOW_SPECULAR (1 << 1)

uint GetFlags(float flagData)
{
    return (uint)round(flagData * 255.0f);
}


float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    const float4 rt0sample = t_rt0.Sample(s_sampler, textureCoords);
    const float4 rt1sample = t_rt1.Sample(s_sampler, textureCoords);
    const float4 rt2sample = t_rt2.Sample(s_sampler, textureCoords);
    const float depth = t_depth.Sample(s_sampler, textureCoords).r;
    
    const float3 diffuse = rt0sample.rgb;
    const float3 normal = rt2sample.rgb * 2.0f - 1.0f;
    uint flags = GetFlags(rt2sample.a);
    
    float roughness;
    float metalness;
    float3 F0;
    
    if (flags & FLAG_WORKFLOW_METALNESS)
    {   
        roughness = rt0sample.a;
        metalness = rt1sample.a;
        const float3 reflectivity = rt1sample.rgb;
        F0 = lerp(float3(0.04, 0.04, 0.04), diffuse, metalness) * reflectivity;
    }
    else if (flags & FLAG_WORKFLOW_SPECULAR)
    {
        const float glosiness = rt1sample.g;
        roughness = 1.0f - glosiness;
        metalness = 0.0f;
        F0 = rt1sample.rgb;
    }
    
    const float4 clipPosition = GetClipPosition(textureCoords, depth);
    const float3 viewPosition = GetCameraViewPosition(clipPosition);
    
    const float3 N = normalize(normal); // normal
    const float3 V = normalize(-viewPosition); // view
    
    const float NdotV = max(dot(V, N), 0.0f);
    
    const float alpha = roughness * roughness;
    
    const float3 lambert = diffuse / _pi;
    
    float3 accumulatedLight = float3(0.0f, 0.0f, 0.0f);
    
    [unroll]
    for (int pointLightIndex = 0; pointLightIndex < 1; pointLightIndex++)
    {
        PointLightData pointlightData = b_pointlights[pointLightIndex];

        const float3 L = normalize(pointlightData.lightPositionInCameraSpace - viewPosition); // light
        
        const bool occluded = GetOcclusion(pointlightData, GetWorldPosition(viewPosition));
        if (occluded)
            continue;
        
        const float3 H = normalize(V + L); // halfway vector
        
        const float NdotL = max(dot(N, L), 0.0f);
        const float NdotH = max(dot(N, H), 0.0f);
        const float VdotH = max(dot(V, H), 0.0f);
        
        const float3 Ks = FrenselSchlick(F0, VdotH);
        const float3 Kd = (1.0f - Ks) * (1.0f - metalness);
        
        const float3 cookTorranceNumerator = NormalDistribution(alpha, NdotH) * GeometrySmith(NdotV, NdotL, roughness) * Ks;
        const float cookTorranceDenominator = max(4.0f * NdotV * NdotL, 0.000001f);
        const float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;
    
        const float3 DiffuseBRDF = Kd * lambert;
        const float3 SpecularBRDF = cookTorrance;
    
        const float lengthOfDistanceToLight = length(pointlightData.lightPositionInCameraSpace - viewPosition);
        const float attenuation = GetAttenuation(pointlightData, lengthOfDistanceToLight);
        const float3 lightIntensity = pointlightData.lightDiffuseColor;
    
        const float3 outgoingLight = (DiffuseBRDF + SpecularBRDF) * NdotL * attenuation * lightIntensity;
        
        accumulatedLight += outgoingLight;
    }

    accumulatedLight.rgb += float3(0.02, 0.02, 0.02) * diffuse;

}