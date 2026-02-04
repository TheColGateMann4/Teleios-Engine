#include "ShaderResourceView.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/CommandList.h"
#include "Macros/ErrorMacros.h"

#include "Texture.h" // for Texture::GetLinearFormat()
#include "Graphics/Resources/GraphicsTexture.h"
#include "Graphics/Resources/GraphicsBuffer.h"

#include "Graphics/Core/BindableResourceList.h"

ShaderResourceViewBase::ShaderResourceViewBase(unsigned int slot)
	:
	RootParameterBinding(std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::AllShaders, slot}}),
	m_computeRootIndex(0)
{

}

void ShaderResourceViewBase::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetGraphicsDescriptorTable(graphics, this, target);
}

void ShaderResourceViewBase::BindToComputeCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetComputeDescriptorTable(graphics, this, target);
}

void ShaderResourceViewBase::BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target)
{
	rootSignature->AddDescriptorTableParameter(this, target);
}

void ShaderResourceViewBase::BindToComputeRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target)
{
	rootSignature->AddComputeDescriptorTableParameter(this, GetTargets().front());
}

BindableType ShaderResourceViewBase::GetBindableType() const
{
	return BindableType::bindable_shaderResourceView;
}

void ShaderResourceViewBase::SetComputeRootIndex(unsigned int rootIndex)
{
	m_computeRootIndex = rootIndex;
}

unsigned int ShaderResourceViewBase::GetComputeRootIndex() const
{
	return m_computeRootIndex;
}

void ShaderResourceViewBase::InitializeTextureSRV(Graphics& graphics, unsigned int targetMip, DescriptorHeap::DescriptorInfo& descriptor, const GraphicsTexture* texture)
{
	THROW_INTERNAL_ERROR_IF("GraphicsTexture was NULL", texture == nullptr);

	descriptor = graphics.GetDescriptorHeap().GetNextHandle();

	// creating UAV
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = Texture::GetLinearFormat(Texture::GetCorrectedFormat(texture->GetFormat()));
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D = {};
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = targetMip + 1;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = targetMip;

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateShaderResourceView(
			texture->GetResource(),
			&srvDesc,
			descriptor.descriptorCpuHandle
		));
	}
}

void ShaderResourceViewBase::InitializeBufferSRV(Graphics& graphics, DescriptorHeap::DescriptorInfo& descriptor, const GraphicsBuffer* buffer)
{
	THROW_INTERNAL_ERROR_IF("GraphicsBuffer was NULL", buffer == nullptr);

	descriptor = graphics.GetDescriptorHeap().GetNextHandle();

	// creating UAV
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

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateShaderResourceView(
			buffer->GetResource(),
			&srvDesc,
			descriptor.descriptorCpuHandle
		));
	}
}

ShaderResourceView::ShaderResourceView(Graphics& graphics, GraphicsTexture* texture, unsigned int targetMip, UINT slot)
	:
	ShaderResourceViewBase(slot),
	m_targetSubresource(targetMip),
	m_resource(texture)
{
	graphics.GetDescriptorHeap().RequestMoreSpace();
}

ShaderResourceView::ShaderResourceView(Graphics& graphics, GraphicsBuffer* buffer, UINT slot)
	:
	ShaderResourceViewBase(slot),
	m_resource(buffer)
{
	graphics.GetDescriptorHeap().RequestMoreSpace();
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceView::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptor.descriptorHeapGpuHandle;
}

UINT ShaderResourceView::GetOffsetInDescriptor(Graphics& graphics) const
{
	return m_descriptor.offsetInDescriptorFromStart;
}

void ShaderResourceView::Initialize(Graphics& graphics)
{
	GraphicsResourceType resourceType = m_resource->GetResourceType();

	if(resourceType == GraphicsResourceType::texture)
	{
		const GraphicsTexture* texture = static_cast<const GraphicsTexture*>(m_resource);
		InitializeTextureSRV(graphics, m_targetSubresource, m_descriptor, texture);
	}
	else if(resourceType == GraphicsResourceType::buffer)
	{
		const GraphicsBuffer* buffer = static_cast<const GraphicsBuffer*>(m_resource);
		InitializeBufferSRV(graphics, m_descriptor, buffer);
	}
}

