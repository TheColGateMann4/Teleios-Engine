#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "GraphicsResource.h"

class Graphics;
class Pipeline;

class GraphicsTexture : public GraphicsResource
{
public:
	// default resource constructor for textures
	GraphicsTexture(Graphics& graphics, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, CPUAccess cpuAccess = CPUAccess::notavailable, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// Render Target resource constructor
	GraphicsTexture(Graphics& graphics, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, DirectX::XMFLOAT4 optimizedClearValue, CPUAccess cpuAccess = CPUAccess::notavailable, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// Depth Stencil resource constructor
	GraphicsTexture(Graphics& graphics, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, float depthClearValue, uint8_t stencilClearValue, CPUAccess cpuAccess = CPUAccess::notavailable, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

private:
	void Initialize(Graphics& graphics, D3D12_RESOURCE_FLAGS flags, D3D12_CLEAR_VALUE* clearValue);

public:
	void CopyResourcesMipTo(Graphics& graphics, CommandList* copyCommandList, GraphicsTexture* dst, unsigned int targetMip = 0);

	virtual D3D12_RESOURCE_STATES GetResourceState(unsigned int targetSubresource = 0) const override;
	virtual D3D12_RESOURCE_STATES GetResourceTargetState(unsigned int targetSubresource = 0) override;
	virtual void SetAllResourceStates(D3D12_RESOURCE_STATES newState) override;
	virtual void SetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource = 0) override;
	virtual void SetTargetResourceState(D3D12_RESOURCE_STATES newState, unsigned int targetSubresource = 0) override;

public:
	void Update(Graphics& graphics, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);
	void Update(Graphics& graphics, Pipeline& pipeline, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);

public:
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

private:
	void UpdateUsingTempResource(Graphics& graphics, Pipeline& pipeline, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);
	void UpdateLocalResource(Graphics& graphics, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);

	static int GetPixelSize(DXGI_FORMAT format);

private:
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_mipLevels;
	std::vector<ResourceStates> m_states;
};