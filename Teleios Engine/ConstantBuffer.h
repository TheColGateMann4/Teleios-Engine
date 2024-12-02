#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "DynamicConstantBuffer.h"
#include "Bindable.h"

class Graphics;
class CommandList;

class ConstantBuffer : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

protected:
	void InternalUpdate(Graphics& graphics, void* data, size_t size);

protected:
	unsigned int resourceIndexInHeap = 0;
	bool m_initializedRootIndex = false;
};


class NonCachedConstantBuffer : public ConstantBuffer
{
public:
	NonCachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferLayout& layout, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	void Update(Graphics& graphics, void* data, size_t size);

private:
	DynamicConstantBuffer::ConstantBufferLayout m_layout;
};

class CachedConstantBuffer : public ConstantBuffer
{
public:
	CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferData& data, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	CachedConstantBuffer(const CachedConstantBuffer&) = delete;

	virtual void Initialize(Graphics& graphics) override;

	void Update(Graphics& graphics);

	DynamicConstantBuffer::ConstantBufferData& GetData();

private:
	DynamicConstantBuffer::ConstantBufferData m_data;
};
