#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"
#include "Graphics/Data/DynamicConstantBuffer.h"
#include "Binding.h"

#include "Graphics/Core/ConstantBufferHeap.h"

class Graphics;
class CommandList;

class BufferBase : public Bindable, public RootParameterBinding
{
protected:
	BufferBase(Graphics& graphics, const DynamicConstantBuffer::Layout& layout, ResourceTargets targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

protected:
	bool m_initializedRootIndex = false;
};

class Buffer : public BufferBase, public DescriptorBindable
{
public:
	Buffer(Graphics& graphics, unsigned int numElements, DynamicConstantBuffer::Layout& layout, ResourceTargets targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum) override;

	virtual void Initialize(Graphics& graphics) override;

	virtual D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHeapGPUHandle(Graphics& graphics) const override;

	virtual DescriptorType GetDescriptorType() const override;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target) override;

	void Update(Graphics& graphics, void* data, size_t size);

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

private:
	std::vector<DescriptorHeap::DescriptorInfo> m_descriptorPerFrame;

	DynamicConstantBuffer::Layout m_layout;
	unsigned int m_numElements;
	DynamicBufferIndex m_bufferIndex = DynamicBufferIndex();
}; 

class ConstantBuffer : public BufferBase
{
public:
	ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::Layout& layout, ResourceTargets = { {ShaderVisibilityGraphic::PixelShader, 0} });

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target) override;

	virtual BindableType GetBindableType() const override;

	virtual RootSignatureBindableType GetRootSignatureBindableType() const override;

protected:
	bool m_initializedRootIndex = false;
};


class NonCachedConstantBuffer : public ConstantBuffer
{
public:
	NonCachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Layout& layout, ResourceTargets targets = { {ShaderVisibilityGraphic::PixelShader, 0} });

	void Update(Graphics& graphics, void* data, size_t size);

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual BindableType GetBindableType() const override;

private:
	DynamicConstantBuffer::Layout m_layout;
	DynamicBufferIndex m_bufferIndex = DynamicBufferIndex();
};

class CachedConstantBuffer : public ConstantBuffer
{
public:
	CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, ResourceTargets targets = { {ShaderVisibilityGraphic::PixelShader, 0} }, bool frequentlyUpdated = false);

	CachedConstantBuffer(const CachedConstantBuffer&) = delete;

	void Update(Graphics& graphics);

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	virtual BindableType GetBindableType() const override;

	DynamicConstantBuffer::Data& GetData();

	void DrawImguiProperties(Graphics& graphics);

private:
	DynamicConstantBuffer::Data m_data;

	union
	{
		DynamicBufferIndex dynamicIndex = DynamicBufferIndex(); // m_frequentlyUpdated == true
		StaticBufferIndex staticIndex; // m_frequentlyUpdated == false
	}m_bufferIndex;

	bool m_frequentlyUpdated;
};

// this buffer is meant for use without previous knowledgement. Compute pipelines
class TempConstantBuffer : public ConstantBuffer
{
public:
	TempConstantBuffer(Graphics& graphics, DynamicConstantBuffer::Data& data, ResourceTargets targets = { {ShaderVisibilityGraphic::PixelShader, 0} }, bool frequentlyUpdated = false);

	TempConstantBuffer(const CachedConstantBuffer&) = delete;

	void Update(Graphics& graphics);

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void AddComputeRootSignatureParam(RootSignatureParams* rootSignatureParams, TargetSlotAndShader& target) override;

	virtual D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress(Graphics& graphics) const override;

	DynamicConstantBuffer::Data& GetData();

private:
	DynamicConstantBuffer::Data m_data;
	TempBufferIndex m_bufferIndex;
};