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

class Pipeline;
class Camera;

class SceneObject
{
protected:
	SceneObject(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation);
	
public:
	virtual ~SceneObject() = default;

public:
	virtual void Draw(Graphics& graphics, Pipeline& pipeline) const = 0;

	virtual void Update(Graphics& graphics, Camera& camera) = 0;

	virtual void DrawImguiWindow(Graphics& graphics, bool isLayerVisible);

	virtual DirectX::XMMATRIX GetTransformMatrix() const = 0;

protected:
	std::unique_ptr<CommandList> m_bundleCommandList;
	std::unique_ptr<PipelineState> m_pipelineState;

	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<CachedConstantBuffer> constantBuffer;
	std::shared_ptr<TransformConstantBuffer> transformConstantBuffer;

protected:
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };
};