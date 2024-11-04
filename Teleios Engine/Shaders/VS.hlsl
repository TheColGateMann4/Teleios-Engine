cbuffer transforms : register(b0)
{
	matrix transform;
	matrix transformInCameraSpace;
	matrix transformInCameraView;
};

struct VSOut
{
#ifdef INPUT_NORMAL
	float3 normal : NORMAL;
#endif

#ifdef INPUT_TEXCCORDS  
	float2 textureCoords : TEXCOORDS;
#endif

#ifdef OUTPUT_CAMAERAPOS
	float3 cameraPosition : CAMERAPOSITION;
#endif

	float4 position : SV_POSITION;
};

VSOut VSMain(
	float3 position : POSITION
	
#ifdef INPUT_NORMAL  
     , float3 normal : NORMAL
#endif	

#ifdef INPUT_TEXCCORDS  
     , float2 textureCoords : TEXCOORDS
#endif
	)
{
	VSOut vsout;

#ifdef INPUT_NORMAL  
     vsout.normal = mul((float3x3)transformInCameraSpace, position);
#endif

#ifdef INPUT_TEXCCORDS  
     vsout.textureCoords = textureCoords;
#endif

#ifdef OUTPUT_CAMAERAPOS
	vsout.cameraPosition = (float3)mul(transformInCameraSpace, float4(position, 1.0f));
#endif

	vsout.position = mul(transformInCameraView, float4(position, 1.0f));

	return vsout;
}