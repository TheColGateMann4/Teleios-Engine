#pragma once
#include "includes/CppIncludes.h"
#include "Bindable.h"

class Pipeline;

class VertexBuffer;
class IndexBuffer;
class TransformConstantBuffer;
class CachedConstantBuffer;
class Texture;
class Shader;

class MeshBindableContainer
{
public:
	void AddStaticBindable(const char* bindableName);
	void AddBindable(std::shared_ptr<Bindable> bindable);

	void Initialize(Pipeline& pipeline);

public:
	const std::vector<CommandListBindable*>& GetCommandListBindables() const;
	const std::vector<DirectCommandListBindable*>& GetDirectCommandListBindables() const;
	const std::vector<RootSignatureBindable*>& GetRootSignatureBindables() const;
	const std::vector<PipelineStateBindable*>& GetPipelineStateBindables() const;

	const std::vector<CachedConstantBuffer*>& GetCachedBuffers() const;
	const std::vector<Texture*>& GetTextures() const;
	
private:
	void SegregateBindable(Bindable* bindable);
	void SegregateBindableAtFirstPos(Bindable* bindable);

private:
	// vector owning potentially shared scene bindables
	std::vector<std::shared_ptr<Bindable>> m_bindables;


	// vector with names of static scene resources
	std::vector<const char*> m_staticBindableNames;

	std::vector<CommandListBindable*> m_commandListBindables;
	std::vector<DirectCommandListBindable*> m_directCommandListBindables;
	std::vector<RootSignatureBindable*> m_rootSignatureBindables;
	std::vector<PipelineStateBindable*> m_pipelineStateBindables;

	std::vector<CachedConstantBuffer*> m_cachedBuffers;
	std::vector<Texture*> m_textures;
};

class ComputeBindableContainer
{
public:
	template<class T, std::enable_if_t<std::is_base_of_v<Bindable, T>, int> = 0> 
	void AddBindable(T&& bindable)
	{
		m_temporaryBindables.push_back(std::make_shared<T>(std::move(bindable)));

		SegregateBindable(m_temporaryBindables.back().get());
	}

	void AddBindable(std::shared_ptr<Bindable> bindable);
	void AddBindable(Bindable* bindable);

public:
	const std::vector<CommandListBindable*>& GetCommandListBindables() const;
	const std::vector<DirectCommandListBindable*>& GetDirectCommandListBindables() const;
	const std::vector<RootSignatureBindable*>& GetRootSignatureBindables() const;
	const std::vector<PipelineStateBindable*>& GetPipelineStateBindables() const;
	const Shader* GetShader() const;

private:
	void SegregateBindable(Bindable* bindable);

private:
	// vector owning potentially shared scene bindables
	std::vector<std::shared_ptr<Bindable>> m_temporaryBindables;

	std::vector<CommandListBindable*> m_commandListBindables;
	std::vector<DirectCommandListBindable*> m_directCommandListBindables;
	std::vector<RootSignatureBindable*> m_rootSignatureBindables;
	std::vector<PipelineStateBindable*> m_pipelineStateBindables;
	Shader* m_shader;
};