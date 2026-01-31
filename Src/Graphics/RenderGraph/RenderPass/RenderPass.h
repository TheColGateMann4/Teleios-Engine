#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Bindables/RenderTarget.h"
#include "Graphics/Bindables/DepthStencilView.h"
#include "Graphics/RenderGraph/RenderJob/RenderJob.h"

class Graphics;
class RenderManager;
class Scene;
class CommandList;
class RenderManager;

enum class ResourceDataOperation : uint8_t
{
	keep,
	clear,
	discard
};

class RenderPass
{
public:
	struct RenderTargetData
	{
		std::shared_ptr<RenderTarget> resource;
		ResourceDataOperation loadOperation = ResourceDataOperation::keep;
		ResourceDataOperation storeOperation = ResourceDataOperation::keep;
	};

	struct DepthStencilData
	{
		std::shared_ptr<DepthStencilViewBase> resource;
		ResourceDataOperation loadOperation = ResourceDataOperation::keep;
		ResourceDataOperation storeOperation = ResourceDataOperation::keep;
	};

public: 
	// resource initialization for passes with their own work
	virtual void Initialize(Graphics& graphics);
	virtual void InitializePassResources(Graphics& graphics, Pipeline& pipeline, Scene& scene);

	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene);

	virtual void SubmitJobs(RenderManager& renderManager);

	void SetCorrectStates(Graphics& graphics, CommandList* commandList);

	// record jobs on command list
	void Execute(Graphics& graphics, CommandList* commandList);

public: // RenderTargets and DepthStecilViews
	void AddRenderTarget(std::shared_ptr<RenderTarget> renderTarget, ResourceDataOperation loadop = ResourceDataOperation::keep, ResourceDataOperation storeop = ResourceDataOperation::keep);
	void SetDepthStencilView(std::shared_ptr<DepthStencilViewBase> depthStencil, ResourceDataOperation loadop = ResourceDataOperation::keep, ResourceDataOperation storeop = ResourceDataOperation::keep);

	const std::vector<RenderTargetData>& GetRenderTargets() const;
	DepthStencilData GetDepthStencilView() const;

protected:
	virtual void ExecutePass(Graphics& graphics, CommandList* commandList) = 0;

private:
	std::vector<RenderTargetData> m_renderTargets;
	DepthStencilData m_depthStencil;
};
