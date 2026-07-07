#include "UnorderedAccessView.h"

#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Graphics/Core/Graphics.h"

#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/CommandList.h"

#include "Graphics/Resources/GraphicsTexture.h"
#include "Graphics/Resources/GraphicsBuffer.h"

#include "Macros/ErrorMacros.h"

UnorderedAccessView::UnorderedAccessView(Graphics& graphics, GraphicsTexture* texture, unsigned int targetMip)
	:
	RootSignatureBindable({ {ShaderVisibilityGraphic::AllShaders, 0} })
{
	THROW_INTERNAL_ERROR_IF("GraphicsTexture was NULL", texture == nullptr);

	HRESULT hr;

	auto descriptor = graphics.GetDescriptorHeap().GetNextHandle();
	m_descriptorIndex = descriptor.offsetInDescriptorFromStart;

	// creating UAV itself
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = GetTypedUAVFormat(texture->GetFormat());
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D = {};
		uavDesc.Texture2D.MipSlice = targetMip;
		uavDesc.Texture2D.PlaneSlice = 0;

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateUnorderedAccessView(
			texture->GetResource(),
			nullptr,
			&uavDesc,
			descriptor.descriptorCpuHandle
		));
	}
}

UnorderedAccessView::UnorderedAccessView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot)
	:
	RootSignatureBindable({ {ShaderVisibilityGraphic::AllShaders, slot} })
{
	THROW_INTERNAL_ERROR_IF("GraphicsBuffer was NULL", buffer == nullptr);

	HRESULT hr;

	auto descriptor = graphics.GetDescriptorHeap().GetNextHandle();
	m_descriptorIndex = descriptor.offsetInDescriptorFromStart;

	// creating UAV itself
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = buffer->GetFormat();
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer = {};
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = buffer->GetNumElements();
		uavDesc.Buffer.StructureByteStride = buffer->GetByteStride();
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateUnorderedAccessView(
			buffer->GetResource(),
			nullptr,
			&uavDesc,
			descriptor.descriptorCpuHandle
		));
	}
}

void UnorderedAccessView::AddGraphicsRootSignatureParam(RootSignatureParams* rootSignatureParams)
{
	for (const auto& target : GetTargets())
		rootSignatureParams->AddUnorderedAccessViewParameter(this, target);
}

void UnorderedAccessView::BindToCommandListAsRootParam(Graphics& graphics, CommandList* commandList, const RootBinding& binding)
{
	commandList->SetComputeDescriptorTable(graphics, this, binding);
}

BindableType UnorderedAccessView::GetBindableType() const
{
	return BindableType::bindable_unorderedAccessView;
}

DescriptorType UnorderedAccessView::GetDescriptorType() const
{
	return DescriptorType::descriptor_UAV;
}

RootSignatureBindableType UnorderedAccessView::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_DescriptorTable;
}

UINT UnorderedAccessView::GetOffsetInDescriptor(Graphics& graphics) const
{
	return graphics.GetDescriptorHeap().GetHandle(m_descriptorIndex).offsetInDescriptorFromStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE UnorderedAccessView::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return graphics.GetDescriptorHeap().GetHandle(m_descriptorIndex).descriptorHeapGpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE UnorderedAccessView::GetCPUDescriptor(Graphics& graphics) const
{
	return graphics.GetDescriptorHeap().GetHandle(m_descriptorIndex).descriptorCpuHandle;
}

DXGI_FORMAT UnorderedAccessView::GetTypedUAVFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return DXGI_FORMAT_B8G8R8A8_UNORM;

		default:
			return format;
	}
}