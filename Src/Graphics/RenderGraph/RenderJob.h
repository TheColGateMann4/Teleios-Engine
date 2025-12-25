#pragma once
#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"

class RenderJob
{
public:
	RenderJob(const std::string& categoryName, RenderStep* step);

public:
	void Execute(Graphics& graphics, CommandList* commandList) const;

private:
	std::string m_categoryName;
	RenderStep* m_step;
};