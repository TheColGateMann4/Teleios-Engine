#pragma once
#include "includes/DirectXIncludes.h"

class DepthStencilState
{
public:
	DepthStencilState();

	D3D12_DEPTH_STENCIL_DESC Get();

private:
	D3D12_DEPTH_STENCIL_DESC m_desc;
};