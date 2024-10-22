#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "RootSignatureResource.h"

class Texture : public RootSignatureResource
{
public:
	Texture(class Graphics& graphics, const wchar_t* path, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual std::vector<TargetSlotAndShader>& GetTargets() override;

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const;
	ID3D12DescriptorHeap* GetDescriptorHeap() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;

private:
	std::wstring m_path;
	bool m_isAlphaOpaque = false;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
	std::vector<TargetSlotAndShader> m_targets;
};