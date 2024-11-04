#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "Bindable.h"

class CommandList;
class RootSignature;

class Texture : public Bindable, public CommandListBindable, public DirectCommandListBindable, public RootSignatureBindable
{
public:
	Texture(class Graphics& graphics, const char* path, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToDirectCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual std::vector<TargetSlotAndShader>& GetTargets() override;

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const;
	ID3D12DescriptorHeap* GetDescriptorHeap() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;

private:
	std::string m_path;
	bool m_isAlphaOpaque = false;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
	std::vector<TargetSlotAndShader> m_targets;
};