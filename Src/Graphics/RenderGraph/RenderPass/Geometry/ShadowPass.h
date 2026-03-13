#pragma once
#include "GeometryPass.h"

class PointLight;

class ShadowPass : public GeometryPass
{
public:
	ShadowPass(Graphics& graphics);

	virtual void Initialize(Graphics& graphics, Scene& scene) override;

	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	virtual RenderJob::JobType GetWantedJob() const override;

	virtual void ExecutePass(Graphics& graphics, CommandList* commandList) override;

private:
	void SetActiveShadowCameraIndex(PointLight* pointLight, unsigned int stage);

private:
	Scene* m_scene;
};