#include "CommandList.h"
#include "Graphics.h"
#include "PipelineState.h"
#include "Macros/ErrorMacros.h"

#include "Includes/BindablesInclude.h"

#include "Scene/Material.h"

#include "Graphics/Resources/QueryHeap.h"
#include "Graphics/Resources/GraphicsResource.h"
#include "Graphics/Resources/GraphicsTexture.h"

#include "Graphics/Core/DescriptorHeap.h"

#include "Graphics/Bindables/DescriptorHeapBindable.h"

#ifdef _DEBUG
	#include <pix3.h>
	#pragma comment(lib, "WinPixEventRuntime.lib")
#endif

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
			THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_pCommandAllocators.at(currAllocatorIndex))));
	}

	ID3D12PipelineState* pipelineState = (pPipelineState == nullptr) ? nullptr : pPipelineState->Get();

	THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateCommandList(0, type, m_pCommandAllocators.at(m_currCommandAllocatorIndex).Get(), pipelineState, IID_PPV_ARGS(&pCommandList)));

	THROW_ERROR(pCommandList->Close());
}

void CommandList::Open(Graphics& graphics, PipelineState* pPipelineState)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);

	HRESULT hr;
	ID3D12PipelineState* pipelineState = (pPipelineState == nullptr) ? nullptr : pPipelineState->Get();
	
	m_currCommandAllocatorIndex = graphics.GetCurrentBufferIndex();

	// reset currently used command allocator
	THROW_ERROR(m_pCommandAllocators.at(m_currCommandAllocatorIndex)->Reset());

	THROW_ERROR(pCommandList->Reset(m_pCommandAllocators.at(m_currCommandAllocatorIndex).Get(), pipelineState));

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

void CommandList::Dispatch(Graphics& graphics, unsigned int workToProcessX, unsigned int workToProcessY, unsigned int workToProcessZ)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can dispatch compute pipeline", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE);

	THROW_INFO_ERROR(pCommandList->Dispatch(workToProcessX, workToProcessY, workToProcessZ));
}

ID3D12GraphicsCommandList* CommandList::Get()
{
	return pCommandList.Get();
}

bool CommandList::IsOpen() const
{
	return m_open;
}

#ifdef _DEBUG
void CommandList::SetMarker(std::string_view name)
{
	static UINT markerColor = PIX_COLOR(0, 255, 255);

	PIXSetMarker(pCommandList.Get(), markerColor, name.data());
}

void CommandList::BeginEvent(std::string_view name)
{
	static UINT eventColor = PIX_COLOR(255, 0, 255);

	PIXBeginEvent(pCommandList.Get(), eventColor, name.data());
}

void CommandList::EndEvent()
{
	PIXEndEvent(pCommandList.Get());
}
#endif

void CommandList::BeginRenderPass(Graphics& graphics, RenderPass* renderPass)
{
	const std::vector<RenderPass::RenderTargetData>& renderTargetViews = renderPass->GetRenderTargets();
	RenderPass::DepthStencilData depthStencilView = renderPass->GetDepthStencilView();
	
	std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> renderPasRenderTargetDescs(renderTargetViews.size(), {});
	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC renderPassDepthStencilDesc = {};

	{
		// initializing descriptions for renderTargets
		{
			for (size_t i = 0; i < renderTargetViews.size(); i++)
			{
				const RenderPass::RenderTargetData& renderTargetInfo = renderTargetViews.at(i);
				D3D12_RENDER_PASS_RENDER_TARGET_DESC& rtDesc = renderPasRenderTargetDescs[i];

				rtDesc.cpuDescriptor = renderTargetInfo.resource->GetDescriptor(graphics);
				rtDesc.BeginningAccess = CreateBeginningAccess(renderTargetInfo.loadOperation, renderTargetInfo.resource.get());
				rtDesc.EndingAccess = CreateEndingAccess(renderTargetInfo.storeOperation);
			}
		}

		// initializing description for depthStencil
		if(depthStencilView.resource)
		{
			renderPassDepthStencilDesc.cpuDescriptor = depthStencilView.resource->GetDescriptor(graphics);
			renderPassDepthStencilDesc.DepthBeginningAccess = CreateBeginningAccess(depthStencilView.loadOperation, depthStencilView.resource.get());
			renderPassDepthStencilDesc.StencilBeginningAccess = CreateBeginningAccess(depthStencilView.loadOperation, depthStencilView.resource.get());
			renderPassDepthStencilDesc.DepthEndingAccess = CreateEndingAccess(depthStencilView.storeOperation);
			renderPassDepthStencilDesc.StencilEndingAccess = CreateEndingAccess(depthStencilView.storeOperation);
		}
	}

	unsigned int numRenderTargets = renderPasRenderTargetDescs.size();
	const D3D12_RENDER_PASS_RENDER_TARGET_DESC* targetRTDesc = numRenderTargets > 0 ? renderPasRenderTargetDescs.data() : nullptr;
	const D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* targetDDSesc = depthStencilView.resource ? &renderPassDepthStencilDesc : nullptr;

	THROW_INFO_ERROR(pCommandList->BeginRenderPass(
		numRenderTargets,
		targetRTDesc,
		targetDDSesc,
		D3D12_RENDER_PASS_FLAG_NONE
	));
}

