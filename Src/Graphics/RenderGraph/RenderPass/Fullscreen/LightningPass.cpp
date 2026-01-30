#include "LightningPass.h"

#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"

LightningPass::LightningPass(Graphics& graphics, RenderManager& renderManager)
	:
	FullscreenRenderPass(graphics, renderManager)
{
	// inverse projection matrix constant buffer
	{
		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Matrix>("inverseProjection");

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Matrix>("inverseProjection") = {};

		std::shared_ptr<CachedConstantBuffer> inverseProjectionBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 2}});

		m_pInverseProjectionBuffer = inverseProjectionBuffer.get();
		m_bindables.push_back(std::move(inverseProjectionBuffer));
	}
}

void LightningPass::Initialize(Graphics& graphics)
{
	StandaloneMesh& mesh = m_meshRenderJob->GetMesh();

	for (const auto& bind : m_bindables)
		mesh.AddBindable(bind);

	mesh.AddStaticBindable("lightBuffer");
	mesh.AddBindable(Shader::GetBindableResource(graphics, L"PS_Lightning", ShaderType::PixelShader)); // ps
	mesh.AddBindable(Shader::GetBindableResource(graphics, L"VS_Fullscreen", ShaderType::VertexShader)); // vs
	mesh.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)); // topology
	mesh.AddBindable(ViewPort::GetBindableResource(graphics)); // vp
}

void LightningPass::InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline)
{
	// before we have any meaningful output-input system on our render passes, we can only get passed bindables by using out bindable buffer
	{
		size_t bindablesLastIndex = m_bindables.size() - 1;

		THROW_INTERNAL_ERROR_IF("Needed RT SRV bindables were not pushed", bindablesLastIndex < 3);

		// initial state checking, to see if resources are correct types
		for (int i = 0; i < 4; i++)
		{
			bool isSRVType = m_bindables.at(bindablesLastIndex - i)->GetBindableType() == BindableType::bindable_shaderResourceView;

			THROW_INTERNAL_ERROR_IF("Passed object was not SRV type", !isSRVType);
		}

		rt0 = static_cast<ShaderResourceViewMultiResource*>(m_bindables.at(bindablesLastIndex - 3).get());
		rt1 = static_cast<ShaderResourceViewMultiResource*>(m_bindables.at(bindablesLastIndex - 2).get());
		rt2 = static_cast<ShaderResourceViewMultiResource*>(m_bindables.at(bindablesLastIndex - 1).get());
		ds = static_cast<ShaderResourceViewMultiResource*>(m_bindables.at(bindablesLastIndex).get());
	}

	// Updating inverse projection matrix
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();
		const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

		DirectX::XMMATRIX inverseProjection = DirectX::XMMatrixInverse(nullptr, currentCamera->GetPerspectiveMatrix());

		DynamicConstantBuffer::ConstantBufferData& cameraData = m_pInverseProjectionBuffer->GetData();
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Matrix>("inverseProjection") = inverseProjection;

		m_pInverseProjectionBuffer->Update(graphics);
	}
}

void LightningPass::PreDraw(Graphics& graphics, CommandList* commandList)
{
	THROW_INTERNAL_ERROR_IF("One of inputs was null", rt0 == nullptr || rt1 == nullptr || rt2 == nullptr || ds == nullptr );

	commandList->SetResourceState(graphics, rt0->GetResource(graphics), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	commandList->SetResourceState(graphics, rt1->GetResource(graphics), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	commandList->SetResourceState(graphics, rt2->GetResource(graphics), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	commandList->SetAllResourcesStates(graphics, ds->GetResource(graphics), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void LightningPass::PostDraw(Graphics& graphics, CommandList* commandList)
{

}