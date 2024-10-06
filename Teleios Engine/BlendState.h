#pragma once
#include "includes/DirectXIncludes.h"

class BlendState
{
public:
	BlendState();

	D3D12_BLEND_DESC Get();

private:
	D3D12_BLEND_DESC m_desc;
};

