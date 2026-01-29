#pragma once
#include "RenderJob.h"
#include "Scene/StandaloneMesh.h"


class MeshRenderJob : public RenderJob
{
public:
	MeshRenderJob(RenderJob::JobType m_type);

public:
	virtual void Execute(Graphics& graphics, CommandList* commandList) const override;

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	StandaloneMesh& GetMesh();

private:
	StandaloneMesh m_mesh;
};