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

StandaloneMesh& MeshRenderJob::GetMesh()
{
	return m_mesh;
}