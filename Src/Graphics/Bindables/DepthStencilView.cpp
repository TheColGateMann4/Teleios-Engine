#include "DepthStencilView.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"

DepthStencilViewBase::DepthStencilViewBase(Graphics& graphics, unsigned int numDescriptors)
{
	HRESULT hr;

	// creating desriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDesc.NumDescriptors = numDescriptors;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;

		THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateDescriptorHeap(
			&descriptorHeapDesc,
			IID_PPV_ARGS(&m_descriptorHeap)
		));
	}
}

void DepthStencilViewBase::CreateDSV(Graphics& graphics, D3D12_CPU_DESCRIPTOR_HANDLE& descriptor, GraphicsTexture* texture)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = texture->GetFormat();
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateDepthStencilView(
		texture->GetResource(),
		&depthStencilViewDesc,
		descriptor
	));
}

void DepthStencilViewBase::CreateDSVArraySlice(Graphics& graphics, D3D12_CPU_DESCRIPTOR_HANDLE& descriptor, GraphicsTexture* texture, unsigned int arraySlice)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = texture->GetFormat();
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;
	depthStencilViewDesc.Texture2DArray.FirstArraySlice = arraySlice;
	depthStencilViewDesc.Texture2DArray.ArraySize = 1;

	THROW_INFO_ERROR(graphics.GetDeviceResources().GetDevice()->CreateDepthStencilView(
		texture->GetResource(),
		&depthStencilViewDesc,
		descriptor
	));
}

