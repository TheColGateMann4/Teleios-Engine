#pragma once
#include "RenderPass.h"

class Graphics;
class ImguiLayer;

class GuiPass : public RenderPass
{
public:
	GuiPass(Graphics& graphics);

public:
	virtual void PreDraw(Graphics& graphics, CommandList* commandList) override;

private:
	ImguiLayer& m_imguiLayer;
};