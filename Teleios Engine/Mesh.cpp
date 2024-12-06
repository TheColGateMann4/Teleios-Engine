#include "Mesh.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "Camera.h"


Mesh::Mesh(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation)
	:
	m_position(position),
	m_rotation(rotation)
{

}

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

		m_bindableContainer.GetTransformConstantBuffer()->SetParentPtr(this);

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

void Mesh::SetPosition(DirectX::XMFLOAT3 position)
{
	m_position = position;
}

DirectX::XMFLOAT3& Mesh::GetPositionLVal()
{
	return m_position;
}

DirectX::XMFLOAT3& Mesh::GetRotationLVal()
{
	return m_rotation;
}

void Mesh::InternalUpdate(Graphics& graphics)
{

}

void Mesh::UpdateTransformMatrix(Graphics& graphics, Camera& camera)
{
	if(m_transformChanged || camera.ViewChanged())
		m_bindableContainer.GetTransformConstantBuffer()->Update(graphics, camera);
}

DirectX::XMMATRIX Mesh::GetTransformMatrix() const
{
	DirectX::FXMVECTOR vecPosition = DirectX::XMLoadFloat3(&m_position);
	DirectX::FXMVECTOR vecRotation = DirectX::XMLoadFloat3(&m_rotation);

	//  multiplying position matrix by rotation matrix
	return DirectX::XMMatrixRotationRollPitchYawFromVector(vecRotation) * DirectX::XMMatrixTranslationFromVector(vecPosition);
}


bool Mesh::TransformChanged() const
{
	return m_transformChanged;
}

void Mesh::SetTransformChanged(bool val)
{
	m_transformChanged = val;
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

void Mesh::SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer)
{
	m_bindableContainer.SetTransformConstantBuffer(transformConstantBuffer);
}

const BindableContainer& Mesh::GetBindableContainter() const
{
	return m_bindableContainer;
}