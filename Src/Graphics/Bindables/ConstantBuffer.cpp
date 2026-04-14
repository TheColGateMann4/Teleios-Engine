#include "ConstantBuffer.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/Core/ConstantBufferHeap.h"
#include "Macros/ErrorMacros.h"

BufferBase::BufferBase(Graphics& graphics, const DynamicConstantBuffer::Layout& layout, ResourceTargets targets)
	:
	RootParameterBinding(targets)
{

}

Buffer::Buffer(Graphics& graphics, unsigned int numElements, DynamicConstantBuffer::Layout& layout, ResourceTargets targets)
	:
	BufferBase(graphics, layout, targets),
	m_layout(std::move(layout.GetFinished(DynamicConstantBuffer::Layout::LayoutType::data))),
	m_numElements(numElements)
{
	graphics.GetDescriptorHeap().RequestMoreSpace(graphics.GetBufferCount());
	m_descriptorPerFrame.resize(graphics.GetBufferCount());

	unsigned int layoutSize = layout.GetSize();

	THROW_INTERNAL_ERROR_IF("This system was written when layout had all elements included in its size", layoutSize < numElements);

	m_bufferIndex = graphics.GetBufferHeap().RequestMoreSpace(graphics, layoutSize, layoutSize / numElements);
}

void Buffer::Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum)
{
	UINT64 offsetInMainBuffer = graphics.GetBufferHeap().GetOffsetOfBuffer(descriptorNum, m_bufferIndex.GetIndex());
	unsigned int stride = m_layout.GetSize() / m_numElements;

	THROW_INTERNAL_ERROR_IF("Failed to get direct offset of buffer", offsetInMainBuffer % stride != 0);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Buffer = {};
	shaderResourceViewDesc.Buffer.FirstElement = offsetInMainBuffer / stride;
	shaderResourceViewDesc.Buffer.NumElements = m_numElements;
	shaderResourceViewDesc.Buffer.StructureByteStride = stride;
	shaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// creating SRV for texture resource on GPU memory
	{
		m_descriptorPerFrame.at(descriptorNum) = descriptorInfo;

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateShaderResourceView(
			graphics.GetBufferHeap().GetDynamicResource(),
			&shaderResourceViewDesc,
			m_descriptorPerFrame.at(descriptorNum).descriptorCpuHandle
		));
	}
}

void Buffer::Initialize(Graphics& graphics)
{
	for (unsigned int i = 0; i < graphics.GetBufferCount(); i++)
	{
		DescriptorHeap::DescriptorInfo descriptorInfo = graphics.GetDescriptorHeap().GetNextHandle();

		Initialize(graphics, descriptorInfo, i);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE Buffer::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptorPerFrame.at(graphics.GetCurrentBufferIndex()).descriptorHeapGpuHandle;
}

DescriptorType Buffer::GetDescriptorType() const
{
	return DescriptorType::descriptor_SRV;
}

void Buffer::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetGraphicsDescriptor(graphics, this, target);
}

void Buffer::BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target)
{
	rootSignatureParams->AddDescriptorParameter(this, target);
}

void Buffer::Update(Graphics& graphics, void* data, size_t size)
{
	graphics.GetBufferHeap().UpdateResource(graphics, m_bufferIndex, data, size);
}

D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUAddress(Graphics& graphics) const
{
	return graphics.GetBufferHeap().GetBufferAddress(graphics, m_bufferIndex);
}

BindableType Buffer::GetBindableType() const
{
	return BindableType::bindable_buffer;
}

RootSignatureBindableType Buffer::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_BufferSRV;
}

ConstantBuffer::ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::Layout& layout, ResourceTargets targets)
	:
	BufferBase(graphics, layout, targets)
{

}

void ConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetGraphicsConstBufferView(graphics, this, target);
}

void ConstantBuffer::BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target)
{
	rootSignatureParams->AddConstBufferViewParameter(this, target);
}

BindableType ConstantBuffer::GetBindableType() const
{
	return BindableType::bindable_constantBuffer;
}

RootSignatureBindableType ConstantBuffer::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_CBV;
}

