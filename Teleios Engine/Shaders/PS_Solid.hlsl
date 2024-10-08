cbuffer data : register(b0)
{
	float3 colorr;
}

float4 PSMain(float3 color : COLOR) : SV_TARGET
{
	return float4(colorr, 1.0f);
}