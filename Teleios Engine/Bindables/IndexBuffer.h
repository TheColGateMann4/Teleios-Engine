#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "Bindable.h"
#include "GraphicsResources/GraphicsBuffer.h"

class Pipeline;
class Graphics;
class CommandList;

class IndexBuffer : public Bindable, public CommandListBindable
{
public:
    IndexBuffer(Graphics& graphics, std::vector<unsigned int> indices);

    IndexBuffer(Graphics& graphics, std::vector<unsigned short> indices);

    IndexBuffer(Graphics& graphics, void* pData, size_t indexCount, DXGI_FORMAT dataFormat);

    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

public:
    static std::shared_ptr<IndexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, std::vector<unsigned int> indices);

    static std::shared_ptr<IndexBuffer> GetBindableResource(Graphics& graphics, std::string identifier, std::vector<unsigned short> indices);

    GraphicsBuffer* GetBuffer();

public:
	void BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline);

    virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

    void Update(Graphics& graphics, void* pData, size_t numElements, size_t structureSize);

    const D3D12_INDEX_BUFFER_VIEW* Get() const;

    size_t GetIndexCount() const;
    DXGI_FORMAT GetFormat() const;

private:
    void UpdateBufferData(Graphics& graphics, void* pData);

private:
    std::shared_ptr<GraphicsBuffer> m_buffer;
    std::shared_ptr<GraphicsBuffer> m_uploadBuffer;

    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
    DXGI_FORMAT m_dataFormat;
};