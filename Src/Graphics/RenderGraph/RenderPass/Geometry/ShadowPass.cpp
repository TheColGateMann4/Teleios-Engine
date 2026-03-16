#include "ShadowPass.h"
#include "Scene/Objects/Camera.h"
#include "Scene/Objects/PointLight.h"
#include "Includes/BindablesInclude.h"
#include "Graphics/Core/Graphics.h"

#include "Scene/Scene.h"

ShadowPass::ShadowPass(Graphics& graphics)
{
	AddBindable(ViewPort::GetResource(graphics, DirectX::XMFLOAT2(graphics.GetWidth(), graphics.GetWidth())));
	m_rasterizerOptions.SetIsShadowRasterizer(true);
}

void ShadowPass::Initialize(Graphics& graphics, Scene& scene)
{
	GeometryPass::Initialize(graphics, scene);

	m_scene = &scene;

	auto depthStencil = GetDepthStencilView();

	THROW_INTERNAL_ERROR_IF("Passed depth stencil to ShadowPass was of invalid type", depthStencil.resource == nullptr || depthStencil.resource->GetDepthStencilType() != DepthStencilType::cubeMultiResource);

	m_depthStencilViewCube = static_cast<DepthStencilViewCubeMultiResource*>(depthStencil.resource.get());
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

	for(int i = 0; i < 6; i++)
	{
		BEGIN_COMMAND_LIST_EVENT(commandList, std::to_string(i));

		SetActiveShadowCamera(pointLight, i);
		GeometryPass::ExecutePass(graphics, commandList);

		END_COMMAND_LIST_EVENT(commandList);
	}
}

void ShadowPass::SetActiveShadowCamera(PointLight* pointLight, unsigned int stage)
{
	ShadowCamera* shadowCamera = pointLight->GetShadowCamera();

	SetCameraTransformIndex(shadowCamera->GetCameraIndex() + stage);

	m_depthStencilViewCube->SetCurrentDepthBuffer(stage);
}