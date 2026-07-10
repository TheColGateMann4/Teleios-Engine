#pragma once
#include "FullscreenPass.h"

class FullscreenPlaceholderPass : public FullscreenPass
{
public:
	FullscreenPlaceholderPass(Graphics& graphics);

	virtual void Initialize(Graphics& graphics, Scene& scene) override;

	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

protected:
	virtual void PreDraw(Graphics& graphics, CommandList* commandList) override;
	virtual void PostDraw(Graphics& graphics, CommandList* commandList) override;

private:
	void UpdateCameraData(Graphics& graphics, Scene& scene);

private:
	std::shared_ptr<ShaderResourceViewMultiResource> m_renderTargetSRV;
	std::shared_ptr<ShaderResourceViewMultiResource> m_depthStencilSRV;

	std::shared_ptr<CachedConstantBuffer> m_pCameraData;
};