#include "PostProcessing.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"
#include "TempCommandList.h"


#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderTarget.h"
#include "InputLayout.h"
#include "Sampler.h"
#include "ShaderResourceView.h"
#include "BlendState.h"
#include "PrimitiveTechnology.h"
#include "RasterizerState.h"
#include "ViewPort.h"

#include "UnorderedAccessView.h"

#include "TempCommandList.h"

PostProcessing::PostProcessing(Graphics& graphics, Pipeline& pipeline)
{
	m_indexBuffer = std::make_shared<IndexBuffer>(graphics, std::vector<unsigned int>{0, 1, 3, 0, 3, 2});

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

	m_vertexBuffer = std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0)));
	m_inputLayout = std::make_shared<InputLayout>(graphics, layout);

	m_finalPixelShader = std::make_shared<Shader>(graphics, L"PS_Fullscreen", ShaderType::PixelShader);
	m_finalVertexShader = std::make_shared<Shader>(graphics, L"VS_Fullscreen", ShaderType::VertexShader);


	m_vertexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_indexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);

	// requesting size for RT and DS SRV's. Each one has special SRV for one frame
	graphics.GetDescriptorHeap().RequestMoreSpace(graphics.GetBufferCount() * 2);
}

void PostProcessing::Initialize(Graphics& graphics)
{
	m_renderTargetSRV = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetBackBuffer(), 0);
	m_depthStencilSRV = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetDepthStencil(), 1);
}

void PostProcessing::ApplyTonemapping(Graphics& graphics, Pipeline& pipeline)
{
	RenderTarget* backBuffer = graphics.GetBackBuffer();

	std::shared_ptr<Shader> computeShader = Shader::GetBindableResource(graphics, L"CS_PostProcess", ShaderType::ComputeShader);

	std::shared_ptr<StaticSampler> sampler = StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, { {ShaderVisibilityGraphic::AllShaders, 0} });

	TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		// setting entry states
		//{
		//	CommandList* commandList = pipeline.GetGraphicCommandList();
		//
		//	commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		//}

		//UnorderedAccessView uav(graphics, backBuffer);
		//
		//computeCommandList.Bind(computeShader);
		//computeCommandList.Bind(sampler); // s0
		//computeCommandList.Bind(std::move(uav)); // u0
		//
		//computeCommandList.Dispatch(graphics, graphics.GetWidth(), graphics.GetHeight());
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));
}

void PostProcessing::Finish(Graphics& graphics, const Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
	GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}
	
	StaticSampler sampler(graphics);
	BlendState blendState(graphics);
	RasterizerState rasterizerState(graphics);
	PrimitiveTechnology topology(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	RenderTarget* renderTarget = graphics.GetSwapChainBuffer();
	ViewPort viewPort(graphics);

	TempGraphicsCommandList tempGraphicsCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		tempGraphicsCommandList.Bind(m_renderTargetSRV); // t0
		tempGraphicsCommandList.Bind(m_depthStencilSRV); // t1
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_finalPixelShader); // ps
		tempGraphicsCommandList.Bind(m_finalVertexShader); // vs
		tempGraphicsCommandList.Bind(std::move(sampler)); // s0
		tempGraphicsCommandList.Bind(m_inputLayout); // il
		tempGraphicsCommandList.Bind(std::move(blendState)); // bs
		tempGraphicsCommandList.Bind(std::move(rasterizerState)); // rs
		tempGraphicsCommandList.Bind(std::move(topology)); // topology
		tempGraphicsCommandList.Bind(std::move(viewPort)); // vp
		tempGraphicsCommandList.Bind(renderTarget); // rt

		tempGraphicsCommandList.DrawIndexed(graphics);
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(tempGraphicsCommandList));

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
}