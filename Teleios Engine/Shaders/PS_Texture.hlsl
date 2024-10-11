Texture2D texture_ : register(t0);
SamplerState sampler_ : register(s0);

cbuffer constBuffer : register(b0)
{
	float texCoordsScale;
};

float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    return texture_.Sample(sampler_, textureCoords * texCoordsScale);
}