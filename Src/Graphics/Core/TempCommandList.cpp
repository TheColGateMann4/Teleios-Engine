#include "TempCommandList.h"

#include "CommandList.h"
#include "Graphics.h"

#include "Macros/ErrorMacros.h"

#include "Includes/BindablesInclude.h"

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

void TempComputeCommandList::Dispatch(Graphics& graphics, unsigned int workToProcessX, unsigned int workToProcessY, unsigned int workToProcessZ)
{
	Finish(graphics);

	m_commandList->SetPipelineState(graphics, m_pipelineState.get());

	m_commandList->SetComputeRootSignature(graphics, m_rootSignature.get());

	m_commandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	for (auto commandListBindable : m_bindableContainer.GetCommandListBindables())
		commandListBindable->BindToComputeCommandList(graphics, m_commandList);

	DirectX::XMUINT3 shaderNumThreads = m_bindableContainer.GetShader()->GetNumThreads();
	workToProcessX = std::ceil(float(workToProcessX) / float(shaderNumThreads.x));
	workToProcessY = std::ceil(float(workToProcessY) / float(shaderNumThreads.y));
	workToProcessZ = std::ceil(float(workToProcessZ) / float(shaderNumThreads.z));

	m_commandList->Dispatch(graphics, workToProcessX, workToProcessY, workToProcessZ);
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
		RootSignatureParams rootParams = {};

		for (auto rootSignatureBindable : m_bindableContainer.GetRootSignatureBindables())
			rootSignatureBindable->BindToRootSignature(&rootParams);

		m_rootSignature = RootSignature::GetResource(graphics, std::move(rootParams));
	}

	// initizalizing pipeline state
	{
		ComputePipelineStateParams pipelineStateParams = {};

		{
			for (auto pipelineStateBindable : m_bindableContainer.GetPipelineStateBindables())
				pipelineStateBindable->AddComputePipelineStateParam(graphics, &pipelineStateParams);

			pipelineStateParams.SetRootSignature(m_rootSignature.get());
		}

		m_pipelineState = ComputePipelineState::GetResource(graphics, std::move(pipelineStateParams));
	}
}


TempGraphicsCommandList::TempGraphicsCommandList(Graphics& graphics, CommandList* commandList)
	:
	m_commandList(commandList)
{

}

TempGraphicsCommandList::TempGraphicsCommandList(const TempGraphicsCommandList& copied)
	:
	m_bindableContainer(copied.m_bindableContainer),
	m_commandList(copied.m_commandList),
	m_rootSignature(copied.m_rootSignature),
	m_pipelineState(copied.m_pipelineState)
{
	THROW_INTERNAL_ERROR("Called copy constructor for temp graphics command list\n");
}

CommandList* TempGraphicsCommandList::Get()
{
	return m_commandList;
}

void TempGraphicsCommandList::DrawIndexed(Graphics& graphics)
{
	Finish(graphics);

	m_commandList->SetPipelineState(graphics, m_pipelineState.get());

	m_commandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	m_commandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	for (auto commandListBindable : m_bindableContainer.GetCommandListBindables())
		commandListBindable->BindToCommandList(graphics, m_commandList);

	m_commandList->DrawIndexed(graphics, m_bindableContainer.GetIndexBuffer()->GetIndexCount());
}

void TempGraphicsCommandList::Bind(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void TempGraphicsCommandList::Bind(Bindable* bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void TempGraphicsCommandList::Finish(Graphics& graphics)
{
	unsigned int frameIndex = graphics.GetCurrentBufferIndex();

	// initializing root signature
	{
		RootSignatureParams rootParams = {};

		for (auto rootSignatureBindable : m_bindableContainer.GetRootSignatureBindables())
			rootSignatureBindable->BindToRootSignature(&rootParams);

		m_rootSignature = RootSignature::GetResource(graphics, std::move(rootParams));
	}

	// initizalizing pipeline state
	{
		GraphicsPipelineStateParams pipelineStateParams = {};
		{
			for (auto pipelineStateBindable : m_bindableContainer.GetPipelineStateBindables())
				pipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);

			pipelineStateParams.SetRootSignature(m_rootSignature.get());

			pipelineStateParams.SetSampleMask(0xffffffff);

			pipelineStateParams.SetSampleDesc(1, 0);

			pipelineStateParams.SetNumRenderTargets(1);

			pipelineStateParams.SetRenderTargetFormat(0, graphics.GetBackBuffer()->GetFormat());

			pipelineStateParams.SetDepthStencilFormat(graphics.GetDepthStencil()->GetResource(graphics)->GetFormat());
		}

		m_pipelineState = GraphicsPipelineState::GetResource(graphics, std::move(pipelineStateParams));
	}
}