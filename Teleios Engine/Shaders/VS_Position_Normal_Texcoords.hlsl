cbuffer transforms : register(b0)
{
	matrix transform;
	matrix transformInCameraSpace;
	matrix transformInCameraView;
};

float4 VSMain( float3 position : POSITION, float3 normal : NORMAL, float2 textureCoords : TEXCOORDS ) : SV_POSITION
{
	return mul(transformInCameraView, float4(position, 1.0f));
}