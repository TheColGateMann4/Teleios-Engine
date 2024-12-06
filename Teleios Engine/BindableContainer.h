#pragma once
#include "includes/CppIncludes.h"
#include "Bindable.h"

class Pipeline;

class VertexBuffer;
class IndexBuffer;
class TransformConstantBuffer;
class CachedConstantBuffer;

class BindableContainer
{
public:
	void AddStaticBindable(const char* bindableName);

	void AddBindable(std::shared_ptr<Bindable> bindable);


	void SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

	void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

	void SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer);


	void Initialize(Pipeline& pipeline);

public:
	const std::vector<CommandListBindable*>& GetCommandListBindables() const;
	const std::vector<DirectCommandListBindable*>& GetDirectCommandListBindables() const;
	const std::vector<RootSignatureBindable*>& GetRootSignatureBindables() const;
	const std::vector<PipelineStateBindable*>& GetPipelineStateBindables() const;

	VertexBuffer* GetVertexBuffer() const;
	IndexBuffer* GetIndexBuffer() const;
	TransformConstantBuffer* GetTransformConstantBuffer() const;
	const std::vector<CachedConstantBuffer*>& GetCachedBuffers() const;

private:
	void SegregateBindable(Bindable* bindable);

	void SegregateBindableOnStartingPos(Bindable* bindable);

private:
	// vector owning potentially shared scene bindables
	std::vector<std::shared_ptr<Bindable>> m_bindables;

	// vector with names of static scene resources
	std::vector<const char*> m_staticBindableNames;

	std::vector<CommandListBindable*> m_commandListBindables;
	std::vector<DirectCommandListBindable*> m_directCommandListBindables;
	std::vector<RootSignatureBindable*> m_rootSignatureBindables;
	std::vector<PipelineStateBindable*> m_pipelineStateBindables;

	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	TransformConstantBuffer* m_transformConstantBuffer = nullptr;

	std::vector<CachedConstantBuffer*> m_cachedBuffers;
};