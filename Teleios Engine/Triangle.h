#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

#include "PipelineState.h"
#include "CommandList.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "TransformConstantBuffer.h"

class Graphics;
class Pipeline;
class Camera;

class Triangle
{
public:
	Triangle(Graphics& graphics, Pipeline& pipeline);

public:
	void Draw(Graphics& graphics, Pipeline& pipeline) const;

	void Update(Graphics& graphics, Camera& camera);

	void DrawImguiWindow(Graphics& graphics);

	DirectX::XMMATRIX GetTransformMatrix() const;

private:
	std::unique_ptr<CommandList> m_bundleCommandList;
	std::unique_ptr<PipelineState> m_pipelineState;

	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<CachedConstantBuffer> constantBuffer;
	std::shared_ptr<TransformConstantBuffer> transformConstantBuffer;

private:
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };
};