struct VSout
{
    float2 textureCoords : TEXCOORDS;
    
    float4 position : SV_POSITION;
};

VSout VSMain(float3 position : POSITION, float2 textureCoords : TEXCOORDS)
{
    VSout vsout;
    
    vsout.textureCoords = textureCoords;
    vsout.position = float4(position, 1.0f);
    
    return vsout;
}