#include "BindableContainer.h"

#include "Pipeline.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "TransformConstantBuffer.h"
#include "ConstantBuffer.h"

void BindableContainer::AddStaticBindable(const char* bindableName)
{
	m_staticBindableNames.push_back(bindableName);
}

void BindableContainer::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindables.push_back(bindable);

	SegregateBindable(bindable.get());
}

void BindableContainer::SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
	m_vertexBuffer = vertexBuffer.get();

	AddBindable(vertexBuffer);
}

void BindableContainer::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
	m_indexBuffer = indexBuffer.get();

	AddBindable(indexBuffer);
}

void BindableContainer::SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer)
{
	m_transformConstantBuffer = transformConstantBuffer.get();

	AddBindable(transformConstantBuffer);
}

void BindableContainer::Initialize(Pipeline& pipeline)
{
	for (auto staticBindableName : m_staticBindableNames)
		SegregateBindableOnStartingPos(pipeline.GetStaticResource(staticBindableName));
}

const std::vector<CommandListBindable*>& BindableContainer::GetCommandListBindables() const
{
	return m_commandListBindables;
}

const std::vector<DirectCommandListBindable*>& BindableContainer::GetDirectCommandListBindables() const
{
	return m_directCommandListBindables;
}

const std::vector<RootSignatureBindable*>& BindableContainer::GetRootSignatureBindables() const
{
	return m_rootSignatureBindables;
}

const std::vector<PipelineStateBindable*>& BindableContainer::GetPipelineStateBindables() const
{
	return m_pipelineStateBindables;
}

VertexBuffer* BindableContainer::GetVertexBuffer() const
{
	return m_vertexBuffer;
}

IndexBuffer* BindableContainer::GetIndexBuffer() const
{
	return m_indexBuffer;
}

TransformConstantBuffer* BindableContainer::GetTransformConstantBuffer() const
{
	return m_transformConstantBuffer;
}

const std::vector<CachedConstantBuffer*>& BindableContainer::GetCachedBuffers() const
{
	return m_cachedBuffers;
}

void BindableContainer::SegregateBindable(Bindable* bindable)
{
	// we don't use this in SegregateBindableOnStartingPos() since that handles static bindables, they are meant to be read-only
	if (auto cachedConstantBuffer = dynamic_cast<CachedConstantBuffer*>(bindable))
		m_cachedBuffers.push_back(cachedConstantBuffer);


	if (auto commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.push_back(commandListBindable);

	if (auto directCommandListBindable = dynamic_cast<DirectCommandListBindable*>(bindable))
		m_directCommandListBindables.push_back(directCommandListBindable);

	if (auto rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.push_back(rootSignatureBindable);

	if (auto pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}

void BindableContainer::SegregateBindableOnStartingPos(Bindable* bindable)
{
	if (auto commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.push_back(commandListBindable);

	if (auto directCommandListBindable = dynamic_cast<DirectCommandListBindable*>(bindable))
		m_directCommandListBindables.insert(m_directCommandListBindables.begin(), directCommandListBindable);

	if (auto rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.insert(m_rootSignatureBindables.begin(), rootSignatureBindable);

	if (auto pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.insert(m_pipelineStateBindables.begin(), pipelineStateBindable);
}