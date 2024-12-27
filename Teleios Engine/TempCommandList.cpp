#include "TempCommandList.h"

#include "CommandList.h"
#include "Graphics.h"

#include "Macros/ErrorMacros.h"

TempComputeCommandList::TempComputeCommandList(Graphics& graphics, CommandList* commandList)
	:
	m_commandList(commandList)
{

}

TempComputeCommandList::TempComputeCommandList(const TempComputeCommandList& copied)
	:
	m_bindableContainer(copied.m_bindableContainer),
	m_commandList(copied.m_commandList),
	m_rootSignature(copied.m_rootSignature),
	m_pipelineState(copied.m_pipelineState)
{
	THROW_INTERNAL_ERROR("Called copy constructor for temp compute command list\n");
}

CommandList* TempComputeCommandList::Get()
{
	return m_commandList;
}

void TempComputeCommandList::Dispatch(Graphics& graphics)
{
	Finish(graphics);

	m_commandList->SetPipelineState(graphics, &m_pipelineState);

	m_commandList->SetComputeRootSignature(graphics, &m_rootSignature);

	m_commandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	for (auto commandListBindable : m_bindableContainer.GetCommandListBindables())
		commandListBindable->BindToComputeCommandList(graphics, m_commandList);

	m_commandList->Dispatch(graphics);
}

void TempComputeCommandList::Bind(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void TempComputeCommandList::Bind(Bindable* bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void TempComputeCommandList::Finish(Graphics& graphics)
{
	unsigned int frameIndex = graphics.GetCurrentBufferIndex();

	// initializing root signature
	{
		for (auto rootSignatureBindable : m_bindableContainer.GetRootSignatureBindables())
			rootSignatureBindable->BindToComputeRootSignature(graphics, &m_rootSignature);

		m_rootSignature.Initialize(graphics);
	}

	// initizalizing pipeline state
	{
		for (auto pipelineStateBindable : m_bindableContainer.GetPipelineStateBindables())
			pipelineStateBindable->BindToComputePipelineState(graphics, &m_pipelineState);

		m_pipelineState.SetRootSignature(&m_rootSignature);

		m_pipelineState.Finish(graphics);
	}
}