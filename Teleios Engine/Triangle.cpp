#include "Triangle.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

Triangle::Triangle(Graphics& graphics)
{
	HRESULT hr;

	// Creating command allocator
	{
		THROW_ERROR(graphics.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&pCommandAllocator)));// or D3D12_COMMAND_LIST_TYPE_BUNDLE made for groups of commands
	}

	// Creating command list
	{
		THROW_ERROR(graphics.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList)));
	}


	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 color;
	};

	std::vector<Vertice> vertices = {
		{{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
	};

	std::vector<unsigned int> indices = {
		0,1,2
	};


	// setting render target for local command list
	graphics.GetBackBuffer()->Bind(graphics, pCommandList.Get());


	//D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	//vertexBufferView.BufferLocation = GetGPUVirtualAddress();

	{
		pPipelineState = std::make_shared<PipelineState>();

		{
			Shader pixelShader("PS_Solid", PixelShader);

			pPipelineState->SetPixelShader(&pixelShader);
		}
		{
			Shader vertexShader("VS", VertexShader);

			pPipelineState->SetVertexShader(&vertexShader);
		}

	}

	THROW_INFO_ERROR(pCommandList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0));
}