#include "RasterizerState.h"

RasterizerState::RasterizerState()
{
	m_desc = {};
	m_desc.FillMode = D3D12_FILL_MODE_SOLID; // D3D12_FILL_MODE_WIREFRAME will be cool
	m_desc.CullMode = D3D12_CULL_MODE_BACK;  // do not draw triangles that are back-facing
	m_desc.FrontCounterClockwise = false;
	m_desc.DepthBias = 0;
	m_desc.DepthBiasClamp = 0.0f;
	m_desc.SlopeScaledDepthBias = 0.0f;
	m_desc.DepthClipEnable = true;
	m_desc.MultisampleEnable = false;
	m_desc.AntialiasedLineEnable = false;
	m_desc.ForcedSampleCount = 0;			 // not forced sample count
	m_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
}

D3D12_RASTERIZER_DESC RasterizerState::Get()
{
	return m_desc;
}