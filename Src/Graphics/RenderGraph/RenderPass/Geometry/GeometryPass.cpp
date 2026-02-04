#include "GeometryPass.h"

#include "Scene/Scene.h"
#include "Scene/Objects/Camera.h"

#include "Graphics/Bindables/RootSignatureConstants.h"

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

const std::vector<std::shared_ptr<Bindable>>& GeometryPass::GetBindables() const
{
	return m_bindables;
}

void GeometryPass::SortJobs()
{
	// TODO: sort jobs by their PSO
}

RenderJob::JobType GeometryPass::GetWantedJob() const
{
	return RenderJob::JobType::None;
}

void GeometryPass::AssignJob(std::shared_ptr<RenderJob> pJob)
{
	m_pJobs.push_back(pJob);
}

void GeometryPass::ExecutePass(Graphics& graphics, CommandList* commandList)
{
	for (auto pJob : m_pJobs)
		pJob->Execute(graphics, commandList);
}