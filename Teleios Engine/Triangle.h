#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "PipelineState.h"
#include "CommandList.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Graphics;

class Triangle
{
public:
	Triangle(Graphics& graphics);

public:
	void Draw(Graphics& graphics) const;

private:
	std::unique_ptr<CommandList> m_bundleCommandList;
	std::unique_ptr<CommandList> m_directCommandList;

	std::unique_ptr<PipelineState> m_pipelineState;

	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<ConstantBuffer> constBuffer;
};