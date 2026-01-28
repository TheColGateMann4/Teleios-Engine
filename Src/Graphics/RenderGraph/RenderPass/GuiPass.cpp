#include "GuiPass.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Imgui/ImguiLayer.h"

GuiPass::GuiPass(Graphics& graphics)
	:
	m_imguiLayer(graphics.GetRenderer().GetImguiLayer())
{

}

void GuiPass::ExecutePass(Graphics& graphics, CommandList* commandList)
{
	m_imguiLayer.Render();

	if (m_imguiLayer.IsVisible())
		m_imguiLayer.Draw(graphics, commandList);
}