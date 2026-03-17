Texture2D t_textures[] : register(t0);
SamplerState s_sampler : register(s0);

float4 SampleTexture(uint id, float2 tc)
{
    return t_textures[id].Sample(s_sampler, tc);
}

cbuffer textureIds : register(b2)
{
    uint b_diffuseTextureID;
}

void PSMain(float2 textureCoords : TEXCOORDS)
{
    float textureOpacity = SampleTexture(b_diffuseTextureID, textureCoords).a;
    
    clip(textureOpacity - 0.99f);
}