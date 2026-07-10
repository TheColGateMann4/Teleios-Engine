#include "Texture.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/Core/TempCommandList.h"
#include "Graphics/Core/DescriptorHeap.h"
#include "Macros/ErrorMacros.h"

#include <DirectXTex.h>

#include "Graphics/Core/ResourceList.h"

#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/CommandList.h"
#include "Shader.h"
#include "UnorderedAccessView.h"
#include "ShaderResourceView.h"
#include "ConstantBuffer.h"
#include "Sampler.h"

#include "Graphics/Core/Pipeline.h"

#include "Graphics/Resources/GraphicsTexture.h"

Texture::Texture(Graphics& graphics, const char* path, TextureType type, int flags)
	:
#ifdef _DEBUG
	m_path(std::string("../../") + path),
#else
	m_path(path),
#endif
	m_type(type),
	m_srgb(IsSRGBTypeTexture(m_type)),
	m_generateMipMaps(!(flags & TextureFlags::NoMipMapping)),
	m_compressImage(!(flags & TextureFlags::NoCompression))
{
	graphics.GetDescriptorHeap().RequestMoreSpace();

	{
		std::string extension = std::filesystem::path(m_path).extension().string();
		m_originalFileType = GetTextureDataType(extension);
	}

	// reading file metadata and creating GPU resource
	{

		std::wstring wPath = std::wstring(m_path.begin(), m_path.end());
		DirectX::WIC_FLAGS flags = m_srgb ? DirectX::WIC_FLAGS_FORCE_SRGB : DirectX::WIC_FLAGS_IGNORE_SRGB;
		DirectX::TexMetadata metaData = {};

		HRESULT hr;

		if (m_originalFileType == TextureFileType::WIC)
		{
			THROW_ERROR(DirectX::GetMetadataFromWICFile(
				wPath.c_str(),
				flags,
				metaData
			));
		}
		else if (m_originalFileType == TextureFileType::TGA)
		{
			THROW_ERROR(DirectX::GetMetadataFromTGAFile(
				wPath.c_str(),
				metaData
			));
		}
		else if (m_originalFileType == TextureFileType::HDR)
		{
			THROW_ERROR(DirectX::GetMetadataFromHDRFile(
				wPath.c_str(),
				metaData
			));
		}
		else
		{
			THROW_INTERNAL_ERROR("Used unhandled texture file type");
		}

		DXGI_FORMAT format = GetCorrectedFormat(metaData.format);
		format = m_srgb ? GetSRGBFormat(format) : GetLinearFormat(format);
		
		m_originalFormat = format;

		if(m_compressImage)
			format = GetCompressedFormat(format);

		m_isAlphaOpaque = metaData.GetAlphaMode() == DirectX::TEX_ALPHA_MODE_OPAQUE;
		m_mipmapLevels = m_generateMipMaps ? GetMipLevels(metaData.width) : 1;

		m_gpuTexture = std::make_shared<GraphicsTexture>(graphics, GraphicsTextureDimensions(metaData.width, metaData.height, m_mipmapLevels), format, GraphicsTexture::CPUAccess::notavailable, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_FLAG_NONE);
	}
}

void Texture::Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum)
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
		m_textureDescriptor = descriptorInfo.offsetInDescriptorFromStart;

		THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateShaderResourceView(
			m_gpuTexture->GetResource(),
			&shaderResourceViewDesc,
			descriptorInfo.descriptorCpuHandle
		));
	}
}

void Texture::Initialize(Graphics& graphics)
{
	DescriptorHeap::DescriptorInfo descriptorInfo = graphics.GetDescriptorHeap().GetNextHandle();

	Initialize(graphics, descriptorInfo, 0);
}

std::shared_ptr<Texture> Texture::GetResource(Graphics& graphics, const char* path, TextureType type, int flags)
{
	return ResourceList::GetResource<Texture>(graphics, path, type, flags);
}

