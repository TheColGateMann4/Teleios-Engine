#pragma once
#include "FullscreenRenderPass.h"

class Graphics;

/*
class FogRenderPass : public FullscreenRenderPass
{
public:
	FogRenderPass(Graphics& graphics);

	virtual void Draw(Graphics& graphics, Pipeline& pipeline) override;

public:
	virtual void InternalInitialize(Graphics& graphics, Pipeline& pipeline) override;
	virtual void InternalUpdate(Graphics& graphics, Pipeline& pipeline) override;
	virtual void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline) override;

private:
	std::shared_ptr<Shader> m_fogPS;
	std::shared_ptr<CachedConstantBuffer> m_fogData;
};
*/