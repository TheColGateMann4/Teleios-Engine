#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "GraphicsResources/GraphicsTexture.h"

class Graphics;

class DepthStencilViewBase
{
protected:
	DepthStencilViewBase(Graphics& graphics, unsigned int numDescriptors);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const = 0;

	virtual const GraphicsTexture* GetResource(Graphics& graphics) const = 0;
	virtual GraphicsTexture* GetResource(Graphics& graphics) = 0;

protected:
	void CreateDSV(Graphics& graphics, D3D12_CPU_DESCRIPTOR_HANDLE& descriptor, GraphicsTexture* texture);

protected:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap = {};
};

class DepthStencilView : public DepthStencilViewBase
{
public:
	 DepthStencilView(Graphics& graphics);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const override;

	virtual const GraphicsTexture* GetResource(Graphics& graphics) const override;
	virtual GraphicsTexture* GetResource(Graphics& graphics) override;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	GraphicsTexture m_texture;
};


class DepthStencilViewMultiResource : public DepthStencilViewBase
{
public:
	DepthStencilViewMultiResource(Graphics& graphics);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const override;

	virtual const GraphicsTexture* GetResource(Graphics& graphics) const override;
	virtual GraphicsTexture* GetResource(Graphics& graphics) override;

	const GraphicsTexture* GetResource(unsigned int i) const;
	GraphicsTexture* GetResource(unsigned int i);

private:
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_descriptors;
	std::vector<std::shared_ptr<GraphicsTexture>> m_textures;
};