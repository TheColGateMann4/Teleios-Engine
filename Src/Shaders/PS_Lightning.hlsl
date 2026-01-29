SamplerState s_sampler : register(s0);

Texture2D t_rt0 : register(t0);
Texture2D t_rt1 : register(t1);
Texture2D t_rt2 : register(t2);

Texture2D t_depth : register(t3);

cbuffer lightBuffer : register(b0)
{
    float3 b_lightPositionInCameraSpace;
    float3 b_lightDiffuseColor;
    float b_attenuationQuadratic;
    float b_attenuationLinear;
    float b_attenuationConstant;
};

cbuffer transforms : register(b2)
{
    matrix b_inverseProjection;
};

float NormalDistribution(const float alpha, const float NdotH)
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

float3 FrenselSchlick(const float3 F0, const float VdotH)
{
    return F0 + (1.0f - F0) * pow(1.0f - VdotH, 5.0f);
}

float GetAttenuation(const float distanceToLight)
{
    return 1.0f / ((b_attenuationConstant) + (b_attenuationLinear * distanceToLight) + (b_attenuationQuadratic * distanceToLight * distanceToLight));
}

float3 GetViewPosition(float2 texcoords, float depth)
{
    float2 ndc = texcoords * 2.0f - 1.0f;
  
    float4 clip = float4(ndc, depth, 1.0f);
  
    float4 view = mul(b_inverseProjection, clip);

    return view.xyz / view.w;
}


static const float _pi = 3.14159265358979f;



float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    float4 rt0sample = t_rt0.Sample(s_sampler, textureCoords);
    float4 rt1sample = t_rt1.Sample(s_sampler, textureCoords);
    float4 rt2sample = t_rt2.Sample(s_sampler, textureCoords);
    float depth = t_depth.Sample(s_sampler, textureCoords).r;
    
    
    float3 diffuse = rt0sample.rgb;
    float3 normal = rt2sample.rgb;
    float metalness = rt1sample.r;
    float roughness = rt1sample.g;
    float3 viewPosition = GetViewPosition(textureCoords, depth);
    
    
    const float3 N = normalize(normal); // normal
    const float3 V = normalize(-viewPosition); // view
    const float3 L = normalize(b_lightPositionInCameraSpace - viewPosition); // light
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
    
    const float3 cookTorranceNumerator = NormalDistribution(alpha, NdotH) * GeometrySmith(NdotV, NdotL, roughness) * Ks;
    const float cookTorranceDenominator = max(4.0f * VdotN * LdotN, 0.000001f);
    const float3 cookTorrance = cookTorranceNumerator / cookTorranceDenominator;
    
    const float3 DiffuseBRDF = Kd * lambert;
    const float3 SpecularBRDF = cookTorrance;
    
    const float lengthOfDistanceToLight = length(b_lightPositionInCameraSpace - viewPosition);
    const float attenuation = GetAttenuation(lengthOfDistanceToLight);
    const float3 lightIntensity = b_lightDiffuseColor;
    
    float3 outgoingLight = (DiffuseBRDF + SpecularBRDF) * LdotN * attenuation * lightIntensity;

    return float4(outgoingLight, 1.0f);
}