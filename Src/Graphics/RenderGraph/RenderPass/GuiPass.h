#pragma once
#include "RenderPass.h"

class Graphics;
class ImguiLayer;

class GuiPass : public RenderPass
{
public:
	GuiPass(Graphics& graphics);

public:
	virtual void ExecutePass(Graphics& graphics, CommandList* commandList, Scene& scene) override;

private:
	ImguiLayer& m_imguiLayer;
};