void CommandList::EndRenderPass(Graphics& graphics)
{ 
	THROW_INFO_ERROR(pCommandList->EndRenderPass());
}

void CommandList::Query(Graphics& graphics, QueryHeap* queryHeap, unsigned int entryIndex, D3D12_QUERY_TYPE queryType)
{
	THROW_OBJECT_STATE_ERROR_IF("Cannot call Query on non-executive command list", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE);
	THROW_INTERNAL_ERROR_IF("Tried to access entries outside of QueryHeap", queryHeap->GetNumElements() < entryIndex + 1);

	THROW_INFO_ERROR(pCommandList->EndQuery(queryHeap->Get(), queryType, entryIndex));
}

void CommandList::ResolveQuery(Graphics& graphics, QueryHeap* queryHeap, D3D12_QUERY_TYPE queryType, unsigned int entryIndex, unsigned int numEntries, GraphicsBuffer* resultBuffer, unsigned int destOffset)
{
	THROW_INTERNAL_ERROR_IF("Tried to access entries outside of QueryHeap", queryHeap->GetNumElements() < entryIndex + numEntries);

	THROW_INFO_ERROR(pCommandList->ResolveQueryData(
		queryHeap->Get(),
		queryType,
		entryIndex,
		numEntries,
		resultBuffer->GetResource(),
		destOffset
	));
}

void CommandList::SetResourceToTargetState(Graphics& graphics, GraphicsResource* resource, unsigned int targetSubresource) const
{
	D3D12_RESOURCE_STATES newState = resource->GetResourceTargetState(targetSubresource);

	SetResourceState(graphics, resource, newState);
}

void CommandList::SetResourcesToTargetStates(Graphics& graphics, GraphicsResource* resource) const
{
	D3D12_RESOURCE_STATES newState = resource->GetResourceTargetState(0);

	SetAllResourcesStates(graphics, resource, newState);
}

void CommandList::SetResourceState(Graphics& graphics, GraphicsResource* resource, D3D12_RESOURCE_STATES newState, unsigned int targetSubresource) const
{
	SetResourceState(graphics, resource->GetResource(), resource->GetResourceState(targetSubresource), newState, targetSubresource);

	resource->SetResourceState(newState, targetSubresource);
}

void CommandList::SetAllResourcesStates(Graphics& graphics, GraphicsResource* resource, D3D12_RESOURCE_STATES newState) const
{
	SetResourceState(graphics, resource->GetResource(), resource->GetResourceState(0), newState, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	resource->SetAllResourceStates(newState);
}

void CommandList::SetResourceState(Graphics& graphics, RenderTarget* renderTarget, D3D12_RESOURCE_STATES newState) const
{
	SetResourceState(graphics, renderTarget->GetResource(graphics), renderTarget->GetResourceState(graphics), newState, 0);

	renderTarget->SetResourceState(graphics, newState);
}

void CommandList::SetResourceState(Graphics& graphics, ID3D12Resource* resource, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES newState, unsigned int targetSubresource) const
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	if (prevState == newState)
		return;

	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = resource;
	resourceBarrier.Transition.Subresource = targetSubresource;
	resourceBarrier.Transition.StateBefore = prevState;
	resourceBarrier.Transition.StateAfter = newState;

	THROW_INFO_ERROR(pCommandList->ResourceBarrier(1, &resourceBarrier));
}

