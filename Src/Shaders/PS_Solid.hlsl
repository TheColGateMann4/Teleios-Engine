cbuffer constBuffer : register(b1)
{
    float4 color;
};

float4 PSMain() : SV_TARGET
{
    return color;
}