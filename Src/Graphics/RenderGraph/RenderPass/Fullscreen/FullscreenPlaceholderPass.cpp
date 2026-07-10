#include "FullscreenPlaceholderPass.h"
#include "Scene/Objects/Camera.h"
#include "Scene/Scene.h"
#include "Graphics/Core/Graphics.h"

FullscreenPlaceholderPass::FullscreenPlaceholderPass(Graphics& graphics)
	:
	FullscreenPass(graphics)
{
	const Camera::Settings defaultCameraSettings = Camera::Settings{};

	// camera data
	{
		DynamicConstantBuffer::Layout layout;
		layout.Add<DynamicConstantBuffer::ElementType::Float>("nearPlane");
		layout.Add<DynamicConstantBuffer::ElementType::Float>("farPlane");

		DynamicConstantBuffer::Data bufferData(layout);
		*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("nearPlane") = defaultCameraSettings.NearZ;
		*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("farPlane") = defaultCameraSettings.FarZ;

		m_pCameraData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, ResourceTargets{ {ShaderVisibilityGraphic::PixelShader, 0} }, true);
	}
}

void FullscreenPlaceholderPass::Initialize(Graphics& graphics, Scene& scene)
{
	m_renderTargetSRV = ShaderResourceViewMultiResource::GetResource(graphics, "BackBuffer", graphics.GetBackBuffer().get(), 0);
	m_depthStencilSRV = ShaderResourceViewMultiResource::GetResource(graphics, "DepthStencil", graphics.GetDepthStencil().get(), 1);

	{
		// Updating camera data
		{
			Camera* currentCamera = scene.GetCurrentCamera();
			const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

			DynamicConstantBuffer::Data& cameraData = m_pCameraData->GetData();
			*cameraData.Get<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
			*cameraData.Get<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

			m_pCameraData->Update(graphics);
		}
	}

	{
		for (const auto& bind : m_bindables)
			m_step->AddBindable(bind);

		m_step->SetAttributeBufferEntry(m_vertexBufferEntry);
		m_step->SetIndexBufferEntry(m_indexBufferEntry);
		m_step->AddBindable(m_pCameraData); // b0
		m_step->AddBindable(m_renderTargetSRV); // t0
		m_step->AddBindable(StaticSampler::GetResource(graphics));
		m_step->AddBindable(BlendState::GetResource(graphics, {}));
		m_step->AddBindable(Shader::GetResource(graphics, L"PS_Fullscreen", ShaderType::PixelShader)); // ps
		m_step->AddBindable(Shader::GetResource(graphics, L"VS_Fullscreen", ShaderType::VertexShader)); // vs
		m_step->AddBindable(PrimitiveTechnology::GetResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)); // topology
		m_step->AddBindable(ViewPort::GetResource(graphics)); // vp
		m_step->AddBindable(RasterizerState::GetResource(graphics, {}, {}));
	}
}

void FullscreenPlaceholderPass::Update(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{
	Camera* currentCamera = scene.GetCurrentCamera();

	if (currentCamera->PerspectiveChanged())
		UpdateCameraData(graphics, scene);
}

void FullscreenPlaceholderPass::PreDraw(Graphics& graphics, CommandList* commandList)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);

	commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void FullscreenPlaceholderPass::PostDraw(Graphics& graphics, CommandList* commandList)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);

	commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void FullscreenPlaceholderPass::UpdateCameraData(Graphics& graphics, Scene& scene)
{
	Camera* currentCamera = scene.GetCurrentCamera();

	const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

	DynamicConstantBuffer::Data& cameraData = m_pCameraData->GetData();
	*cameraData.Get<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
	*cameraData.Get<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

	m_pCameraData->Update(graphics);
}