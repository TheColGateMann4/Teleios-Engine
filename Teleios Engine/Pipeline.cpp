#include "Pipeline.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"

Pipeline::Pipeline(Graphics& graphics)
{
	m_graphicsCommandList = std::make_shared<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Pipeline::BeginRender(Graphics& graphics) const
{
	m_graphicsCommandList->Open(graphics);	// opening graphics command list and clearning allocator

	// setting render target
	m_graphicsCommandList->SetRenderTarget(graphics, graphics.GetBackBuffer(), graphics.GetDepthStencil());

	m_graphicsCommandList->SetResourceState(graphics, graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET); // setting backbuffer state to renderTarget on drawing time

	{
		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = graphics.GetWidth();
		viewport.Height = graphics.GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_graphicsCommandList->Get()->RSSetViewports(1, &viewport); // setting viewports

		D3D12_RECT viewportRect = {};
		viewportRect.left = viewportRect.top = 0;
		viewportRect.bottom = graphics.GetHeight();
		viewportRect.right = graphics.GetWidth();

		m_graphicsCommandList->Get()->RSSetScissorRects(1, &viewportRect); // setting scissor rects
	}

	m_graphicsCommandList->ClearRenderTargetView(graphics, graphics.GetBackBuffer()); // clearning render target from previous frames
	m_graphicsCommandList->ClearDepthStencilView(graphics, graphics.GetDepthStencil()); // clearning depth stencil from previous frames
}

void Pipeline::FinishRender(Graphics& graphics) const
{
	m_graphicsCommandList->SetResourceState(graphics, graphics.GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT); // setting backbuffer state to present since we finished drawing
	m_graphicsCommandList->Close(graphics); // closing graphics command list
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

void Pipeline::AddBufferToCopyPipeline(Buffer* dst, Buffer* src)
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