#include "Texture.h"
#include "Graphics.h"
#include "CommandList.h"
#include "TempCommandList.h"
#include "DescriptorHeap.h"
#include "Macros/ErrorMacros.h"

#include <DirectXTex/DirectXTex.h>

#include "BindableResourceList.h"

#include "PipelineState.h"
#include "CommandList.h"
#include "Shader.h"
#include "UnorderedAccessView.h"
#include "ConstantBuffer.h"
#include "Sampler.h"

#include "Pipeline.h"

Texture::Texture(Graphics& graphics, const char* path, bool generateMips, std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(targets),
	m_path(std::string("../../") + path),
	m_generateMipMaps(generateMips)
{
	graphics.GetDescriptorHeap().RequestMoreSpace();

	for (const auto& targetShader : targets)
		if (targetShader.target != ShaderVisibilityGraphic::PixelShader)
			m_targetResourceState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

	HRESULT hr;

	std::wstring wPath = std::wstring(m_path.begin(), m_path.end());

	DirectX::ScratchImage image = {};
	DirectX::TexMetadata metaData = {};

	// reading image data from file
	{
		THROW_ERROR(DirectX::GetMetadataFromWICFile(
			wPath.c_str(),
			DirectX::WIC_FLAGS_NONE,
			metaData
		));

		THROW_ERROR(DirectX::LoadFromWICFile(
			wPath.c_str(),
			DirectX::WIC_FLAGS_NONE,
			&metaData,
			image
		));
	}

	m_isAlphaOpaque = metaData.GetAlphaMode() == DirectX::TEX_ALPHA_MODE_OPAQUE;
	m_format = metaData.format;
	m_format = m_format == DXGI_FORMAT_B8G8R8X8_UNORM ? DXGI_FORMAT_B8G8R8A8_UNORM : m_format;
	m_format = m_format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? DXGI_FORMAT_R8G8B8A8_UNORM : m_format;
	m_format = m_format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ? DXGI_FORMAT_B8G8R8A8_UNORM : m_format;
	m_minmapLevels = m_generateMipMaps ? GetMipLevels(metaData.width) : 1;

	// upload resource creation
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
		resourceDesc.MipLevels = m_minmapLevels;
		resourceDesc.Format = m_format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			m_targetResourceState,
			nullptr,
			IID_PPV_ARGS(&pUploadTexture)
		));
	}

	// GPU resource creation
	{
		D3D12_HEAP_PROPERTIES heapPropeties = {};
		heapPropeties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapPropeties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		heapPropeties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
		heapPropeties.VisibleNodeMask = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = metaData.width;
		resourceDesc.Height = metaData.height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = m_minmapLevels;
		resourceDesc.Format = m_format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		THROW_ERROR(graphics.GetDevice()->CreateCommittedResource(
			&heapPropeties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			m_targetResourceState,
			nullptr,
			IID_PPV_ARGS(&pTexture)
		));
	}

	// copying data from image to resource
	{
		THROW_ERROR(pUploadTexture->WriteToSubresource(
			0,
			nullptr,
			image.GetImages()->pixels,
			image.GetImages()->rowPitch,
			0
		));
	}
}

void Texture::Initialize(Graphics& graphics)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = m_format;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = m_minmapLevels;
	shaderResourceViewDesc.Texture2D.PlaneSlice = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0;

	// creating SRV for texture resource on GPU memory
	{
		m_textureDescriptor = graphics.GetDescriptorHeap().GetNextHandle();

		THROW_INFO_ERROR(graphics.GetDevice()->CreateShaderResourceView(
			pTexture.Get(),
			&shaderResourceViewDesc,
			m_textureDescriptor.descriptorCpuHandle
		));
	}

	// creating SRV for upload resource
	{
		m_uploadResourceDescriptor = graphics.GetDescriptorHeap().GetNextHandle();

		THROW_INFO_ERROR(graphics.GetDevice()->CreateShaderResourceView(
			pUploadTexture.Get(),
			&shaderResourceViewDesc,
			m_uploadResourceDescriptor.descriptorCpuHandle
		));
	}
}

std::shared_ptr<Texture> Texture::GetBindableResource(Graphics& graphics, const char* path, bool generateMips, std::vector<TargetSlotAndShader> targets)
{
	return BindableResourceList::GetBindableResource<Texture>(graphics, path, generateMips, targets);
}

std::string Texture::GetIdentifier(const char* path, bool generateMips, std::vector<TargetSlotAndShader> targets)
{
	std::string resultString = "Texture#";

	resultString += path;
	resultString += '#';

	for (const auto target : targets)
	{
		resultString += target.slot;
		resultString += '#';

		resultString += std::to_string(size_t(target.target));
		resultString += '#';
	}

	return resultString;
}

