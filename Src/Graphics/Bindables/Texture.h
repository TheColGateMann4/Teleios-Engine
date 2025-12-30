#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Shaders/TargetShaders.h"
#include "Bindable.h"

#include "Graphics/Core/DescriptorHeap.h"

class CommandList;
class RootSignature;
class Graphics;
class Pipeline;

class GraphicsTexture;

class Texture : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	Texture(Graphics& graphics, const char* path, bool allowSRGB = false, bool generateMips = true, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

protected:
	virtual void Initialize(Graphics& graphics) override;

public:
	static std::shared_ptr<Texture> GetBindableResource(Graphics& graphics, const char* path, bool srgb = false, bool generateMips = true, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	static std::string GetIdentifier(const char* path, bool srgb, bool generateMips, std::vector<TargetSlotAndShader> targets);

public:
	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;
	
	virtual BindableType GetBindableType() const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	UINT GetOffsetInDescriptor() const;

	GraphicsTexture* GetTexture() const;

	unsigned int GetComputeRootIndex() const;

	void SetComputeRootIndex(unsigned int rootIndex);
	
	static DXGI_FORMAT GetCorrectedFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetLinearFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT format);

private:
	static unsigned int GetMipLevels(unsigned int textureWidth);

private:
	void UploadData(Graphics& graphics, Pipeline& pipeline);
	void GenerateMipMaps(Graphics& graphics, Pipeline& pipeline);

private:
	std::shared_ptr<GraphicsTexture> m_gpuTexture;

	std::string m_path;
	bool m_isAlphaOpaque = false;
	bool m_srgb = false;
	unsigned int m_mipmapLevels = 1;

	DescriptorHeap::DescriptorInfo m_textureDescriptor = {};

	bool m_generateMipMaps;
	bool m_resourcesInitialized = false;

	unsigned int m_computeRootIndex = 0;
};