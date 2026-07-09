#pragma once
#include "FullscreenPass.h"

class LightningPass : public FullscreenPass
{
public:
	LightningPass(Graphics& graphics);

public:
	virtual void Initialize(Graphics& graphics, Scene& scene) override;
	virtual void InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	virtual void PreDraw(Graphics& graphics, CommandList* commandList) override;
	virtual void PostDraw(Graphics& graphics, CommandList* commandList) override;


private:
	void UpdateInverseProjectionMatrix(Graphics& graphics, Scene& scene);

private:
	CachedConstantBuffer* m_pInverseMatriesBuffer = nullptr;
	ShaderResourceViewMultiResource* rt0 = nullptr;
	ShaderResourceViewMultiResource* rt1 = nullptr;
	ShaderResourceViewMultiResource* rt2 = nullptr;
	ShaderResourceViewMultiResource* ds = nullptr;
	ShaderResourceViewMultiResource* shadowMap = nullptr;
};