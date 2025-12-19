Texture2D texture_ : register(t0);
SamplerState sampler_ : register(s0);

cbuffer constBuffer : register(b0)
{
	float texCoordsScale;
	float brightness;
};

float4 PSMain(float2 textureCoords : TEXCOORDS) : SV_TARGET
{
    float4 sample = texture_.Sample(sampler_, textureCoords * texCoordsScale);

    return float4(sample.rgb * brightness, sample.a);
}