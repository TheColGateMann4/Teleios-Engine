#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

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

class ID3D12Resource;

class CommandList
{
public:
	CommandList(Graphics& graphics, D3D12_COMMAND_LIST_TYPE type, PipelineState* pPipelineState = nullptr);

public:
	// this function is used to open CommandList, it also resets local allocator
	void Open(Graphics& graphics, PipelineState* pPipelineState = nullptr);

	void Close(Graphics& graphics);

	void DrawIndexed(Graphics& graphics, unsigned int indices);
	
	ID3D12GraphicsCommandList* Get();

	bool IsOpen() const;

public:
	void SetResourceState(Graphics& graphics, RenderTarget* renderTarget, D3D12_RESOURCE_STATES newState) const;
	
	void SetResourceState(Graphics& graphics, ID3D12Resource* resource, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES newState) const;
	
	void SetRenderTarget(Graphics& graphics, RenderTarget* renderTarget, DepthStencilView* depthStencilView = nullptr);

	void SetVertexBuffer(Graphics& graphics, VertexBuffer* vertexBuffer);

	void SetIndexBuffer(Graphics& graphics, IndexBuffer* indexBuffer);

	void SetPrimitiveTopology(Graphics& graphics, D3D_PRIMITIVE_TOPOLOGY primitiveTechnology);

	void SetGraphicsRootSignature(Graphics& graphics, RootSignature* rootSignature);

	void SetGraphicsConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetDescriptorHeap(Graphics& graphics, DescriptorHeap* descriptorHeap);

	void SetGraphicsDescriptorTable(Graphics& graphics, Texture* texture);

	void ExecuteBundle(Graphics& graphics, CommandList* commandList);

	void ClearRenderTargetView(Graphics& graphics, RenderTarget* renderTarget);

	void ClearDepthStencilView(Graphics& graphics, DepthStencilView* depthStencilView);

	void SetPipelineState(Graphics& graphics, PipelineState* pPipelineState);

	// compute
	void SetComputeRootSignature(Graphics& graphics, RootSignature* rootSignature);

	void SetComputeConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer);

	void SetComputeDescriptorTable(Graphics& graphics, Texture* texture);

	// copy
	void CopyBufferRegion(Graphics& graphics, ID3D12Resource* dstResource, UINT64 dstOffset, ID3D12Resource* srcResource, UINT64 srcOffset, UINT64 numBytes);

	void CopyResource(Graphics& graphics, ID3D12Resource* dstResource, ID3D12Resource* srcResource);

private:
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_pCommandAllocators;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> pCommandList;
	D3D12_COMMAND_LIST_TYPE m_type;
	bool m_initialized;
	bool m_open;
	unsigned int m_currCommandAllocatorIndex;
};