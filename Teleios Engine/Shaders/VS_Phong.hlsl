cbuffer transforms : register(b0)
{
	matrix transform;
	matrix transformInCameraSpace;
	matrix transformInCameraView;
};

struct VSOut
{
	float3 positionInCameraSpace : CAMERAPOSITION;
	float2 textureCoords : TEXCOORDS;
	float4 position : SV_POSITION;
}; 

VSOut VSMain( float3 position : POSITION, float2 textureCoords : TEXCOORDS )
{
	VSOut vsout;
	vsout.positionInCameraSpace = (float3)mul(transformInCameraSpace, float4(position, 1.0f));
	vsout.textureCoords = textureCoords;
	vsout.position = mul(transformInCameraView, float4(position, 1.0f));

	return vsout;
}