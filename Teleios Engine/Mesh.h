#pragma once
#include "includes/CppIncludes.h"

#include "PipelineState.h"
#include "CommandList.h"
#include "RootSignature.h"
#include "BindableContainer.h"

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

	void InternalUpdate(Graphics& graphics, Pipeline& pipeline);

	void DrawConstantBuffers(Graphics& graphics);

public:
	void UpdateTransformMatrix(Graphics& graphics, Camera& camera);

	DirectX::XMMATRIX GetTransformMatrix() const;

	bool TransformChanged() const;

	void SetTransformChanged(bool val);

public: // bindable container functions
	void AddStaticBindable(const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	void SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

	void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

	void SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer);

	const BindableContainer& GetBindableContainter() const;

protected:
	BindableContainer m_bindableContainer;

	std::unique_ptr<GraphicsPipelineState> m_pipelineState;
	std::unique_ptr<RootSignature> m_rootSignature;

protected:
	DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };

	bool m_transformChanged = true;
};

