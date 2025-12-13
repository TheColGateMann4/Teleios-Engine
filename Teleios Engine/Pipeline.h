#pragma once
#include "Includes/CppIncludes.h"
#include "CommandList.h"

class Bindable;
class Camera;

class Pipeline
{
public:
	Pipeline(Graphics& graphics);

public:
	void BeginRender(Graphics& graphics) const;
	void FinishRender(Graphics& graphics) const;

	CommandList* GetGraphicCommandList() const;

	Bindable* GetStaticResource(const char* resourceName) const;

	void AddStaticResource(const char* resourceName, Bindable* bindable);

public:
	Camera* GetCurrentCamera() const;
	void SetCurrentCamera(Camera* newCurrentCamera);

public:
	void AddBufferToCopyPipeline(GraphicsResource* dst, GraphicsResource* src);

public:
	void Execute(Graphics& graphics);

	void ExecuteCopyCalls(Graphics& graphics);

public:
	std::shared_ptr<CommandList> m_graphicsCommandList;

	std::vector<std::pair<const char*, Bindable*>> m_staticResources;
	Camera* m_camera = nullptr;

	struct CopyData
	{
		GraphicsResource* dst;
		GraphicsResource* src;
	};

	std::vector<CopyData> m_buffersToCopy;
};