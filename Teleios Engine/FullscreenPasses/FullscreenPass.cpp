#include "FullscreenPass.h"

#include "Camera.h"
#include "Pipeline.h"
#include "TempCommandList.h"

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

		if (currentCamera->ViewChanged())
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

	RenderTarget* renderTarget = graphics.GetSwapChainBuffer();

	TempGraphicsCommandList tempGraphicsCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		tempGraphicsCommandList.Bind(m_renderTargetSRV); // t0
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_fullscreenPS); // ps
		tempGraphicsCommandList.Bind(m_fullscreenVS); // vs
		tempGraphicsCommandList.Bind(m_sampler); // s0
		tempGraphicsCommandList.Bind(m_inputLayout); // il
		tempGraphicsCommandList.Bind(m_blendState); // bs
		tempGraphicsCommandList.Bind(m_rasterizerState); // rs
		tempGraphicsCommandList.Bind(m_topology); // topology
		tempGraphicsCommandList.Bind(m_viewPort); // vp
		tempGraphicsCommandList.Bind(renderTarget); // rt

		tempGraphicsCommandList.DrawIndexed(graphics);
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(tempGraphicsCommandList));

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}

void FullscreenPass::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{

}

void FullscreenPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void FullscreenPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{

}