void CommandList::SetRenderTarget(Graphics& graphics, RenderTarget* renderTarget, DepthStencilViewBase* depthStencilView)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	// binding render target to command list
	{
		// here we can set to bind arrays of rtv and dsv descriptors, for now we will just pass ptr to single descriptor
		const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetViewDescriptor = &renderTarget->GetDescriptor(graphics);

		const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilViewDescriptor = depthStencilView != nullptr ? &depthStencilView->GetDescriptor(graphics) : nullptr;

		THROW_INFO_ERROR(pCommandList->OMSetRenderTargets(1, pRenderTargetViewDescriptor, false, pDepthStencilViewDescriptor));
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

void CommandList::SetViewPort(Graphics& graphics, ViewPort* viewPort)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only graphics command list can set viewport", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->RSSetViewports(1, &viewPort->GetViewport()));
	THROW_INFO_ERROR(pCommandList->RSSetScissorRects(1, &viewPort->GetViewportRect()));
}

void CommandList::SetGraphicsRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics root signature", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootSignature(rootSignature->Get()));
}

void CommandList::SetGraphicsConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics constant buffers", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootConstantBufferView(target.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetDescriptorHeap(Graphics& graphics, DescriptorHeap* descriptorHeap)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Copy command lists cannot set descriptor heaps", m_type == D3D12_COMMAND_LIST_TYPE_COPY);

	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap->Get()};

	THROW_INFO_ERROR(pCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps));
}

