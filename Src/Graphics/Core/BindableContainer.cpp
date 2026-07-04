#include "BindableContainer.h"

#include "Pipeline.h"

#include "Includes/BindablesInclude.h"
#include "Graphics/Bindables/RootSignatureBindableWrapper.h"

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

void BindableContainer::AddStaticBindable(const char* bindableName)
{
	m_staticBindableNames.push_back(bindableName);
}

void BindableContainer::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	for (auto staticBindableName : m_staticBindableNames)
		SegregateBindableBaseFunctionality(pipeline.GetStaticResource(staticBindableName).get());

	m_commandListLastSeenRevisions.resize(m_commandListBindables.size());
	m_descriptorLastSeenRevisions.resize(m_descriptorBindables.size());
	m_rootSignatureLastSeenRevisions.resize(m_rootSignatureBindables.size());
	m_pipelineStateLastSeenRevisions.resize(m_pipelineStateBindables.size());
}

template<class T, ENABLE_IF((std::is_base_of_v<UpdatableBindable, T>))>
bool CheckForRevisionChanged(const std::vector<T*>& bindables, std::vector<CachedRevision>& cachedRevisions)
{
	auto RebuildCachedRevisions = [&]()
		{
			cachedRevisions.clear();
			cachedRevisions.reserve(bindables.size());
			for (size_t i = 0; i < bindables.size(); i++)
				cachedRevisions.emplace_back(bindables[i], bindables[i]->GetRevision());
		};

	if (bindables.size() != cachedRevisions.size())
	{
		RebuildCachedRevisions();
		return true;
	}

	bool result = false;

	for (size_t i = 0; i < bindables.size(); i++)
	{
		if (cachedRevisions[i].object != bindables[i])
		{
			RebuildCachedRevisions();
			return true;
		}

		if (bindables[i]->GetRevision() != cachedRevisions[i].revision)
		{
			cachedRevisions[i].revision = bindables[i]->GetRevision();
			result = true;
		}
	}

	return result;
}

void BindableContainer::Update()
{
	if (CheckForRevisionChanged(m_commandListBindables, m_commandListLastSeenRevisions))
		m_revision.commandListRevision++;

	if (CheckForRevisionChanged(m_descriptorBindables, m_descriptorLastSeenRevisions))
		m_revision.descriptorRevision++;

	if (CheckForRevisionChanged(m_rootSignatureBindables, m_rootSignatureLastSeenRevisions))
		m_revision.rootSignatureRevision++;

	if (CheckForRevisionChanged(m_pipelineStateBindables, m_pipelineStateLastSeenRevisions))
		m_revision.pipelineStateRevision++;
}

void BindableContainer::SegregateBindableByClass(Bindable* bindable)
{

}

void BindableContainer::SegregateBindableBaseFunctionality(Bindable* bindable)
{
	if (auto* rootParameterBinding = dynamic_cast<RootParameterBinding*>(bindable))
		AddBindableWrapper(std::make_shared<RootParameterBindableWrapper>(rootParameterBinding));
	else
	{
		if (auto* rootSignatureBinding = dynamic_cast<RootSignatureBinding*>(bindable))
			AddBindableWrapper(std::make_shared<RootSignatureBindableWrapper>(rootSignatureBinding));

		if (auto* rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(bindable))
			m_rootSignatureBindables.push_back(rootSignatureBindable);

		if (auto* commandListBindable = dynamic_cast<CommandListBindable*>(bindable))
			m_commandListBindables.push_back(commandListBindable);
	}

	if (auto* descriptorBindable = dynamic_cast<DescriptorBindable*>(bindable))
		m_descriptorBindables.push_back(descriptorBindable);

	if (auto* pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(bindable))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}

const std::vector<CommandListBindable*>& BindableContainer::GetCommandListBindables() const
{
	return m_commandListBindables;
}

const std::vector<DescriptorBindable*>& BindableContainer::GetDescriptorBindables() const
{
	return m_descriptorBindables;
}

const std::vector<RootSignatureBindable*>& BindableContainer::GetRootSignatureBindables() const
{
	return m_rootSignatureBindables;
}

const std::vector<PipelineStateBindable*>& BindableContainer::GetPipelineStateBindables() const
{
	return m_pipelineStateBindables;
}

BindableContainerRevision BindableContainer::GetRevision() const
{
	return m_revision;
}

