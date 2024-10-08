#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;
class RenderTarget;
class DepthStencilView;
class IndexBuffer;
class VertexBuffer;
class RootSignature;
class ConstantBuffer;

class CommandList
{
public:
	CommandList(Graphics& graphics, D3D12_COMMAND_LIST_TYPE type,ID3D12PipelineState* pPipelineState = nullptr);

public:
	// this function is used to open CommandList, it also resets local allocator
	void Open(Graphics& graphics, ID3D12PipelineState* pPipelineState = nullptr);

	void Close(Graphics& graphics);

	void DrawIndexed(Graphics& graphics, unsigned int indices);
	
	ID3D12GraphicsCommandList* Get();

public:
	void ResourceBarrier(Graphics& graphics, RenderTarget* renderTarget, D3D12_RESOURCE_STATES previousState, D3D12_RESOURCE_STATES afterState) const;
	
	void SetRenderTarget(Graphics& graphics, RenderTarget* renderTarget, DepthStencilView* depthStencilView = nullptr);

	void SetVertexBuffer(Graphics& graphics, VertexBuffer* vertexBuffer);

	void SetIndexBuffer(Graphics& graphics, IndexBuffer* indexBuffer);

	void SetPrimitiveTopology(Graphics& graphics, D3D_PRIMITIVE_TOPOLOGY primitiveTechnology);

	void SetRootSignature(Graphics& graphics, RootSignature* rootSignature);

	void SetConstBufferView(Graphics& graphics, ConstantBuffer* constBuffer);

	void ExecuteBundle(Graphics& graphics, CommandList* commandList);

	void ClearRenderTargetView(Graphics& graphics, RenderTarget* renderTarget);

	void ClearDepthStencilView(Graphics& graphics, DepthStencilView* depthStencilView);

private:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
	D3D12_COMMAND_LIST_TYPE m_type;
	bool m_initialized;
	bool m_closed;
};

