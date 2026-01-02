#pragma once
#include "Includes/BindablesInclude.h"
#include "Scene/StandaloneMesh.h"
#include "Graphics/RenderGraph/RenderPass/RenderPass.h"

class Graphics;
class RenderManager;

class FullscreenRenderPass : public RenderPass
{
public:
	FullscreenRenderPass(Graphics& graphics, RenderManager& renderManager);
	virtual ~FullscreenRenderPass() = default;

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline, RenderManager& renderManager) override;
	void Update(Graphics& graphics, Pipeline& pipeline);

	virtual void PreDraw(Graphics& graphics, CommandList* commandList) override;
	virtual void PostDraw(Graphics& graphics, CommandList* commandList) override;

	virtual RenderJob::JobType GetWantedJob() const override;

	virtual void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline);

private:
	virtual void InternalUpdate(Graphics& graphics, Pipeline& pipeline);

protected:
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<InputLayout> m_inputLayout;

	std::shared_ptr<ShaderResourceViewMultiResource> m_renderTargetSRV;
	std::shared_ptr<ShaderResourceViewMultiResource> m_depthStencilSRV;

	std::shared_ptr<CachedConstantBuffer> m_cameraData;
};