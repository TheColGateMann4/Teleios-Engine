#pragma once
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"

struct GraphicsRenderData
{
	RenderJob::JobType type;
	class RenderGraphicsStep* step;
};