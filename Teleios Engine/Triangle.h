#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "PipelineState.h"
#include "CommandList.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Graphics;

class Triangle
{
public:
	Triangle(Graphics& graphics);

public:
	void Draw(Graphics& graphics) const;

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;

	std::unique_ptr<PipelineState> m_pipelineState;
	std::unique_ptr<CommandList> m_commandList;

	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
};