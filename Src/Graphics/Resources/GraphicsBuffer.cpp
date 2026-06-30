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

void GraphicsBuffer::CopyResourcesToTexture(Graphics& graphics, CommandList* copyCommandList, GraphicsResource* dst, int targetMip)
{
	THROW_INTERNAL_ERROR_IF("Dest resource was NULL", dst == nullptr);

	BEGIN_COMMAND_LIST_EVENT(copyCommandList, "Copying GraphicsBuffer to GraphicsTexture");

	copyCommandList->SetResourceState(graphics, this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	copyCommandList->SetResourceState(graphics, dst, D3D12_RESOURCE_STATE_COPY_DEST, targetMip);

	ResourceFootprint dstFootprint = dst->GetResourceFootprint(graphics, targetMip);

	copyCommandList->CopyBufferToTexture(graphics, dst->GetResource(), dstFootprint, this->GetResource(), targetMip);

	copyCommandList->SetResourceState(graphics, dst, dst->GetResourceTargetState(), targetMip);
	copyCommandList->SetResourceState(graphics, this, this->GetResourceTargetState());

	END_COMMAND_LIST_EVENT(copyCommandList);
}

void GraphicsBuffer::CopyPartiallyTo(Graphics& graphics, CommandList* copyCommandList, size_t srcOffset, size_t srcSize, GraphicsResource* dst, size_t dstOffset)
{
	THROW_INTERNAL_ERROR_IF("Dest resource was NULL", dst == nullptr);

	BEGIN_COMMAND_LIST_EVENT(copyCommandList, "Copying GraphicsBuffer partially to GraphicsBuffer");

	copyCommandList->SetResourceState(graphics, this, D3D12_RESOURCE_STATE_COPY_SOURCE);
	copyCommandList->SetResourceState(graphics, dst, D3D12_RESOURCE_STATE_COPY_DEST);

	copyCommandList->CopyBufferRegion(graphics, dst->GetResource(), dstOffset, this->GetResource(), srcOffset, srcSize);

	copyCommandList->SetResourceState(graphics, dst, dst->GetResourceTargetState());
	copyCommandList->SetResourceState(graphics, this, this->GetResourceTargetState());

	END_COMMAND_LIST_EVENT(copyCommandList);
}

GraphicsResourceType GraphicsBuffer::GetResourceType()
{
	return GraphicsResourceType::buffer;
}

void GraphicsBuffer::Read(Graphics& graphics, void* data, unsigned int size, unsigned int offset)
{
	THROW_INTERNAL_ERROR_IF("Tried to read resource without cpu access", m_cpuAccess != CPUAccess::readwrite);
	THROW_INTERNAL_ERROR_IF("Tried to read out of buffer", m_byteSize < offset + size);

	HRESULT hr;
	ID3D12Resource* pConstBuffer = GetResource();

	{
		unsigned char* pMappedData = nullptr;

		THROW_ERROR(pConstBuffer->Map(
			0,
			nullptr,
			reinterpret_cast<void**>(&pMappedData)
		));

		memcpy_s(data, size, pMappedData + offset, size);

		pConstBuffer->Unmap(0, nullptr);
	}
}

void GraphicsBuffer::Update(Graphics& graphics, const void* data, size_t size, size_t offset)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, size, 1, size, size, offset);
	}
	else
	{
		UpdateUsingTempResource(graphics, data, size, offset);
	}
}

void GraphicsBuffer::Update(Graphics& graphics, const void* data, size_t rowSize, size_t rows, size_t dataRowPitch, size_t targetRowPitch, size_t offset)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, rowSize, rows, dataRowPitch, targetRowPitch, offset);
	}
	else
	{
		THROW_INTERNAL_ERROR("Tried to Update resource without CPU access. Use pipeline access to update it using temp resource");
	}
}

void* GraphicsBuffer::Map(Graphics& graphics, SIZE_T readStart, SIZE_T readEnd)
{
	THROW_INTERNAL_ERROR_IF("Tried to read out of buffer data", readStart > m_byteSize || readEnd > m_byteSize );
	THROW_INTERNAL_ERROR_IF("Tried to map buffer that doesn't have write CPU access", m_cpuAccess != CPUAccess::readwrite && m_cpuAccess != CPUAccess::write);
	THROW_INTERNAL_ERROR_IF("Tried to map and read buffer that doesn't have CPU read access", m_cpuAccess == CPUAccess::write && (readStart != 0 || readEnd != 0));
	THROW_INTERNAL_ERROR_IF("Tried to map already mapped buffer", m_mapped);

	HRESULT hr;

	D3D12_RANGE readRange = { .Begin = readStart , .End = readEnd };
	void* pMappedData = nullptr;
	THROW_ERROR(m_pResource->Map(0, &readRange, &pMappedData));

	m_mapped = true;

	return pMappedData;
}

void GraphicsBuffer::UnMap(SIZE_T writeStart, SIZE_T writeEnd)
{
	THROW_INTERNAL_ERROR_IF("Tried to write out of buffer data", writeStart > m_byteSize || writeEnd > m_byteSize);
	THROW_INTERNAL_ERROR_IF("Tried to unmap not mapped buffer", !m_mapped);

	D3D12_RANGE writeRange = { .Begin = writeStart , .End = writeEnd };
	m_pResource->Unmap(0, &writeRange);

	m_mapped = false;
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

void GraphicsBuffer::UpdateUsingTempResource(Graphics& graphics, const void* data, size_t size, size_t offset)
{
	std::shared_ptr<GraphicsBuffer> uploadBuffer = std::make_shared<GraphicsBuffer>(graphics, size, 1, CPUAccess::write);
	uploadBuffer->Update(graphics, data, size);

	if(offset != 0 || size != m_byteSize)
		uploadBuffer->CopyPartiallyTo(graphics, graphics.GetRenderer().GetPipeline().GetGraphicCommandList(), 0, size, this, offset);
	else
		uploadBuffer->CopyResourcesTo(graphics, graphics.GetRenderer().GetPipeline().GetGraphicCommandList(), this);

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(uploadBuffer));
}

void GraphicsBuffer::UpdateLocalResource(Graphics& graphics, const void* data, size_t rowSize, size_t rows, size_t dataRowPitch, size_t targetRowPitch, size_t offset)
{
	THROW_INTERNAL_ERROR_IF("GraphicsBuffer was larger than resource itself", targetRowPitch * rows + offset - (targetRowPitch - rowSize) > m_byteSize);

	ID3D12Resource* pConstBuffer = GetResource();

	HRESULT hr;

	// passing data to constant buffer resource
	{
		unsigned char* pMappedData = nullptr;
		const unsigned char* pData = static_cast<const unsigned char*>(data);

		THROW_ERROR(pConstBuffer->Map(
			0,
			nullptr,
			reinterpret_cast<void**>(&pMappedData)
		));

		for(int row = 0; row < rows; row++)
		{
			memcpy_s(
				pMappedData + row * targetRowPitch + offset,
				rowSize,
				pData + row * dataRowPitch,
				rowSize
			);
		}

		pConstBuffer->Unmap(0, nullptr);
	}
}