#pragma once
#include "FullscreenRenderPass.h"

class LightningPass : public FullscreenRenderPass
{
public:
	LightningPass(Graphics& graphics, RenderManager& renderManager);

public:
	virtual void Initialize(Graphics& graphics) override;
	virtual void InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	virtual void PreDraw(Graphics& graphics, CommandList* commandList) override;
	virtual void PostDraw(Graphics& graphics, CommandList* commandList) override;

	virtual void InternalUpdate(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

private:
	void UpdateInverseProjectionMatrix(Graphics& graphics, Scene& scene);

private:
	CachedConstantBuffer* m_pInverseProjectionBuffer = nullptr;
	ShaderResourceViewMultiResource* rt0 = nullptr;
	ShaderResourceViewMultiResource* rt1 = nullptr;
	ShaderResourceViewMultiResource* rt2 = nullptr;
	ShaderResourceViewMultiResource* ds = nullptr;
};