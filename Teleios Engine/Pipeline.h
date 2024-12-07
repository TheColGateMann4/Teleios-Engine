#pragma once
#include "Includes/CppIncludes.h"
#include "CommandList.h"

class VertexBuffer;
class IndexBuffer;
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
	void AddBufferToCopyPipeline(VertexBuffer* vertexBuffer);

	void AddBufferToCopyPipeline(IndexBuffer* indexBuffer);

public:
	void Execute(Graphics& graphics);

	void ExecuteCopyCalls(Graphics& graphics);

public:
	std::shared_ptr<CommandList> m_graphicsCommandList;

	std::vector<std::pair<const char*, Bindable*>> m_staticResources;
	Camera* m_camera = nullptr;

	std::vector<VertexBuffer*> m_vertexBuffersToCopy;
	std::vector<IndexBuffer*> m_indexBuffersToCopy;
};