void BindableContainer::AddBindableWrapper(std::shared_ptr<Bindable> wrapper)
{
	THROW_INTERNAL_ERROR_IF("Passed bindable was not wrapper", wrapper->GetBindableType() != BindableType::bindable_rootSignatureWrapper);

	Bindable* pWrapper = wrapper.get();

	m_bindables.push_back(wrapper);

	if (auto* commandListBindable = dynamic_cast<CommandListBindable*>(pWrapper))
		m_commandListBindables.push_back(commandListBindable);

	if (auto* rootSignatureBindable = dynamic_cast<RootSignatureBindable*>(pWrapper))
		m_rootSignatureBindables.push_back(rootSignatureBindable);

	if (auto* pipelineStateBindable = dynamic_cast<PipelineStateBindable*>(pWrapper))
		m_pipelineStateBindables.push_back(pipelineStateBindable);
}

MeshBindableContainer& MeshBindableContainer::operator+=(const MeshBindableContainer& other)
{
	// calling base constructor
	BindableContainer::operator+=(other);

	m_staticBindableNames.insert(m_staticBindableNames.end(), other.m_staticBindableNames.begin(), other.m_staticBindableNames.end());

	if (!m_attributeBuffer) m_attributeBuffer = other.m_attributeBuffer;
	if (!m_positionBuffer) m_positionBuffer = other.m_positionBuffer;
	if (m_indexBuffer == nullptr) m_indexBuffer = other.m_indexBuffer;

	m_cachedBuffers.insert(m_cachedBuffers.end(), other.m_cachedBuffers.begin(), other.m_cachedBuffers.end());
	m_textures.insert(m_textures.end(), other.m_textures.begin(), other.m_textures.end());

	return *this;
}

void MeshBindableContainer::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	BindableContainer::Initialize(graphics, pipeline);

	for (auto* texture : m_textures)
		texture->InitializeGraphicResources(graphics, pipeline);

	for (auto* cachedBuffer : m_cachedBuffers)
		cachedBuffer->Update(graphics);
}

void MeshBindableContainer::SetAttributeBufferEntry(std::shared_ptr<VertexBufferEntry> attributeBufferEntry)
{
	m_attributeBuffer = std::move(attributeBufferEntry);
}

void MeshBindableContainer::SetPositionBufferEntry(std::shared_ptr<VertexBufferEntry> positionBufferEntry)
{
	m_positionBuffer = std::move(positionBufferEntry);
}

void MeshBindableContainer::SetIndexBufferEntry(std::shared_ptr<IndexBufferEntry> indexBufferEntry)
{
	m_indexBuffer = std::move(indexBufferEntry);
}

std::shared_ptr<VertexBufferEntry> MeshBindableContainer::GetAttributeVertexBufferEntry() const
{
	return m_attributeBuffer;
}

std::shared_ptr<VertexBufferEntry> MeshBindableContainer::GetPositionVertexBufferEntry() const
{
	return m_positionBuffer;
}

std::shared_ptr<IndexBufferEntry> MeshBindableContainer::GetIndexBufferEntry() const
{
	return m_indexBuffer;
}

InputLayout* MeshBindableContainer::GetInputLayout() const
{
	return m_inputLayout;
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

	THROW_INTERNAL_ERROR_IF("Tried to bind RootSignatureBindableWrapper like normal bindable", type == BindableType::bindable_rootSignatureWrapper);

	switch (type)
	{
		case BindableType::bindable_indexBuffer:
		{
			THROW_INTERNAL_ERROR("IndexBuffer was pushed as regular bindable. Use IndexBufferEntry for using index data. Then bind it using SetIndexBuffer()");
			break;
		}
		case BindableType::bindable_indexBufferEntry:
		{
			THROW_INTERNAL_ERROR("IndexBufferEntry was pushed as regular bindable. Bind it using SetIndexBuffer()");
			break;
		}
		case BindableType::bindable_vertexBuffer:
		{
			THROW_INTERNAL_ERROR("VertexBuffer was pushed as regular bindable. Use VertexBufferEntry for using vertex data. Then bind it using SetAttributeBuffer() or SetPositionBuffer()");
			break;
		}
		case BindableType::bindable_vertexBufferEntry:
		{
			THROW_INTERNAL_ERROR("VertexBufferEntry was pushed as regular bindable. Bind it using SetAttributeBuffer() or SetPositionBuffer()");
			break;
		}
		case BindableType::bindable_inputLayout:
		{
			m_inputLayout = static_cast<InputLayout*>(bindable);
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

	THROW_INTERNAL_ERROR_IF("Tried to bind RootSignatureBindableWrapper like normal bindable", type == BindableType::bindable_rootSignatureWrapper);

	if(type == BindableType::bindable_shader)
		m_shader = static_cast<Shader*>(bindable);
}