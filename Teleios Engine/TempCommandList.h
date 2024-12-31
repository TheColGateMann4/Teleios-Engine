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

	template<class T, std::enable_if_t<std::is_base_of_v<Bindable, T>, int> = 0>
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

	RootSignature m_rootSignature;
	ComputePipelineState m_pipelineState;
};