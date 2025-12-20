#include "Texture.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/Core/TempCommandList.h"
#include "Graphics/Core/DescriptorHeap.h"
#include "Macros/ErrorMacros.h"

#include <DirectXTex/DirectXTex.h>

#include "Graphics/Core/BindableResourceList.h"

#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/CommandList.h"
#include "Shader.h"
#include "UnorderedAccessView.h"
#include "ShaderResourceView.h"
#include "ConstantBuffer.h"
#include "Sampler.h"

#include "Graphics/Core/Pipeline.h"

#include "Graphics/Resources/GraphicsTexture.h"

Texture::Texture(Graphics& graphics, const char* path, bool srgb, bool generateMips, std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(targets),

#ifdef _DEBUG
	m_path(std::string("../../") + path),
#else
	m_path(path),
#endif
	m_srgb(srgb),
	m_generateMipMaps(generateMips)
{
	graphics.GetDescriptorHeap().RequestMoreSpace();

	// reading file metadata and creating GPU resource
	{
		std::wstring wPath = std::wstring(m_path.begin(), m_path.end());
		DirectX::WIC_FLAGS flags = m_srgb ? DirectX::WIC_FLAGS_FORCE_SRGB : DirectX::WIC_FLAGS_IGNORE_SRGB;
		DirectX::TexMetadata metaData = {};
		D3D12_RESOURCE_STATES targetResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		const auto& targetSlots = GetTargets();

		for (const auto& targetShader : targetSlots)
			if (targetShader.target != ShaderVisibilityGraphic::PixelShader)
				targetResourceState = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

		HRESULT hr;

		THROW_ERROR(DirectX::GetMetadataFromWICFile(
			wPath.c_str(),
			flags,
			metaData
		));

		DXGI_FORMAT format = GetCorrectedFormat(metaData.format);
		format = m_srgb ? GetSRGBFormat(format) : GetLinearFormat(format);

		m_isAlphaOpaque = metaData.GetAlphaMode() == DirectX::TEX_ALPHA_MODE_OPAQUE;
		m_mipmapLevels = m_generateMipMaps ? GetMipLevels(metaData.width) : 1;

		m_gpuTexture = std::make_shared<GraphicsTexture>(graphics, metaData.width, metaData.height, m_mipmapLevels, format, GraphicsTexture::CPUAccess::notavailable, targetResourceState, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}
}

void Texture::Initialize(Graphics& graphics)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = m_gpuTexture->GetFormat();
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = m_mipmapLevels;
	shaderResourceViewDesc.Texture2D.PlaneSlice = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0;

	// creating SRV for texture resource on GPU memory
	{
		m_textureDescriptor = graphics.GetDescriptorHeap().GetNextHandle();

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateShaderResourceView(
			m_gpuTexture->GetResource(),
			&shaderResourceViewDesc,
			m_textureDescriptor.descriptorCpuHandle
		));
	}
}

std::shared_ptr<Texture> Texture::GetBindableResource(Graphics& graphics, const char* path, bool srgb, bool generateMips, std::vector<TargetSlotAndShader> targets)
{
	return BindableResourceList::GetBindableResource<Texture>(graphics, path, srgb, generateMips, targets);
}

std::string Texture::GetIdentifier(const char* path, bool allowSRGB, bool generateMips, std::vector<TargetSlotAndShader> targets)
{
	std::string resultString = "Texture#";

	resultString += path;
	resultString += '#';

	resultString += std::to_string(generateMips);
	resultString += '#';

	resultString += std::to_string(allowSRGB);
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
	if (m_resourcesInitialized)
		return;

	BEGIN_COMMAND_LIST_EVENT(pipeline.GetGraphicCommandList(), "Initializing Texture " + m_path);

	UploadData(graphics, pipeline);

	GenerateMipMaps(graphics, pipeline);

	END_COMMAND_LIST_EVENT(pipeline.GetGraphicCommandList());

	m_resourcesInitialized = true;
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

UINT Texture::GetOffsetInDescriptor() const
{
	return m_textureDescriptor.offsetInDescriptorFromStart;
}

GraphicsTexture* Texture::GetTexture() const
{
	return m_gpuTexture.get();
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

DXGI_FORMAT Texture::GetCorrectedFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		default:
			return format;
	}
}

