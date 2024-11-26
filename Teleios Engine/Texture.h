#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "Bindable.h"

class CommandList;
class RootSignature;
class DescriptorHeap;

class Texture : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	Texture(class Graphics& graphics, const char* path, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

protected:
	virtual void Initialize(Graphics& graphics) override;

public:
	static std::shared_ptr<Texture> GetBindableResource(class Graphics& graphics, const char* path, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	static std::string GetIdentifier(const char* path, std::vector<TargetSlotAndShader> targets);

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptor(Graphics& graphics) const;

	DXGI_FORMAT GetFormat() const;

	UINT GetOffsetInDescriptor() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pTexture;

private:
	std::string m_path;
	bool m_isAlphaOpaque = false;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;

	D3D12_GPU_DESCRIPTOR_HANDLE m_descriptorHeapGPUHandle = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptorCPUHandle = {};
	UINT m_offsetInDescriptorFromStart = 0;
};