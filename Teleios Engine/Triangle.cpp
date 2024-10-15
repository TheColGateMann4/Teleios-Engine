#include "Triangle.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

#include "Shader.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "ConstantBuffer.h"

#include "DynamicConstantBuffer.h"

#include <imgui.h>

Triangle::Triangle(Graphics& graphics)
{
	m_bundleCommandList = std::make_unique<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_BUNDLE);
	m_directCommandList = std::make_unique<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_DIRECT);

	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoords;
	};

	float aspectRatioX = float(graphics.GetWidth()) / graphics.GetHeight();

	std::vector<InputLayout::Item> layoutElements = { {"POSITION", InputLayout::ItemType::Position3}, {"TEXCOORDS", InputLayout::ItemType::TexCoords} };

	std::vector<Vertice> vertices = {
		{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f,  0.5f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}}
	};

	std::vector<unsigned int> indices = {
		0,1,3,
		0,3,2
	};


	m_vertexBuffer = std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0)));
	m_indexBuffer = std::make_shared<IndexBuffer>(graphics, indices);

	Shader pixelShader("PS_Texture", PixelShader);
	Shader vertexShader("VS", VertexShader);
	BlendState blendState = {};
	RasterizerState rasterizerState = {};
	DepthStencilState depthStencilState = {};
	InputLayout inputLayout(layoutElements);
	RootSignature rootSignature;

	DynamicConstantBuffer::ConstantBufferLayout layout;
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("texcoordsScale");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("brightness");

	DynamicConstantBuffer::ConstantBufferData bufferData(layout);
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("texcoordsScale") = 1.0f;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("brightness") = 1.0f;

	constantBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData);
	constantBuffer->Update(graphics);

	texture = std::make_shared<Texture>(graphics, L"brickwall.jpg");

	// initializing root signature
	{
	    rootSignature.AddConstBufferViewParameter(constantBuffer.get());
	    rootSignature.AddDescriptorTableParameter(texture.get());

		rootSignature.AddStaticSampler(0, TargetShader::PixelShader);

		rootSignature.Initialize(graphics);
	}

	// initialize pipeline state object
	{
		m_pipelineState = std::make_unique<PipelineState>();

		// setting up pipeline state desc
		{
			// pRootSignature 
			m_pipelineState->SetRootSignature(&rootSignature);

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
			
			// DSVFormats
			m_pipelineState->SetDepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);

			// SampleDesc
			m_pipelineState->SetSampleDesc(1, 0);
		}

		m_pipelineState->Finish(graphics); // Finish() call gets object from desc it made up

		m_bundleCommandList->Open(graphics, m_pipelineState->Get());
	}
	

	m_bundleCommandList->SetPrimitiveTopology(graphics, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_bundleCommandList->SetVertexBuffer(graphics, m_vertexBuffer.get());

	m_bundleCommandList->SetIndexBuffer(graphics, m_indexBuffer.get());

	m_bundleCommandList->SetRootSignature(graphics, &rootSignature);

	m_bundleCommandList->SetConstBufferView(graphics, constantBuffer.get());

	m_bundleCommandList->SetDescriptorHeap(graphics, texture.get());

	m_bundleCommandList->SetDescriptorTable(graphics, texture.get());

	m_bundleCommandList->DrawIndexed(graphics, indices.size());

	m_bundleCommandList->Close(graphics);
}

void Triangle::Draw(Graphics& graphics) const
{
	if (ImGui::Begin("Triangle"))
	{
		bool changed = false;

		auto checkChanged = [&changed](bool expressionReturn) mutable
			{
				changed =  changed || expressionReturn;
			};

		checkChanged(ImGui::SliderFloat("Texcoords Scale", constantBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("texcoordsScale"), 0.1f, 10.0f));
		checkChanged(ImGui::SliderFloat("Brightness", constantBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("brightness"), 0.01f, 5.0f));
			
		if(changed)
			constantBuffer->Update(graphics);

		ImGui::End();
	}

	graphics.GetImguiManager()->Render();

	m_directCommandList->Open(graphics, m_pipelineState->Get());

	m_directCommandList->ResourceBarrier(graphics, graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_directCommandList->SetRenderTarget(graphics, graphics.GetBackBuffer(), graphics.GetDepthStencil());

	{
		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = graphics.GetWidth();
		viewport.Height = graphics.GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_directCommandList->Get()->RSSetViewports(1, &viewport);

		D3D12_RECT viewportRect = {};
		viewportRect.left = viewportRect.top = 0;
		viewportRect.bottom = graphics.GetHeight();
		viewportRect.right = graphics.GetWidth();

		m_directCommandList->Get()->RSSetScissorRects(1, &viewportRect);
	}

	m_directCommandList->ClearRenderTargetView(graphics, graphics.GetBackBuffer());

	m_directCommandList->ClearDepthStencilView(graphics, graphics.GetDepthStencil());

	m_directCommandList->SetDescriptorHeap(graphics, texture.get());

	m_directCommandList->ExecuteBundle(graphics, m_bundleCommandList.get());
	
	graphics.GetImguiCommands(m_directCommandList->Get());

	m_directCommandList->ResourceBarrier(graphics, graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_directCommandList->Close(graphics);

	// making this as temporary, later we will probably save points to some CommandQueue class and execute them from there

	ID3D12CommandList* ppCommandLists[] = { m_directCommandList->Get() };

	graphics.GetCommandQueue()->ExecuteCommandLists(1, ppCommandLists);
}