#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;

class IndexBuffer
{
public:
    IndexBuffer(Graphics& graphics, std::vector<unsigned int> indices);

public:
    const D3D12_INDEX_BUFFER_VIEW* Get() const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
};