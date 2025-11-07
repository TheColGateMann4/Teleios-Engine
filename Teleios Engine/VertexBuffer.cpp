#include "VertexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "CommandList.h"

#include "Pipeline.h"

#include "DynamicVertex.h"

#include "BindableResourceList.h"

VertexBuffer::VertexBuffer(Graphics& graphics, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
	:
	VertexBuffer(graphics, dynamicVertexBuffer.GetData(), dynamicVertexBuffer.GetNumVertices(), dynamicVertexBuffer.GetLayout().GetSize())
{

}

VertexBuffer::VertexBuffer(Graphics& graphics, void* pData, size_t numElements, size_t dataStride)
{
	m_buffer = std::make_shared<Buffer>(graphics, numElements * dataStride, DXGI_FORMAT_UNKNOWN, Buffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_uploadBuffer = std::make_shared<Buffer>(graphics, numElements * dataStride, DXGI_FORMAT_UNKNOWN, Buffer::CPUAccess::write);

	m_vertexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = numElements * dataStride;
	m_vertexBufferView.StrideInBytes = dataStride;

	// passing data to vetex buffer resource
	UpdateBufferData(graphics, pData);
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, DynamicVertex::DynamicVertex& dynamicVertexBuffer)
{
	return GetBindableResource(graphics, identifier, dynamicVertexBuffer.GetData(), dynamicVertexBuffer.GetNumVertices(), dynamicVertexBuffer.GetLayout().GetSize());
}

std::shared_ptr<VertexBuffer> VertexBuffer::GetBindableResource(Graphics& graphics, std::string identifier, void* pData, size_t numElements, size_t dataStride)
{
	return BindableResourceList::GetBindableResourceByID<VertexBuffer>(graphics, "VertexBuffer#" + identifier, pData, numElements, dataStride);
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
	if (m_buffer->GetSize() != numElements * dataStride)
	{
		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(m_buffer));
		m_buffer = std::make_shared<Buffer>(graphics, numElements * dataStride, DXGI_FORMAT_UNKNOWN, Buffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		m_vertexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
		m_vertexBufferView.SizeInBytes = numElements * dataStride;
		m_vertexBufferView.StrideInBytes = dataStride;
	}

	m_uploadBuffer = std::make_shared<Buffer>(graphics, numElements * dataStride, DXGI_FORMAT_UNKNOWN, Buffer::CPUAccess::write);
	UpdateBufferData(graphics, pData);
}

Buffer* VertexBuffer::GetBuffer()
{
	return m_buffer.get();
}

void VertexBuffer::UpdateBufferData(Graphics& graphics, void* pData)
{
	HRESULT hr;

	D3D12_RANGE readRange = {};
	readRange.Begin = 0;
	readRange.End = 0;

	D3D12_RANGE writeRange = {};
	writeRange.Begin = 0;
	writeRange.End = m_uploadBuffer->GetSize();

	void* pMappedData = nullptr;

	THROW_ERROR(m_uploadBuffer->GetResource()->Map(
		0,
		&readRange,
		&pMappedData
	));

	memcpy_s(pMappedData, m_uploadBuffer->GetSize(), pData, m_uploadBuffer->GetSize());

	m_uploadBuffer->GetResource()->Unmap(0, &writeRange);
}

const D3D12_VERTEX_BUFFER_VIEW* VertexBuffer::Get() const
{
	return &m_vertexBufferView;
}