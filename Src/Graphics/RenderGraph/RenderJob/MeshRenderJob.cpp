#include "MeshRenderJob.h"

MeshRenderJob::MeshRenderJob(RenderJob::JobType m_type)
	:
	RenderJob(m_type)
{

}

void MeshRenderJob::Execute(Graphics& graphics, CommandList* commandList) const
{
	m_mesh.Draw(graphics, commandList);
}

void MeshRenderJob::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_mesh.Initialize(graphics, pipeline);
}

StandaloneMesh& MeshRenderJob::GetMesh()
{
	return m_mesh;
}