std::string Texture::GetIdentifier(const char* path, TextureType type, int flags)
{
	std::string resultString = "Texture#";

	resultString += path;
	resultString += '#';

	resultString += std::to_string(static_cast<int>(type));
	resultString += '#';
	resultString += std::to_string(static_cast<int>(flags));

	return resultString;
}

void Texture::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	if (m_resourcesInitialized)
		return;

	BEGIN_COMMAND_LIST_EVENT(pipeline.GetGraphicCommandList(), "Initializing Texture " + m_path);

	ReadAndUploadData(graphics, pipeline);

	END_COMMAND_LIST_EVENT(pipeline.GetGraphicCommandList());

	m_resourcesInitialized = true;
}

BindableType Texture::GetBindableType() const
{
	return BindableType::bindable_texture;
}

DescriptorType Texture::GetDescriptorType() const
{
	return DescriptorType::descriptor_SRV;
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetCPUDescriptor(Graphics& graphics) const
{
	return graphics.GetDescriptorHeap().GetHandle(m_textureDescriptor).descriptorCpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return graphics.GetDescriptorHeap().GetHandle(m_textureDescriptor).descriptorHeapGpuHandle;
}

TextureType Texture::GetTextureType() const
{
	return m_type;
}

UINT Texture::GetOffsetInDescriptor() const
{
	return m_textureDescriptor;
}

GraphicsTexture* Texture::GetTexture() const
{
	return m_gpuTexture.get();
}

bool Texture::IsAlphaOpaque() const
{
	return m_isAlphaOpaque;
}

DXGI_FORMAT Texture::GetOriginalFormat() const
{
	return m_originalFormat;
}

bool Texture::IsSRGBTypeTexture(TextureType type)
{
	switch (type)
	{
	case TextureType::texture_albedo:
		return true;

	default:
		return false;
	}
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

		case DXGI_FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		default:
			return format;
	}
}

DXGI_FORMAT Texture::GetCompressedFormat(DXGI_FORMAT format)
{
	DXGI_FORMAT compressed = DXGI_FORMAT_UNKNOWN;

	unsigned int numChannels = DirectX::BitsPerPixel(format) / DirectX::BitsPerColor(format);

	if (numChannels == 1)
		compressed = DXGI_FORMAT_BC4_UNORM;
	else if (numChannels == 2)
		compressed = DXGI_FORMAT_BC5_UNORM;
	else
		compressed = DXGI_FORMAT_BC1_UNORM;

	return DirectX::IsSRGB(format) ? GetSRGBFormat(compressed) : compressed;
}

void Texture::ReadAndUploadData(Graphics& graphics, Pipeline& pipeline)
{
	HRESULT hr;
	DirectX::ScratchImage readImage = {};
	DirectX::ScratchImage mipmappedImage = {};
	DirectX::ScratchImage compressedImage = {};

	// reading image from file
	TextureProcessingStage processingStageRead = LoadImage(graphics, readImage);

	// generating mip mapps
	if (m_generateMipMaps && processingStageRead < TextureProcessingStage::mipmaps)
	{
		GenerateMipMaps(graphics, readImage.GetImages()[0], mipmappedImage, m_mipmapLevels);

		readImage.Release();
	}

	DirectX::ScratchImage& uncompressedImage = m_generateMipMaps ? mipmappedImage : readImage;

	// compressing image to BC format
	if (m_compressImage && processingStageRead < TextureProcessingStage::compressed)
	{
		CompressImage(graphics, uncompressedImage, compressedImage);

		uncompressedImage.Release();
	}

	const DirectX::ScratchImage& imageToUpload = processingStageRead == TextureProcessingStage::compressed ? readImage : (m_compressImage ? compressedImage : uncompressedImage);

	// uploading image to gpu
	UploadImage(graphics, pipeline, imageToUpload);

	// saving image we processed to dds file
	SaveProcessedTexture(graphics, imageToUpload);
}

