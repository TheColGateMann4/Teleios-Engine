#include "FullscreenRenderPass.h"

#include "Scene/Objects/Camera.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Core/TempCommandList.h"
#include "Graphics/RenderGraph/RenderJob/MeshRenderJob.h"
#include "Scene/StandaloneMesh.h"

FullscreenRenderPass::FullscreenRenderPass(Graphics& graphics, RenderManager& renderManager)
{
	m_indexBuffer = IndexBuffer::GetBindableResource(graphics, "FullscreenMesh", std::vector<unsigned int>{0, 1, 3, 0, 3, 2});

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

	m_vertexBuffer = VertexBuffer::GetBindableResource(graphics, "FullscreenMesh", vertices.data(), vertices.size(), sizeof(vertices.at(0)));
	m_inputLayout = InputLayout::GetBindableResource(graphics, layout);

	const Camera::Settings defaultCameraSettings = Camera::Settings{};

	// camera data
	{
		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("nearPlane");
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("farPlane");

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = defaultCameraSettings.NearZ;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = defaultCameraSettings.FarZ;

		m_cameraData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}});
	}
}

void FullscreenRenderPass::Initialize(Graphics& graphics, Pipeline& pipeline, RenderManager& renderManager)
{
	m_vertexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_indexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);

	m_renderTargetSRV = ShaderResourceViewMultiResource::GetBindableResource(graphics, "BackBuffer", graphics.GetBackBuffer().get(), 0);
	m_depthStencilSRV = ShaderResourceViewMultiResource::GetBindableResource(graphics, "DepthStencil", graphics.GetDepthStencil().get(), 1);

	// Updating camera data
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();
		const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

		DynamicConstantBuffer::ConstantBufferData& cameraData = m_cameraData->GetData();
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

		m_cameraData->Update(graphics);
	}

	std::shared_ptr<MeshRenderJob> meshRenderJob = std::make_shared<MeshRenderJob>(RenderJob::JobType::FullscreenPass);

	{
		StandaloneMesh& mesh = meshRenderJob->GetMesh();

		mesh.AddBindable(m_renderTargetSRV); // t0
		mesh.AddBindable(m_indexBuffer); // ib
		mesh.AddBindable(m_vertexBuffer); // vb
		mesh.AddBindable(m_inputLayout); // il
		mesh.AddBindable(Shader::GetBindableResource(graphics, L"PS_Fullscreen", ShaderType::PixelShader)); // ps
		mesh.AddBindable(Shader::GetBindableResource(graphics, L"VS_Fullscreen", ShaderType::VertexShader)); // vs
		mesh.AddBindable(StaticSampler::GetBindableResource(graphics)); // s0
		mesh.AddBindable(BlendState::GetBindableResource(graphics)); // bs
		mesh.AddBindable(RasterizerState::GetBindableResource(graphics)); // rs
		mesh.AddBindable(PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)); // topology
		mesh.AddBindable(ViewPort::GetBindableResource(graphics)); // vp

		mesh.Initialize(graphics, pipeline);
	}

	renderManager.AddJob(std::move(meshRenderJob));
}

void FullscreenRenderPass::Update(Graphics& graphics, Pipeline& pipeline)
{
	// if camera viewmatrix updated then update cameraData cbuffer
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();

		if (currentCamera->PerspectiveChanged())
		{
			const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

			DynamicConstantBuffer::ConstantBufferData& cameraData = m_cameraData->GetData();
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

			m_cameraData->Update(graphics);
		}
	}

	InternalUpdate(graphics, pipeline);
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

RenderJob::JobType FullscreenRenderPass::GetWantedJob() const
{
	return RenderJob::JobType::FullscreenPass;
}

void FullscreenRenderPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void FullscreenRenderPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{

}