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
	static std::shared_ptr<Texture> GetBindableResource(class Graphics& graphics, const char* path, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	static std::string GetIdentifier(const char* path, std::vector<TargetSlotAndShader> targets);

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToDirectCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptor(Graphics& graphics) const override;
	ID3D12DescriptorHeap* GetDescriptorHeap() const;

	DXGI_FORMAT GetFormat() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;

private:
	std::string m_path;
	bool m_isAlphaOpaque = false;
	DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
};