std::optional<std::wstring> Texture::GetDDSImagePath()
{
	std::filesystem::path imagePath = m_path;

	THROW_INTERNAL_ERROR_IF("File does not exist", !std::filesystem::exists(imagePath));
	THROW_INTERNAL_ERROR_IF("Path was not pointing to a file", !std::filesystem::is_regular_file(imagePath));
	
	imagePath.replace_extension(".dds");

	// checking in target image path
	if (std::filesystem::exists(imagePath))
		return imagePath;

	// checking in local Textures/ folder
	{
		std::filesystem::path filePath;

		bool found = false;
		for (const auto& part : imagePath)
		{
			if (found)
				filePath /= part;

			if (part == L"Models")
				found = true;
		}

		if (!found)
			filePath = imagePath.filename();


		std::filesystem::path ddsTexture = std::filesystem::current_path() / "Textures" / filePath;

		if (std::filesystem::exists(ddsTexture))
			return ddsTexture;
	}

	return std::nullopt;
}

Texture::TextureProcessingStage Texture::LoadImage(Graphics& graphics, DirectX::ScratchImage& targetImage)
{
	if (m_originalFileType != TextureFileType::DDS)
	{
		auto optDdsImagePath = GetDDSImagePath();

		if (optDdsImagePath)
			return LoadDDSImage(graphics, targetImage, optDdsImagePath.value());
	}

	switch (m_originalFileType)
	{
		case TextureFileType::WIC:
		{
			LoadWICImage(graphics, targetImage);
			break;
		}
		case TextureFileType::TGA:
		{
			LoadTGAImage(graphics, targetImage);
			break;
		}
		case TextureFileType::HDR:
		{
			LoadHDRImage(graphics, targetImage);
			break;
		}
		case TextureFileType::DDS:
			return LoadDDSImage(graphics, targetImage, std::wstring(m_path.begin(), m_path.end()));
		default:
			THROW_INTERNAL_ERROR("Unhandled texture file type");
	}

	return TextureProcessingStage::unprocessed;
}

void Texture::LoadWICImage(Graphics& graphics, DirectX::ScratchImage& targetImage)
{
	HRESULT hr;

	std::wstring wPath = std::wstring(m_path.begin(), m_path.end());
	DirectX::WIC_FLAGS flags = m_srgb ? DirectX::WIC_FLAGS_FORCE_SRGB : DirectX::WIC_FLAGS_IGNORE_SRGB;

	THROW_ERROR(DirectX::LoadFromWICFile(
		wPath.c_str(),
		flags,
		nullptr,
		targetImage
	));
}

void Texture::LoadTGAImage(Graphics& graphics, DirectX::ScratchImage& targetImage)
{
	HRESULT hr;

	std::wstring wPath = std::wstring(m_path.begin(), m_path.end());

	THROW_ERROR(DirectX::LoadFromTGAFile(
		wPath.c_str(),
		nullptr,
		targetImage
	));
}

void Texture::LoadHDRImage(Graphics& graphics, DirectX::ScratchImage& targetImage)
{
	HRESULT hr;

	std::wstring wPath = std::wstring(m_path.begin(), m_path.end());

	THROW_ERROR(DirectX::LoadFromHDRFile(
		wPath.c_str(),
		nullptr,
		targetImage
	));
}

Texture::TextureProcessingStage Texture::LoadDDSImage(Graphics& graphics, DirectX::ScratchImage& targetImage, const std::wstring& imagePath)
{
	HRESULT hr;

	THROW_ERROR(DirectX::LoadFromDDSFile(
		imagePath.c_str(),
		DirectX::DDS_FLAGS_NONE,
		nullptr,
		targetImage
	));

	const DirectX::TexMetadata& readImageMetaData = targetImage.GetMetadata();
	unsigned int readImageMipsLevels = readImageMetaData.mipLevels;

	bool isCompressed = DirectX::IsCompressed(readImageMetaData.format);
	bool hasMips = readImageMipsLevels > 1;

	THROW_INTERNAL_ERROR_IF("Read image had wrong number of mip levels", m_mipmapLevels != readImageMipsLevels);
	THROW_INTERNAL_ERROR_IF("Read image didn't have mips, but was already compressed", isCompressed && !hasMips);

	if (isCompressed)
		return TextureProcessingStage::compressed;

	if (hasMips)
		return TextureProcessingStage::mipmaps;

	return TextureProcessingStage::unprocessed;
}

