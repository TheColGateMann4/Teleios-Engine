#include "SkyboxPass.h"
#include "Graphics/Core/Graphics.h"
#include "Scene/Scene.h"
#include "Graphics/Bindables/DescriptorHeapBindable.h"

#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"
#include "Scene/Scene.h"

SkyboxPass::SkyboxPass(Graphics& graphics)
	:
	FullscreenPass(graphics)
{
	// inverse projection matrix constant buffer
	{
		DynamicConstantBuffer::Layout layout;
		layout.Add<DynamicConstantBuffer::ElementType::Matrix>("inverseProjection");
		layout.Add<DynamicConstantBuffer::ElementType::Matrix>("inverseView");

		DynamicConstantBuffer::Data bufferData(layout);
		*bufferData.Get<DynamicConstantBuffer::ElementType::Matrix>("inverseProjection") = {};
		*bufferData.Get<DynamicConstantBuffer::ElementType::Matrix>("inverseView") = {};

		std::shared_ptr<CachedConstantBuffer> inverseMatriesBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData, ResourceTargets{ {ShaderVisibilityGraphic::PixelShader, 1} }, true);

		m_pInverseMatriesBuffer = inverseMatriesBuffer.get();
		m_bindables.push_back(std::move(inverseMatriesBuffer));
	}

	{
		DynamicConstantBuffer::Layout layout;
		layout.Add<DynamicConstantBuffer::ElementType::Int>("skyboxTextureIndex");

		layout.GetFinished(DynamicConstantBuffer::Layout::LayoutType::data);

		DynamicConstantBuffer::Data bufferData(layout);
		*bufferData.Get<DynamicConstantBuffer::ElementType::Int>("skyboxTextureIndex") = 0;

		std::shared_ptr<RootSignatureConstants> skyboxTextureIndexConstants = std::make_shared<RootSignatureConstants>(bufferData, ResourceTargets{ {ShaderVisibilityGraphic::PixelShader, 0} });

		m_pSkyboxTextureIndexConstants = skyboxTextureIndexConstants.get();
		m_bindables.push_back(std::move(skyboxTextureIndexConstants));
	}

	int skyboxTextureFlags = TextureFlags::NoMipMapping | TextureFlags::NoCompression;
	m_skyboxTexture = Texture::GetResource(graphics, "Assets/Images/wooden_studio_09_4k.hdr", TextureType::texture_none, skyboxTextureFlags);
}

void SkyboxPass::Initialize(Graphics& graphics, Scene& scene)
{
	{
		for (const auto& bind : m_bindables)
			m_step->AddBindable(bind);

		m_step->SetAttributeBufferEntry(m_vertexBufferEntry);
		m_step->SetIndexBufferEntry(m_indexBufferEntry);
		m_step->AddBindable(StaticSampler::GetResource(graphics));
		m_step->AddBindable(BlendState::GetResource(graphics, {}));
		m_step->AddBindable(Shader::GetResource(graphics, L"PS_Skybox", ShaderType::PixelShader)); // ps
		m_step->AddBindable(Shader::GetResource(graphics, L"VS_Fullscreen", ShaderType::VertexShader)); // vs
		m_step->AddBindable(m_skyboxTexture);
		m_step->AddBindable(DescriptorHeapBindable::GetResource());
		m_step->AddBindable(PrimitiveTechnology::GetResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)); // topology
		m_step->AddBindable(ViewPort::GetResource(graphics)); // vp
		m_step->AddBindable(RasterizerState::GetResource(graphics, {}, {}));

		DepthStencilStateOptions depthStencilStateOptions = {};
		depthStencilStateOptions.SetDepthComparisonFunction(ComparisonFunction::Equal);
		m_step->AddBindable(DepthStencilState::GetResource(graphics, depthStencilStateOptions));
	}
}

void SkyboxPass::InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{
	*m_pSkyboxTextureIndexConstants->GetData().Get<DynamicConstantBuffer::ElementType::Int>("skyboxTextureIndex") = m_skyboxTexture->GetOffsetInDescriptor();
	m_pSkyboxTextureIndexConstants->SetUpdated(true);
}

void SkyboxPass::Update(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{
	Camera* currentCamera = scene.GetCurrentCamera();

	if (currentCamera->PerspectiveChanged())
		UpdateInverseProjectionMatrix(graphics, scene);
}

void SkyboxPass::UpdateInverseProjectionMatrix(Graphics& graphics, Scene& scene)
{
	Camera* currentCamera = scene.GetCurrentCamera();
	const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

	DirectX::XMMATRIX inverseProjection = DirectX::XMMatrixInverse(nullptr, currentCamera->GetPerspectiveMatrix());
	DirectX::XMMATRIX inverseView = DirectX::XMMatrixInverse(nullptr, currentCamera->GetViewMatrix());

	DynamicConstantBuffer::Data& cameraData = m_pInverseMatriesBuffer->GetData();
	*cameraData.Get<DynamicConstantBuffer::ElementType::Matrix>("inverseProjection") = inverseProjection;
	*cameraData.Get<DynamicConstantBuffer::ElementType::Matrix>("inverseView") = inverseView;

	m_pInverseMatriesBuffer->Update(graphics);
}