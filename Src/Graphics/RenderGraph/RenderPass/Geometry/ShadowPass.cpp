#include "ShadowPass.h"
#include "Scene/Objects/Camera.h"
#include "Scene/Objects/PointLight.h"
#include "Includes/BindablesInclude.h"
#include "Graphics/Core/Graphics.h"

#include "Scene/Scene.h"

ShadowPass::ShadowPass(Graphics& graphics)
{
	AddBindable(ViewPort::GetResource(graphics, DirectX::XMFLOAT2(graphics.GetWidth(), graphics.GetWidth())));
}

void ShadowPass::Initialize(Graphics& graphics, Scene& scene)
{
	GeometryPass::Initialize(graphics, scene);

	m_scene = &scene;
}

void ShadowPass::Update(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{

}

RenderJob::JobType ShadowPass::GetWantedJob() const
{
	return RenderJob::JobType::Depth;
}

void ShadowPass::ExecutePass(Graphics& graphics, CommandList* commandList)
{
	PointLight* pointLight = m_scene->GetPointLights().front();
	ShadowCamera* shadowCamera = pointLight->GetShadowCamera();
	ObjectTransform* shadowCameraTransform = shadowCamera->GetTransform();

	//static constexpr DirectX::XMFLOAT3 faceRotation[6] =
	//{
	//	{ 0.0f,	  -90.0f, 0.0f },
	//	{ 0.0f,	  90.0f,  0.0f },
	//	{ -90.0f, 0.0f,	  0.0f },
	//	{ 90.0f,  0.0f,	  0.0f },
	//	{ 180.0f, 0.0f,	  0.0f },
	//	{ 0.0f,	  0.0f,	  0.0f }
	//};
	//
	//for(int i = 0; i < ARRAYSIZE(faceRotation); i++)
	//{
	//  SetActiveShadowCameraIndex(pointLight, stage);
	//	shadowCameraTransform->SetEulerRotation(faceRotation[i]);
	//	GeometryPass::ExecutePass(graphics, commandList);
	//}

	SetActiveShadowCameraIndex(pointLight, 0);
	GeometryPass::ExecutePass(graphics, commandList);
}

void ShadowPass::SetActiveShadowCameraIndex(PointLight* pointLight, unsigned int stage)
{
	ShadowCamera* shadowCamera = pointLight->GetShadowCamera();

	SetCameraTransformIndex(shadowCamera->GetCameraIndex() + stage);
}