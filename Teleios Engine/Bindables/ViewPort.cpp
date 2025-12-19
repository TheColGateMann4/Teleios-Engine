#include "ViewPort.h"

#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"

#include "BindableResourceList.h"

ViewPort::ViewPort(Graphics& graphics)
{
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = graphics.GetWidth();
	m_viewport.Height = graphics.GetHeight();
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_viewportRect.left = m_viewportRect.top = 0;
	m_viewportRect.bottom = graphics.GetHeight();
	m_viewportRect.right = graphics.GetWidth();
}

const D3D12_VIEWPORT& ViewPort::GetViewport() const
{
	return m_viewport;
}

const D3D12_RECT& ViewPort::GetViewportRect() const
{
	return m_viewportRect;
}

void ViewPort::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetViewPort(graphics, this);
}

std::shared_ptr<ViewPort> ViewPort::GetBindableResource(Graphics& graphics)
{
	return BindableResourceList::GetBindableResource<ViewPort>(graphics);
}

std::string ViewPort::GetIdentifier()
{
	std::string resultString = "ViewPort#";

	return resultString;
}