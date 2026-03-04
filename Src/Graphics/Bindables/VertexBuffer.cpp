#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Data/DynamicVertex.h"

#include "Graphics/Core/ResourceList.h"

VertexBuffer::VertexBuffer(const DynamicVertex::DynamicVertexLayout& layout)
	:
	m_layout(layout)
{
	m_layout.Finish();
	std::cout << m_layout.GetIdentifier() << "\n";
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetResource(const DynamicVertex::DynamicVertexLayout& layout)
{
	return ResourceList::GetResourceByID<VertexBuffer>("VertexBuffer#" + layout.GetIdentifier(), layout);
}

VertexBufferEntryInfo VertexBuffer::PushData(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
{
	THROW_INTERNAL_ERROR_IF("Passed vertex data layout didn't match with VertexBuffer layout", dynamicVertexBuffer.GetLayout().GetIdentifier() != m_layout.GetIdentifier());
	THROW_INTERNAL_ERROR_IF("DynamicVertexBuffer didn't have any vertices pushed", dynamicVertexBuffer.GetNumVertices() <= 0);

	unsigned int previouslyAccumulatedElements = m_accumulatedElements;
	unsigned int numElements = static_cast<unsigned int>(dynamicVertexBuffer.GetNumVertices());

	AddUploadBuffer(graphics, numElements, m_layout.GetSize(), dynamicVertexBuffer.GetData());

	return VertexBufferEntryInfo{
		previouslyAccumulatedElements,
		numElements
	};
}

VertexBufferEntryInfo VertexBuffer::PushData(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements)
{
	THROW_INTERNAL_ERROR_IF("Passed vertex data layout didn't match with VertexBuffer layout", layout.GetIdentifier() != m_layout.GetIdentifier());
	THROW_INTERNAL_ERROR_IF("Passed data was NULL", pData == nullptr);
	THROW_INTERNAL_ERROR_IF("Num elements passed to VertexBuffer was 0", numElements <= 0);

	unsigned int previouslyAccumulatedElements = m_accumulatedElements;

	AddUploadBuffer(graphics, numElements, m_layout.GetSize(), pData);

	return VertexBufferEntryInfo{
		previouslyAccumulatedElements,
		numElements
	};
}

void VertexBuffer::Build(Graphics& graphics)
{
	// Yes this is dirty, we have to move vertexBuffers to be owned by pipeline, special owning place just for VertexBuffers. 
	// Very important since we will need a lot of managing with them
	if (m_buffer) 
		return;

	unsigned int dataStride = m_layout.GetSize();
	unsigned int numElements = m_accumulatedElements;

	m_buffer = std::make_shared<GraphicsBuffer>(graphics, numElements, dataStride, GraphicsBuffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	m_vertexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = numElements * dataStride;
	m_vertexBufferView.StrideInBytes = dataStride;
}

void VertexBuffer::BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline)
{
	Build(graphics);

	if (m_toUpload.empty()) // if uploadResource ptr is not holding value then we don't want to update
		return;

	for(auto& upload : m_toUpload)
	{
		unsigned int srcSize = upload.uploadBuffer->GetByteSize();

		pipeline.AddBufferRegionToCopyPipeline(DestinationBufferRegionCopyData(m_buffer.get(), upload.offset), SourceBufferRegionCopyData(upload.uploadBuffer.get(), 0, srcSize));

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(upload.uploadBuffer));
	}

	m_toUpload.clear();
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
	return m_buffer.get();
}

const DynamicVertex::DynamicVertexLayout& VertexBuffer::GetLayout() const
{
	return m_layout;
}

void VertexBuffer::AddUploadBuffer(Graphics& graphics, unsigned int numElements, unsigned int dataStride, void* pData)
{
	std::unique_ptr<GraphicsBuffer> uploadBuffer = std::make_unique<GraphicsBuffer>(graphics, numElements, dataStride, GraphicsResource::CPUAccess::write);

	unsigned int byteStride = m_layout.GetSize();

	m_toUpload.emplace_back(m_accumulatedElements * byteStride, std::move(uploadBuffer));

	UpdateBufferData(graphics, pData);

	m_accumulatedElements += numElements;
}

void VertexBuffer::UpdateBufferData(Graphics& graphics, void* pData)
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

const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::Get() const
{
	return &m_vertexBufferView;
}

VertexBufferEntry::VertexBufferEntry(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements)
{
	m_vertexBuffer = ResourceList::GetResourceByID<VertexBuffer>("VertexBuffer#" + layout.GetIdentifier(), layout);

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

VertexBufferEntryInfo VertexBufferEntry::GetEntryInfo() const
{
	return m_entryInfo;
}