DXGI_FORMAT Texture::GetLinearFormat(DXGI_FORMAT format)
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

DXGI_FORMAT Texture::GetSRGBFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

	default:
		return format;
	}
}

void Texture::UploadData(Graphics& graphics, Pipeline& pipeline)
{
	HRESULT hr;

	std::wstring wPath = std::wstring(m_path.begin(), m_path.end());
	DirectX::ScratchImage image = {};
	DirectX::TexMetadata metaData = {};
	DirectX::WIC_FLAGS flags = m_srgb ? DirectX::WIC_FLAGS_FORCE_SRGB : DirectX::WIC_FLAGS_IGNORE_SRGB;

	// reading image data from file
	THROW_ERROR(DirectX::LoadFromWICFile(
		wPath.c_str(),
		flags,
		&metaData,
		image
	));

	DXGI_FORMAT format = GetCorrectedFormat(metaData.format);
	format = m_srgb ? GetSRGBFormat(format) : GetLinearFormat(format);

	m_gpuTexture->Update(graphics, pipeline, image.GetImages()->pixels, metaData.width, metaData.height, format);
}

void Texture::GenerateMipMaps(Graphics& graphics, Pipeline& pipeline)
{
	D3D12_RESOURCE_STATES entryResourceTargetState = m_gpuTexture->GetResourceTargetState();

	// when we don't want to generate mip maps, we still want our resource to be copied
	if (!m_generateMipMaps)
	{
		m_resourcesInitialized = true;
		return;
	}

	// compute stage
	{
		// compute shader
		std::shared_ptr<Shader> computeShader = Shader::GetBindableResource(graphics, L"CS_MipMapGeneration", ShaderType::ComputeShader);

		// sampler to sample from texture SRV
		std::shared_ptr<StaticSampler> sampler = StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, { {ShaderVisibilityGraphic::AllShaders, 0} });

		// pre-setting resource states in bulk
		{
			CommandList* commandList = pipeline.GetGraphicCommandList();

			for (unsigned int targetMipLevel = 1; targetMipLevel < m_mipmapLevels; targetMipLevel++)
				commandList->SetResourceState(graphics, m_gpuTexture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, targetMipLevel);
		}

		for (unsigned int targetMipLevel = 1; targetMipLevel < m_mipmapLevels; targetMipLevel++)
		{
			// setting higher LOD mip level as SRV to sample from it
			{
				CommandList* commandList = pipeline.GetGraphicCommandList();

				commandList->SetResourceState(graphics, m_gpuTexture.get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, targetMipLevel - 1);
			}

			TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

			// resource binding and creating stage-specific resources
			{
				ShaderResourceView srv(graphics, this->GetTexture(), targetMipLevel - 1);
				UnorderedAccessView uav(graphics, this->GetTexture(), targetMipLevel);

				computeCommandList.Bind(computeShader);
				computeCommandList.Bind(sampler);
				computeCommandList.Bind(std::move(srv)); // binding SRV of desired mip map
				computeCommandList.Bind(std::move(uav)); // binding UAV

				computeCommandList.Dispatch(graphics, m_gpuTexture->GetWidth(), m_gpuTexture->GetHeight());
			}

			graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));
		}
	}

	// setting target resource state after execution
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();

		if (entryResourceTargetState != D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE)
		{
			// if state we want is only pixel_shared_resource then we have to change all mip levels states
			for (unsigned int targetMipLevel = 0; targetMipLevel < m_mipmapLevels; targetMipLevel++)
				commandList->SetResourceState(graphics, m_gpuTexture.get(), entryResourceTargetState, targetMipLevel);
		}
		else
		{
			// if state we want is all_shader_resource then out compute pipeline already set most of them correctly, only last one to set
			commandList->SetResourceState(graphics, m_gpuTexture.get(), entryResourceTargetState, m_mipmapLevels - 1);
		}
	}
}