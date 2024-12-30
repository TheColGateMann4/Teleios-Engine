#include "CommandList.h"
#include "Graphics.h"
#include "PipelineState.h"
#include "Macros/ErrorMacros.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include "UnorderedAccessView.h"
#include "TextureMipView.h"

#include "DescriptorHeap.h"

CommandList::CommandList(Graphics& graphics, D3D12_COMMAND_LIST_TYPE type, PipelineState* pPipelineState)
	:
	m_pCommandAllocators(graphics.GetBufferCount()),
	m_type(type),
	m_initialized(true),
	m_open(false),
	m_currCommandAllocatorIndex(0)
{
	HRESULT hr;

	// initializing command allocators
	{
		for(unsigned int currAllocatorIndex = 0; currAllocatorIndex < m_pCommandAllocators.size(); currAllocatorIndex++)
			THROW_ERROR(graphics.GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_pCommandAllocators.at(currAllocatorIndex))));
	}

	ID3D12PipelineState* pipelineState = (pPipelineState == nullptr) ? nullptr : pPipelineState->Get();

	THROW_ERROR(graphics.GetDevice()->CreateCommandList(0, type, m_pCommandAllocators.at(m_currCommandAllocatorIndex).Get(), pipelineState, IID_PPV_ARGS(&pCommandList)));

	THROW_ERROR(pCommandList->Close());
}

void CommandList::Open(Graphics& graphics, PipelineState* pPipelineState)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);

	HRESULT hr;
	ID3D12PipelineState* pipelineState = (pPipelineState == nullptr) ? nullptr : pPipelineState->Get();
	
	// reset currently used command allocator
	THROW_ERROR(m_pCommandAllocators.at(m_currCommandAllocatorIndex)->Reset());

	THROW_ERROR(pCommandList->Reset(m_pCommandAllocators.at(m_currCommandAllocatorIndex).Get(), pipelineState));

	// proceeding to next allocator for new frame
	m_currCommandAllocatorIndex = graphics.GetNextBufferIndex();

	m_open = true;
}

void CommandList::Close(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);

	HRESULT hr;

	THROW_ERROR(pCommandList->Close());

	m_open = false;
}

void CommandList::DrawIndexed(Graphics& graphics, unsigned int indices)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can DrawIndexed", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->DrawIndexedInstanced(indices, 1, 0, 0, 0));
}

void CommandList::Dispatch(Graphics& graphics)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can dispatch compute pipeline", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE);

	THROW_INFO_ERROR(pCommandList->Dispatch(1, 1, 1));
}

ID3D12GraphicsCommandList* CommandList::Get()
{
	return pCommandList.Get();
}

bool CommandList::IsOpen() const
{
	return m_open;
}

void CommandList::SetResourceState(Graphics& graphics, RenderTarget* renderTarget, D3D12_RESOURCE_STATES newState) const
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	// marking out current front buffer as the one that will be changing states, from present back to render target
	{
		ID3D12Resource* pCurrFrontBuffer = renderTarget->GetResource(graphics);

		D3D12_RESOURCE_BARRIER resourceBarrier = {};
		resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		resourceBarrier.Transition.pResource = pCurrFrontBuffer;
		resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		resourceBarrier.Transition.StateBefore = renderTarget->GetResourceState(graphics);
		resourceBarrier.Transition.StateAfter = newState;

		THROW_INFO_ERROR(pCommandList->ResourceBarrier(1, &resourceBarrier));
	}

	renderTarget->SetResourceState(graphics, newState);
}

void CommandList::SetResourceState(Graphics& graphics, ID3D12Resource* resource, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES newState, unsigned int targetSubresource) const
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = resource;
	resourceBarrier.Transition.Subresource = targetSubresource;
	resourceBarrier.Transition.StateBefore = prevState;
	resourceBarrier.Transition.StateAfter = newState;

	THROW_INFO_ERROR(pCommandList->ResourceBarrier(1, &resourceBarrier));
}

void CommandList::SetRenderTarget(Graphics& graphics, RenderTarget* renderTarget, DepthStencilView* depthStencilView)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	// binding render target to command list
	{
		const D3D12_CPU_DESCRIPTOR_HANDLE* renderTargetViewDescriptor = renderTarget->GetDescriptor(graphics);

		const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilViewDescriptor = depthStencilView != nullptr ? depthStencilView->GetDescriptor() : nullptr;

		THROW_INFO_ERROR(pCommandList->OMSetRenderTargets(1, renderTargetViewDescriptor, false, depthStencilViewDescriptor));
	}
}

void CommandList::SetVertexBuffer(Graphics& graphics, VertexBuffer* vertexBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set vertex buffers", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->IASetVertexBuffers(0, 1, vertexBuffer->Get()));
}

void CommandList::SetIndexBuffer(Graphics& graphics, IndexBuffer* indexBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set index buffers", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->IASetIndexBuffer(indexBuffer->Get()));
}

