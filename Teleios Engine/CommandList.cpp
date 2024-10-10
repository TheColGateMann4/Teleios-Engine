#include "CommandList.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

CommandList::CommandList(Graphics& graphics, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pPipelineState)
	:
	m_type(type),
	m_initialized(true),
	m_closed(true)
{
	HRESULT hr;
	
	//creating command allocator for given command list type
	THROW_ERROR(graphics.GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_pCommandAllocator)));

	THROW_ERROR(graphics.GetDevice()->CreateCommandList(0, type, m_pCommandAllocator.Get(), pPipelineState, IID_PPV_ARGS(&pCommandList)));

	THROW_ERROR(pCommandList->Close());
}

void CommandList::Open(Graphics& graphics, ID3D12PipelineState* pPipelineState)
{
	THROW_INTERNAL_ERROR_IF("Cannot call Open when command list is not initialized", !m_initialized);

	HRESULT hr;

	THROW_ERROR(m_pCommandAllocator->Reset());

	THROW_ERROR(pCommandList->Reset(m_pCommandAllocator.Get(), pPipelineState));
}

void CommandList::Close(Graphics& graphics)
{
	THROW_INTERNAL_ERROR_IF("Cannot call Close when command list is not initialized", !m_initialized);

	HRESULT hr;

	THROW_ERROR(pCommandList->Close());
}

void CommandList::DrawIndexed(Graphics& graphics, unsigned int indices)
{
	THROW_INTERNAL_ERROR_IF("Cannot call DrawIndexed when command list is not initialized", !m_initialized);

	THROW_INFO_ERROR(pCommandList->DrawIndexedInstanced(indices, 1, 0, 0, 0));
}

ID3D12GraphicsCommandList* CommandList::Get()
{
	return pCommandList.Get();
}

void CommandList::ResourceBarrier(Graphics& graphics, RenderTarget* renderTarget, D3D12_RESOURCE_STATES previousState, D3D12_RESOURCE_STATES afterState) const
{
	THROW_INTERNAL_ERROR_IF("Cannot call ResourceBarrier when command list is not initialized", !m_initialized);
	THROW_INTERNAL_ERROR_IF("Cannot call ResourceBarrier on bundle command list object", m_type == D3D12_COMMAND_LIST_TYPE_BUNDLE);

	// marking out current front buffer as the one that will be changing states, from present back to render target
	{
		ID3D12Resource* pCurrFrontBuffer = renderTarget->GetResource(graphics);

		D3D12_RESOURCE_BARRIER resourceBarrier = {};
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = pCurrFrontBuffer;
		resourceBarrier.Transition.StateBefore = previousState;
		resourceBarrier.Transition.StateAfter = afterState;

		THROW_INFO_ERROR(pCommandList->ResourceBarrier(1, &resourceBarrier));
	}
}

void CommandList::SetRenderTarget(Graphics& graphics, RenderTarget* renderTarget, DepthStencilView* depthStencilView)
{
	THROW_INTERNAL_ERROR_IF("Cannot call SetRenderTarget when command list is not initialized", !m_initialized);
	THROW_INTERNAL_ERROR_IF("Cannot call ResourceBarrier on bundle command list object", m_type == D3D12_COMMAND_LIST_TYPE_BUNDLE);

	// binding render target to command list
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetViewDescriptor = renderTarget->GetDescriptor(graphics);

		const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilViewDescriptor = depthStencilView != nullptr ? depthStencilView->GetDescriptor() : nullptr;

		THROW_INFO_ERROR(pCommandList->OMSetRenderTargets(1, renderTargetViewDescriptor, false, depthStencilViewDescriptor));
	}
}

void CommandList::SetVertexBuffer(Graphics& graphics, VertexBuffer* vertexBuffer)
{
	THROW_INTERNAL_ERROR_IF("Cannot call SetVertexBuffer when command list is not initialized", !m_initialized);

	THROW_INFO_ERROR(pCommandList->IASetVertexBuffers(0, 1, vertexBuffer->Get()));
}

void CommandList::SetIndexBuffer(Graphics& graphics, IndexBuffer* indexBuffer)
{
	THROW_INTERNAL_ERROR_IF("Cannot call SetIndexBuffer when command list is not initialized", !m_initialized);

	THROW_INFO_ERROR(pCommandList->IASetIndexBuffer(indexBuffer->Get()));
}

void CommandList::SetPrimitiveTopology(Graphics& graphics, D3D_PRIMITIVE_TOPOLOGY primitiveTechnology)
{
	THROW_INTERNAL_ERROR_IF("Cannot call SetPrimitiveTopology when command list is not initialized", !m_initialized);

	THROW_INFO_ERROR(pCommandList->IASetPrimitiveTopology(primitiveTechnology));
}

void CommandList::SetRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	THROW_INTERNAL_ERROR_IF("Cannot call SetRootSignature when command list is not initialized", !m_initialized);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootSignature(rootSignature->Get()));
}

void CommandList::SetConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer)
{
	THROW_INTERNAL_ERROR_IF("Cannot call ExecuteBundle when command list is not initialized", !m_initialized);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootConstantBufferView(constBuffer->GetRootIndex(), constBuffer->GetGPUAddress()));
}

void CommandList::ExecuteBundle(Graphics& graphics, CommandList* commandList)
{
	THROW_INTERNAL_ERROR_IF("Cannot call ExecuteBundle when command list is not initialized", !m_initialized);
	THROW_INTERNAL_ERROR_IF("Cannot call ExecuteBundle on non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->ExecuteBundle(commandList->Get()));
}

void CommandList::ClearRenderTargetView(Graphics& graphics, RenderTarget* renderTarget)
{
	THROW_INTERNAL_ERROR_IF("Cannot call ClearRenderTargetView when command list is not initialized", !m_initialized);
	THROW_INTERNAL_ERROR_IF("Cannot call ClearRenderTargetView on non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	FLOAT clearColor[] = { 0.01f, 0.02f, 0.03f, 1.0f };

	THROW_INFO_ERROR(pCommandList->ClearRenderTargetView(
		*graphics.GetBackBuffer()->GetDescriptor(graphics),
		clearColor,
		0,
		nullptr
	));
};

void CommandList::ClearDepthStencilView(Graphics& graphics, DepthStencilView* depthStencilView)
{
	THROW_INTERNAL_ERROR_IF("Cannot call ClearDepthStencilView when command list is not initialized", !m_initialized);
	THROW_INTERNAL_ERROR_IF("Cannot call ClearDepthStencilView on non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);


	THROW_INFO_ERROR(pCommandList->ClearDepthStencilView(
		*graphics.GetDepthStencil()->GetDescriptor(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0.0f,
		0,
		nullptr
	));
};