NonCachedConstantBuffer::NonCachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Layout& layout, ResourceTargets targets)
	:
	ConstantBuffer(graphics, layout.GetFinished(), targets),
	m_layout(std::move(layout))
{
	// requesting size on non static heap
	m_bufferIndex = graphics.GetConstantBufferHeap().RequestMoreSpace(graphics, m_layout.GetSize());
}

void NonCachedConstantBuffer::Update(Graphics& graphics, void* data, size_t size)
{
	graphics.GetConstantBufferHeap().UpdateResource(graphics, m_bufferIndex, data, size);
}

D3D12_GPU_VIRTUAL_ADDRESS NonCachedConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	return graphics.GetConstantBufferHeap().GetBufferAddress(graphics, m_bufferIndex);
}

BindableType NonCachedConstantBuffer::GetBindableType() const
{
	return BindableType::bindable_nonCachedConstantBuffer;
}

CachedConstantBuffer::CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, ResourceTargets targets, bool frequentlyUpdated)
	:
	ConstantBuffer(graphics, data.GetLayout(), targets),
	m_data(std::move(data)),
	m_frequentlyUpdated(frequentlyUpdated)
{
	// if resource is frequently updated we will keep it in regular buffer
	if(m_frequentlyUpdated)
		m_bufferIndex.dynamicIndex = graphics.GetConstantBufferHeap().RequestMoreSpace(graphics, m_data.GetLayout().GetSize());
	else
		m_bufferIndex.staticIndex = graphics.GetConstantBufferHeap().RequestMoreStaticSpace(m_data.GetLayout().GetSize());
}

void CachedConstantBuffer::Update(Graphics& graphics)
{
	if (m_frequentlyUpdated)
		graphics.GetConstantBufferHeap().UpdateFrequentlyUpdatedResource(graphics, m_bufferIndex.dynamicIndex, m_data.GetPtr(), m_data.GetLayout().GetSize());
	else
		graphics.GetConstantBufferHeap().UpdateResource(graphics, m_bufferIndex.staticIndex, m_data.GetPtr(), m_data.GetLayout().GetSize());
}

D3D12_GPU_VIRTUAL_ADDRESS CachedConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	if (m_frequentlyUpdated)
		return graphics.GetConstantBufferHeap().GetBufferAddress(graphics, m_bufferIndex.dynamicIndex);
	else
		return graphics.GetConstantBufferHeap().GetBufferAddress(m_bufferIndex.staticIndex);

}

BindableType CachedConstantBuffer::GetBindableType() const
{
	return BindableType::bindable_cachedConstantBuffer;
}

DynamicConstantBuffer::Data& CachedConstantBuffer::GetData()
{
	return m_data;
}

void CachedConstantBuffer::DrawImguiProperties(Graphics& graphics)
{
	if (m_data.DrawImguiProperties())
		Update(graphics);
}

TempConstantBuffer::TempConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, ResourceTargets targets, bool frequentlyUpdated)
	:
	ConstantBuffer(graphics, data.GetLayout(), targets),
	m_data(std::move(data))
{
	m_bufferIndex = graphics.GetConstantBufferHeap().GetNextTempIndex(m_data.GetLayout().GetSize());

	Update(graphics);
}

void TempConstantBuffer::Update(Graphics& graphics)
{
	graphics.GetConstantBufferHeap().UpdateResource(graphics, m_bufferIndex, m_data.GetPtr(), m_data.GetLayout().GetSize());
}

void TempConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetComputeConstBufferView(graphics, this, target);
}

void TempConstantBuffer::AddComputeRootSignatureParam(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target)
{
	rootSignatureParams->AddConstBufferViewParameter(this, target);
}

D3D12_GPU_VIRTUAL_ADDRESS TempConstantBuffer::GetGPUAddress(Graphics& graphics) const
{
	return graphics.GetConstantBufferHeap().GetBufferAddress(m_bufferIndex);
}

DynamicConstantBuffer::Data& TempConstantBuffer::GetData()
{
	return m_data;
}