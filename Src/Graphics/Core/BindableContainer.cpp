#include "BindableContainer.h"

#include "Pipeline.h"

#include "Includes/BindablesInclude.h"

void BindableContainer::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindables.push_back(bindable);

	AddBindable(bindable.get());
}

void BindableContainer::AddBindable(Bindable* bindable)
{
	SegregateBindableByClass(bindable);

	SegregateBindableBaseFunctionality(bindable);
}

void BindableContainer::SegregateBindableBaseFunctionality(Bindable* bindable)
{
	if (auto* commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.push_back(commandListBindable);

	if (auto* rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.push_back(rootSignatureBindable);

	if (auto* pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}

void BindableContainer::SegregateBindableAtFirstPos(Bindable* bindable)
{
	if (auto* commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
		m_commandListBindables.insert(m_commandListBindables.begin(), commandListBindable);

	if (auto* rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
		m_rootSignatureBindables.insert(m_rootSignatureBindables.begin(), rootSignatureBindable);

	if (auto* pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.insert(m_pipelineStateBindables.begin(), pipelineStateBindable);
}

const std::vector<CommandListBindable*>& BindableContainer::GetCommandListBindables() const
{
	return m_commandListBindables;
}

const std::vector<RootSignatureBindable*>& BindableContainer::GetRootSignatureBindables() const
{
	return m_rootSignatureBindables;
}

const std::vector<PipelineStateBindable*>& BindableContainer::GetPipelineStateBindables() const
{
	return m_pipelineStateBindables;
}

void MeshBindableContainer::AddStaticBindable(const char* bindableName)
{
	m_staticBindableNames.push_back(bindableName);
}

void MeshBindableContainer::Initialize(Pipeline& pipeline)
{
	for (auto staticBindableName : m_staticBindableNames)
		SegregateBindableAtFirstPos(pipeline.GetStaticResource(staticBindableName));
}

VertexBuffer* MeshBindableContainer::GetVertexBuffer() const
{
	return m_vertexBuffer;
}

IndexBuffer* MeshBindableContainer::GetIndexBuffer() const
{
	return m_indexBuffer;
}

InputLayout* MeshBindableContainer::GetInputLayout() const
{
	return m_inputLayout;
}

TransformConstantBuffer* MeshBindableContainer::GetTransformConstantBuffer() const
{
	return m_transformConstantBuffer;
}

const std::vector<CachedConstantBuffer*>& MeshBindableContainer::GetCachedBuffers() const
{
	return m_cachedBuffers;
}

const std::vector<Texture*>& MeshBindableContainer::GetTextures() const
{
	return m_textures;
}

void MeshBindableContainer::SegregateBindableByClass(Bindable* bindable)
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
		case BindableType::bindable_inputLayout:
		{
			m_inputLayout = static_cast<InputLayout*>(bindable);
			break;
		}
		case BindableType::bindable_transformConstantBuffer:
		{
			m_transformConstantBuffer = static_cast<TransformConstantBuffer*>(bindable);
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

const Shader* ComputeBindableContainer::GetShader() const
{
	return m_shader;
}

void ComputeBindableContainer::SegregateBindableByClass(Bindable* bindable)
{
	BindableType type = bindable->GetBindableType();

	if(type == BindableType::bindable_shader)
		m_shader = static_cast<Shader*>(bindable);
}