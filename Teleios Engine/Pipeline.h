#pragma once
#include "Includes/CppIncludes.h"
#include "CommandList.h"

class ID3D12CommandList;
class Bindable;

class Pipeline
{
public:
	Pipeline(Graphics& graphics);

public:
	void BeginRender(Graphics& graphics);
	void FinishRender(Graphics& graphics);

	CommandList* GetGraphicCommandList() const;

	Bindable* GetStaticResource(const char* resourceName) const;

	void AddStaticResource(const char* resourceName, Bindable* bindable);

public:
	void Execute(Graphics& graphics);

private:
	size_t GetNumberOfCommandLists() const;
	std::vector<ID3D12CommandList*> GetCommandListPtrs();

public:
	std::shared_ptr<CommandList> m_graphicsCommandList;
	size_t m_currentWorkingIndex = 0;

	std::vector<std::pair<const char*, Bindable*>> m_staticResources;
};