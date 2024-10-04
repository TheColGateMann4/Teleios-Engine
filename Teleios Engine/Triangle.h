#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "PipelineState.h"
#include "Shader.h"

class Triangle
{
public:
	Triangle(class Graphics& graphics);

private:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;

	std::shared_ptr<PipelineState> pPipelineState;
};