#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Bindable.h"
#include "Graphics/Resources/GraphicsBuffer.h"
#include "Graphics/Core/GraphicsBufferAllocatorManager.h"

class Pipeline;
class Graphics;
class CommandList;

class IndexBuffer : public Bindable, public CommandListBindable, public BufferAllocatorUpdateListener
{
public:
    IndexBuffer(Graphics& graphics, unsigned int stride);
    virtual ~IndexBuffer() override;

    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    static std::shared_ptr<IndexBuffer> GetResource(Graphics& graphics, unsigned int stride);

public:
    std::shared_ptr<BufferAllocatorChunk> PushData(Graphics& graphics, std::vector<unsigned int>&& indices);
    std::shared_ptr<BufferAllocatorChunk> PushData(Graphics& graphics, std::vector<unsigned short>&& indices);
    std::shared_ptr<BufferAllocatorChunk> PushData(Graphics& graphics, void* pData, unsigned int indexCount, unsigned int stride);

    virtual void UpdateCallback() override;

public:
    GraphicsBuffer* GetBuffer();

public:
	void BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline);

    virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

    virtual BindableType GetBindableType() const override;

    const D3D12_INDEX_BUFFER_VIEW* Get() const;

    DXGI_FORMAT GetFormat() const;

private:
    std::shared_ptr<GraphicsBufferSuballocator> m_buffer;

    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
    DXGI_FORMAT m_dataFormat;
    unsigned int m_stride;
    size_t m_accumulatedElements = 0;
};

class IndexBufferEntry : public Bindable, public CommandListBindable
{
public:
    IndexBufferEntry(Graphics& graphics, std::vector<unsigned int>&& indices);

    IndexBufferEntry(Graphics& graphics, std::vector<unsigned short>&& indices);

    static std::shared_ptr<IndexBufferEntry> GetResource(Graphics& graphics, const std::string& identifier, std::vector<unsigned int>&& indices);

    static std::shared_ptr<IndexBufferEntry> GetResource(Graphics& graphics, const std::string& identifier, std::vector<unsigned short>&& indices);

public:
    virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

    virtual BindableType GetBindableType() const override;

    IndexBuffer* GetIndexBuffer() const;

    BufferAllocatorChunk* GetEntryInfo() const;

    unsigned int GetIndexCount() const;

    unsigned int GetStride() const;

private:
    std::shared_ptr<IndexBuffer> m_indexBuffer;
    std::shared_ptr<BufferAllocatorChunk> m_entryInfo;
    unsigned int m_indexCount;
    unsigned int m_stride;
};