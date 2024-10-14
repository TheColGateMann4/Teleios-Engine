#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"
#include "TargetShaders.h"
#include "DynamicConstantBuffer.h"

class Graphics;

class ConstantBuffer
{
public:
	ConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, TargetShader target = TargetShader::PixelShader, UINT slot = 0);

public:
	void SetRootIndex(UINT rootIndex);

	UINT GetRootIndex() const;
	TargetShader GetTarget() const;
	UINT GetSlot() const;

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const;

protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> pConstBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescriptorHeap;
	UINT m_rootIndex = 0;
	bool m_initializedRootIndex = false;
	TargetShader m_target;
	UINT m_slot;
};


class NonCachedConstantBuffer : public ConstantBuffer
{
public:
	NonCachedConstantBuffer(Graphics& graphics, const DynamicConstantBuffer::ConstantBufferLayout& layout, TargetShader target = TargetShader::PixelShader, UINT slot = 0);

	void Update(Graphics& graphics, void* data, size_t size);

private:
	DynamicConstantBuffer::ConstantBufferLayout m_layout;
};

class CachedConstantBuffer : public ConstantBuffer
{
public:
	CachedConstantBuffer(Graphics& graphics, DynamicConstantBuffer::ConstantBufferData& data, TargetShader target = TargetShader::PixelShader, UINT slot = 0);

	void Update(Graphics& graphics);

	DynamicConstantBuffer::ConstantBufferData& GetData();

private:
	DynamicConstantBuffer::ConstantBufferData m_data;
};
