#include "TextureMipView.h"

#include "includes/DirectXIncludes.h"
#include "Macros/ErrorMacros.h"

#include "Graphics.h"
#include "CommandList.h"
#include "RootSignature.h"

#include "Texture.h"

TextureMipView::TextureMipView(Graphics& graphics, Texture* texture, unsigned int targetMip)
	:
	RootSignatureBindable({ {ShaderVisibilityGraphic::AllShaders, 0} })
{
	HRESULT hr;

	m_descriptor = graphics.GetDescriptorHeap().GetNextHandle();

	// creating UAV itself
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texture->GetFormat();
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D = {};
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = targetMip + 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = targetMip;

		THROW_INFO_ERROR(graphics.GetDevice()->CreateShaderResourceView(
			texture->GetResource(),
			&srvDesc,
			m_descriptor.descriptorCpuHandle
		));
	}
}

void TextureMipView::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	THROW_INTERNAL_ERROR("Tried to bind mip SRV to graphic root signature");
}

void TextureMipView::BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddDescriptorTableParameter(this);
}

void TextureMipView::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	THROW_INTERNAL_ERROR("Tried to bind mip SRV to graphic command list");
}

void TextureMipView::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetComputeDescriptorTable(graphics, this);
}

UINT TextureMipView::GetOffsetInDescriptor() const
{
	return m_descriptor.offsetInDescriptorFromStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureMipView::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptor.descriptorHeapGpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureMipView::GetCPUDescriptor(Graphics& graphics) const
{
	return m_descriptor.descriptorCpuHandle;
}