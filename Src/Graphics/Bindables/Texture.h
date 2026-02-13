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

class Texture : public Bindable, public RootParameterBinding, public DescriptorBindable
{
private:
	enum class TextureProcessingStage
	{
		unprocessed,
		mipmaps,
		compressed // if there are no mip maps in compressed texture we are going to throw
	};

public:
	Texture(Graphics& graphics, const char* path, bool allowSRGB = false, bool generateMips = true, bool compress = true, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

protected:
	virtual void Initialize(Graphics& graphics) override;

public:
	static std::shared_ptr<Texture> GetBindableResource(Graphics& graphics, const char* path, bool srgb = false, bool generateMips = true, bool compress = true,  std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	static std::string GetIdentifier(const char* path, bool srgb, bool generateMips, bool compress, std::vector<TargetSlotAndShader> targets);

public:
	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual void BindToComputeRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;
	
	virtual BindableType GetBindableType() const override;

	virtual DescriptorType GetDescriptorType() const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	UINT GetOffsetInDescriptor() const;

	GraphicsTexture* GetTexture() const;

	unsigned int GetComputeRootIndex() const;

	void SetComputeRootIndex(unsigned int rootIndex);
	
	bool IsAlphaOpaque() const;

	static DXGI_FORMAT GetCorrectedFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetLinearFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetCompressedFormat(DXGI_FORMAT format);

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
	bool m_isAlphaOpaque = false;
	bool m_srgb = false;
	unsigned int m_mipmapLevels = 1;

	DescriptorHeap::DescriptorInfo m_textureDescriptor = {};

	bool m_generateMipMaps;
	bool m_compressImage;
	bool m_resourcesInitialized = false;

	unsigned int m_computeRootIndex = 0;
};