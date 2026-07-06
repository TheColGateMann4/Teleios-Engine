#pragma once
#include "Graphics/RenderGraph/RenderPass/RenderPass.h"
#include "Graphics/RenderGraph/RenderJob/GraphicsRenderData.h"
#include "Graphics/RenderGraph/RenderJob/GraphicsStepRenderJob.h"
#include "Graphics/Bindables/RasterizerState.h"
#include "Graphics/Core/BindableContainer.h"

class RenderJob;
class Material;

class RootSignatureConstants;

class GeometryPass : public RenderPass
{
public:
	GeometryPass();

	virtual void Initialize(Graphics& graphics, Scene& scene);

	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

public: // Handling for pass specific bindables
	void AddBindable(std::shared_ptr<Bindable> bindable);
	void AddStaticBindable(const char* staticBindableName);
	const BindableContainer& GetBindableContainer() const;

public: // job handling
	// sort jobs so executing them on GPU is more effecient
	void SortJobs();

public:  // enlisting and pushing jobs
	// every renderPass that will inherit will return its own wanted jobs, like "Albedo"
	virtual RenderJob::JobType GetWantedJob() const;

	// called by RenderManager to assign jobs that RenderPass enlisted for
	void AssignRenderData(GraphicsRenderData renderData);

	void GatherJobBindables();
	void InitializeJobs(Graphics& graphics, Pipeline& pipeline);

	RenderPassRasterizerStateOptions GetRasterizerOptions() const;

	unsigned int GetActiveCameraIndex() const;

protected:
	void SetCameraTransformIndex(unsigned int cameraIndex);

protected:
	virtual void ExecutePass(Graphics& graphics, CommandList* commandList, Scene& scene) override;

protected:
	std::shared_ptr<RootSignatureConstants> m_cameraRootConstant;

	BindableContainer m_bindableContainer;

	std::vector<std::unique_ptr<GraphicsStepRenderJob>> m_jobs;

	unsigned int m_currentCameraIndex = UINT_MAX;

	Material* currentlyBoundMaterial = nullptr;

	RenderPassRasterizerStateOptions m_rasterizerOptions = {};
};