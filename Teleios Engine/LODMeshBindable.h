#pragma once
#include "Bindable.h"

class Graphics;
class Pipeline;
class VertexBuffer;
class IndexBuffer;

class LODMesh;

class LODMeshBindable : public Bindable, public CommandListBindable
{
public:
	void Initialize(Graphics& graphics, Pipeline& pipeline, LODMesh* mesh);

public:
	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;
	
	void SetLODLevel(int lodLevel);

	VertexBuffer* GetVertexBuffer(int lod);
	IndexBuffer* GetIndexBuffer(int lod);

private:
	void SimplyfyMesh(Graphics& graphics, Pipeline& pipeline, VertexBuffer* targetVertexBuffer, IndexBuffer* targetIndexBuffer, int lod);

	int GetLODLevel(Graphics& graphics);

private:
	LODMesh* m_mesh;

	static constexpr int m_LODLevels = 9;
	std::vector<VertexBuffer> m_vertexBufferLODs = {};
	std::vector<IndexBuffer> m_indexBufferLODs = {};

	DirectX::XMUINT3 m_gridResolution = { 64, 64, 64 };
};