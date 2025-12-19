#pragma once
#include "Includes/CppIncludes.h"

#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/CommandList.h"
#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/BindableContainer.h"

#include "Includes/BindablesInclude.h"

class Graphics;
class Pipeline;
class Camera;

class Mesh
{
public:
	Mesh() = default;
	Mesh(Mesh&&) noexcept = default;
	Mesh(const Mesh&) = delete;

	virtual ~Mesh() = default;

public:
	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void DrawMesh(Graphics& graphics, Pipeline& pipeline) const;

	void InternalUpdate(Graphics& graphics, Pipeline& pipeline);

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	void DrawConstantBuffers(Graphics& graphics);

public: // bindable container functions
	void AddStaticBindable(const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	void SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

	void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

	const MeshBindableContainer& GetBindableContainter() const;

protected:
	MeshBindableContainer m_bindableContainer;

	std::unique_ptr<GraphicsPipelineState> m_pipelineState;
	std::unique_ptr<RootSignature> m_rootSignature;
};

