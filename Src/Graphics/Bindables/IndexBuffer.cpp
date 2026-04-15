#include "IndexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Core/ResourceList.h"

IndexBuffer::IndexBuffer(unsigned int stride)
    :
	m_dataFormat(stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT),
	m_stride(stride)
{
	THROW_INTERNAL_ERROR_IF("Stride was invalid", stride != 2 && stride != 4);
}

std::shared_ptr<IndexBuffer> IndexBuffer::GetResource(unsigned int stride)
{
	return ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + std::to_string(stride), stride);
}

IndexBufferEntryInfo IndexBuffer::PushData(Graphics& graphics, std::vector<unsigned int>&& indices)
{
	return PushData(graphics, indices.data(), indices.size(), sizeof(indices.front()));
}

IndexBufferEntryInfo IndexBuffer::PushData(Graphics& graphics, std::vector<unsigned short>&& indices)
{
	return PushData(graphics, indices.data(), indices.size(), sizeof(indices.front()));
}

IndexBufferEntryInfo IndexBuffer::PushData(Graphics& graphics, void* pData, unsigned int indexCount, unsigned int stride)
{
	THROW_INTERNAL_ERROR_IF("Passed data was NULL", pData == nullptr);
	THROW_INTERNAL_ERROR_IF("Num elements passed to IndexBuffer was 0", indexCount == 0);
	THROW_INTERNAL_ERROR_IF("Tried to push indice data with different stride than target buffer", stride != m_stride);

	unsigned int previouslyAccumulatedElements = m_accumulatedElements;

	AddUploadBuffer(graphics, indexCount, pData);

	return IndexBufferEntryInfo{
		previouslyAccumulatedElements,
		indexCount
	};
}

void IndexBuffer::Build(Graphics& graphics)
{
	if (m_buffer)
		return;

	m_buffer = std::make_shared<GraphicsBuffer>(graphics, m_accumulatedElements, m_stride, GraphicsBuffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	m_indexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = m_accumulatedElements * m_stride;
	m_indexBufferView.Format = m_dataFormat;
}

GraphicsBuffer* IndexBuffer::GetBuffer()
{
	return m_buffer.get();
}

void IndexBuffer::BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline)
{
	Build(graphics);

	if (m_toUpload.empty()) // if uploadResource ptr is not holding value then we don't want to update
		return;

	for (auto& upload : m_toUpload)
	{
		unsigned int srcSize = upload.uploadBuffer->GetByteSize();

		pipeline.AddBufferRegionToCopyPipeline(DestinationBufferRegionCopyData(m_buffer.get(), upload.offset), SourceBufferRegionCopyData(upload.uploadBuffer.get(), 0, srcSize));

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(upload.uploadBuffer));
	}

	m_toUpload.clear();
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

size_t IndexBuffer::GetIndexCount() const
{
    return m_buffer->GetNumElements();
}

DXGI_FORMAT IndexBuffer::GetFormat() const
{
	return m_dataFormat;
}

void IndexBuffer::AddUploadBuffer(Graphics& graphics, unsigned int numElements, void* pData)
{
	std::unique_ptr<GraphicsBuffer> uploadBuffer = std::make_unique<GraphicsBuffer>(graphics, numElements, m_stride, GraphicsResource::CPUAccess::write);

	m_toUpload.emplace_back(m_accumulatedElements * m_stride, std::move(uploadBuffer));

	UpdateBufferData(graphics, pData);

	m_accumulatedElements += numElements;
}

void IndexBuffer::UpdateBufferData(Graphics& graphics, void* pData)
{
	GraphicsBuffer* uploadBuffer = m_toUpload.back().uploadBuffer.get();

	HRESULT hr;

	D3D12_RANGE readRange = {};
	readRange.Begin = 0;
	readRange.End = 0;

	D3D12_RANGE writeRange = {};
	writeRange.Begin = 0;
	writeRange.End = uploadBuffer->GetByteSize();

	void* pMappedData = nullptr;

	THROW_ERROR(uploadBuffer->GetResource()->Map(
		0,
		&readRange,
		&pMappedData
	));

	memcpy_s(pMappedData, uploadBuffer->GetByteSize(), pData, uploadBuffer->GetByteSize());

	uploadBuffer->GetResource()->Unmap(0, &writeRange);
}

IndexBufferEntry::IndexBufferEntry(Graphics& graphics, std::vector<unsigned int>&& indices)
{
	m_indexBuffer = ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + std::to_string(sizeof(indices.front())), sizeof(indices.front()));
	m_indexCount = indices.size();

	m_entryInfo = m_indexBuffer->PushData(graphics, std::move(indices));
}

IndexBufferEntry::IndexBufferEntry(Graphics& graphics, std::vector<unsigned short>&& indices)
{
	m_indexBuffer = ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + std::to_string(sizeof(indices.front())), sizeof(indices.front()));
	m_indexCount = indices.size();

	m_entryInfo = m_indexBuffer->PushData(graphics, std::move(indices));
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

IndexBufferEntryInfo IndexBufferEntry::GetEntryInfo() const
{
	return m_entryInfo;
}

unsigned int IndexBufferEntry::GetIndexCount() const
{
	return m_indexCount;
}