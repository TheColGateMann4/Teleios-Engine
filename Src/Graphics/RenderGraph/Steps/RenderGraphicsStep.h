#pragma once
#include "RenderStep.h"
#include "Graphics/Core/BindableContainer.h"
#include "Includes/BindablesInclude.h"

class RenderGraphicsStep : public RenderStep
{
public:
	RenderGraphicsStep();
	RenderGraphicsStep(const std::string& name);

public:
	void AddStaticBindable(const char* bindableName);

	void SetAttributeBufferEntry(std::shared_ptr<VertexBufferEntry> attributeBufferEntry);

	void SetPositionBufferEntry(std::shared_ptr<VertexBufferEntry> positionBufferEntry);

	void SetIndexBufferEntry(std::shared_ptr<IndexBufferEntry> indexBufferEntry);

	void AddBindable(std::shared_ptr<Bindable> bindable);

	void AddBindable(Bindable* bindable);

	const MeshBindableContainer& GetBindableContainer() const;

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	virtual void Update() override;

protected:
	MeshBindableContainer m_bindableContainer;
};