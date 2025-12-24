#include "BindableContainer.h"

#include "Pipeline.h"

#include "Includes/BindablesInclude.h"

void MeshBindableContainer::AddStaticBindable(const char* bindableName)
{
	m_staticBindableNames.push_back(bindableName);
}

void MeshBindableContainer::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindables.push_back(bindable);

	SegregateBindable(bindable.get());
}

void MeshBindableContainer::AddBindable(Bindable* bindable)
{
	SegregateBindable(bindable);
}


void MeshBindableContainer::SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
	m_vertexBuffer = vertexBuffer.get();

	AddBindable(vertexBuffer);
}

void MeshBindableContainer::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
	m_indexBuffer = indexBuffer.get();

	AddBindable(indexBuffer);
}


void MeshBindableContainer::Initialize(Pipeline& pipeline)
{
	for (auto staticBindableName : m_staticBindableNames)
		SegregateBindableAtFirstPos(pipeline.GetStaticResource(staticBindableName));
}


const std::vector<CommandListBindable*>& MeshBindableContainer::GetCommandListBindables() const
{
	return m_commandListBindables;
}

const std::vector<RootSignatureBindable*>& MeshBindableContainer::GetRootSignatureBindables() const
{
	return m_rootSignatureBindables;
}

const std::vector<PipelineStateBindable*>& MeshBindableContainer::GetPipelineStateBindables() const
{
	return m_pipelineStateBindables;
}

VertexBuffer* MeshBindableContainer::GetVertexBuffer() const
{
	return m_vertexBuffer;
}

IndexBuffer* MeshBindableContainer::GetIndexBuffer() const
{
	return m_indexBuffer;
}

const std::vector<CachedConstantBuffer*>& MeshBindableContainer::GetCachedBuffers() const
{
	return m_cachedBuffers;
}

const std::vector<Texture*>& MeshBindableContainer::GetTextures() const
{
	return m_textures;
}

void MeshBindableContainer::SegregateBindable(Bindable* bindable)
{
	if (auto* cachedConstantBuffer = dynamic_cast<CachedConstantBuffer*>(bindable))
		m_cachedBuffers.push_back(cachedConstantBuffer);

	if (auto* texture = dynamic_cast<Texture*>(bindable))
		m_textures.push_back(texture);


	if (auto* commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.push_back(commandListBindable);

	if (auto* rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.push_back(rootSignatureBindable);

	if (auto* pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}

void MeshBindableContainer::SegregateBindableAtFirstPos(Bindable* bindable)
{
	if (auto* commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.insert(m_commandListBindables.begin(), commandListBindable);

	if (auto* rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.insert(m_rootSignatureBindables.begin(), rootSignatureBindable);

	if (auto* pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.insert(m_pipelineStateBindables.begin(), pipelineStateBindable);
}



void ComputeBindableContainer::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_temporaryBindables.push_back(bindable);

	SegregateBindable(bindable.get());
}

void ComputeBindableContainer::AddBindable(Bindable* bindable)
{
	SegregateBindable(bindable);
}

const std::vector<CommandListBindable*>& ComputeBindableContainer::GetCommandListBindables() const
{
	return m_commandListBindables;
}

const std::vector<RootSignatureBindable*>& ComputeBindableContainer::GetRootSignatureBindables() const
{
	return m_rootSignatureBindables;
}

const std::vector<PipelineStateBindable*>& ComputeBindableContainer::GetPipelineStateBindables() const
{
	return m_pipelineStateBindables;
}

const Shader* ComputeBindableContainer::GetShader() const
{
	return m_shader;
}

void ComputeBindableContainer::SegregateBindable(Bindable* bindable)
{
	if (auto* shader = dynamic_cast<Shader*>(bindable))
		m_shader = shader;
	
	if (auto* commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.push_back(commandListBindable);

	if (auto* rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.push_back(rootSignatureBindable);

	if (auto* pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}