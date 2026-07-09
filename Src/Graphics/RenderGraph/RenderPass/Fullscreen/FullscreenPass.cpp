#include "FullscreenPass.h"

#include "Scene/Objects/Camera.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Scene/Scene.h"
#include "Graphics/Core/TempCommandList.h"

FullscreenPass::FullscreenPass(Graphics& graphics)
{
	m_indexBufferEntry = IndexBufferEntry::GetResource(graphics, "FullscreenMesh", std::vector<unsigned int>{0, 1, 3, 0, 3, 2});

	DynamicVertex::DynamicVertexLayout layout;
	layout.AddElement<DynamicVertex::ElementType::Position>();
	layout.AddElement<DynamicVertex::ElementType::TextureCoords>();

	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoords;
	};

	std::vector<Vertice> vertices = {
		{{ -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }},
		{{ 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }},
		{{ -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f }},
		{{ 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }}
	};

	m_vertexBufferEntry = VertexBufferEntry::GetResource(graphics, "FullscreenMesh", vertices.data(), layout, vertices.size());

	m_bindables.push_back(InputLayout::GetResource(graphics, layout));

	m_step = std::make_shared<RenderGraphicsFullscreenStep>(this);
}

void FullscreenPass::Initialize(Graphics& graphics, Scene& scene)
{

}

void FullscreenPass::InitializePassResources(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{
	// initializing step
	m_step->Initialize(graphics, pipeline);

	m_job = std::make_shared<RenderGraphicsFullscreenJob>(GraphicsRenderData(RenderJob::JobType::None, m_step.get()), this);

	m_job->Initialize(graphics, pipeline);

	InitializeFullscreenResources(graphics, pipeline, scene);
}

void FullscreenPass::InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{

}

void FullscreenPass::Update(Graphics& graphics, Pipeline& pipeline, Scene& scene)
{

}

void FullscreenPass::AddBindable(std::shared_ptr<Bindable> bind)
{
	m_bindables.push_back(std::move(bind));
}

void FullscreenPass::ExecutePass(Graphics& graphics, CommandList* commandList, Scene& scene)
{
	commandList->BeginRenderPass(graphics, this);

	PreDraw(graphics, commandList);

	m_job->Execute(graphics, commandList);

	PostDraw(graphics, commandList);

	commandList->EndRenderPass(graphics);
}

void FullscreenPass::PreDraw(Graphics& graphics, CommandList* commandList)
{

}

void FullscreenPass::PostDraw(Graphics& graphics, CommandList* commandList)
{

}

void FullscreenPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{

}