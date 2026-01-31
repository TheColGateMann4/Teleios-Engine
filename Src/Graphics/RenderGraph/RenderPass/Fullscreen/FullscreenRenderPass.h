#pragma once
#include "Includes/BindablesInclude.h"
#include "Scene/StandaloneMesh.h"
#include "Graphics/RenderGraph/RenderPass/RenderPass.h"
#include "Graphics/RenderGraph/RenderJob/MeshRenderJob.h"

class FullscreenRenderPass : public RenderPass
{
public:
	FullscreenRenderPass(Graphics& graphics, RenderManager& renderManager);
	virtual ~FullscreenRenderPass() = default;

	virtual void Initialize(Graphics& graphics, Scene& scene) override;
	virtual void InitializePassResources(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;
	virtual void InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline, Scene& scene);
	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	void AddBindable(std::shared_ptr<Bindable> bind);

public: // Execution
	virtual void ExecutePass(Graphics& graphics, CommandList* commandList) override;
	virtual void PreDraw(Graphics& graphics, CommandList* commandList);
	virtual void PostDraw(Graphics& graphics, CommandList* commandList);

	virtual void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline);

protected:
	virtual void InternalUpdate(Graphics& graphics, Pipeline& pipeline, Scene& scene);

	void UpdateCameraData(Graphics& graphics, Scene& scene);

protected:
	std::shared_ptr<MeshRenderJob> m_meshRenderJob;

	std::vector<std::shared_ptr<Bindable>> m_bindables;

	std::shared_ptr<ShaderResourceViewMultiResource> m_renderTargetSRV;
	std::shared_ptr<ShaderResourceViewMultiResource> m_depthStencilSRV;

	std::shared_ptr<CachedConstantBuffer> m_pCameraData;
	IndexBuffer* m_pIndexBuffer;
	VertexBuffer* m_pVertexBuffer;
};