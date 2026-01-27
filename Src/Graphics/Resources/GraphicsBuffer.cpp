#include "GraphicsBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Core/CommandList.h"

GraphicsBuffer::GraphicsBuffer(Graphics& graphics, unsigned int numElements, unsigned int byteStride, CPUAccess cpuAccess, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flags)
	:
	GraphicsResource(DXGI_FORMAT_UNKNOWN, cpuAccess, state),
	m_byteSize(numElements * byteStride),
	m_byteStride(byteStride),
	m_numElements(numElements)
{
	HRESULT hr;
	unsigned int numberOfBuffers = graphics.GetBufferCount();
	
	// creating resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = GetHardwareHeapType(cpuAccess);
		heapPropeties.CPUPageProperty = GetHardwareHeapUsagePropety(cpuAccess);
		heapPropeties.MemoryPoolPreference = GetHardwareHeapMemoryPool(cpuAccess);
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_byteSize;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = m_format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = flags; 

		// this is very incorrect practice since creating many different commited resources for one purpose is bad practice. It is only temporary solution
		THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_pResource)
		));
	}
}

void GraphicsBuffer::Update(Graphics& graphics, const void* data, size_t size)
{
	Update(graphics, data, size, 1, size);
}

void GraphicsBuffer::Update(Graphics& graphics, const void* data, size_t rowSize, size_t rows, size_t rowPitch)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, rowSize, rows, rowPitch);
	}
	else
	{
		THROW_INTERNAL_ERROR("Tried to Update resource without CPU access. Use pipeline access to update it using temp resource");
	}
}

void GraphicsBuffer::Update(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, size, 1, size);
	}
	else
	{
		UpdateUsingTempResource(graphics, pipeline, data, size);
	}
}

size_t GraphicsBuffer::GetByteSize() const
{
	return m_byteSize;
}

size_t GraphicsBuffer::GetNumElements() const
{
	return m_numElements;
}

size_t GraphicsBuffer::GetByteStride() const
{
	return m_byteStride;
}

void GraphicsBuffer::UpdateUsingTempResource(Graphics& graphics, Pipeline& pipeline, const void* data, size_t size)
{
	std::shared_ptr<GraphicsBuffer> uploadBuffer = std::make_shared<GraphicsBuffer>(graphics, size, 1, CPUAccess::write);
	uploadBuffer->Update(graphics, data, size);

	uploadBuffer->CopyResourcesTo(graphics, pipeline.GetGraphicCommandList(), this);

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(uploadBuffer));
}
void GraphicsBuffer::UpdateLocalResource(Graphics& graphics, const void* data, size_t rowSize, size_t rows, size_t rowPitch)
{
	THROW_INTERNAL_ERROR_IF("GraphicsBuffer was larger than resource itself", rowSize > m_byteSize);

	ID3D12Resource* pConstBuffer = GetResource();

	HRESULT hr;

	// passing data to constant buffer resource
	{
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;

		D3D12_RANGE writeRange = {};
		writeRange.Begin = 0;
		writeRange.End = rowSize * rows;

		unsigned char* pMappedData = nullptr;
		const unsigned char* pData = static_cast<const unsigned char*>(data);

		THROW_ERROR(pConstBuffer->Map(
			0,
			&readRange,
			reinterpret_cast<void**>(&pMappedData)
		));

		for(int row = 0; row < rows; row++)
		{
			memcpy_s(
				pMappedData + row * rowSize,
				rowSize,
				pData + row * rowPitch,
				rowSize
			);
		}

		pConstBuffer->Unmap(0, &writeRange);
	}
}