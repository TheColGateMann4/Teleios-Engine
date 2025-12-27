#pragma once
#include "FullscreenRenderPass.h"

class Graphics;

class DepthOfFieldRenderPass : public FullscreenRenderPass
{
public:
	DepthOfFieldRenderPass(Graphics& graphics);

	virtual void Draw(Graphics& graphics, Pipeline& pipeline) override;

	virtual void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline) override;

private:
	virtual void InternalInitialize(Graphics& graphics, Pipeline& pipeline) override;
	virtual void InternalUpdate(Graphics& graphics, Pipeline& pipeline) override;

private:
	std::shared_ptr<Shader> m_depthOfFieldPS;
	std::shared_ptr<Shader> m_depthFromScreenCS;
	std::shared_ptr<CachedConstantBuffer> m_depthOfFieldData;
	std::shared_ptr<UnorderedAccessView> m_DepthBufferUAV;
	std::shared_ptr<ShaderResourceView> m_DepthBufferSRV;
	std::shared_ptr<GraphicsBuffer> m_DepthBuffer;
};