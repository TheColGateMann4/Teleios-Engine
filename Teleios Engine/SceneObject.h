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

#include "Bindable.h"

class Pipeline;
class Camera;

class SceneObject
{
protected:
	SceneObject(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation);
	
public:
	virtual ~SceneObject() = default;

protected:
	void AddStaticBindable(Pipeline& pipeline, const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	void SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

	void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

	void SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer);

private:
	void SegregateBindable(Bindable* bindable);

public:
	void Initialize(Graphics& graphics);

	void Draw(Graphics& graphics, Pipeline& pipeline) const;

	virtual void Update(Graphics& graphics);

	virtual void DrawImguiWindow(Graphics& graphics, bool isLayerVisible);

	void UpdateTransformMatrix(Graphics& graphics, Camera& camera);

	DirectX::XMMATRIX GetTransformMatrix() const;

protected:
	std::unique_ptr<CommandList> m_bundleCommandList;
	std::unique_ptr<PipelineState> m_pipelineState;

protected:
	std::vector<std::shared_ptr<Bindable>> m_bindables;

	std::vector<CommandListBindable*> m_commandListBindables;
	std::vector<DirectCommandListBindable*> m_directCommandListBindables;
	std::vector<RootSignatureBindable*> m_rootSignatureBindables;
	std::vector<PipelineStateBindable*> m_pipelineStateBindables;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	TransformConstantBuffer* m_transformConstantBuffer = nullptr;

protected:
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };
};