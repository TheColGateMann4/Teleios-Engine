#include "GeometryPass.h"

#include "Scene/Scene.h"
#include "Scene/Objects/Camera.h"

#include "Graphics/Bindables/RootSignatureConstants.h"

#include "Graphics/RenderGraph/RenderJob/GraphicsStepRenderJob.h"

#include "Graphics/Core/Graphics.h"

GeometryPass::GeometryPass()
{
	DynamicConstantBuffer::Layout layout;
	layout.Add<DynamicConstantBuffer::ElementType::Int>("cameraTransformIndex");

	layout.GetFinished(DynamicConstantBuffer::Layout::LayoutType::data);

	DynamicConstantBuffer::Data bufferData(layout);
	*bufferData.Get<DynamicConstantBuffer::ElementType::Int>("cameraTransformIndex") = 0;

	m_cameraRootConstant = std::make_shared<RootSignatureConstants>(bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::VertexShader, 2}});

	AddBindable(m_cameraRootConstant);
}

void GeometryPass::Initialize(Graphics& graphics, Scene& scene)
{
	Pipeline& pipeline = graphics.GetRenderer().GetPipeline();

	for(const auto* staticBindableName : m_staticBindables)
	{
		std::shared_ptr<Bindable> resolvedBindable = pipeline.GetStaticResource(staticBindableName);
		AddBindable(resolvedBindable);
	}
}

void GeometryPass::Update(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{
	unsigned int currentCameraIndex = scene.GetCurrentCamera()->GetCameraIndex();

	if (m_prevCameraIndex != currentCameraIndex)
	{
		*m_cameraRootConstant->GetData().Get<DynamicConstantBuffer::ElementType::Int>("cameraTransformIndex") = currentCameraIndex;
		m_prevCameraIndex = currentCameraIndex;
	}
}

void GeometryPass::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindables.push_back(bindable);
}

void GeometryPass::AddStaticBindable(const char* staticBindableName)
{
	m_staticBindables.push_back(staticBindableName);
}

const std::vector<std::shared_ptr<Bindable>>& GeometryPass::GetBindables() const
{
	return m_bindables;
}

void GeometryPass::SortJobs()
{
	std::sort(
		m_pJobs.begin(), m_pJobs.end(),
		[](const std::shared_ptr<GraphicsStepRenderJob>& a, const std::shared_ptr<GraphicsStepRenderJob>& b)
		{
			// currently sorting by pointers, our only goal for now is just grouping the same-material render jobs together
			return a->GetStep()->GetMaterial() < b->GetStep()->GetMaterial();
		}
	);
}

RenderJob::JobType GeometryPass::GetWantedJob() const
{
	return RenderJob::JobType::None;
}

void GeometryPass::AssignJob(std::shared_ptr<RenderJob> pJob)
{
	THROW_INTERNAL_ERROR_IF("Tried to push non-geometry render job to GeometryPass", pJob->GetGroup() != RenderJob::JobGroup::Geometry);

	std::shared_ptr<GraphicsStepRenderJob> graphicsJob = std::static_pointer_cast<GraphicsStepRenderJob>(pJob);

	m_pJobs.push_back(graphicsJob);
}

void GeometryPass::ExecutePass(Graphics& graphics, CommandList* commandList)
{
	for (auto pJob : m_pJobs)
	{
		pJob->Execute(graphics, commandList);
	}
}