#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "Bindable.h"

#include "DescriptorHeap.h"

class CommandList;
class RootSignature;
class DescriptorHeap;
class Graphics;
class Pipeline;

class Texture : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	Texture(Graphics& graphics, const char* path, bool generateMips = true, bool disableSRGBCorrection = false, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

protected:
	virtual void Initialize(Graphics& graphics) override;

public:
	static std::shared_ptr<Texture> GetBindableResource(Graphics& graphics, const char* path, bool generateMips = true, bool disableSRGBCorrection = false, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	static std::string GetIdentifier(const char* path, bool generateMips, bool disableSRGBCorrection, std::vector<TargetSlotAndShader> targets);

public:
	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToComputeCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual void BindToComputeRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;
	
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	DXGI_FORMAT GetFormat() const;

	UINT GetOffsetInDescriptor() const;

	ID3D12Resource* GetResource() const;

	unsigned int GetComputeRootIndex() const;

	void SetComputeRootIndex(unsigned int rootIndex);

private:
	static unsigned int GetMipLevels(unsigned int textureWidth);
	
	DXGI_FORMAT SetCorrectedFormat(DXGI_FORMAT format);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pTexture;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadTexture;

private:
	D3D12_RESOURCE_STATES m_targetResourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	std::string m_path;
	bool m_isAlphaOpaque = false;
	bool m_isSRGB = false;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
	unsigned int m_minmapLevels = 1;
	unsigned int m_width = 0;
	unsigned int m_height = 0;

	DescriptorHeap::DescriptorInfo m_textureDescriptor = {};

	bool m_SRGBCorrection;
	bool m_generateMipMaps;
	bool m_resourcesInitialized = false;

	unsigned int m_computeRootIndex = 0;
};