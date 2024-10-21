#include "Texture.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"

#include <DirectXTex/DirectXTex.h>

Texture::Texture(Graphics& graphics, const wchar_t* path, ShaderVisibilityGraphic target, UINT slot)
	:
#ifdef _DEBUG
	m_path(std::wstring(L"../../Images/") + path),
#else
	m_path(std::wstring(L"Images/") + path),
#endif
	m_target(target),
	m_slot(slot)
{
	D3D12_RESOURCE_STATES resourceStateFlag = (target == ShaderVisibilityGraphic::PixelShader) ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

	HRESULT hr;

	DirectX::ScratchImage image = {};
	DirectX::TexMetadata metaData = {};

	// reading image data from file
	{
		THROW_ERROR(DirectX::GetMetadataFromWICFile(
			m_path.c_str(),
			DirectX::WIC_FLAGS_NONE,
			metaData
		));

		THROW_ERROR(DirectX::LoadFromWICFile(
			m_path.c_str(),
			DirectX::WIC_FLAGS_NONE,
			&metaData,
			image
		));
	}

	m_isAlphaOpaque = metaData.GetAlphaMode() == DirectX::TEX_ALPHA_MODE_OPAQUE;
	m_format = metaData.format;

	// resource creation
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = metaData.width;
		resourceDesc.Height = metaData.height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = m_format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			resourceStateFlag,
			nullptr,
			IID_PPV_ARGS(&pTexture)
		));
	}

	// creating descriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&pDescriptorHeap)));
	}

	// creating shader resource view
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = m_format;
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		shaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0;

		THROW_INFO_ERROR(graphics.GetDevice()->CreateShaderResourceView(
			pTexture.Get(),
			&shaderResourceViewDesc,
			pDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
		));
	}

	// copying data from image to resource
	{
		THROW_ERROR(pTexture->WriteToSubresource(
			0, 
			nullptr,
			image.GetImages()->pixels,
			image.GetImages()->rowPitch,
			0
		));
	}
}

void Texture::SetRootIndex(UINT index)
{
	m_rootNodeIndex = index;
}

UINT Texture::GetRootIndex() const
{
	return m_rootNodeIndex;
}

ShaderVisibilityGraphic Texture::GetTarget() const
{
	return m_target;
}

UINT Texture::GetSlot() const
{
	return m_slot;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGPUDescriptor() const
{
	return pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

ID3D12DescriptorHeap* Texture::GetDescriptorHeap() const
{
	return pDescriptorHeap.Get();
}