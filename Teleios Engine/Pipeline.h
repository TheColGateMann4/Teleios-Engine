#pragma once
#include "Includes/CppIncludes.h"
#include "CommandList.h"

class ID3D12CommandList;
class Bindable;
class Camera;

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
	Camera* GetCurrentCamera();
	void SetCurrentCamera(Camera* newCurrentCamera);

public:
	void Execute(Graphics& graphics);

private:
	size_t GetNumberOfCommandLists() const;
	std::vector<ID3D12CommandList*> GetCommandListPtrs();

public:
	std::shared_ptr<CommandList> m_graphicsCommandList;

	std::vector<std::pair<const char*, Bindable*>> m_staticResources;
	Camera* m_camera;
};