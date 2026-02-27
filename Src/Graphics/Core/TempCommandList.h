#pragma once
#include "BindableContainer.h"
#include "RootSignature.h"
#include "PipelineState.h"

class CommandList;

class TempComputeCommandList
{
public:
	TempComputeCommandList(Graphics& graphics, CommandList* commandList);

	TempComputeCommandList(TempComputeCommandList&&) noexcept = default;

	TempComputeCommandList(const TempComputeCommandList& copied);

public:
	CommandList* Get();

public:
	void Dispatch(Graphics& graphics, unsigned int workToProcessX = 1, unsigned int workToProcessY = 1, unsigned int workToProcessZ = 1);

	template<class T, ENABLE_IF((std::is_base_of_v<Bindable, T>))>
	void Bind(T&& bindable)
	{
		m_bindableContainer.AddBindable(std::move(bindable));
	}

	void Bind(std::shared_ptr<Bindable> bindable);
	void Bind(Bindable* bindable);

private:
	void Finish(Graphics& graphics);

private:
	ComputeBindableContainer m_bindableContainer;
	CommandList* m_commandList;

	std::shared_ptr<RootSignature> m_rootSignature;
	ComputePipelineState m_pipelineState;
};

class TempGraphicsCommandList
{
public:
	TempGraphicsCommandList(Graphics& graphics, CommandList* commandList);

	TempGraphicsCommandList(TempGraphicsCommandList&&) noexcept = default;

	TempGraphicsCommandList(const TempGraphicsCommandList& copied);

public:
	CommandList* Get();

public:
	void DrawIndexed(Graphics& graphics);

	template<class T, ENABLE_IF((std::is_base_of_v<Bindable, T>))>
	void Bind(T&& bindable)
	{
		m_bindableContainer.AddBindable(std::move(bindable));
	}

	void Bind(std::shared_ptr<Bindable> bindable);
	void Bind(Bindable* bindable);

private:
	void Finish(Graphics& graphics);

private:
	MeshBindableContainer m_bindableContainer;
	CommandList* m_commandList;

	std::shared_ptr<RootSignature> m_rootSignature;
	GraphicsPipelineState m_pipelineState;
};