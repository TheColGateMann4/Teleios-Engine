#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "Bindable.h"

class Graphics;
class CommandList;

class IndexBuffer : public Bindable, public CommandListBindable
{
private:
    IndexBuffer(Graphics& graphics, void* pData, size_t indexCount, DXGI_FORMAT dataFormat);

public:
    IndexBuffer(Graphics& graphics, std::vector<unsigned int> indices);

    IndexBuffer(Graphics& graphics, std::vector<unsigned short> indices);

public:
    virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

    const D3D12_INDEX_BUFFER_VIEW* Get() const;

    size_t GetIndexCount() const;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    size_t m_indexCount;
};