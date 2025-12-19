#include "GraphicsTexture.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"

GraphicsTexture::GraphicsTexture(Graphics& graphics, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, CPUAccess cpuAccess, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flags)
	:
	GraphicsResource(format, cpuAccess, state),
	m_width(width),
	m_height(height),
	m_mipLevels(mipLevels),
	m_states(m_mipLevels, { D3D12_RESOURCE_STATE_COMMON, state })
{
	Initialize(graphics, flags, nullptr);
}

GraphicsTexture::GraphicsTexture(Graphics& graphics, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, DirectX::XMFLOAT4 optimizedClearValue, CPUAccess cpuAccess, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flags)
	:
	GraphicsResource(format, cpuAccess, state),
	m_width(width),
	m_height(height),
	m_mipLevels(mipLevels),
	m_states(m_mipLevels, { D3D12_RESOURCE_STATE_COMMON, state })
{
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;
	clearValue.Color[0] = optimizedClearValue.x;
	clearValue.Color[1] = optimizedClearValue.y;
	clearValue.Color[2] = optimizedClearValue.z;
	clearValue.Color[3] = optimizedClearValue.w;

	Initialize(graphics, flags, &clearValue);
}

GraphicsTexture::GraphicsTexture(Graphics& graphics, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, float depthClearValue, uint8_t stencilClearValue, CPUAccess cpuAccess, D3D12_RESOURCE_STATES state, D3D12_RESOURCE_FLAGS flags)
	:
	GraphicsResource(format, cpuAccess, state),
	m_width(width),
	m_height(height),
	m_mipLevels(mipLevels),
	m_states(m_mipLevels, { D3D12_RESOURCE_STATE_COMMON, state })
{
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;
	clearValue.DepthStencil = {};
	clearValue.DepthStencil.Depth = depthClearValue;
	clearValue.DepthStencil.Stencil = stencilClearValue;

	Initialize(graphics, flags, &clearValue);
}

void GraphicsTexture::Initialize(Graphics& graphics, D3D12_RESOURCE_FLAGS flags, D3D12_CLEAR_VALUE* clearValue)
{
	HRESULT hr;
	unsigned int numberOfBuffers = graphics.GetBufferCount();

	// creating resource
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = GetHardwareHeapType(m_cpuAccess);
		heapPropeties.CPUPageProperty = GetHardwareHeapUsagePropety(m_cpuAccess);
		heapPropeties.MemoryPoolPreference = GetHardwareHeapMemoryPool(m_cpuAccess);
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = m_width;
		resourceDesc.Height = m_height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = m_mipLevels;
		resourceDesc.Format = m_format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = flags;

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			clearValue,
			IID_PPV_ARGS(&m_pResource)
		));
	}
}

void GraphicsTexture::CopyResourcesMipTo(Graphics& graphics, CommandList* copyCommandList, GraphicsTexture* dst, unsigned int targetMip)
{
	THROW_INTERNAL_ERROR_IF("Dest resource was NULL", dst == nullptr);

	BEGIN_COMMAND_LIST_EVENT(copyCommandList, std::string("Copying GraphicsTexture Mip " + std::to_string(targetMip)));

	copyCommandList->SetResourceState(graphics, this, D3D12_RESOURCE_STATE_COPY_SOURCE, targetMip);
	copyCommandList->SetResourceState(graphics, dst, D3D12_RESOURCE_STATE_COPY_DEST, targetMip);

	copyCommandList->CopyTextureRegion(graphics, dst->GetResource(), this->GetResource(), targetMip);

	copyCommandList->SetResourceState(graphics, dst, dst->GetResourceTargetState(targetMip), targetMip);
	copyCommandList->SetResourceState(graphics, this, this->GetResourceTargetState(targetMip), targetMip);

	END_COMMAND_LIST_EVENT(copyCommandList);
}

D3D12_RESOURCE_STATES GraphicsTexture::GetResourceState(unsigned int targetSubresource) const
{
	return m_states.at(targetSubresource).currentState;
}

D3D12_RESOURCE_STATES GraphicsTexture::GetResourceTargetState(unsigned int targetSubresource)
{
	return m_states.at(targetSubresource).targetState;
}

void GraphicsTexture::SetAllResourceStates(D3D12_RESOURCE_STATES newState)
{
	for (auto& mipState : m_states)
		mipState.currentState = newState;
}

void GraphicsTexture::SetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource)
{
	m_states.at(targetSubresource).currentState = newState;
}

void GraphicsTexture::SetTargetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource)
{
	m_states.at(targetSubresource).targetState = newState;
}

void GraphicsTexture::Update(Graphics& graphics, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, width, height, format);
	}
	else
	{
		THROW_INTERNAL_ERROR("Tried to Update resource without CPU access. Use pipeline access to update it using temp resource");
	}
}

void GraphicsTexture::Update(Graphics& graphics, Pipeline& pipeline, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format)
{
	if (m_cpuAccess == CPUAccess::readwrite || m_cpuAccess == CPUAccess::write)
	{
		UpdateLocalResource(graphics, data, width, height, format);
	}
	else
	{
		UpdateUsingTempResource(graphics, pipeline, data, width, height, format);
	}
}

unsigned int GraphicsTexture::GetWidth() const
{
	return m_width;
}

unsigned int GraphicsTexture::GetHeight() const
{
	return m_height;
}

void GraphicsTexture::UpdateUsingTempResource(Graphics& graphics, Pipeline& pipeline, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format)
{
	std::shared_ptr<GraphicsTexture> uploadResource = std::make_shared<GraphicsTexture>(graphics, width, height, 1, format, CPUAccess::write);
	uploadResource->Update(graphics, data, width, height, format);

	uploadResource->CopyResourcesMipTo(graphics, pipeline.GetGraphicCommandList(), this, 0);

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(uploadResource));
}

void GraphicsTexture::UpdateLocalResource(Graphics& graphics, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format)
{
	THROW_INTERNAL_ERROR_IF("GraphicsTextures weren't same dimensions", m_width != width || m_height != height || m_format != format);

	HRESULT hr;

	// passing data to upload resource
	THROW_ERROR(m_pResource->WriteToSubresource(
		0,
		nullptr,
		data,
		width * GetPixelSize(format),
		0
	));
}

int GraphicsTexture::GetPixelSize(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SINT:
			return 1;

		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SINT:
			return 2;

		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SINT:
			return 4;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return 4;

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return 8;

		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
			return 4;

		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
			return 8;

		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 12;

		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 16;

		default:
			THROW_INTERNAL_ERROR("Failed to map texture type to known one");
	}
}