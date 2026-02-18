#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Shaders/TargetShaders.h"
#include "Binding.h"

#include "Graphics/Core/DescriptorHeap.h"

class CommandList;
class RootSignature;
class Graphics;
class Pipeline;

class GraphicsTexture;

namespace DirectX
{
	struct Image;
	class ScratchImage;
};

enum class TextureType : int
{
	texture_none = -1,
	texture_albedo,
	texture_normal,
	texture_metalness_roughness,
	texture_metalness,
	texture_roughness,
	texture_reflectivity,
	texture_ambient,
	texture_opacity,

	texture_types_num
};

class Texture : public Bindable, public DescriptorBindable
{
private:
	enum class TextureProcessingStage
	{
		unprocessed,
		mipmaps,
		compressed // if there are no mip maps in compressed texture we are going to throw
	};

public:
	Texture(Graphics& graphics, const char* path, TextureType type);

public:
	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

protected:
	virtual void Initialize(Graphics& graphics) override;

public:
	static std::shared_ptr<Texture> GetBindableResource(Graphics& graphics, const char* path, TextureType type);

	static std::string GetIdentifier(const char* path, TextureType type);

public:
	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);
	
	virtual BindableType GetBindableType() const override;

	virtual DescriptorType GetDescriptorType() const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	TextureType GetTextureType() const;

	UINT GetOffsetInDescriptor() const;

	GraphicsTexture* GetTexture() const;

	unsigned int GetComputeRootIndex() const;

	void SetComputeRootIndex(unsigned int rootIndex);
	
	bool IsAlphaOpaque() const;

	static DXGI_FORMAT GetCorrectedFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetLinearFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetCompressedFormat(DXGI_FORMAT format);

	static bool IsSRGBTypeTexture(TextureType type);

private:
	static unsigned int GetMipLevels(unsigned int textureWidth);

private:
	void ReadAndUploadData(Graphics& graphics, Pipeline& pipeline);

	// texture loading
	std::optional<std::wstring> GetDDSImagePath();

	TextureProcessingStage LoadImage(Graphics& graphics, DirectX::ScratchImage& targetImage);
	void LoadWICImage(Graphics& graphics, DirectX::ScratchImage& targetImage);
	TextureProcessingStage LoadDDSImage(Graphics& graphics, DirectX::ScratchImage& targetImage, std::wstring ddsImagePath);

	// texture saving
	void SaveProcessedTexture(Graphics& graphics, const DirectX::ScratchImage& image);

	// texture processing
	void GenerateMipMaps(Graphics& graphics, const DirectX::Image& uncompressedImage, DirectX::ScratchImage& targetImage, unsigned int mipLevels);
	void CompressImage(Graphics& graphics, const DirectX::ScratchImage& uncompressedImage, DirectX::ScratchImage& targetImage);

	// data uploading
	void UploadImage(Graphics& graphics, Pipeline& pipeline, const DirectX::ScratchImage& targetImage);

private:
	std::shared_ptr<GraphicsTexture> m_gpuTexture;

	std::string m_path;
	TextureType m_type;
	bool m_isAlphaOpaque = false;
	bool m_srgb = false;
	unsigned int m_mipmapLevels = 1;

	DescriptorHeap::DescriptorInfo m_textureDescriptor = {};

	bool m_generateMipMaps;
	bool m_compressImage;
	bool m_resourcesInitialized = false;

	unsigned int m_computeRootIndex = 0;
};