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

	AddBindable(bindable.get());
}

void MeshBindableContainer::AddBindable(Bindable* bindable)
{
	SegregateBindableClass(bindable);

	SegregateBindableBaseFunctionality(bindable);
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

void MeshBindableContainer::SegregateBindableClass(Bindable* bindable)
{
	BindableType type = bindable->GetBindableType();

	switch (type)
	{
		case BindableType::bindable_indexBuffer:
		{
			m_indexBuffer = static_cast<IndexBuffer*>(bindable);
			break;
		}
		case BindableType::bindable_vertexBuffer:
		{
			m_vertexBuffer = static_cast<VertexBuffer*>(bindable);
			break;
		}
		case BindableType::bindable_cachedConstantBuffer:
		{
			m_cachedBuffers.push_back(static_cast<CachedConstantBuffer*>(bindable));
			break;
		}
		case BindableType::bindable_texture:
		{
			m_textures.push_back(static_cast<Texture*>(bindable));
			break;
		}

		default:
			break;
	}
}

void MeshBindableContainer::SegregateBindableBaseFunctionality(Bindable* bindable)
{
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