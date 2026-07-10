#include "RenderGraphicsStep.h"

RenderGraphicsStep::RenderGraphicsStep()
	:
	RenderStep()
{

}

RenderGraphicsStep::RenderGraphicsStep(const std::string& name)
	:
	RenderStep(name)
{

}

void RenderGraphicsStep::AddStaticBindable(const char* bindableName)
{
	m_bindableContainer.AddStaticBindable(bindableName);
}

void RenderGraphicsStep::SetAttributeBufferEntry(std::shared_ptr<VertexBufferEntry> attributeBufferEntry)
{
	m_bindableContainer.SetAttributeBufferEntry(std::move(attributeBufferEntry));
}

void RenderGraphicsStep::SetPositionBufferEntry(std::shared_ptr<VertexBufferEntry> positionBufferEntry)
{
	m_bindableContainer.SetPositionBufferEntry(std::move(positionBufferEntry));
}

void RenderGraphicsStep::SetIndexBufferEntry(std::shared_ptr<IndexBufferEntry> indexBufferEntry)
{
	m_bindableContainer.SetIndexBufferEntry(std::move(indexBufferEntry));
}

void RenderGraphicsStep::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(std::move(bindable));
}

void RenderGraphicsStep::AddBindable(Bindable* bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

const MeshBindableContainer& RenderGraphicsStep::GetBindableContainer() const
{
	return m_bindableContainer;
}

void RenderGraphicsStep::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.Initialize(graphics, pipeline);
}

void RenderGraphicsStep::Update()
{
	m_bindableContainer.Update();
}