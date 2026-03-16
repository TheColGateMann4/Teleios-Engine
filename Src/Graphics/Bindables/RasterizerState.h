#pragma once
#include "Includes/DirectXIncludes.h"
#include "Bindable.h"

class Graphics;
class GraphicsPipelineState;

class RenderPassRasterizerStateOptions
{
public:
	std::string GetIdentifier();

	bool GetIsShadowRasterizer() const;

	void SetIsShadowRasterizer(bool isShadowRasterizer);

private:
	bool m_isShadowRasterizer = false;
};

enum class CullingMode
{
	cull_none,
	cull_front,
	cull_back
};

class ObjectRasterizerStateOptions
{
public:
	std::string GetIdentifier();

	D3D12_CULL_MODE GetCullMode() const;

	void SetCulling(CullingMode cullingMode);

private:
	CullingMode m_cullingMode = CullingMode::cull_back;
};

class RasterizerState : public Bindable, public PipelineStateBindable
{
public:
	RasterizerState(Graphics& graphics, RenderPassRasterizerStateOptions renderPassOptions, ObjectRasterizerStateOptions objectOptions);

public:
	static std::shared_ptr<RasterizerState> GetResource(Graphics& graphics, RenderPassRasterizerStateOptions renderPassOptions, ObjectRasterizerStateOptions objectOptions);

	static std::string GetIdentifier(RenderPassRasterizerStateOptions renderPassOptions, ObjectRasterizerStateOptions objectOptions);

public:
	D3D12_RASTERIZER_DESC Get();

	virtual void AddPipelineStateParam(Graphics& graphics, GraphicsPipelineStateParams* params) override;

	virtual BindableType GetBindableType() const override;

private:
	D3D12_RASTERIZER_DESC m_desc;
};

