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

class StandaloneMesh
{
public:
	StandaloneMesh() = default;
	StandaloneMesh(StandaloneMesh&&) noexcept = default;
	StandaloneMesh(const StandaloneMesh&) = delete;

	virtual ~StandaloneMesh() = default;

public:
	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void Draw(Graphics& graphics, Pipeline& pipeline) const;

	void Update(Graphics& graphics, Pipeline& pipeline);

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	void DrawConstantBuffers(Graphics& graphics);

public: // bindable container functions
	void AddStaticBindable(const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	const MeshBindableContainer& GetBindableContainter() const;

protected:
	MeshBindableContainer m_bindableContainer;

	std::unique_ptr<GraphicsPipelineState> m_pipelineState;
	std::unique_ptr<RootSignature> m_rootSignature;
};