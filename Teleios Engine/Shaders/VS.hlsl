cbuffer transforms : register(b0)
{
	matrix transform;
	matrix transformInCameraSpace;
	matrix transformInCameraView;
};

float4 VSMain( float3 position : POSITION) : SV_POSITION
{
	return mul(transformInCameraView, float4(position, 1.0f));
}