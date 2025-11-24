#pragma once
#include "Mesh.h"
#include "LODMeshBindable.h"

class Graphics;
class Pipeline;

class LODMesh : public Mesh
{
public:
	LODMesh();
	LODMesh(LODMesh&&) noexcept = default;
	LODMesh(const LODMesh&) = delete;

	virtual ~LODMesh() override = default;

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	virtual void SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) override;
	virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) override;

	Buffer* GetBoundaryBoxMinGPUResource() const;
	Buffer* GetBoundaryBoxMaxGPUResource() const;

private:
	void GetModelBounds(Graphics& graphics, Pipeline& pipeline);

private:
	std::shared_ptr<Buffer> m_boundaryBoxMin;
	std::shared_ptr<Buffer> m_boundaryBoxMax;

	std::shared_ptr<LODMeshBindable> m_lodMeshBindable;
};