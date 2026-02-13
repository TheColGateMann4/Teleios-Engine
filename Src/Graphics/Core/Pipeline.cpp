#include "Pipeline.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"

#include "Graphics/Bindables/ViewPort.h"
#include "Graphics/Bindables/IndexBuffer.h"

void Pipeline::Initialize(Graphics& graphics)
{
	m_graphicsCommandList = std::make_shared<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Pipeline::BeginRender(Graphics& graphics) const
{
	m_graphicsCommandList->Open(graphics);	// opening graphics command list and clearning allocator

	static ViewPort viewPort(graphics);

	viewPort.BindToCommandList(graphics, m_graphicsCommandList.get());
}

void Pipeline::FinishRender(Graphics& graphics)
{
	m_graphicsCommandList->SetResourceState(graphics, graphics.GetSwapChainBuffer().get(), D3D12_RESOURCE_STATE_PRESENT);

	m_graphicsCommandList->Close(graphics); // closing graphics command list
}

void Pipeline::FinishInitialization(Graphics& graphics)
{
	GetGraphicCommandList()->Close(graphics);

	Execute(graphics);
}

CommandList* Pipeline::GetGraphicCommandList() const
{
	return m_graphicsCommandList.get();
}

std::shared_ptr<Bindable> Pipeline::GetStaticResource(const char* resourceName) const
{
	for (auto& typedResource : m_staticResources)
		if (strcmp(resourceName, typedResource.first) == 0)
			return typedResource.second;
	
	THROW_INTERNAL_ERROR("Could not find resource with given name");
}

void Pipeline::AddStaticResource(const char* resourceName, std::shared_ptr<Bindable> bindable)
{
	THROW_OBJECT_STATE_ERROR_IF("Cannot add object with nullptr name", resourceName == nullptr);
	THROW_OBJECT_STATE_ERROR_IF("Bindable data was empty while adding it as static resource", !bindable);

	m_staticResources.push_back({ resourceName, bindable });
}

void Pipeline::AddBufferToCopyPipeline(GraphicsResource* dst, GraphicsResource* src)
{
	m_buffersToCopy.push_back({ dst , src});
}

void Pipeline::Execute(Graphics& graphics)
{
	ID3D12CommandList* pCommandLists[] = {m_graphicsCommandList->Get()};

	graphics.GetDeviceResources().GetCommandQueue()->ExecuteCommandLists(_countof(pCommandLists), pCommandLists);
}

void Pipeline::ExecuteCopyCalls(Graphics& graphics)
{
	// temporarily we will be using graphic command list for copy calls
	for (auto& copyData : m_buffersToCopy)
		copyData.src->CopyResourcesTo(graphics, m_graphicsCommandList.get(), copyData.dst);

	m_buffersToCopy.clear();
}