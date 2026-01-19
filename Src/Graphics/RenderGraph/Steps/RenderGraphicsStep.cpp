#include "RenderGraphicsStep.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Bindables/IndexBuffer.h"
#include "Graphics/Bindables/VertexBuffer.h"
#include "Graphics/Bindables/ConstantBuffer.h"
#include "Graphics/Bindables/Texture.h"

RenderGraphicsStep::RenderGraphicsStep(const std::string& name)
	:
	RenderStep(name)
{

}

RenderGraphicsStep::RenderGraphicsStep()
	:
	RenderStep()
{

}

void RenderGraphicsStep::DrawConstantBuffers(Graphics& graphics)
{
	const std::vector<CachedConstantBuffer*>& cachedBuffers = m_bindableContainer.GetCachedBuffers();

	for (auto& cachedBuffer : cachedBuffers)
		cachedBuffer->DrawImguiProperties(graphics);
}

void RenderGraphicsStep::AddStaticBindable(const char* bindableName)
{
	m_bindableContainer.AddStaticBindable(bindableName);
}

void RenderGraphicsStep::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void RenderGraphicsStep::AddBindable(Bindable* bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

const MeshBindableContainer& RenderGraphicsStep::GetBindableContainter() const
{
	return m_bindableContainer;
}