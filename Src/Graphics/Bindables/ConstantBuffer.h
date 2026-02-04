#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Graphics/Data/DynamicConstantBuffer.h"
#include "Binding.h"

class Graphics;
class CommandList;

class ConstantBuffer : public Bindable, public RootParameterBinding
{
public:
	ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::Layout& layout, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual BindableType GetBindableType() const override;

protected:
	void InternalUpdate(Graphics& graphics, void* data, size_t size);

protected:
	unsigned int resourceIndexInHeap = 0;
	bool m_initializedRootIndex = false;
};


class NonCachedConstantBuffer : public ConstantBuffer
{
public:
	NonCachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Layout& layout, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	void Update(Graphics& graphics, void* data, size_t size);

	virtual BindableType GetBindableType() const override;

private:
	DynamicConstantBuffer::Layout m_layout;
};

class CachedConstantBuffer : public ConstantBuffer
{
public:
	CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} }, bool frequentlyUpdated = false);

	CachedConstantBuffer(const CachedConstantBuffer&) = delete;

	virtual void Initialize(Graphics& graphics) override;

	void Update(Graphics& graphics);

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual BindableType GetBindableType() const override;

	DynamicConstantBuffer::Data& GetData();

	void DrawImguiProperties(Graphics& graphics);

private:
	DynamicConstantBuffer::Data m_data;
	bool m_frequentlyUpdated;
};

// this buffer is meant for use without previous knowledgement. Compute pipelines
class TempConstantBuffer : public ConstantBuffer
{
public:
	TempConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::PixelShader, 0} }, bool frequentlyUpdated = false);

	TempConstantBuffer(const CachedConstantBuffer&) = delete;

	void Update(Graphics& graphics);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToComputeRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	DynamicConstantBuffer::Data& GetData();

private:
	DynamicConstantBuffer::Data m_data;
};