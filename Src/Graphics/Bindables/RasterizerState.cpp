#include "RasterizerState.h"
#include "Graphics/Core/PipelineState.h"

#include "Graphics/Core//ResourceList.h"
#include "Macros/ErrorMacros.h"

std::string RenderPassRasterizerStateOptions::GetIdentifier()
{
	std::string resultString = "RenderPassRasterizerStateOptions#";

	resultString += std::to_string(m_isShadowRasterizer);

	return resultString;
}

bool RenderPassRasterizerStateOptions::GetIsShadowRasterizer() const
{
	return m_isShadowRasterizer;
}

void RenderPassRasterizerStateOptions::SetIsShadowRasterizer(bool isShadowRasterizer)
{
	m_isShadowRasterizer = isShadowRasterizer;
}

std::string ObjectRasterizerStateOptions::GetIdentifier()
{
	std::string resultString = "ObjectRasterizerStateOptions#";

	resultString += std::to_string(static_cast<std::underlying_type_t<decltype(m_cullingMode)>>(m_cullingMode));

	return resultString;
}

D3D12_CULL_MODE ObjectRasterizerStateOptions::GetCullMode() const
{
	switch (m_cullingMode)
	{
		case CullingMode::cull_none:
			return D3D12_CULL_MODE_NONE;
		case CullingMode::cull_front:
			return D3D12_CULL_MODE_FRONT;
		case CullingMode::cull_back:
			return D3D12_CULL_MODE_BACK;
	}

	THROW_INTERNAL_ERROR("Unmapped culling mode");
}

void ObjectRasterizerStateOptions::SetCulling(CullingMode cullingMode)
{
	m_cullingMode = cullingMode;
}

RasterizerState::RasterizerState(Graphics& graphics, RenderPassRasterizerStateOptions renderPassOptions, ObjectRasterizerStateOptions objectOptions)
{
	bool isShadowRasterizer = renderPassOptions.GetIsShadowRasterizer();

	m_desc = {};
	m_desc.FillMode = D3D12_FILL_MODE_SOLID;
	m_desc.CullMode = objectOptions.GetCullMode();
	m_desc.FrontCounterClockwise = false;
	m_desc.DepthBias = isShadowRasterizer ? 40 : 0;
	m_desc.DepthBiasClamp = isShadowRasterizer ? 0.00365f : 0.0f;
	m_desc.SlopeScaledDepthBias = isShadowRasterizer ? 3.464f : 0.0f;
	m_desc.DepthClipEnable = true;
	m_desc.MultisampleEnable = false;
	m_desc.AntialiasedLineEnable = false;
	m_desc.ForcedSampleCount = 0;
	m_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

std::shared_ptr<RasterizerState> RasterizerState::GetResource(Graphics& graphics, RenderPassRasterizerStateOptions renderPassOptions, ObjectRasterizerStateOptions objectOptions)
{
	return ResourceList::GetResource<RasterizerState>(graphics, renderPassOptions, objectOptions);
}

std::string RasterizerState::GetIdentifier(RenderPassRasterizerStateOptions renderPassOptions, ObjectRasterizerStateOptions objectOptions)
{
	std::string resultString = "RasterizerState#";

	resultString += renderPassOptions.GetIdentifier();
	resultString += objectOptions.GetIdentifier();

	return resultString;
}

D3D12_RASTERIZER_DESC RasterizerState::Get()
{
	return m_desc;
}

void RasterizerState::AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params)
{
	params->SetRasterizerState(this);
}

BindableType RasterizerState::GetBindableType() const
{
	return BindableType::bindable_rasterizerState;
}