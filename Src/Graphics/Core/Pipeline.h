#pragma once
#include "Includes/CppIncludes.h"
#include "CommandList.h"

class Bindable;
class Camera;

class Pipeline
{
public:
	void Initialize(Graphics& graphics);

	void BeginRender(Graphics& graphics) const;
	void FinishRender(Graphics& graphics);

	void FinishInitialization(Graphics& graphics);

	CommandList* GetGraphicCommandList() const;

	std::shared_ptr<Bindable> GetStaticResource(const char* resourceName) const;

	void AddStaticResource(const char* resourceName, std::shared_ptr<Bindable> bindable);

public:
	void AddBufferToCopyPipeline(GraphicsResource* dst, GraphicsResource* src);

public:
	void Execute(Graphics& graphics);

	void ExecuteCopyCalls(Graphics& graphics);

public:
	std::shared_ptr<CommandList> m_graphicsCommandList;

	std::vector<std::pair<const char*, std::shared_ptr<Bindable>>> m_staticResources;

	struct CopyData
	{
		GraphicsResource* dst;
		GraphicsResource* src;
	};

	std::vector<CopyData> m_buffersToCopy;
};