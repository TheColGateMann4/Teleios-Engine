#include "BindableContainer.h"

#include "Pipeline.h"

#include "Includes/BindablesInclude.h"

BindableContainer& BindableContainer::operator+=(const BindableContainer& other)
{
	m_bindables.insert(m_bindables.end(), other.m_bindables.begin(), other.m_bindables.end());
	m_commandListBindables.insert(m_commandListBindables.end(), other.m_commandListBindables.begin(), other.m_commandListBindables.end());
	m_rootSignatureBindables.insert(m_rootSignatureBindables.end(), other.m_rootSignatureBindables.begin(), other.m_rootSignatureBindables.end());
	m_pipelineStateBindables.insert(m_pipelineStateBindables.end(), other.m_pipelineStateBindables.begin(), other.m_pipelineStateBindables.end());

	return *this;
}

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

MeshBindableContainer& MeshBindableContainer::operator+=(const MeshBindableContainer& other)
{
	// calling base constructor
	BindableContainer::operator+=(other);

	m_staticBindableNames.insert(m_staticBindableNames.end(), other.m_staticBindableNames.begin(), other.m_staticBindableNames.end());

	if (m_vertexBuffer == nullptr) m_vertexBuffer = other.m_vertexBuffer;
	if (m_indexBuffer == nullptr) m_indexBuffer = other.m_indexBuffer;
	if (m_inputLayout == nullptr) m_inputLayout = other.m_inputLayout;
	if (m_transformConstantBuffer == nullptr) m_transformConstantBuffer = other.m_transformConstantBuffer;

	m_cachedBuffers.insert(m_cachedBuffers.end(), other.m_cachedBuffers.begin(), other.m_cachedBuffers.end());
	m_textures.insert(m_textures.end(), other.m_textures.begin(), other.m_textures.end());

	return *this;
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

ComputeBindableContainer& ComputeBindableContainer::operator+=(const ComputeBindableContainer& other)
{
	// calling base constructor
	BindableContainer::operator+=(other);

	if (m_shader == nullptr) m_shader = other.m_shader;

	return *this;
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