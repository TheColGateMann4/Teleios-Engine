#pragma once
#include "Bindable.h"

class Graphics;
class Pipeline;

class ViewPort : public Bindable, public CommandListBindable
{
public:
	ViewPort(Graphics& graphics, DirectX::XMFLOAT2 dimensions = {});

public:
	const D3D12_VIEWPORT& GetViewport() const;
	const D3D12_RECT& GetViewportRect() const;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual BindableType GetBindableType() const override;

public:
	static std::shared_ptr<ViewPort> GetResource(Graphics& graphics, DirectX::XMFLOAT2 dimensions = {});

	static std::string GetIdentifier(DirectX::XMFLOAT2 dimensions);

private:
	D3D12_VIEWPORT m_viewport = {};
	D3D12_RECT m_viewportRect = {};
};