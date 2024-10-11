#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"

class Texture
{
public:
	Texture(class Graphics& graphics, const wchar_t* path, TargetShader target = TargetShader::PixelShader, UINT slot = 0);

public:
	void SetRootIndex(UINT index);

	UINT GetRootIndex() const;
	TargetShader GetTarget() const;
	UINT GetSlot() const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor() const;
	ID3D12DescriptorHeap* GetDescriptorHeap() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;

private:
	std::wstring m_path;
	bool m_isAlphaOpaque = false;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
	UINT m_rootNodeIndex = 0;
	TargetShader m_target;
	UINT m_slot;
};