void CommandList::SetPrimitiveTopology(Graphics& graphics, D3D_PRIMITIVE_TOPOLOGY primitiveTechnology)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set topology", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->IASetPrimitiveTopology(primitiveTechnology));
}

void CommandList::SetGraphicsRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics root signature", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootSignature(rootSignature->Get()));
}

void CommandList::SetGraphicsConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics constant buffers", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	auto& targets = constBuffer->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetGraphicsRootConstantBufferView(targetShader.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetDescriptorHeap(Graphics& graphics, DescriptorHeap* descriptorHeap)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Copy command lists cannot set descriptor heaps", m_type == D3D12_COMMAND_LIST_TYPE_COPY);

	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap->Get()};

	THROW_INFO_ERROR(pCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps));
}

void CommandList::SetGraphicsDescriptorTable(Graphics& graphics, Texture* texture)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics constant buffers", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	auto& targets = texture->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetGraphicsRootDescriptorTable(targetShader.rootIndex, texture->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::ExecuteBundle(Graphics& graphics, CommandList* commandList)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->ExecuteBundle(commandList->Get()));
}

void CommandList::ClearRenderTargetView(Graphics& graphics, RenderTarget* renderTarget)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);
	
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
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->ClearDepthStencilView(
		*graphics.GetDepthStencil()->GetDescriptor(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0.0f,
		0,
		nullptr
	));
};

void CommandList::SetPipelineState(Graphics& graphics, PipelineState* pPipelineState)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);

	THROW_INFO_ERROR(pCommandList->SetPipelineState(pPipelineState->Get()));
}

void CommandList::SetComputeRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute root signature", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootSignature(rootSignature->Get()));
}

void CommandList::SetComputeConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute constant buffer view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	auto& targets = constBuffer->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetComputeRootConstantBufferView(targetShader.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetComputeDescriptorTable(Graphics& graphics, Texture* texture)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute descriptor table", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootDescriptorTable(texture->GetComputeRootIndex(), texture->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetComputeDescriptorTable(Graphics& graphics, UnorderedAccessView* uav)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute descriptor table", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	auto& targets = uav->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetComputeRootDescriptorTable(targetShader.rootIndex, uav->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetComputeDescriptorTable(Graphics& graphics, TextureMipView* srv)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute descriptor table", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	auto& targets = srv->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetComputeRootDescriptorTable(targetShader.rootIndex, srv->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetComputeRootShaderResourceView(Graphics& graphics, ConstantBuffer* constBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute shader resource view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	auto& targets = constBuffer->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetComputeRootShaderResourceView(targetShader.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetComputeRootUnorderedAccessView(Graphics& graphics, ConstantBuffer* constBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute unordered access view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	auto& targets = constBuffer->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetComputeRootUnorderedAccessView(targetShader.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetComputeRootConstantBufferView(Graphics& graphics, ConstantBuffer* constBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute constant buffer view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	auto& targets = constBuffer->GetTargets();

	for (auto& targetShader : targets)
		THROW_INFO_ERROR(pCommandList->SetComputeRootConstantBufferView(targetShader.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::CopyBufferRegion(Graphics& graphics, ID3D12Resource* dstResource, UINT64 dstOffset, ID3D12Resource* srcResource, UINT64 srcOffset, UINT64 numBytes)
{
	THROW_OBJECT_STATE_ERROR_IF("Bundle command lists cannot copy resources", m_type == D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->CopyBufferRegion(
		dstResource,
		dstOffset,
		srcResource,
		srcOffset,
		numBytes
	));
}

void CommandList::CopyTextureRegion(Graphics& graphics, ID3D12Resource* dstResource, ID3D12Resource* srcResource, unsigned int MipMapLvel)
{
	THROW_OBJECT_STATE_ERROR_IF("Bundle command lists cannot copy resources", m_type == D3D12_COMMAND_LIST_TYPE_BUNDLE);

	D3D12_TEXTURE_COPY_LOCATION dstTexture;
	dstTexture.pResource = dstResource;
	dstTexture.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstTexture.SubresourceIndex = MipMapLvel;

	D3D12_TEXTURE_COPY_LOCATION srcTexture;
	srcTexture.pResource = srcResource;
	srcTexture.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	srcTexture.SubresourceIndex = MipMapLvel;

	THROW_INFO_ERROR(pCommandList->CopyTextureRegion(
		&dstTexture,
		0,
		0,
		0,
		&srcTexture,
		nullptr
	));
}

void CommandList::CopyResource(Graphics& graphics, ID3D12Resource* dstResource, ID3D12Resource* srcResource)
{
	THROW_OBJECT_STATE_ERROR_IF("Bundle command lists cannot copy resources", m_type == D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->CopyResource(
		dstResource,
		srcResource
	));
}