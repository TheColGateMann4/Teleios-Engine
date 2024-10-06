#pragma once
#include "includes/DirectXIncludes.h"

class RasterizerState
{
public:
	RasterizerState();

	D3D12_RASTERIZER_DESC Get();

private:
	D3D12_RASTERIZER_DESC m_desc;
};

