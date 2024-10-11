struct VSOut
{
	float2 textureCoords : TEXCOORDS;
	float4 position : SV_POSITION;
}; 

VSOut VSMain( float3 position : POSITION, float2 textureCoords : TEXCOORDS )
{
	VSOut vsout;
	vsout.textureCoords = textureCoords;
	vsout.position = float4(position, 1.0f);

	return vsout;
}