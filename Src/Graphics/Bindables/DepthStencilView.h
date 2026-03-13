#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Graphics/Resources/GraphicsTexture.h"

class Graphics;

enum class DepthStencilType
{
	unknown,
	singleResource,
	multiResource,
	cubeMultiResource
};

class DepthStencilViewBase
{
protected:
	DepthStencilViewBase(Graphics& graphics, unsigned int numDescriptors);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const = 0;

	virtual const GraphicsTexture* GetResource(Graphics& graphics) const = 0;
	virtual GraphicsTexture* GetResource(Graphics& graphics) = 0;

	virtual DepthStencilType GetDepthStencilType() const = 0;

	virtual DXGI_FORMAT GetFormat() const = 0;

	virtual DepthStencilClearValue GetClearValue() const = 0;

protected:
	void CreateDSV(Graphics& graphics, D3D12_CPU_DESCRIPTOR_HANDLE& descriptor, GraphicsTexture* texture);
	void CreateDSVArraySlice(Graphics& graphics, D3D12_CPU_DESCRIPTOR_HANDLE& descriptor, GraphicsTexture* texture, unsigned int arraySlice);

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

	virtual DepthStencilType GetDepthStencilType() const override;

	virtual DXGI_FORMAT GetFormat() const override;

	virtual DepthStencilClearValue GetClearValue() const override;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_descriptor;
	GraphicsTexture m_texture;
};


class DepthStencilViewMultiResource : public DepthStencilViewBase
{
public:
	DepthStencilViewMultiResource(Graphics& graphics, DirectX::XMFLOAT2 dimensions = {});

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const override;

	virtual const GraphicsTexture* GetResource(Graphics& graphics) const override;
	virtual GraphicsTexture* GetResource(Graphics& graphics) override;

	const GraphicsTexture* GetResource(unsigned int i) const;
	GraphicsTexture* GetResource(unsigned int i);

	virtual DepthStencilType GetDepthStencilType() const override;

	virtual DXGI_FORMAT GetFormat() const override;

	virtual DepthStencilClearValue GetClearValue() const override;

private:
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_descriptors;
	std::vector<std::shared_ptr<GraphicsTexture>> m_textures;
};

class DepthStencilViewCubeMultiResource : public DepthStencilViewBase
{
public:
	DepthStencilViewCubeMultiResource(Graphics& graphics);

public:
	virtual const D3D12_CPU_DESCRIPTOR_HANDLE& GetDescriptor(Graphics& graphics) const override;

	virtual const GraphicsTexture* GetResource(Graphics& graphics) const override;
	virtual GraphicsTexture* GetResource(Graphics& graphics) override;

	const GraphicsTexture* GetResource(unsigned int i) const;
	GraphicsTexture* GetResource(unsigned int i);

	virtual DepthStencilType GetDepthStencilType() const override;

	virtual DXGI_FORMAT GetFormat() const override;

	virtual DepthStencilClearValue GetClearValue() const override;

private:
	struct FrameDescriptor
	{
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> faces;
	};
	
	std::vector<FrameDescriptor> m_descriptors;
	std::vector<std::shared_ptr<GraphicsTexture>> m_textures;
};