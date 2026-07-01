#include "StandaloneMesh.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Scene/Objects/Camera.h"

void StandaloneMesh::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_bindableContainer.Initialize(pipeline);

	// initializing root signature
	{
		RootSignatureParams rootParams = {};
		{
			for (auto& descriptorBindable : m_bindableContainer.GetDescriptorBindables())
				descriptorBindable->Initialize(graphics);

			for (auto& rootSignatureBindable : m_bindableContainer.GetRootSignatureBindables())
				rootSignatureBindable->BindToRootSignature(&rootParams);
		}

		m_rootSignature = RootSignature::GetResource(graphics, std::move(rootParams));
	}

	// initialize pipeline state object
	{
		GraphicsPipelineStateParams pipelineStateParams = {};
		{
			{
				const auto& pipelineStateBindables = m_bindableContainer.GetPipelineStateBindables();

				for (auto& pPipelineStateBindable : pipelineStateBindables)
					pPipelineStateBindable->AddPipelineStateParam(graphics, &pipelineStateParams);
			}

			pipelineStateParams.SetRasterizerState(BuildAndGetRasterizerState(graphics));

			pipelineStateParams.SetRootSignature(m_rootSignature.get());

			pipelineStateParams.SetSampleMask(0xffffffff);

			pipelineStateParams.SetSampleDesc(1, 0);

			pipelineStateParams.SetNumRenderTargets(1);

			pipelineStateParams.SetRenderTargetFormat(0, graphics.GetBackBuffer()->GetFormat());

			pipelineStateParams.SetDepthStencilFormat(DXGI_FORMAT_UNKNOWN);
		}

		m_pipelineState = GraphicsPipelineState::GetResource(graphics, std::move(pipelineStateParams));
	}

	for (auto* cachedBuffer : m_bindableContainer.GetCachedBuffers())
		cachedBuffer->Update(graphics);
}

void StandaloneMesh::Draw(Graphics& graphics, CommandList* commandList) const
{
	std::shared_ptr<IndexBufferEntry> indexBufferEntry = m_bindableContainer.GetIndexBufferEntry();
	std::shared_ptr<VertexBufferEntry> vertexBufferEntry;
	{
		auto attribBufferEntry = m_bindableContainer.GetAttributeVertexBufferEntry();
		auto positionBufferEntry = m_bindableContainer.GetPositionVertexBufferEntry();

		if (attribBufferEntry)
			vertexBufferEntry = attribBufferEntry;
		else if (positionBufferEntry)
			vertexBufferEntry = positionBufferEntry;
		else
		{
			THROW_INTERNAL_ERROR("Position buffer and Attribute buffer were both NULL");
		}
	}

	commandList->SetPipelineState(graphics, m_pipelineState.get());

	commandList->SetGraphicsRootSignature(graphics, m_rootSignature.get());

	{
		const auto& commandListBindables = m_bindableContainer.GetCommandListBindables();

		for (auto& pCommandListBindable : commandListBindables)
			pCommandListBindable->BindToCommandList(graphics, commandList);

		vertexBufferEntry->BindToCommandList(graphics, commandList);
		indexBufferEntry->BindToCommandList(graphics, commandList);
	}

	unsigned int indices = m_bindableContainer.GetIndexBufferEntry()->GetIndexCount();
	unsigned int baseVertexOffset = vertexBufferEntry->GetEntryInfo().offset;
	unsigned int startIndexOffset = indexBufferEntry->GetEntryInfo().offset;

	commandList->DrawIndexed(graphics, indices, baseVertexOffset, startIndexOffset);
};

void StandaloneMesh::Update(Graphics& graphics, Pipeline& pipeline)
{

}

void StandaloneMesh::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	for (auto texture : m_bindableContainer.GetTextures())
		texture->InitializeGraphicResources(graphics, pipeline);
}

void StandaloneMesh::DrawConstantBuffers(Graphics& graphics)
{
	const std::vector<CachedConstantBuffer*>& cachedBuffers = m_bindableContainer.GetCachedBuffers();

	for (auto& cachedBuffer : cachedBuffers)
		cachedBuffer->DrawImguiProperties(graphics);
}

void StandaloneMesh::AddStaticBindable(const char* bindableName)
{
	m_bindableContainer.AddStaticBindable(bindableName);
}

void StandaloneMesh::AddBindable(std::shared_ptr<Bindable> bindable)
{
	m_bindableContainer.AddBindable(bindable);
}

void StandaloneMesh::SetAttributeBufferEntry(std::shared_ptr<VertexBufferEntry> attributeBufferEntry)
{
	m_bindableContainer.SetAttributeBufferEntry(std::move(attributeBufferEntry));
}

void StandaloneMesh::SetPositionBufferEntry(std::shared_ptr<VertexBufferEntry> positionBufferEntry)
{
	m_bindableContainer.SetPositionBufferEntry(std::move(positionBufferEntry));
}

void StandaloneMesh::SetIndexBufferEntry(std::shared_ptr<IndexBufferEntry> indexBufferEntry)
{
	m_bindableContainer.SetIndexBufferEntry(std::move(indexBufferEntry));
}

const MeshBindableContainer& StandaloneMesh::GetBindableContainter() const
{
	return m_bindableContainer;
}

ObjectRasterizerStateOptions StandaloneMesh::GetRasterizerOptions() const
{
	return m_rasterizerOptions;
}

void StandaloneMesh::SetRasterizerOptions(ObjectRasterizerStateOptions rasterizerOptions)
{
	m_rasterizerOptions = rasterizerOptions;
}

RasterizerState* StandaloneMesh::BuildAndGetRasterizerState(Graphics& graphics)
{
	std::shared_ptr<RasterizerState> rasterizerState = RasterizerState::GetResource(graphics, RenderPassRasterizerStateOptions(), m_rasterizerOptions);
	RasterizerState* pRasterizerState = rasterizerState.get();

	m_bindableContainer.AddBindable(std::move(rasterizerState));

	return pRasterizerState;
}