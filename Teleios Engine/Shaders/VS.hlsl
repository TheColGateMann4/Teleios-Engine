struct VSOut
{
	float3 color : COLOR;
	float4 position : SV_POSITION;
}; 

VSOut VSMain( float3 position : POSITION, float3 color : COLOR )
{
	VSOut vsout;
	vsout.color = color;
	vsout.position = float4(position, 1.0f);

	return vsout;
}