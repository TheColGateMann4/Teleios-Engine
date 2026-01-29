#include "FullscreenRenderPass.h"

#include "Scene/Objects/Camera.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Core/TempCommandList.h"
#include "Scene/StandaloneMesh.h"

FullscreenRenderPass::FullscreenRenderPass(Graphics& graphics, RenderManager& renderManager)
{
	std::shared_ptr<IndexBuffer> indexBuffer = IndexBuffer::GetBindableResource(graphics, "FullscreenMesh", std::vector<unsigned int>{0, 1, 3, 0, 3, 2});
	m_pIndexBuffer = indexBuffer.get();
	m_bindables.push_back(std::move(indexBuffer));

	DynamicVertex::DynamicVertexLayout layout;
	layout.AddElement<DynamicVertex::ElementType::Position>();
	layout.AddElement<DynamicVertex::ElementType::TextureCoords>();

	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoords;
	};

	std::vector<Vertice> vertices = {
		{{ -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}
	};

	std::shared_ptr<VertexBuffer> vertexBuffer = VertexBuffer::GetBindableResource(graphics, "FullscreenMesh", vertices.data(), vertices.size(), sizeof(vertices.at(0)));
	m_pVertexBuffer = vertexBuffer.get();
	m_bindables.push_back(std::move(vertexBuffer));

	m_bindables.push_back(InputLayout::GetBindableResource(graphics, layout));

	const Camera::Settings defaultCameraSettings = Camera::Settings{};

	// camera data
	{
		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("nearPlane");
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("farPlane");

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = defaultCameraSettings.NearZ;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = defaultCameraSettings.FarZ;

		std::shared_ptr<CachedConstantBuffer> cameraData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}});

		m_pCameraData = cameraData.get();
		m_bindables.push_back(std::move(cameraData));
	}

	m_bindables.push_back(StaticSampler::GetBindableResource(graphics));
	m_bindables.push_back(BlendState::GetBindableResource(graphics));
	m_bindables.push_back(RasterizerState::GetBindableResource(graphics));

	m_meshRenderJob = std::make_shared<MeshRenderJob>(RenderJob::JobType::None);
}

void FullscreenRenderPass::Initialize(Graphics& graphics)
{
	m_renderTargetSRV = ShaderResourceViewMultiResource::GetBindableResource(graphics, "BackBuffer", graphics.GetBackBuffer().get(), 0);
	m_depthStencilSRV = ShaderResourceViewMultiResource::GetBindableResource(graphics, "DepthStencil", graphics.GetDepthStencil().get(), 1);

	{
		StandaloneMesh& mesh = m_meshRenderJob->GetMesh();

		for (const auto& bind : m_bindables)
			mesh.AddBindable(bind);

		mesh.AddBindable(m_renderTargetSRV); // t0
		mesh.AddBindable(Shader::GetBindableResource(graphics, L"PS_Fullscreen", ShaderType::PixelShader)); // ps
		mesh.AddBindable(Shader::GetBindableResource(graphics, L"VS_Fullscreen", ShaderType::VertexShader)); // vs
		mesh.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)); // topology
		mesh.AddBindable(ViewPort::GetBindableResource(graphics)); // vp
	}
}

void FullscreenRenderPass::InitializePassResources(Graphics& graphics, Pipeline& pipeline)
{
	// copy index and vertex buffers to gpu
	m_pVertexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_pIndexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);

	// Updating camera data
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();
		const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

		DynamicConstantBuffer::ConstantBufferData& cameraData = m_pCameraData->GetData();
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

		m_pCameraData->Update(graphics);
	}

	// initializing job
	m_meshRenderJob->Initialize(graphics, pipeline);

	InitializeFullscreenResources(graphics, pipeline);
}

void FullscreenRenderPass::InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline)
{

}

void FullscreenRenderPass::Update(Graphics& graphics, Pipeline& pipeline)
{
	// if camera viewmatrix updated then update cameraData cbuffer
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();

		if (currentCamera->PerspectiveChanged())
		{
			const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

			DynamicConstantBuffer::ConstantBufferData& cameraData = m_pCameraData->GetData();
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

			m_pCameraData->Update(graphics);
		}
	}

	InternalUpdate(graphics, pipeline);
}

void FullscreenRenderPass::AddBindable(std::shared_ptr<Bindable> bind)
{
	m_bindables.push_back(std::move(bind));
}

void FullscreenRenderPass::ExecutePass(Graphics& graphics, CommandList* commandList)
{
	PreDraw(graphics, commandList);

	m_meshRenderJob->Execute(graphics, commandList);

	PostDraw(graphics, commandList);
}

void FullscreenRenderPass::PreDraw(Graphics& graphics, CommandList* commandList)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);

	commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

void FullscreenRenderPass::PostDraw(Graphics& graphics, CommandList* commandList)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);

	commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void FullscreenRenderPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{

}

void FullscreenRenderPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}