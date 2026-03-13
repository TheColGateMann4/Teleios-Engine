#include "ViewPort.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Core/CommandList.h"

#include "Graphics/Core/ResourceList.h"

ViewPort::ViewPort(Graphics& graphics, DirectX::XMFLOAT2 dimensions)
{
	if(dimensions.x == 0.0f)
		dimensions.x = graphics.GetWidth();
	if(dimensions.y == 0.0f)
		dimensions.y = graphics.GetHeight();

	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = dimensions.x;
	m_viewport.Height = dimensions.y;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_viewportRect.left = m_viewportRect.top = 0;
	m_viewportRect.bottom = dimensions.y;
	m_viewportRect.right = dimensions.x;
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

BindableType ViewPort::GetBindableType() const
{
	return BindableType::bindable_viewPort;
}

std::shared_ptr<ViewPort> ViewPort::GetResource(Graphics& graphics, DirectX::XMFLOAT2 dimensions)
{
	return ResourceList::GetResource<ViewPort>(graphics, dimensions);
}

std::string ViewPort::GetIdentifier(DirectX::XMFLOAT2 dimensions)
{
	std::string resultString = "ViewPort#";

	resultString += std::to_string(dimensions.x);
	resultString += std::to_string(dimensions.y);

	return resultString;
}