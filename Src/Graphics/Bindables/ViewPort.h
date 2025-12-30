#pragma once
#include "Bindable.h"

class Graphics;
class Pipeline;

class ViewPort : public Bindable, public CommandListBindable
{
public:
	ViewPort(Graphics& graphics);

public:
	const D3D12_VIEWPORT& GetViewport() const;
	const D3D12_RECT& GetViewportRect() const;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

public:
	static std::shared_ptr<ViewPort> GetBindableResource(Graphics& graphics);

	static std::string GetIdentifier();

private:
	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_viewportRect = {};
};