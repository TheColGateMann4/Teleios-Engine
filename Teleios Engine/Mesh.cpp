#include "Mesh.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "Camera.h"

void Mesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.Initialize(pipeline);

	m_rootSignature = std::make_unique<RootSignature>();

	// initializing root signature
	{
		{
			const auto& rootSignatureBindables = m_bindableContainer.GetRootSignatureBindables();

			for (auto& pRootSignatureBindable : rootSignatureBindables)
			{
				pRootSignatureBindable->InternalInitialize(graphics);
				pRootSignatureBindable->BindToRootSignature(graphics, m_rootSignature.get());
			}
		}

		m_rootSignature->Initialize(graphics);
	}

	// initialize pipeline state object
{
		m_pipelineState = std::make_unique<GraphicsPipelineState>();

		// initializing pipeline state desc
		{
			{
				const auto& pipelineStateBindables = m_bindableContainer.GetPipelineStateBindables();

				for (auto& pPipelineStateBindable : pipelineStateBindables)
					pPipelineStateBindable->BindToPipelineState(graphics, m_pipelineState.get());
}

			m_pipelineState->SetRootSignature(m_rootSignature.get());

			m_pipelineState->SetSampleMask(0xffffffff);

			m_pipelineState->SetSampleDesc(1, 0);

			m_pipelineState->SetNumRenderTargets(1);

			m_pipelineState->SetRenderTargetFormat(0, graphics.GetBackBuffer()->GetFormat());

			m_pipelineState->SetDepthStencilFormat(graphics.GetDepthStencil()->GetFormat());
		}

		m_pipelineState->Finish(graphics); // Finish() call gets object from desc it made up
	}
}

void Mesh::DrawMesh(Graphics& graphics, Pipeline& pipeline) const
{
	CommandList* directCommandList = pipeline.GetGraphicCommandList();

	directCommandList->SetPipelineState(graphics, m_pipelineState.get());

	{
		const auto& cirectCommandListBindables = m_bindableContainer.GetDirectCommandListBindables();

		for (auto& pDirectCommandListBindable : cirectCommandListBindables)
			pDirectCommandListBindable->BindToDirectCommandList(graphics, directCommandList);
	}

	directCommandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	directCommandList->SetDescriptorHeap(graphics, &graphics.GetDescriptorHeap());

	{
		const auto& commandListBindables = m_bindableContainer.GetCommandListBindables();

		for (auto& pCommandListBindable : commandListBindables)
			pCommandListBindable->BindToCommandList(graphics, directCommandList);
	}

	directCommandList->DrawIndexed(graphics, m_bindableContainer.GetIndexBuffer()->GetIndexCount());
};

void Mesh::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.GetVertexBuffer()->BindToCopyPipelineIfNeeded(pipeline);

	m_bindableContainer.GetIndexBuffer()->BindToCopyPipelineIfNeeded(pipeline);
}

void Mesh::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	for (auto texture : m_bindableContainer.GetTextures())
		texture->InitializeGraphicResources(graphics, pipeline);
}

void Mesh::DrawConstantBuffers(Graphics& graphics)
{
	const std::vector<CachedConstantBuffer*>& cachedBuffers = m_bindableContainer.GetCachedBuffers();

	for (auto& cachedBuffer : cachedBuffers)
		cachedBuffer->DrawImguiProperties(graphics);
}

void Mesh::AddStaticBindable(const char* bindableName)
{
	m_bindableContainer.AddStaticBindable(bindableName);
}

void Mesh::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void Mesh::SetVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
{
	m_bindableContainer.SetVertexBuffer(vertexBuffer);
}

void Mesh::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
{
	m_bindableContainer.SetIndexBuffer(indexBuffer);
}

const MeshBindableContainer& Mesh::GetBindableContainter() const
{
	return m_bindableContainer;
}