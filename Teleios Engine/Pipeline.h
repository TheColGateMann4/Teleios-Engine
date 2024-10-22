#pragma once
#include "Includes/CppIncludes.h"
#include "CommandList.h"
#include "RootSignatureResource.h"

class ID3D12CommandList;

class Pipeline
{
public:
	Pipeline(Graphics& graphics);

public:
	CommandList* GetGraphicCommandList() const;

	RootSignatureTypedResource GetStaticRootResource(const char* resourceName) const;

	void AddStaticRootResource(const char* resourceName, RootSignatureResourceType resourceType, RootSignatureResource* pResource);

public:
	void Execute(Graphics& graphics);

private:
	size_t GetNumberOfCommandLists() const;
	std::vector<ID3D12CommandList*> GetCommandListPtrs();

public:
	std::shared_ptr<CommandList> m_graphicsCommandList;
	size_t m_currentWorkingIndex = 0;

	std::vector<std::pair<const char*, RootSignatureTypedResource>> m_staticResources;
};