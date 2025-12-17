#include "Pipeline.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"
#include "PostProcessing.h"
#include "ViewPort.h"

#include "IndexBuffer.h"

Pipeline::Pipeline(Graphics& graphics)
	:
	m_postProcessing(graphics, *this)
{
	m_graphicsCommandList = std::make_shared<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Pipeline::BeginRender(Graphics& graphics) const
{
	m_graphicsCommandList->Open(graphics);	// opening graphics command list and clearning allocator

	// setting correct states
	m_graphicsCommandList->SetAllResourcesStates(graphics, graphics.GetDepthStencil()->GetResource(graphics), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_graphicsCommandList->SetResourceState(graphics, graphics.GetBackBuffer()->GetTexture(graphics), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// setting render target
	m_graphicsCommandList->SetRenderTarget(graphics, graphics.GetBackBuffer(), graphics.GetDepthStencil());

	static ViewPort viewPort(graphics);

	viewPort.BindToCommandList(graphics, m_graphicsCommandList.get());

	m_graphicsCommandList->ClearRenderTargetView(graphics, graphics.GetBackBuffer()); // clearning render target from previous frames
	m_graphicsCommandList->ClearDepthStencilView(graphics, graphics.GetDepthStencil()); // clearning depth stencil from previous frames
}

void Pipeline::FinishRender(Graphics& graphics)
{
	m_graphicsCommandList->SetResourceState(graphics, graphics.GetSwapChainBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// drawing backbuffer on top of swapchain
	m_postProcessing.Finish(graphics, *this);

	m_graphicsCommandList->SetResourceState(graphics, graphics.GetSwapChainBuffer(), D3D12_RESOURCE_STATE_PRESENT);

	m_graphicsCommandList->Close(graphics); // closing graphics command list
}

void Pipeline::FinishInitialization(Graphics& graphics)
{
	m_postProcessing.Initialize(graphics, *this);

	GetGraphicCommandList()->Close(graphics);

	Execute(graphics);
}

void Pipeline::DrawEffectsImguiWindow(Graphics& graphics)
{
	m_postProcessing.Update(graphics, *this);
}

CommandList* Pipeline::GetGraphicCommandList() const
{
	return m_graphicsCommandList.get();
}

Bindable* Pipeline::GetStaticResource(const char* resourceName) const
{
	for (auto& typedResource : m_staticResources)
		if (strcmp(resourceName, typedResource.first) == 0)
			return typedResource.second;
	
	THROW_INTERNAL_ERROR("Could not find resource with given name");
}

void Pipeline::AddStaticResource(const char* resourceName, Bindable* bindable)
{
	THROW_OBJECT_STATE_ERROR_IF("Cannot add object with nullptr name", resourceName == nullptr);
	THROW_OBJECT_STATE_ERROR_IF("Cannot add typedObject without object itself", bindable == nullptr);

	m_staticResources.push_back({ resourceName, bindable });
}

Camera* Pipeline::GetCurrentCamera() const
{
	return m_camera;
}

void Pipeline::SetCurrentCamera(Camera* newCurrentCamera)
{
	m_camera = newCurrentCamera;
}

void Pipeline::AddBufferToCopyPipeline(GraphicsResource* dst, GraphicsResource* src)
{
	m_buffersToCopy.push_back({ dst , src});
}

void Pipeline::Execute(Graphics& graphics)
{
	ID3D12CommandList* pCommandLists[] = {m_graphicsCommandList->Get()};

	graphics.GetCommandQueue()->ExecuteCommandLists(_countof(pCommandLists), pCommandLists);
}

void Pipeline::ExecuteCopyCalls(Graphics& graphics)
{
	// temporarily we will be using graphic command list for copy calls
	for (auto& copyData : m_buffersToCopy)
		copyData.src->CopyResourcesTo(graphics, m_graphicsCommandList.get(), copyData.dst);

	m_buffersToCopy.clear();
}