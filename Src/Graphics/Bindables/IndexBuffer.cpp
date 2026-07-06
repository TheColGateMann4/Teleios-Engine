#include "IndexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Core/ResourceList.h"

IndexBuffer::IndexBuffer(Graphics& graphics, unsigned int stride)
    :
	m_dataFormat(stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT),
	m_stride(stride)
{
	m_buffer = graphics.GetGraphicsBufferAllocatorManager()->RequestBufferAllocator(graphics, 0, stride);
	m_buffer->RegisterForUpdates(this);

	THROW_INTERNAL_ERROR_IF("Stride was invalid", stride != 2 && stride != 4);
}

IndexBuffer::~IndexBuffer()
{
	m_buffer->UnregisterFromUpdates(this);
}

std::shared_ptr<IndexBuffer> IndexBuffer::GetResource(Graphics& graphics, unsigned int stride)
{
	return ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + std::to_string(stride), graphics, stride);
}

std::shared_ptr<BufferAllocatorChunk> IndexBuffer::PushData(Graphics& graphics, std::vector<unsigned int>&& indices)
{
	return PushData(graphics, indices.data(), indices.size(), sizeof(indices.front()));
}

std::shared_ptr<BufferAllocatorChunk> IndexBuffer::PushData(Graphics& graphics, std::vector<unsigned short>&& indices)
{
	return PushData(graphics, indices.data(), indices.size(), sizeof(indices.front()));
}

std::shared_ptr<BufferAllocatorChunk> IndexBuffer::PushData(Graphics& graphics, void* pData, unsigned int indexCount, unsigned int stride)
{
	THROW_INTERNAL_ERROR_IF("Passed data was NULL", pData == nullptr);
	THROW_INTERNAL_ERROR_IF("Num elements passed to IndexBuffer was 0", indexCount == 0);
	THROW_INTERNAL_ERROR_IF("Tried to push indice data with different stride than target buffer", stride != m_stride);

	return m_buffer->Push(graphics, pData, indexCount * stride, stride);
}

void IndexBuffer::UpdateCallback()
{
	m_indexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUAddress();
	m_indexBufferView.SizeInBytes = m_buffer->GetResource()->GetByteSize();
	m_indexBufferView.Format = m_dataFormat;
}

GraphicsBuffer* IndexBuffer::GetBuffer()
{
	return m_buffer->GetResource();
}

void IndexBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetIndexBuffer(graphics, this);
}

BindableType IndexBuffer::GetBindableType() const
{
	return BindableType::bindable_indexBuffer;
}

const D3D12_INDEX_BUFFER_VIEW* IndexBuffer::Get() const
{
    return &m_indexBufferView;
}

DXGI_FORMAT IndexBuffer::GetFormat() const
{
	return m_dataFormat;
}

IndexBufferEntry::IndexBufferEntry(Graphics& graphics, std::vector<unsigned int>&& indices)
{
	m_indexBuffer = ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + std::to_string(sizeof(indices.front())), graphics, sizeof(indices.front()));
	m_indexCount = indices.size();
	m_stride = sizeof(indices.front());

	m_entryInfo = std::move(m_indexBuffer->PushData(graphics, std::move(indices)));
}

IndexBufferEntry::IndexBufferEntry(Graphics& graphics, std::vector<unsigned short>&& indices)
{
	m_indexBuffer = ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + std::to_string(sizeof(indices.front())), graphics, sizeof(indices.front()));
	m_indexCount = indices.size();
	m_stride = sizeof(indices.front());

	m_entryInfo = std::move(m_indexBuffer->PushData(graphics, std::move(indices)));
}

std::shared_ptr<IndexBufferEntry> IndexBufferEntry::GetResource(Graphics& graphics, const std::string& identifier, std::vector<unsigned int>&& indices)
{
	return ResourceList::GetResourceByID<IndexBufferEntry>(identifier, graphics, std::move(indices));
}

std::shared_ptr<IndexBufferEntry> IndexBufferEntry::GetResource(Graphics& graphics, const std::string& identifier, std::vector<unsigned short>&& indices)
{
	return ResourceList::GetResourceByID<IndexBufferEntry>(identifier, graphics, std::move(indices));
}

void IndexBufferEntry::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetIndexBuffer(graphics, m_indexBuffer.get());
}

BindableType IndexBufferEntry::GetBindableType() const
{
	return BindableType::bindable_indexBufferEntry;
}

IndexBuffer* IndexBufferEntry::GetIndexBuffer() const
{
	return m_indexBuffer.get();
}

BufferAllocatorChunk* IndexBufferEntry::GetEntryInfo() const
{
	return m_entryInfo.get();
}

unsigned int IndexBufferEntry::GetIndexCount() const
{
	return m_indexCount;
}

unsigned int IndexBufferEntry::GetStride() const
{
	return m_stride;
}