void Texture::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	if (m_mipsGenerated || !m_generateMipMaps)
		return;

	// before any processing we copy mip level 0 to gpu resource
	// and set resource states needed in next stage
	{
		CommandList* copyCommandList = pipeline.GetGraphicCommandList();

		copyCommandList->SetResourceState(graphics, pUploadTexture.Get(), m_targetResourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);
		copyCommandList->SetResourceState(graphics, pTexture.Get(), m_targetResourceState, D3D12_RESOURCE_STATE_COPY_DEST);

		copyCommandList->CopyTextureRegion(graphics, pTexture.Get(), pUploadTexture.Get(), 0);

		copyCommandList->SetResourceState(graphics, pUploadTexture.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE); // all shader resouce since this will be used as SRV in compute shader
		copyCommandList->SetResourceState(graphics, pTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
	
	// compute stage
	{
		// compute shader
		std::shared_ptr<Shader> computeShader = Shader::GetBindableResource(graphics, L"CS_Test", ShaderType::ComputeShader);

		// sampler to sample from texture SRV
		std::shared_ptr<StaticSampler> sampler = StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, { {ShaderVisibilityGraphic::AllShaders, 0} });

		for (unsigned int targetMipLevel = 1; targetMipLevel < m_minmapLevels; targetMipLevel++)
		{
			TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

			// resource binding and creating stage-specific resources
			{
				UnorderedAccessView uav(graphics, this, targetMipLevel);

				DynamicConstantBuffer::ConstantBufferLayout layout;
				layout.AddElement<DynamicConstantBuffer::ElementType::Int>("LevelOfDetail");

				DynamicConstantBuffer::ConstantBufferData bufferData(layout);
				*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Int>("LevelOfDetail") = targetMipLevel;

				// cached buffer containing info about current LOD
				TempConstantBuffer lodDataBuffer(graphics, bufferData, { {ShaderVisibilityGraphic::AllShaders, 0} });

				computeCommandList.Bind(computeShader);
				computeCommandList.Bind(sampler);
				computeCommandList.Bind(this); // binding SRV of local texture
				computeCommandList.Bind(std::move(lodDataBuffer)); // binding buffer with info about LOD level
				computeCommandList.Bind(std::move(uav)); // binding UAV

				computeCommandList.Dispatch(graphics);
			}

			graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));
		}
	}

	// setting target resource states
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();

		commandList->SetResourceState(graphics, pUploadTexture.Get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, m_targetResourceState);
		commandList->SetResourceState(graphics, pTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, m_targetResourceState);
	}

	// cleanup
	{
		// this is needed since textures are usually shared and multiple objects will call this function
		m_mipsGenerated = true;

		graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(pUploadTexture));
		// something like "graphics.GetDescriptorHeap()->DeleteDescriptor(m_uploadResourceDescriptor);" to mark that descriptor space can be reused
		m_uploadResourceDescriptor = {};
	}
}

void Texture::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetGraphicsDescriptorTable(graphics, this);
}

void Texture::BindToComputeCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetComputeDescriptorTable(graphics, this);
}

void Texture::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddDescriptorTableParameter(this);
}

void Texture::BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddComputeDescriptorTableParameter(this, {ShaderVisibilityGraphic::AllShaders, 0});
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_textureDescriptor.descriptorHeapGpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetCPUDescriptor(Graphics& graphics) const
{
	return m_textureDescriptor.descriptorCpuHandle;
}

DXGI_FORMAT Texture::GetFormat() const
{
	return m_format;
}

UINT Texture::GetOffsetInDescriptor() const
{
	return m_textureDescriptor.offsetInDescriptorFromStart;
}

UINT Texture::GetUploadResourceOffsetInDescriptor() const
{
	return m_uploadResourceDescriptor.offsetInDescriptorFromStart;
}

ID3D12Resource* Texture::GetResource() const
{
	return pTexture.Get();
}

unsigned int Texture::GetComputeRootIndex() const
{
	return m_computeRootIndex;
}

void Texture::SetComputeRootIndex(unsigned int rootIndex)
{
	m_computeRootIndex = rootIndex;
}

unsigned int Texture::GetMipLevels(unsigned int textureWidth)
{
	THROW_INTERNAL_ERROR_IF("Texture width passed was equal to 0", textureWidth == 0);

	return std::floor(std::log2(textureWidth)) + 1;
}