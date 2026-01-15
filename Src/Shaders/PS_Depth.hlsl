SamplerState s_sampler : register(s0);
Texture2D t_diffuse : register(t0);

void PSMain(float2 textureCoords : TEXCOORDS)
{
    float4 diffuseSample = t_diffuse.Sample(s_sampler, textureCoords);
    float textureOpacity = diffuseSample.a;
    
    clip(textureOpacity - 0.99f);
}