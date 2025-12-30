#pragma once
#include "Includes/CppIncludes.h"

#include "RenderStep.h"

#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/BindableContainer.h"

class RenderGraphicsStep : public RenderStep
{
public:
	RenderGraphicsStep(const std::string& name);
	RenderGraphicsStep();
	RenderGraphicsStep(RenderGraphicsStep&&) noexcept = default;
	RenderGraphicsStep(const RenderGraphicsStep&) = delete;

public:
	void Initialize(Graphics& graphics, Pipeline& pipeline);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const override;

	void Update(Graphics& graphics, Pipeline& pipeline);

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	void DrawConstantBuffers(Graphics& graphics);

public: // bindable container functions
	void AddStaticBindable(const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	const MeshBindableContainer& GetBindableContainter() const;

private:
	MeshBindableContainer m_bindableContainer;

	std::unique_ptr<GraphicsPipelineState> m_pipelineState;
	std::unique_ptr<RootSignature> m_rootSignature;
};