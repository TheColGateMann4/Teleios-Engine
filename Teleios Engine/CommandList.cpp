#include "CommandList.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

CommandList::CommandList(Graphics& graphics, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pPipelineState)
	:
	m_initialized(true),
	m_closed(true)
{
	HRESULT hr;

	THROW_ERROR(graphics.GetDevice()->CreateCommandList(0, type, pCommandAllocator, pPipelineState, IID_PPV_ARGS(&pCommandList)));

	THROW_ERROR(pCommandList->Close());
}

void CommandList::Open(Graphics& graphics, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pPipelineState)
{
	HRESULT hr;

	THROW_ERROR(pCommandList->Reset(pCommandAllocator, pPipelineState));
}

void CommandList::Close(Graphics& graphics)
{
	HRESULT hr;

	THROW_ERROR(pCommandList->Close());
}

void CommandList::DrawIndexed(Graphics& graphics, unsigned int indices)
{
	THROW_INFO_ERROR(pCommandList->DrawIndexedInstanced(indices, 1, 0, 0, 0));
}

ID3D12GraphicsCommandList* CommandList::Get()
{
	return pCommandList.Get();
}

void CommandList::SetRenderTarget(Graphics& graphics, RenderTarget* renderTarget, DepthStencilView* depthStencilView)
{
	// marking out current front buffer as the one that will be changing states, from present back to render target
	{
		ID3D12Resource* pCurrFrontBuffer = renderTarget->GetResource(graphics);

		D3D12_RESOURCE_BARRIER resourceBarrier = {};
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = pCurrFrontBuffer;
		resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		THROW_INFO_ERROR(pCommandList->ResourceBarrier(1, &resourceBarrier));
	}

	// binding render target to command list
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetViewDescriptor = renderTarget->GetDescriptor(graphics);

		const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilViewDescriptor = depthStencilView != nullptr ? depthStencilView->GetDescriptor() : nullptr;

		THROW_INFO_ERROR(pCommandList->OMSetRenderTargets(1, renderTargetViewDescriptor, 1, depthStencilViewDescriptor));
	}
}

void CommandList::SetVertexBuffer(Graphics& graphics, VertexBuffer* vertexBuffer)
{
	THROW_INFO_ERROR(pCommandList->IASetVertexBuffers(0, 1, vertexBuffer->Get()));
}

void CommandList::SetIndexBuffer(Graphics& graphics, IndexBuffer* indexBuffer)
{
	THROW_INFO_ERROR(pCommandList->IASetIndexBuffer(indexBuffer->Get()));
}

void CommandList::SetPrimitiveTopology(Graphics& graphics, D3D_PRIMITIVE_TOPOLOGY primitiveTechnology)
{
	THROW_INFO_ERROR(pCommandList->IASetPrimitiveTopology(primitiveTechnology));
}