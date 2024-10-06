#include "Triangle.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"

Triangle::Triangle(Graphics& graphics)
{
	HRESULT hr;

	// Creating command allocator
	{
		THROW_ERROR(graphics.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&pCommandAllocator)));// or D3D12_COMMAND_LIST_TYPE_BUNDLE made for groups of commands
	}

	m_commandList = std::make_unique<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_BUNDLE, pCommandAllocator.Get());


	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
	};

	std::vector<InputLayout::Item> layoutElements = { {"POSITION", InputLayout::ItemType::Position3}, {"COLOR", InputLayout::ItemType::Color3} };

	std::vector<Vertice> vertices = {
		{{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
	};

	std::vector<unsigned int> indices = {
		0,1,2
	};


	m_vertexBuffer = std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0)));
	m_indexBuffer = std::make_shared<IndexBuffer>(graphics, indices);

	Shader pixelShader("PS_Solid", PixelShader);
	Shader vertexShader("VS", VertexShader);
	BlendState blendState = {};
	RasterizerState rasterizerState = {};
	DepthStencilState depthStencilState = {};
	InputLayout inputLayout(layoutElements);

	// initialize pipeline state object
	{
		m_pipelineState = std::make_unique<PipelineState>();

		// setting up pipeline state desc
		{
			// pRootSignature 
			m_pipelineState->SetRootSignature(graphics.GetRootSignature());

			// Pixel Shader
			m_pipelineState->SetPixelShader(&pixelShader);

			// Vertex Shader
			m_pipelineState->SetVertexShader(&vertexShader);

			// BlendState 
			m_pipelineState->SetBlendState(&blendState);

			// SampleMask 
			m_pipelineState->SetSampleMask(0xffffffff);

			// RasterizerState 
			m_pipelineState->SetRasterizerState(&rasterizerState);
			
			// DepthStencilState
			m_pipelineState->SetDepthStencilState(&depthStencilState);

			// InputLayout 
			m_pipelineState->SetInputLayout(&inputLayout);

			// PrimitiveTopologyType 
			m_pipelineState->SetPrimitiveTechnologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

			// NumRenderTargets 
			m_pipelineState->SetNumRenderTargets(1);

			// RTVFormats
			m_pipelineState->SetRenderTargetFormat(0, graphics.GetColorSpace());

			// SampleDesc
			m_pipelineState->SetSampleDesc(1, 0);
		}

		m_pipelineState->Finish(graphics); // Finish() call gets object from desc it made up

		m_commandList->Open(graphics, pCommandAllocator.Get(), m_pipelineState->Get());
	}
	
	// SetGraphicsRootSignature
	// RSSetViewports
	// RSSetScissorRects

	// ClearRenderTargetView
	// ClearDepthStencilView

	// setting render target for local command list
	m_commandList->SetRenderTarget(graphics, graphics.GetBackBuffer(), graphics.GetDepthStencil());

	m_commandList->SetPrimitiveTopology(graphics, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetVertexBuffer(graphics, m_vertexBuffer.get());

	m_commandList->SetIndexBuffer(graphics, m_indexBuffer.get());

	m_commandList->DrawIndexed(graphics, indices.size());

	// ResourceBarrier RenderTarget D3D12_RESOURCE_STATE_RENDER_TARGET -> D3D12_RESOURCE_STATE_PRESENT

	m_commandList->Close(graphics);

	// wait for resources to get uploaded to gpu
}

void Triangle::Draw(Graphics& graphics) const
{
	// making this as temporary, later we will probably save points to some CommandQueue class and execute them from there

	ID3D12CommandList* ppCommandLists[] = { m_commandList->Get() };

	graphics.GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists);
}