DepthStencilView::DepthStencilView(Graphics& graphics)
	:
	DepthStencilViewBase(graphics, 1),
	m_texture(graphics, GraphicsTextureDimensions(graphics.GetWidth(), graphics.GetHeight()), DXGI_FORMAT_D24_UNORM_S8_UINT, DepthStencilClearValue(1.0f, 0), GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
{
	CreateDSV(graphics, m_descriptor, &m_texture);
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilView::GetDescriptor(Graphics& graphics) const
{
	return m_descriptor;
}

const GraphicsTexture* DepthStencilView::GetResource(Graphics& graphics) const
{
	return &m_texture;
}

GraphicsTexture* DepthStencilView::GetResource(Graphics& graphics)
{
	return &m_texture;
}

DepthStencilType DepthStencilView::GetDepthStencilType() const
{
	return DepthStencilType::singleResource;
}

DXGI_FORMAT DepthStencilView::GetFormat() const
{
	return m_texture.GetFormat();
}

DepthStencilClearValue DepthStencilView::GetClearValue() const
{
	return m_texture.GetDepthStencilClearValue();
}

DepthStencilViewMultiResource::DepthStencilViewMultiResource(Graphics& graphics, DirectX::XMFLOAT2 dimensions)
	:
	DepthStencilViewBase(graphics, graphics.GetBufferCount())
{
	unsigned int numBuffers = graphics.GetBufferCount();

	m_textures.reserve(numBuffers);
	m_descriptors.resize(numBuffers);

	if (dimensions.x == 0.0f)
		dimensions.x = graphics.GetWidth();

	if (dimensions.y == 0.0f)
		dimensions.y = graphics.GetHeight();

	for(int i = 0; i < numBuffers; i++)
	{
		m_textures.push_back(std::make_shared<GraphicsTexture>(graphics, GraphicsTextureDimensions(dimensions.x, dimensions.y), DXGI_FORMAT_D24_UNORM_S8_UINT, DepthStencilClearValue(1.0f, 0), GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
		
		D3D12_CPU_DESCRIPTOR_HANDLE& descriptor = m_descriptors.at(i);
		descriptor = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		descriptor.ptr += graphics.GetDeviceResources().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV) * i;
		
		CreateDSV(graphics, descriptor, m_textures.at(i).get());
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilViewMultiResource::GetDescriptor(Graphics& graphics) const
{
	return m_descriptors.at(graphics.GetCurrentBufferIndex());
}

const GraphicsTexture* DepthStencilViewMultiResource::GetResource(Graphics& graphics) const
{
	return GetResource(graphics.GetCurrentBufferIndex());
}

GraphicsTexture* DepthStencilViewMultiResource::GetResource(Graphics& graphics)
{
	return GetResource(graphics.GetCurrentBufferIndex());
}

const GraphicsTexture* DepthStencilViewMultiResource::GetResource(unsigned int i) const
{
	return m_textures.at(i).get();
}

GraphicsTexture* DepthStencilViewMultiResource::GetResource(unsigned int i)
{
	return m_textures.at(i).get();
}

DepthStencilType DepthStencilViewMultiResource::GetDepthStencilType() const
{
	return DepthStencilType::multiResource;
}

DXGI_FORMAT DepthStencilViewMultiResource::GetFormat() const
{
	THROW_INTERNAL_ERROR_IF("Depth Stencil didn't own any resources", m_textures.empty());

	return m_textures.front()->GetFormat();
}

DepthStencilClearValue DepthStencilViewMultiResource::GetClearValue() const
{
	THROW_INTERNAL_ERROR_IF("Depth Stencil didn't own any resources", m_textures.empty());

	return m_textures.front()->GetDepthStencilClearValue();
}

DepthStencilViewCubeMultiResource::DepthStencilViewCubeMultiResource(Graphics& graphics)
	:
	DepthStencilViewBase(graphics, graphics.GetBufferCount() * 6) // 6 faces
{
	unsigned int numFaces = 6;
	unsigned int numBuffers = graphics.GetBufferCount();

	m_textures.reserve(numBuffers);
	m_descriptors.resize(numBuffers);

	for (int iFrame = 0; iFrame < graphics.GetBufferCount(); iFrame++)
	{
		m_textures.push_back(std::make_shared<GraphicsTexture>(graphics, GraphicsTextureDimensions(graphics.GetWidth(), graphics.GetWidth(), 1, 6), DXGI_FORMAT_D24_UNORM_S8_UINT, DepthStencilClearValue(1.0f, 0), GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));

		FrameDescriptor& frameDescriptors = m_descriptors.at(iFrame);
		frameDescriptors.faces.resize(numFaces);

		for (unsigned int iFace = 0; iFace < numFaces; iFace++)
		{
			auto& face = frameDescriptors.faces.at(iFace);

			face = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
			face.ptr += graphics.GetDeviceResources().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV) * ((iFrame * 6) + iFace);

			CreateDSVArraySlice(graphics, face, m_textures.at(iFrame).get(), iFace);
		}
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE& DepthStencilViewCubeMultiResource::GetDescriptor(Graphics& graphics) const
{
	return m_descriptors.at(graphics.GetCurrentBufferIndex()).faces.at(m_currentDepthBuffer);
}

const GraphicsTexture* DepthStencilViewCubeMultiResource::GetResource(Graphics& graphics) const
{
	return m_textures.at(graphics.GetCurrentBufferIndex()).get();
}

GraphicsTexture* DepthStencilViewCubeMultiResource::GetResource(Graphics& graphics)
{
	return m_textures.at(graphics.GetCurrentBufferIndex()).get();
}

const GraphicsTexture* DepthStencilViewCubeMultiResource::GetResource(unsigned int i) const
{
	return m_textures.at(i).get();
}

GraphicsTexture* DepthStencilViewCubeMultiResource::GetResource(unsigned int i)
{	
	return m_textures.at(i).get();
}

DepthStencilType DepthStencilViewCubeMultiResource::GetDepthStencilType() const
{
	return DepthStencilType::cubeMultiResource;
}

DXGI_FORMAT DepthStencilViewCubeMultiResource::GetFormat() const
{
	THROW_INTERNAL_ERROR_IF("Depth Stencil didn't own any resources", m_textures.empty());

	return m_textures.front()->GetFormat();
}

DepthStencilClearValue DepthStencilViewCubeMultiResource::GetClearValue() const
{
	THROW_INTERNAL_ERROR_IF("Depth Stencil didn't own any resources", m_textures.empty());

	return m_textures.front()->GetDepthStencilClearValue();
}

void DepthStencilViewCubeMultiResource::SetCurrentDepthBuffer(unsigned int newCurrentDepthBuffer)
{
	m_currentDepthBuffer = newCurrentDepthBuffer;
}