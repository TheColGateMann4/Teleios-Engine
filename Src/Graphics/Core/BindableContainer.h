#pragma once
#include "Includes/CppIncludes.h"
#include "Graphics/Bindables/Bindable.h"

class Pipeline;

class VertexBufferEntry;
class IndexBufferEntry;
class IndexBuffer;
class InputLayout;
class TransformConstantBuffer;
class CachedConstantBuffer;
class Texture;
class Shader;

class BindableContainer
{
public:
	BindableContainer() = default;
	BindableContainer(const BindableContainer& other) = default;
	BindableContainer(BindableContainer&& other) noexcept = default;

	BindableContainer& operator=(BindableContainer&& other) noexcept = default;
	BindableContainer& operator=(const BindableContainer& other) = default;

	BindableContainer& operator+=(const BindableContainer& other);

	virtual ~BindableContainer() = default;
	
public:
	void AddBindable(std::shared_ptr<Bindable> bindable);
	void AddBindable(Bindable* bindable);

	template<class T, ENABLE_IF((std::is_base_of_v<Bindable, T>))>
	void AddBindable(T&& bindable)
	{
		AddBindable(std::make_shared<T>(std::move(bindable)));
	}

	void AddStaticBindable(const char* bindableName);

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline);
	
	virtual void SegregateBindableByClass(Bindable* bindable);
	void SegregateBindableBaseFunctionality(Bindable* bindable);

	
	const std::vector<CommandListBindable*>& GetCommandListBindables() const;
	const std::vector<DescriptorBindable*>& GetDescriptorBindables() const;
	const std::vector<RootSignatureBindable*>& GetRootSignatureBindables() const;
	const std::vector<PipelineStateBindable*>& GetPipelineStateBindables() const;

private:
	void AddBindableWrapper(std::shared_ptr<Bindable> wrapper);

protected:
	// vector owning potentially shared bindables
	std::vector<std::shared_ptr<Bindable>> m_bindables;

	std::vector<CommandListBindable*> m_commandListBindables;
	std::vector<DescriptorBindable*> m_descriptorBindables;
	std::vector<RootSignatureBindable*> m_rootSignatureBindables;
	std::vector<PipelineStateBindable*> m_pipelineStateBindables;

	// vector with names of static scene resources
	std::vector<const char*> m_staticBindableNames;
};

class MeshBindableContainer : public BindableContainer
{
public:
	MeshBindableContainer& operator+=(const MeshBindableContainer& other);

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

public:
	void SetAttributeBufferEntry(std::shared_ptr<VertexBufferEntry> attributeBufferEntry);
	void SetPositionBufferEntry(std::shared_ptr<VertexBufferEntry> positionBufferEntry);
	void SetIndexBufferEntry(std::shared_ptr<IndexBufferEntry> indexBufferEntry);

public:
	std::shared_ptr<VertexBufferEntry> GetAttributeVertexBufferEntry() const;
	std::shared_ptr<VertexBufferEntry> GetPositionVertexBufferEntry() const;
	std::shared_ptr<IndexBufferEntry> GetIndexBufferEntry() const;

	InputLayout* GetInputLayout() const;
	const std::vector<CachedConstantBuffer*>& GetCachedBuffers() const;
	const std::vector<Texture*>& GetTextures() const;

private:
	virtual void SegregateBindableByClass(Bindable* bindable) override;

private:
	std::shared_ptr<VertexBufferEntry> m_attributeBuffer;
	std::shared_ptr<VertexBufferEntry> m_positionBuffer;
	std::shared_ptr<IndexBufferEntry> m_indexBuffer;

	InputLayout* m_inputLayout = nullptr;

	std::vector<CachedConstantBuffer*> m_cachedBuffers;
	std::vector<Texture*> m_textures;
};

class ComputeBindableContainer : public BindableContainer
{
public:
	ComputeBindableContainer& operator+=(const ComputeBindableContainer& other);

public:
	const Shader* GetShader() const;

private:
	virtual void SegregateBindableByClass(Bindable* bindable) override;

private:
	Shader* m_shader;
};