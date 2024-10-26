#include "Pipeline.h"
#include "Graphics.h"
#include "Macros/ErrorMacros.h"
#include "PipelineState.h"

Pipeline::Pipeline(Graphics& graphics)
{
	m_graphicsCommandList = std::make_shared<CommandList>(graphics, D3D12_COMMAND_LIST_TYPE_DIRECT);
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

void Pipeline::Execute(Graphics& graphics)
{
	graphics.GetCommandQueue()->ExecuteCommandLists(GetNumberOfCommandLists(), GetCommandListPtrs().data());

	m_currentWorkingIndex = 0;
}

size_t Pipeline::GetNumberOfCommandLists() const
{
	size_t result = 0;

	if (m_graphicsCommandList)
		result++;

	THROW_INTERNAL_ERROR_IF("There were no initialized command lists", result == 0);

	return result;
}

std::vector<ID3D12CommandList*> Pipeline::GetCommandListPtrs()
{
	std::vector<ID3D12CommandList*> resultCommandListPtrs;

	if (m_graphicsCommandList)
		resultCommandListPtrs.push_back(m_graphicsCommandList->Get());

	return resultCommandListPtrs;
}