#pragma once
#include "RenderJob.h"
#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/RootSignatureLayout.h"

class Graphics;
class Pipeline;

class GraphicsRenderJob : public RenderJob
{
public:
	GraphicsRenderJob(RenderJob::JobType m_type);

	virtual void Execute(Graphics& graphics, CommandList* commandList) const = 0;

protected:
	std::shared_ptr<GraphicsPipelineState> m_pipelineState;
	std::shared_ptr<RootSignature> m_rootSignature;

	RootSignatureLayout m_rootSignatureLayout = {};
};