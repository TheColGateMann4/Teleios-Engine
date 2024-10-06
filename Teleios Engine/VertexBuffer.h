#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class VertexBuffer
{
public:
	// data has to be aligned in 16 bytes
	VertexBuffer(Graphics& graphics, void* pData, size_t dataSize, size_t dataStride);

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
};

