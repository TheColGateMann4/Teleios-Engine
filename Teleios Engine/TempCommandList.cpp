#include "TempCommandList.h"

#include "CommandList.h"
#include "Graphics.h"

#include "Macros/ErrorMacros.h"

#include "Shader.h"
#include "IndexBuffer.h"

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

	m_commandList->SetPipelineState(graphics, &m_pipelineState);

	m_commandList->SetComputeRootSignature(graphics, &m_rootSignature);

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

	m_commandList->SetPipelineState(graphics, &m_pipelineState);

	m_commandList->SetGraphicsRootSignature(graphics, &m_rootSignature);

	m_commandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	for (auto commandListBindable : m_bindableContainer.GetCommandListBindables())
		commandListBindable->BindToCommandList(graphics, m_commandList);

	m_commandList->DrawIndexed(graphics, m_bindableContainer.GetIndexBuffer()->GetIndexCount());
}

void TempGraphicsCommandList::Bind(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void TempGraphicsCommandList::BindIndexBuffer(std::shared_ptr<IndexBuffer> bindable)
{
	m_bindableContainer.SetIndexBuffer(bindable);
}

void TempGraphicsCommandList::BindVertexBuffer(std::shared_ptr<VertexBuffer> bindable)
{
	m_bindableContainer.SetVertexBuffer(bindable);
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
		for (auto rootSignatureBindable : m_bindableContainer.GetRootSignatureBindables())
			rootSignatureBindable->BindToRootSignature(graphics, &m_rootSignature);

		m_rootSignature.Initialize(graphics);
	}

	// initizalizing pipeline state
	{
		for (auto pipelineStateBindable : m_bindableContainer.GetPipelineStateBindables())
			pipelineStateBindable->BindToPipelineState(graphics, &m_pipelineState);

		m_pipelineState.SetRootSignature(&m_rootSignature);

		m_pipelineState.SetSampleMask(0xffffffff);
					   
		m_pipelineState.SetSampleDesc(1, 0);
					   
		m_pipelineState.SetNumRenderTargets(1);
					   
		m_pipelineState.SetRenderTargetFormat(0, graphics.GetBackBuffer()->GetFormat());
					   
		m_pipelineState.SetDepthStencilFormat(graphics.GetDepthStencil()->GetResource(graphics)->GetFormat());

		m_pipelineState.Finish(graphics);
	}
}