#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "BindableContainer.h"

class Graphics;
class PipelineState;

class RenderTarget;
class DepthStencilView;
class IndexBuffer;
class VertexBuffer;
class RootSignature;
class ConstantBuffer;
class Texture;
class DescriptorHeap;
class UnorderedAccessView;
class TextureMipView;

struct ID3D12Resource;

class CommandList
{
public:
	CommandList(Graphics& graphics, D3D12_COMMAND_LIST_TYPE type, PipelineState* pPipelineState = nullptr);

public:
	// this function is used to open CommandList, it also resets local allocator
	void Open(Graphics& graphics, PipelineState* pPipelineState = nullptr);

	void Close(Graphics& graphics);

	void DrawIndexed(Graphics& graphics, unsigned int indices);

	void Dispatch(Graphics& graphics, unsigned int workToProcessX = 1, unsigned int workToProcessY = 1, unsigned int workToProcessZ = 1);
	
	ID3D12GraphicsCommandList* Get();

	bool IsOpen() const;

public:
	void SetResourceState(Graphics& graphics, RenderTarget* renderTarget, D3D12_RESOURCE_STATES newState) const;

	void SetResourceState(Graphics& graphics, ID3D12Resource* resource, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES newState, unsigned int targetSubresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) const;

	void SetVertexBuffer(Graphics& graphics, VertexBuffer* vertexBuffer);

	void SetIndexBuffer(Graphics& graphics, IndexBuffer* indexBuffer);

	void SetPrimitiveTopology(Graphics& graphics, D3D_PRIMITIVE_TOPOLOGY primitiveTechnology);

	void SetDescriptorHeap(Graphics& graphics, DescriptorHeap* descriptorHeap);

	void SetRenderTarget(Graphics& graphics, RenderTarget* renderTarget, DepthStencilView* depthStencilView = nullptr);

	void SetPipelineState(Graphics& graphics, PipelineState* pPipelineState);

	// graphics
	void SetGraphicsRootSignature(Graphics& graphics, RootSignature* rootSignature);

	void SetGraphicsConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetGraphicsRootUnorderedAccessView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetGraphicsRootShaderResourceView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetGraphicsDescriptorTable(Graphics& graphics, Texture* texture);

	void ClearRenderTargetView(Graphics& graphics, RenderTarget* renderTarget);

	void ClearDepthStencilView(Graphics& graphics, DepthStencilView* depthStencilView);

	void ExecuteBundle(Graphics& graphics, CommandList* commandList);

	// compute
	void SetComputeRootSignature(Graphics& graphics, RootSignature* rootSignature);

	void SetComputeConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetComputeDescriptorTable(Graphics& graphics, Texture* texture);
	void SetComputeDescriptorTable(Graphics& graphics, UnorderedAccessView* uav);
	void SetComputeDescriptorTable(Graphics& graphics, TextureMipView* srv);

	void SetComputeRootShaderResourceView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetComputeRootUnorderedAccessView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetComputeRootConstantBufferView(Graphics& graphics, ConstantBuffer* constBuffer);

	// copy
	void CopyBufferRegion(Graphics& graphics, ID3D12Resource* dstResource, UINT64 dstOffset, ID3D12Resource* srcResource, UINT64 srcOffset, UINT64 numBytes);

	void CopyTextureRegion(Graphics& graphics, ID3D12Resource* dstResource, ID3D12Resource* srcResource, unsigned int MipMapLvel);

	void CopyResource(Graphics& graphics, ID3D12Resource* dstResource, ID3D12Resource* srcResource);

private:
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_pCommandAllocators;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> pCommandList;
	D3D12_COMMAND_LIST_TYPE m_type;
	bool m_initialized;
	bool m_open;
	unsigned int m_currCommandAllocatorIndex;
};