ShaderResourceViewMultiResource::ShaderResourceViewMultiResource(Graphics& graphics, BackBufferRenderTarget* renderTarget, UINT slot)
	:
	ShaderResourceViewBase(slot)
{
	unsigned int numBuffers = graphics.GetBufferCount();

	graphics.GetDescriptorHeap().RequestMoreSpace(numBuffers);

	m_resources.reserve(numBuffers);

	for (int i = 0; i < numBuffers; i++)
	{
		GraphicsTexture* frameTexture = renderTarget->GetTexture(i);

		THROW_INTERNAL_ERROR_IF("GraphicsTexture from BackBufferRenderTarget was null", frameTexture == nullptr);

		m_resources.push_back(frameTexture);
	}
}

ShaderResourceViewMultiResource::ShaderResourceViewMultiResource(Graphics& graphics, DepthStencilViewMultiResource* depthStencil, UINT slot)
	:
	ShaderResourceViewBase(slot)
{
	unsigned int numBuffers = graphics.GetBufferCount();

	graphics.GetDescriptorHeap().RequestMoreSpace(numBuffers);

	m_resources.reserve(numBuffers);

	for (int i = 0; i < numBuffers; i++)
	{
		GraphicsTexture* frameTexture = depthStencil->GetResource(i);

		THROW_INTERNAL_ERROR_IF("GraphicsTexture from DepthStencilViewMultiResource was null", frameTexture == nullptr);

		m_resources.push_back(frameTexture);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceViewMultiResource::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptors.at(graphics.GetCurrentBufferIndex()).descriptorHeapGpuHandle;
}

unsigned int ShaderResourceViewMultiResource::GetOffsetInDescriptor(Graphics& graphics) const
{
	return m_descriptors.at(graphics.GetCurrentBufferIndex()).offsetInDescriptorFromStart;
}

void ShaderResourceViewMultiResource::Initialize(Graphics& graphics)
{
	unsigned int numResources = static_cast<unsigned int>(m_resources.size());

	m_descriptors.resize(numResources);

	for (int i = 0; i < numResources; i++)
	{
		GraphicsResource* targetResource = m_resources.at(i);

		GraphicsResourceType resourceType = targetResource->GetResourceType();

		THROW_INTERNAL_ERROR_IF("GraphicsResource type was not texture", resourceType != GraphicsResourceType::texture);

		const GraphicsTexture* texture = static_cast<const GraphicsTexture*>(targetResource);

		InitializeTextureSRV(graphics, 0, m_descriptors.at(i), texture);
	}
}

std::shared_ptr<ShaderResourceViewMultiResource> ShaderResourceViewMultiResource::GetBindableResource(Graphics& graphics, std::string identifier, BackBufferRenderTarget* renderTarget, UINT slot)
{
	return BindableResourceList::GetBindableResourceByID<ShaderResourceViewMultiResource>(graphics, identifier, renderTarget, slot);
}

std::shared_ptr<ShaderResourceViewMultiResource> ShaderResourceViewMultiResource::GetBindableResource(Graphics& graphics, std::string identifier, DepthStencilViewMultiResource* depthStencil, UINT slot)
{
	return BindableResourceList::GetBindableResourceByID<ShaderResourceViewMultiResource>(graphics, identifier, depthStencil, slot);
}

std::string ShaderResourceViewMultiResource::GetIdentifier(std::string identifier)
{
	std::string resultString = "ShaderResourceViewMultiResource#";

	resultString += identifier;

	return resultString;
}

GraphicsResource* ShaderResourceViewMultiResource::GetResource(Graphics& graphics) const
{
	return m_resources.at(graphics.GetCurrentBufferIndex());
}