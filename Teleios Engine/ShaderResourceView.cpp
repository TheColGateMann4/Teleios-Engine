#include "ShaderResourceView.h"
#include "RootSignature.h"
#include "CommandList.h"
#include "Macros/ErrorMacros.h"

#include "GraphicsBuffer.h"

ShaderResourceView::ShaderResourceView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot)
	:
	RootSignatureBindable(std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::AllShaders, slot}})
{
	THROW_INTERNAL_ERROR_IF("GraphicsBuffer was NULL", buffer == nullptr);

	m_descriptor = graphics.GetDescriptorHeap().GetNextHandle();

	// creating SRV
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Buffer = {};
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = buffer->GetNumElements();
		srvDesc.Buffer.StructureByteStride = buffer->GetByteStride();
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		THROW_INFO_ERROR(graphics.GetDevice()->CreateShaderResourceView(
			buffer->GetResource(),
			&srvDesc,
			m_descriptor.descriptorCpuHandle
		));
	}
}

void ShaderResourceView::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetGraphicsDescriptorTable(graphics, this);
}

void ShaderResourceView::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetComputeDescriptorTable(graphics, this);
}

void ShaderResourceView::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddDescriptorTableParameter(this);
}

void ShaderResourceView::BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddComputeDescriptorTableParameter(this, GetTargets().front());
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceView::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptor.descriptorHeapGpuHandle;
}

void ShaderResourceView::SetComputeRootIndex(unsigned int rootIndex)
{
	m_computeRootIndex = rootIndex;
}

unsigned int ShaderResourceView::GetComputeRootIndex() const
{
	return m_computeRootIndex;
}

UINT ShaderResourceView::GetOffsetInDescriptor() const
{
	return m_descriptor.offsetInDescriptorFromStart;
}