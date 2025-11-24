#include "UnorderedAccessView.h"

#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Graphics.h"

#include "RootSignature.h"
#include "CommandList.h"

#include "Texture.h"
#include "Buffer.h"

#include "Macros/ErrorMacros.h"

UnorderedAccessView::UnorderedAccessView(Graphics& graphics, Texture* texture, unsigned int targetMip)
	:
	RootSignatureBindable({ {ShaderVisibilityGraphic::AllShaders, 0} })
{
	THROW_INTERNAL_ERROR_IF("Texture was NULL", texture == nullptr);

	HRESULT hr;

	m_descriptor = graphics.GetDescriptorHeap().GetNextHandle();

	// creating UAV itself
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = texture->GetFormat();
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D = {};
		uavDesc.Texture2D.MipSlice = targetMip;
		uavDesc.Texture2D.PlaneSlice = 0;

		THROW_INFO_ERROR(graphics.GetDevice()->CreateUnorderedAccessView(
			texture->GetResource(),
			nullptr,
			&uavDesc,
			m_descriptor.descriptorCpuHandle
		));
	}
}

UnorderedAccessView::UnorderedAccessView(Graphics& graphics, Buffer* buffer, UINT slot)
	:
	RootSignatureBindable({ {ShaderVisibilityGraphic::AllShaders, slot} })
{
	THROW_INTERNAL_ERROR_IF("Buffer was NULL", buffer == nullptr);

	HRESULT hr;

	m_descriptor = graphics.GetDescriptorHeap().GetNextHandle();

	// creating UAV itself
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer = {};
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = buffer->GetNumElements();
		uavDesc.Buffer.StructureByteStride = buffer->GetByteStride();
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		THROW_INFO_ERROR(graphics.GetDevice()->CreateUnorderedAccessView(
			buffer->GetResource(),
			nullptr,
			&uavDesc,
			m_descriptor.descriptorCpuHandle
		));
	}
}

void UnorderedAccessView::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	THROW_INTERNAL_ERROR("Tried to bind UAV to graphic root signature");
}

void UnorderedAccessView::BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddUnorderedAccessViewParameter(this);
}

void UnorderedAccessView::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	THROW_INTERNAL_ERROR("Tried to bind UAV to graphic command list");
}

void UnorderedAccessView::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetComputeDescriptorTable(graphics, this);
}

UINT UnorderedAccessView::GetOffsetInDescriptor() const
{
	return m_descriptor.offsetInDescriptorFromStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE UnorderedAccessView::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptor.descriptorHeapGpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE UnorderedAccessView::GetCPUDescriptor(Graphics& graphics) const
{
	return m_descriptor.descriptorCpuHandle;
}