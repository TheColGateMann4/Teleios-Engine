Texture2D t_textures[] : register(t0);
SamplerState s_sampler : register(s0);


cbuffer skyboxTextureIndexConstants : register(b0)
{
    unsigned int b_skyboxTextureIndex;
};


cbuffer transforms : register(b1)
{
    matrix b_inverseProjection;
    matrix b_inverseView;
};

static const float _pi = 3.14159265358979f;

float3 ReconstructWorldRay(float2 uv)
{
    float2 ndc;
    ndc.x = uv.x * 2.0f - 1.0f;
    ndc.y = 1.0f - uv.y * 2.0f;
    
    float4 clip = float4(ndc, 1.0f, 1.0f);
    float4 view = mul(b_inverseProjection, clip);
    float3 viewRay = normalize(view.xyz / view.w);    
    float3 worldRay = normalize(mul((float3x3) b_inverseView, viewRay));

    return worldRay;
}

float2 DirectionToMap(float3 dir)
{
    dir = normalize(dir);

    float2 uv;
    uv.x = 0.5f + atan2(dir.x, dir.z) / (2.0f * _pi);
    uv.y = 0.5f - asin(dir.y) / _pi;

    return uv;
}

float4 SampleTexture(uint id, float2 tc)
{
    return t_textures[id].Sample(s_sampler, tc);
}

float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    float3 worldRay = ReconstructWorldRay(textureCoords);
    float2 envMapUV = DirectionToMap(worldRay);
    
    return float4(SampleTexture(b_skyboxTextureIndex, envMapUV).rgb, 1.0f);
}