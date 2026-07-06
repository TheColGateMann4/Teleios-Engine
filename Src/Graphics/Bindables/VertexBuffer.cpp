#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Data/DynamicVertex.h"

#include "Graphics/Core/ResourceList.h"

VertexBuffer::VertexBuffer(Graphics& graphics, const DynamicVertex::DynamicVertexLayout& layout)
	:
	m_layout(layout)
{
	m_layout.Finish();

	m_buffer = graphics.GetGraphicsBufferAllocatorManager()->RequestBufferAllocator(graphics, 0, layout.GetSize());
	m_buffer->RegisterForUpdates(this);
}

VertexBuffer::~VertexBuffer()
{
	m_buffer->UnregisterFromUpdates(this);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetResource(Graphics& graphics, const DynamicVertex::DynamicVertexLayout& layout)
{
	return ResourceList::GetResourceByID<VertexBuffer>("VertexBuffer#" + layout.GetIdentifier(), graphics, layout);
}

std::shared_ptr<BufferAllocatorChunk> VertexBuffer::PushData(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
{
	THROW_INTERNAL_ERROR_IF("Passed vertex data layout didn't match with VertexBuffer layout", dynamicVertexBuffer.GetLayout().GetIdentifier() != m_layout.GetIdentifier());
	THROW_INTERNAL_ERROR_IF("DynamicVertexBuffer didn't have any vertices pushed", dynamicVertexBuffer.GetNumVertices() <= 0);

	unsigned int numElements = static_cast<unsigned int>(dynamicVertexBuffer.GetNumVertices());
	unsigned int stride = m_layout.GetSize();

	return m_buffer->Push(graphics, dynamicVertexBuffer.GetData(), numElements * stride, stride);
}

std::shared_ptr<BufferAllocatorChunk> VertexBuffer::PushData(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements)
{
	THROW_INTERNAL_ERROR_IF("Passed vertex data layout didn't match with VertexBuffer layout", layout.GetIdentifier() != m_layout.GetIdentifier());
	THROW_INTERNAL_ERROR_IF("Passed data was NULL", pData == nullptr);
	THROW_INTERNAL_ERROR_IF("Num elements passed to VertexBuffer was 0", numElements <= 0);

	unsigned int stride = m_layout.GetSize();

	return m_buffer->Push(graphics, pData, numElements * stride, stride);
}

void VertexBuffer::UpdateCallback()
{
	m_vertexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUAddress();
	m_vertexBufferView.SizeInBytes = m_buffer->GetResource()->GetByteSize();
	m_vertexBufferView.StrideInBytes = m_layout.GetSize();
}

void VertexBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetVertexBuffer(graphics, this);
}

BindableType VertexBuffer::GetBindableType() const
{
	return BindableType::bindable_vertexBuffer;
}

GraphicsBuffer* VertexBuffer::GetBuffer()
{
	return m_buffer->GetResource();
}

const DynamicVertex::DynamicVertexLayout& VertexBuffer::GetLayout() const
{
	return m_layout;
}

const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::Get() const
{
	return &m_vertexBufferView;
}

VertexBufferEntry::VertexBufferEntry(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements)
{
	m_vertexBuffer = ResourceList::GetResourceByID<VertexBuffer>("VertexBuffer#" + layout.GetIdentifier(), graphics, layout);

	m_entryInfo = m_vertexBuffer->PushData(graphics, pData, layout, numElements);
}

VertexBufferEntry::VertexBufferEntry(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
	:
	VertexBufferEntry(graphics, dynamicVertexBuffer.GetData(), dynamicVertexBuffer.GetLayout(), dynamicVertexBuffer.GetNumVertices())
{

}

std::shared_ptr<VertexBufferEntry> VertexBufferEntry::GetResource(Graphics& graphics, const std::string& identifier, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements)
{
	return ResourceList::GetResourceByID<VertexBufferEntry>(identifier, graphics, pData, layout, numElements);
}

std::shared_ptr<VertexBufferEntry> VertexBufferEntry::GetResource(Graphics& graphics, const std::string& identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
{
	return ResourceList::GetResourceByID<VertexBufferEntry>(identifier, graphics, dynamicVertexBuffer);
}

void VertexBufferEntry::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetVertexBuffer(graphics, m_vertexBuffer.get());
}

BindableType VertexBufferEntry::GetBindableType() const
{
	return BindableType::bindable_vertexBufferEntry;
}

VertexBuffer* VertexBufferEntry::GetVertexBuffer() const
{
	return m_vertexBuffer.get();
}

BufferAllocatorChunk* VertexBufferEntry::GetEntryInfo() const
{
	return m_entryInfo.get();
}