void Texture::SaveProcessedTexture(Graphics& graphics, const DirectX::ScratchImage& image)
{
	HRESULT hr;

	std::filesystem::path originalImagePath = m_path;
	std::filesystem::path filePath;

	bool found = false;
	for (const auto& part : originalImagePath)
	{
		if (found)
			filePath /= part;

		if (part == L"Models")
			found = true;
	}

	if (!found)
		filePath = originalImagePath.filename();

	std::filesystem::path targetImagePath = std::filesystem::current_path() / "Textures" / filePath.replace_extension(".dds");

	std::filesystem::path pathToNewImage = targetImagePath;
	pathToNewImage.remove_filename();

	std::filesystem::create_directories(pathToNewImage);

	THROW_ERROR(DirectX::SaveToDDSFile(
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		DirectX::DDS_FLAGS_NONE,
		targetImagePath.c_str()
	));
}

void Texture::GenerateMipMaps(Graphics& graphics, const DirectX::Image& uncompressedImage, DirectX::ScratchImage& targetImage, unsigned int mipLevels)
{
	HRESULT hr;

	THROW_ERROR(DirectX::GenerateMipMaps(
		uncompressedImage,
		DirectX::TEX_FILTER_DEFAULT,
		mipLevels,
		targetImage
	));
}

void Texture::CompressImage(Graphics& graphics, const DirectX::ScratchImage& uncompressedImage, DirectX::ScratchImage& targetImage)
{
	HRESULT hr;

	const DirectX::TexMetadata& uncompressedMetadata = uncompressedImage.GetMetadata();

	// compressing read image to BC format
	THROW_ERROR(DirectX::Compress(
		uncompressedImage.GetImages(),
		uncompressedImage.GetImageCount(),
		uncompressedMetadata,
		GetCompressedFormat(uncompressedMetadata.format),
		DirectX::TEX_COMPRESS_PARALLEL,
		DirectX::TEX_THRESHOLD_DEFAULT,
		targetImage
	));
}

void Texture::UploadImage(Graphics& graphics, Pipeline& pipeline, const DirectX::ScratchImage& targetImage)
{
	DXGI_FORMAT format = GetCorrectedFormat(targetImage.GetMetadata().format);
	format = m_srgb ? GetSRGBFormat(format) : GetLinearFormat(format);

	for (int targetMip = 0; targetMip < m_mipmapLevels; targetMip++)
	{
		const DirectX::Image* pTargetImageData = targetImage.GetImage(targetMip, 0, 0);

		THROW_INTERNAL_ERROR_IF("Failed to get image mip", pTargetImageData == nullptr);

		const DirectX::Image& targetImageData = *pTargetImageData;

		unsigned int mipSize = targetImageData.slicePitch;
		unsigned int rowSize = targetImageData.rowPitch; // DirectXTex provides data padding-less, so pitch=size
		unsigned int numRows = mipSize / rowSize;

		m_gpuTexture->Update(graphics, pipeline, targetImageData.pixels, rowSize, numRows, targetImageData.rowPitch, targetMip, format);
	}
}

Texture::TextureFileType Texture::GetTextureDataType(std::string extension)
{
	std::transform(
		extension.begin(),
		extension.end(),
		extension.begin(),
		[](unsigned char c)
		{
			return std::tolower(c);
		}
	);

	if (extension == ".png" ||
		extension == ".jpg" ||
		extension == ".jpeg" ||
		extension == ".bmp" ||
		extension == ".gif" ||
		extension == ".tif" ||
		extension == ".tiff")
		return Texture::TextureFileType::WIC;

	if (extension == ".tga")
		return Texture::TextureFileType::TGA;

	if (extension == ".hdr")
		return Texture::TextureFileType::HDR;

	if (extension == ".dds")
		return Texture::TextureFileType::DDS;


	THROW_INTERNAL_ERROR("Failed to map extension to texture data type");
	return Texture::TextureFileType::Unknown;
}