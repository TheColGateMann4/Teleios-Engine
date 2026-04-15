#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Bindable.h"
#include "Graphics/Resources/GraphicsBuffer.h"

class Pipeline;
class Graphics;
class CommandList;

struct IndexBufferEntryInfo
{
    unsigned int offset;
    unsigned int size;
};

class IndexBuffer : public Bindable, public CommandListBindable
{
    struct IndexBufferUploadData
    {
        unsigned int offset;
        std::unique_ptr<GraphicsBuffer> uploadBuffer;
    };

public:
    IndexBuffer(unsigned int stride);

    IndexBuffer(IndexBuffer&&) noexcept = default;
    IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

    static std::shared_ptr<IndexBuffer> GetResource(unsigned int stride);

public:
    IndexBufferEntryInfo PushData(Graphics& graphics, std::vector<unsigned int>&& indices);
    IndexBufferEntryInfo PushData(Graphics& graphics, std::vector<unsigned short>&& indices);
    IndexBufferEntryInfo PushData(Graphics& graphics, void* pData, unsigned int indexCount, unsigned int stride);

    void Build(Graphics& graphics);

public:
    GraphicsBuffer* GetBuffer();

public:
	void BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline);

    virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

    virtual BindableType GetBindableType() const override;

    const D3D12_INDEX_BUFFER_VIEW* Get() const;

    size_t GetIndexCount() const;
    DXGI_FORMAT GetFormat() const;

private:
    void AddUploadBuffer(Graphics& graphics, unsigned int numElements, void* pData);
    void UpdateBufferData(Graphics& graphics, void* pData);

private:
    std::shared_ptr<GraphicsBuffer> m_buffer;
    std::vector<IndexBufferUploadData> m_toUpload;

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

    IndexBufferEntryInfo GetEntryInfo() const;

    unsigned int GetIndexCount() const;

private:
    std::shared_ptr<IndexBuffer> m_indexBuffer;
    IndexBufferEntryInfo m_entryInfo;
    unsigned int m_indexCount;
};