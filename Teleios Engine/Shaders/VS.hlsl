cbuffer transforms : register(b0)
{
	matrix transform;
	matrix transformInCameraSpace;
	matrix transformInCameraView;
};

struct VSOut
{

#ifdef OUTPUT_CAMAERAPOS
	float3 cameraPosition : CAMERAPOSITION;
#endif

#ifdef INPUT_TEXCCORDS  
	float2 textureCoords : TEXCOORDS;
#endif

#ifdef INPUT_NORMAL
	float3 normal : NORMAL;
#endif

#ifdef INPUT_TANGENT  
     float3 tangent : TANGENT;
#endif	

#ifdef INPUT_BITANGENT  
     float3 bitangent : BITANGENT;
#endif	

	float4 position : SV_POSITION;
};

VSOut VSMain(
	float3 position : POSITION
	
#ifdef INPUT_TEXCCORDS  
     , float2 textureCoords : TEXCOORDS
#endif

#ifdef INPUT_NORMAL  
     , float3 normal : NORMAL
#endif	

#ifdef INPUT_TANGENT  
     , float3 tangent : TANGENT
#endif	

#ifdef INPUT_BITANGENT  
     , float3 bitangent : BITANGENT
#endif	

	)
{
	VSOut vsout;

#ifdef OUTPUT_CAMAERAPOS
	vsout.cameraPosition = (float3)mul(transformInCameraSpace, float4(position, 1.0f));
#endif

#ifdef INPUT_TEXCCORDS  
     vsout.textureCoords = textureCoords;
#endif

#ifdef INPUT_NORMAL  
     vsout.normal = mul((float3x3)transformInCameraSpace, normal);
#endif

#ifdef INPUT_TANGENT  
     vsout.tangent = mul((float3x3)transformInCameraSpace, tangent);
#endif

#ifdef INPUT_BITANGENT  
     vsout.bitangent = mul((float3x3)transformInCameraSpace, bitangent);
#endif



	vsout.position = mul(transformInCameraView, float4(position, 1.0f));

	return vsout;
}