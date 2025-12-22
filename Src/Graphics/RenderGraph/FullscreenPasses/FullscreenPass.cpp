#include "FullscreenPass.h"

#include "Scene/Objects/Camera.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Core/TempCommandList.h"

FullscreenPass::FullscreenPass(Graphics& graphics)
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

	m_sampler = StaticSampler::GetBindableResource(graphics);
	m_blendState = BlendState::GetBindableResource(graphics);
	m_rasterizerState = RasterizerState::GetBindableResource(graphics);
	m_viewPort = ViewPort::GetBindableResource(graphics);
	m_topology = PrimitiveTechnology::GetBindableResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	m_fullscreenPS = Shader::GetBindableResource(graphics, L"PS_Fullscreen", ShaderType::PixelShader);
	m_fullscreenVS = Shader::GetBindableResource(graphics, L"VS_Fullscreen", ShaderType::VertexShader);

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

void FullscreenPass::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_vertexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_indexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);

	m_renderTargetSRV = ShaderResourceViewMultiResource::GetBindableResource(graphics, "BackBuffer", graphics.GetBackBuffer(), 0);
	m_depthStencilSRV = ShaderResourceViewMultiResource::GetBindableResource(graphics, "DepthStencil", graphics.GetDepthStencil(), 1);

	// Updating camera data
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();
		const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

		DynamicConstantBuffer::ConstantBufferData& cameraData = m_cameraData->GetData();
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

		m_cameraData->Update(graphics);
	}

	InternalInitialize(graphics, pipeline);
}

void FullscreenPass::Update(Graphics& graphics, Pipeline& pipeline)
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

void FullscreenPass::Draw(Graphics& graphics, Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

	m_mesh.DrawMesh(graphics, pipeline);

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}

void FullscreenPass::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	auto renderTarget = graphics.GetSwapChainBuffer();

	m_mesh.AddBindable(m_renderTargetSRV); // t0
	m_mesh.SetIndexBuffer(m_indexBuffer); // ib
	m_mesh.SetVertexBuffer(m_vertexBuffer); // vb
	m_mesh.AddBindable(m_fullscreenPS); // ps
	m_mesh.AddBindable(m_fullscreenVS); // vs
	m_mesh.AddBindable(m_sampler); // s0
	m_mesh.AddBindable(m_inputLayout); // il
	m_mesh.AddBindable(m_blendState); // bs
	m_mesh.AddBindable(m_rasterizerState); // rs
	m_mesh.AddBindable(m_topology); // topology
	m_mesh.AddBindable(m_viewPort); // vp
	m_mesh.AddBindable(renderTarget); // rt

	m_mesh.Initialize(graphics, pipeline);
}

void FullscreenPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void FullscreenPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{

}