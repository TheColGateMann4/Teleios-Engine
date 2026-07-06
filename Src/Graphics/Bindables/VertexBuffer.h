#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Bindable.h"
#include "Graphics/Resources/GraphicsBuffer.h"
#include "Graphics/Data/DynamicVertex.h"
#include "Graphics/Core/GraphicsBufferAllocatorManager.h"

class Pipeline;
class Graphics;
class CommandList;

struct VertexBufferEntryInfo
{
	unsigned int offset;
	unsigned int size;
};

class VertexBuffer : public Bindable, public CommandListBindable, public BufferAllocatorUpdateListener
{
public:
	VertexBuffer(Graphics& graphics, const DynamicVertex::DynamicVertexLayout& layout);
	virtual ~VertexBuffer() override;

	VertexBuffer(VertexBuffer&&) noexcept = default;
	VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

	static std::shared_ptr<VertexBuffer> GetResource(Graphics& graphics, const DynamicVertex::DynamicVertexLayout& layout);

public:
	// data has to be aligned in 16 bytes
	std::shared_ptr<BufferAllocatorChunk> PushData(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer);
	std::shared_ptr<BufferAllocatorChunk> PushData(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);

	virtual void UpdateCallback() override;

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

	GraphicsBuffer* GetBuffer();

	const DynamicVertex::DynamicVertexLayout& GetLayout() const;

public:
	const D3D12_VERTEX_BUFFER_VIEW* Get() const;

private:
	std::shared_ptr<GraphicsBufferSuballocator> m_buffer;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	DynamicVertex::DynamicVertexLayout m_layout;
};

class VertexBufferEntry : public Bindable, public CommandListBindable
{
public:
	VertexBufferEntry(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);

	VertexBufferEntry(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

	static std::shared_ptr<VertexBufferEntry> GetResource(Graphics& graphics, const std::string& identifier, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements);

	static std::shared_ptr<VertexBufferEntry> GetResource(Graphics& graphics, const std::string& identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer);

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

	VertexBuffer* GetVertexBuffer() const;

	BufferAllocatorChunk* GetEntryInfo() const;

private:
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<BufferAllocatorChunk> m_entryInfo;
};