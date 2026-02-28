#include "IndexBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Core/ResourceList.h"

IndexBuffer::IndexBuffer(Graphics& graphics, std::vector<unsigned int> indices)
	:
	IndexBuffer(graphics, indices.data(), indices.size(), DXGI_FORMAT_R32_UINT)
{

}

IndexBuffer::IndexBuffer(Graphics& graphics, std::vector<unsigned short> indices)
    :
    IndexBuffer(graphics, indices.data(), indices.size(), DXGI_FORMAT_R16_UINT)
{

}

IndexBuffer::IndexBuffer(Graphics& graphics, void* pData, size_t indexCount, DXGI_FORMAT dataFormat)
	:
	m_dataFormat(dataFormat)
{
	THROW_OBJECT_STATE_ERROR_IF("Haven't handled other indice formats", m_dataFormat != DXGI_FORMAT_R32_UINT && m_dataFormat != DXGI_FORMAT_R16_UINT);

	uint8_t structureSize = m_dataFormat == DXGI_FORMAT_R32_UINT ? 4 : 2; // 4bytes for 32bits and 2bytes for 16bits

	m_buffer = std::make_shared<GraphicsBuffer>(graphics, indexCount, structureSize, GraphicsBuffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	m_uploadBuffer = std::make_shared<GraphicsBuffer>(graphics, indexCount, structureSize, GraphicsBuffer::CPUAccess::write);

	m_indexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = indexCount * structureSize;
	m_indexBufferView.Format = m_dataFormat;

	// passing data to index buffer resource
	if (pData != nullptr)
		UpdateBufferData(graphics, pData);
}

std::shared_ptr<IndexBuffer> IndexBuffer::GetResource(Graphics& graphics, std::string identifier, std::vector<unsigned int> indices)
{
	return ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + identifier, graphics, indices);
}

std::shared_ptr<IndexBuffer> IndexBuffer::GetResource(Graphics& graphics, std::string identifier, std::vector<unsigned short> indices)
{
	return ResourceList::GetResourceByID<IndexBuffer>("IndexBuffer#" + identifier, graphics, indices);
}

GraphicsBuffer* IndexBuffer::GetBuffer()
{
	return m_buffer.get();
}

void IndexBuffer::BindToCopyPipelineIfNeeded(Graphics& graphics, Pipeline& pipeline)
{
	if (!m_uploadBuffer) // if uploadResource ptr is not holding value then we don't want to update
		return;

	pipeline.AddBufferToCopyPipeline(m_buffer.get(), m_uploadBuffer.get());

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(m_uploadBuffer));
}

void IndexBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetIndexBuffer(graphics, this);
}

BindableType IndexBuffer::GetBindableType() const
{
	return BindableType::bindable_indexBuffer;
}

void IndexBuffer::Update(Graphics& graphics, void* pData, size_t numElements, size_t structureSize)
{
	// if sizes are not the same, we will be creating new GPU resource.
	if (m_buffer->GetByteSize() != numElements * structureSize)
	{
		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(m_buffer));
		m_buffer = std::make_shared<GraphicsBuffer>(graphics, numElements, structureSize, GraphicsBuffer::CPUAccess::unknown, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		m_indexBufferView.BufferLocation = m_buffer->GetResource()->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = numElements * structureSize;
		m_indexBufferView.Format = m_dataFormat;
	}

	m_uploadBuffer = std::make_shared<GraphicsBuffer>(graphics, numElements, structureSize, GraphicsBuffer::CPUAccess::write);
	UpdateBufferData(graphics, pData);
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

void IndexBuffer::UpdateBufferData(Graphics& graphics, void* pData)
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