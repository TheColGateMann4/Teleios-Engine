#include "GPUProfiler.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Resources/GraphicsBuffer.h"
#include "Graphics/Resources/QueryHeap.h"

void GPUProfiler::Initialize(Graphics& graphics)
{
	unsigned int bufferCount = graphics.GetBufferCount();

	m_timestampHeap = std::make_unique<QueryHeap>(graphics, 2 * bufferCount);

	m_timestapReadbackBuffer = std::make_unique<GraphicsBuffer>(graphics, 2 * bufferCount, sizeof(UINT64), GraphicsResource::CPUAccess::readwrite, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE);
}

double GPUProfiler::GetData() const
{
	return m_data;
}

void GPUProfiler::SetBeginData(Graphics& graphics, CommandList* commandList)
{
	unsigned int bufferIndex = graphics.GetCurrentBufferIndex();

	commandList->Query(
		graphics, 
		m_timestampHeap.get(),
		2 * bufferIndex,
		D3D12_QUERY_TYPE_TIMESTAMP
	);
}

void GPUProfiler::SetEndData(Graphics& graphics, CommandList* commandList)
{
	unsigned int bufferIndex = graphics.GetCurrentBufferIndex();

	commandList->Query(
		graphics, 
		m_timestampHeap.get(),
		2 * bufferIndex + 1, 
		D3D12_QUERY_TYPE_TIMESTAMP
	);

	commandList->ResolveQuery(
		graphics,
		m_timestampHeap.get(),
		D3D12_QUERY_TYPE_TIMESTAMP,
		bufferIndex * 2,
		2,
		m_timestapReadbackBuffer.get(),
		sizeof(UINT64) * 2 * bufferIndex
	);

	struct TimeStampData
	{
		UINT64 start, end;
	};

	TimeStampData timeStampData = {};
	m_timestapReadbackBuffer->Read(graphics, &timeStampData, sizeof(timeStampData), sizeof(timeStampData) * bufferIndex);


	UINT64 gpuTimestampFrequency = 0;
	graphics.GetDeviceResources().GetCommandQueue()->GetTimestampFrequency(&gpuTimestampFrequency);

	double gpuTime = static_cast<double>(timeStampData.end - timeStampData.start) / static_cast<double>(gpuTimestampFrequency);

	m_data = gpuTime;
}