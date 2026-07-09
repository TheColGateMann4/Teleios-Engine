#pragma once
#include "Includes/CppIncludes.h"
#include "RenderGraphicsStep.h"

class RenderPass;

class RenderGraphicsFullscreenStep : public RenderGraphicsStep
{
public:
	RenderGraphicsFullscreenStep(RenderPass* renderPass, const std::string& name);
	RenderGraphicsFullscreenStep(RenderPass* renderPass);
};