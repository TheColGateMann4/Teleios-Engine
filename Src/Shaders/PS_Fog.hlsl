SamplerState s_sampler : register(s0);
Texture2D t_renderTarget : register(t0);
Texture2D t_depthStencil : register(t1);

cbuffer CameraData : register(b0)
{
    float b_nearPlane;
    float b_farPlane;
};

cbuffer FogData : register(b1)
{
    float3 b_fogColor;
    float b_fogStart;
    float b_fogEnd;
    float b_fogDensity;
};

float GeViewSpaceDepth(float depth)
{
    return (b_nearPlane * b_farPlane) / (b_farPlane - depth * (b_farPlane - b_nearPlane));
}

float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    float4 renderTargetSample = t_renderTarget.Sample(s_sampler, textureCoords);
    float depth = t_depthStencil.Sample(s_sampler, textureCoords).r;

    float viewDepth = GeViewSpaceDepth(depth);
    
    float linearFog = saturate((viewDepth - b_fogStart) / (b_fogEnd - b_fogStart));
    float expFog = 1.0f - exp(-viewDepth * b_fogDensity);
    float fogFactor = saturate(max(linearFog, expFog));
    
    return float4(lerp(renderTargetSample.rgb, b_fogColor, fogFactor), renderTargetSample.a);
}