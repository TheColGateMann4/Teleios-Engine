#include "RasterizerState.h"
#include "Graphics/Core/PipelineState.h"

#include "Graphics/Core//BindableResourceList.h"

RasterizerState::RasterizerState(Graphics& graphics, bool twoSided)
{
	m_desc = {};
	m_desc.FillMode = D3D12_FILL_MODE_SOLID; // D3D12_FILL_MODE_WIREFRAME will be cool
	m_desc.CullMode = twoSided ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
	m_desc.FrontCounterClockwise = false;
	m_desc.DepthBias = 0;
	m_desc.DepthBiasClamp = 0.0f;
	m_desc.SlopeScaledDepthBias = 0.0f;
	m_desc.DepthClipEnable = true;
	m_desc.MultisampleEnable = false;
	m_desc.AntialiasedLineEnable = false;
	m_desc.ForcedSampleCount = 0;			 // not forced sample count
	m_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

std::shared_ptr<RasterizerState> RasterizerState::GetBindableResource(Graphics& graphics, bool twoSided)
{
	return BindableResourceList::GetBindableResource<RasterizerState>(graphics, twoSided);
}

std::string RasterizerState::GetIdentifier(bool twoSided)
{
	std::string resultString = "RasterizerState#";

	resultString += std::to_string(twoSided);

	return resultString;
}

D3D12_RASTERIZER_DESC RasterizerState::Get()
{
	return m_desc;
}

void RasterizerState::BindToPipelineState(Graphics& graphics, GraphicsPipelineState* pipelineState)
{
	pipelineState->SetRasterizerState(this);
}

BindableType RasterizerState::GetBindableType() const
{
	return BindableType::bindable_rasterizerState;
}