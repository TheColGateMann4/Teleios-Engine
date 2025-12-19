#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Data/DynamicVertex.h"

#include "Graphics/Core/BindableResourceList.h"

VertexBuffer::VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	m_buffer = std::make_shared<GraphicsBuffer>(graphics, numElements, dataStride, GraphicsBuffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	m_uploadBuffer = std::make_shared<GraphicsBuffer>(graphics, numElements, dataStride, GraphicsBuffer::CPUAccess::write);

	m_vertexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = numElements * dataStride;
	m_vertexBufferView.StrideInBytes = dataStride;

	// passing data to vetex buffer resource
	if (pData != nullptr)
		UpdateBufferData(graphics, pData);
}

VertexBuffer::VertexBuffer(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
	:
	VertexBuffer(graphics, dynamicVertexBuffer.GetData(), dynamicVertexBuffer.GetNumVertices(), dynamicVertexBuffer.GetLayout().GetSize())
{
	m_layout = dynamicVertexBuffer.GetLayout();
}

VertexBuffer::VertexBuffer(Graphics& graphics, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements)
	:
	VertexBuffer(graphics, nullptr, numElements, layout.GetSize())
{
	m_layout = layout;
}

VertexBuffer::VertexBuffer(Graphics& graphics, void* pData, const DynamicVertex::DynamicVertexLayout& layout, unsigned int numElements)
	:
	VertexBuffer(graphics, pData, numElements, layout.GetSize())
{
	m_layout = layout;
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, void* pData, size_t numElements, size_t dataStride)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>(graphics, "VertexBuffer#" + identifier, pData, numElements, dataStride);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>(graphics, "VertexBuffer#" + identifier, dynamicVertexBuffer);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, const DynamicVertex::DynamicVertexLayout& layout, size_t numElements)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>(graphics, "VertexBuffer#" + identifier, layout, numElements);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, void* pData, const DynamicVertex::DynamicVertexLayout& layout, size_t numElements)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>(graphics, "VertexBuffer#" + identifier, pData, layout, numElements);
}

void VertexBuffer::BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline)
{
	if (!m_uploadBuffer) // if uploadResource ptr is not holding value then we don't want to update
		return;

	pipeline.AddBufferToCopyPipeline(m_buffer.get(), m_uploadBuffer.get());

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(m_uploadBuffer));
}

void VertexBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetVertexBuffer(graphics, this);
}

void VertexBuffer::Update(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	// if sizes are not the same, we will be creating new GPU resource.
	if (m_buffer->GetByteSize() != numElements * dataStride)
	{
		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(m_buffer));
		m_buffer = std::make_shared<GraphicsBuffer>(graphics, numElements, dataStride, GraphicsBuffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		m_vertexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = numElements * dataStride;
		m_vertexBufferView.StrideInBytes = dataStride;
	}

	m_uploadBuffer = std::make_shared<GraphicsBuffer>(graphics, numElements, dataStride, GraphicsBuffer::CPUAccess::write);
	UpdateBufferData(graphics, pData);
}

GraphicsBuffer* VertexBuffer::GetBuffer()
{
	return m_buffer.get();
}

const DynamicVertex::DynamicVertexLayout& VertexBuffer::GetLayout() const
{
	return m_layout;
}

void VertexBuffer::UpdateBufferData(Graphics& graphics, void* pData)
{
	HRESULT hr;

	D3D12_RANGE readRange = {};
	readRange.Begin = 0;
	readRange.End = 0;

	D3D12_RANGE writeRange = {};
	writeRange.Begin = 0;
	writeRange.End = m_uploadBuffer->GetByteSize();

	void* pMappedData = nullptr;

	THROW_ERROR(m_uploadBuffer->GetResource()->Map(
		0,
		&readRange,
		&pMappedData
	));

	memcpy_s(pMappedData, m_uploadBuffer->GetByteSize(), pData, m_uploadBuffer->GetByteSize());

	m_uploadBuffer->GetResource()->Unmap(0, &writeRange);
}

const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::Get() const
{
	return &m_vertexBufferView;
}