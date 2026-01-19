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
	void DrawConstantBuffers(Graphics& graphics);

public: // bindable container functions
	void AddStaticBindable(const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	void AddBindable(Bindable* bindable);

	const MeshBindableContainer& GetBindableContainter() const;

private:
	MeshBindableContainer m_bindableContainer;
};