void CommandList::SetGraphicsDescriptorTable(Graphics& graphics, DescriptorHeapBindable* descriptorHeapBindable)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics Descriptor Tables", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	TargetSlotAndShader& target = descriptorHeapBindable->GetTargets().front();

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootDescriptorTable(target.rootIndex, descriptorHeapBindable->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetGraphicsDescriptorTable(Graphics& graphics, Texture* texture, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics Descriptor Tables", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootDescriptorTable(target.rootIndex, texture->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetGraphicsDescriptorTable(Graphics& graphics, ShaderResourceViewBase* srv, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics Descriptor Tables", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRootDescriptorTable(target.rootIndex, srv->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetRootConstants(Graphics& graphics, RootSignatureConstants* constants, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Direct and Bundle command lists can set graphics constant buffers", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT && m_type != D3D12_COMMAND_LIST_TYPE_BUNDLE);

	THROW_INFO_ERROR(pCommandList->SetGraphicsRoot32BitConstants(target.rootIndex, constants->GetNumValues(), constants->GetDataPtr(), 0));
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
		renderTarget->GetDescriptor(graphics),
		clearColor,
		0,
		nullptr
	));
};

void CommandList::ClearDepthStencilView(Graphics& graphics, DepthStencilViewBase* depthStencilView)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Non-direct command list object", m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->ClearDepthStencilView(
		depthStencilView->GetDescriptor(graphics),
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

void CommandList::SetComputeConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute constant buffer view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootConstantBufferView(target.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetComputeDescriptorTable(Graphics& graphics, Texture* texture, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute descriptor table", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootDescriptorTable(target.rootIndex, texture->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetComputeDescriptorTable(Graphics& graphics, ShaderResourceViewBase* srv, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute descriptor table", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootDescriptorTable(target.rootIndex, srv->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetComputeDescriptorTable(Graphics& graphics, UnorderedAccessView* uav, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute descriptor table", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootDescriptorTable(target.rootIndex, uav->GetDescriptorHeapGPUHandle(graphics)));
}

void CommandList::SetComputeRootShaderResourceView(Graphics& graphics, ConstantBuffer* constBuffer, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute shader resource view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootShaderResourceView(target.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetComputeRootUnorderedAccessView(Graphics& graphics, ConstantBuffer* constBuffer, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute unordered access view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootUnorderedAccessView(target.rootIndex, constBuffer->GetGPUAddress(graphics)));
}

void CommandList::SetComputeRootConstantBufferView(Graphics& graphics, ConstantBuffer* constBuffer, TargetSlotAndShader target)
{
	THROW_OBJECT_STATE_ERROR_IF("Command list is not initialized", !m_initialized);
	THROW_OBJECT_STATE_ERROR_IF("Only Compute and Direct command lists can set compute constant buffer view", m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && m_type != D3D12_COMMAND_LIST_TYPE_DIRECT);

	THROW_INFO_ERROR(pCommandList->SetComputeRootConstantBufferView(target.rootIndex, constBuffer->GetGPUAddress(graphics)));
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

void CommandList::CopyBufferToTexture(Graphics& graphics, ID3D12Resource* dstResource, ResourceFootprint& dstFootprint, ID3D12Resource* srcResource, unsigned int MipMapLvel)
{
	THROW_OBJECT_STATE_ERROR_IF("Bundle command lists cannot copy resources", m_type == D3D12_COMMAND_LIST_TYPE_BUNDLE);

	D3D12_TEXTURE_COPY_LOCATION dstTexture;
	dstTexture.pResource = dstResource;
	dstTexture.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstTexture.SubresourceIndex = MipMapLvel;

	D3D12_TEXTURE_COPY_LOCATION srcTexture;
	srcTexture.pResource = srcResource;
	srcTexture.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcTexture.PlacedFootprint = dstFootprint.layout;

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

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE CommandList::GetBeginningOP(ResourceDataOperation op)
{
	switch (op)
	{
	case ResourceDataOperation::keep:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
	case ResourceDataOperation::clear:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
	case ResourceDataOperation::discard:
		return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
	default:
		THROW_INTERNAL_ERROR("Failed to map resource operation to DX counterpart");
	}
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE CommandList::GetEndingOP(ResourceDataOperation op)
{
	switch (op)
	{
	case ResourceDataOperation::keep:
		return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
	case ResourceDataOperation::discard:
		return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
	default:
		THROW_INTERNAL_ERROR("Failed to map resource operation to DX counterpart");
	}
}

D3D12_RENDER_PASS_BEGINNING_ACCESS CommandList::CreateBeginningAccess(ResourceDataOperation op, auto* resource)
{
	if (op == ResourceDataOperation::clear)
	{
		auto GetClearValue = [](auto* resource)
			{
				D3D12_CLEAR_VALUE clearValue = {};
				clearValue.Format = resource->GetFormat();

				using T = std::remove_pointer_t<decltype(resource)>;

				if constexpr (std::is_base_of_v<RenderTarget, T>)
				{
					RenderTargetClearValue rtcv = resource->GetClearValue();

					clearValue.Color[0] = rtcv.x;
					clearValue.Color[1] = rtcv.y;
					clearValue.Color[2] = rtcv.z;
					clearValue.Color[3] = rtcv.w;
				}
				else if constexpr (std::is_base_of_v<DepthStencilViewBase, T>)
				{
					DepthStencilClearValue dscv = resource->GetClearValue();

					clearValue.DepthStencil = {
						.Depth = dscv.depth,
						.Stencil = dscv.stencil
					};
				}
				else
				{
					static_assert(false, "unsupported resource type was passed");
				}

				return clearValue;
			};

		return D3D12_RENDER_PASS_BEGINNING_ACCESS{ .Type = GetBeginningOP(op), .Clear = {.ClearValue = GetClearValue(resource)} };
	}

	// if discard is passed, .PreserveLocal being initialized doesn't matter
	return D3D12_RENDER_PASS_BEGINNING_ACCESS{ .Type = GetBeginningOP(op), .PreserveLocal = {.AdditionalWidth = 0, .AdditionalHeight = 0} };
}

D3D12_RENDER_PASS_ENDING_ACCESS CommandList::CreateEndingAccess(ResourceDataOperation op)
{
	return D3D12_RENDER_PASS_ENDING_ACCESS{ .Type = GetEndingOP(op), .PreserveLocal = {.AdditionalWidth = 0, .AdditionalHeight = 0} };
}