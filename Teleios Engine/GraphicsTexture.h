#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "GraphicsResource.h"

class Graphics;
class Pipeline;

class GraphicsTexture : public GraphicsResource
{
public:
	enum class Flags
	{
		None,

	};

public:
	GraphicsTexture(Graphics& graphics, unsigned int width, unsigned int height, unsigned int mipLevels, DXGI_FORMAT format, CPUAccess cpuAccess = CPUAccess::notavailable, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

public:
	void CopyResourcesMipTo(Graphics& graphics, CommandList* copyCommandList, GraphicsResource* dst, unsigned int targetMip = 0);

public:
	void Update(Graphics& graphics, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);
	void Update(Graphics& graphics, Pipeline& pipeline, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);

public:
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

	D3D12_RESOURCE_STATES GetResourceMipState(unsigned int mip) const;
	void SetResourceMipState(D3D12_RESOURCE_STATES newState, unsigned int mip);

private:
	void UpdateUsingTempResource(Graphics& graphics, Pipeline& pipeline, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);
	void UpdateLocalResource(Graphics& graphics, const void* data, unsigned int width, unsigned int height, DXGI_FORMAT format);

	static int GetPixelSize(DXGI_FORMAT format);

private:
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_mipLevels;
	std::vector<D3D12_RESOURCE_STATES> m_mipStates;
};