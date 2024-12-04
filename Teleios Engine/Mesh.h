#pragma once
#include "includes/CppIncludes.h"

#include "PipelineState.h"
#include "CommandList.h"
#include "RootSignature.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "TransformConstantBuffer.h"

#include "Bindable.h"

class Graphics;
class Pipeline;
class Camera;

class Mesh
{
public:
	Mesh(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation);
	Mesh(Mesh&&) noexcept = default;
	virtual ~Mesh() = default;

	Mesh(const Mesh&) = delete;

public:
	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void RecordBundleList(Graphics& graphics);

	void DrawMesh(Graphics& graphics, Pipeline& pipeline) const;

	void SetPosition(DirectX::XMFLOAT3 position);

	DirectX::XMFLOAT3& GetPositionLVal();

	DirectX::XMFLOAT3& GetRotationLVal();

	void InternalUpdate(Graphics& graphics);

public:
	void UpdateTransformMatrix(Graphics& graphics, Camera& camera);

	DirectX::XMMATRIX GetTransformMatrix() const;

	bool TransformChanged() const;

	void SetTransformChanged(bool val);

public:
	void AddStaticBindable(const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	void SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

	void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

	void SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer);

protected:
	void SegregateBindable(Bindable* bindable);

	void SegregateBindableOnStartingPos(Bindable* bindable);

public:
	VertexBuffer* GetVertexBuffer();

	IndexBuffer* GetIndexBuffer();

protected:
	std::unique_ptr<GraphicsPipelineState> m_pipelineState;
	std::unique_ptr<RootSignature> m_rootSignature;

protected:
	std::vector<std::shared_ptr<Bindable>> m_bindables;

	std::vector<CommandListBindable*> m_commandListBindables;
	std::vector<DirectCommandListBindable*> m_directCommandListBindables;
	std::vector<RootSignatureBindable*> m_rootSignatureBindables;
	std::vector<PipelineStateBindable*> m_pipelineStateBindables;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	TransformConstantBuffer* m_transformConstantBuffer = nullptr;
	
	std::vector<const char*> m_staticBindableNames;

protected:
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };

	bool m_transformChanged = true;
};

