cbuffer modelTransforms : register(b0)
{
    row_major matrix transform;
};

#ifndef NUM_CAMERAS
#define NUM_CAMERAS 10
#endif

struct CameraData
{
    row_major matrix view;
    row_major matrix projection;
};

cbuffer cameraTransforms : register(b1)
{
	CameraData cameras[NUM_CAMERAS];
};

cbuffer constants : register(b2)
{
    int cameraTransformIndex;
}

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
    matrix transformInCameraSpace = mul(transform, cameras[cameraTransformIndex].view);
    matrix transformInCameraView = mul(transformInCameraSpace, cameras[cameraTransformIndex].projection);
    
	VSOut vsout;

#ifdef OUTPUT_CAMAERAPOS
	vsout.cameraPosition = (float3)mul(float4(position, 1.0f), transformInCameraSpace);
#endif

#ifdef INPUT_TEXCCORDS  
     vsout.textureCoords = textureCoords;
#endif

#ifdef INPUT_NORMAL  
     vsout.normal = mul(normal, (float3x3)transformInCameraSpace);
#endif

#ifdef INPUT_TANGENT  
     vsout.tangent = mul(tangent, (float3x3)transformInCameraSpace);
#endif

#ifdef INPUT_BITANGENT  
     vsout.bitangent = mul(bitangent, (float3x3)transformInCameraSpace);
#endif



    vsout.position = mul(float4(position, 1.0f), transformInCameraView);

	return vsout;
}