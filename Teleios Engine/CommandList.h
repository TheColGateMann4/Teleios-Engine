#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;
class RenderTarget;
class DepthStencilView;
class IndexBuffer;
class VertexBuffer;

class CommandList
{
public:
	CommandList(Graphics& graphics, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pPipelineState = nullptr);

public:
	void Open(Graphics& graphics, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pPipelineState = nullptr);

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

	void ExecuteBundle(Graphics& graphics, CommandList* commandList);


private:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList;
	D3D12_COMMAND_LIST_TYPE m_type;
	bool m_initialized;
	bool m_closed;
};

