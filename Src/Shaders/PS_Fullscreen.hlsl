SamplerState s_sampler : register(s0);
Texture2D t_texture : register(t0);

float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    return t_texture.Sample(s_sampler, textureCoords);
}