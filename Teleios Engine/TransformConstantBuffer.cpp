#include "TransformConstantBuffer.h"
#include "Graphics.h"
#include "TargetShaders.h"
#include "Camera.h"
#include "Drawable.h"
#include "DynamicConstantBuffer.h"

TransformConstantBuffer::TransformConstantBuffer(Graphics& graphics, Drawable* pObject, std::vector<TargetSlotAndShader> targets)
	:
	RootSignatureBindable(targets),
	m_pObject(pObject),
	m_updated(false)
{
	DynamicConstantBuffer::ConstantBufferLayout layout;
	layout.AddElement<DynamicConstantBuffer::ElementType::Matrix>("transform");
	layout.AddElement<DynamicConstantBuffer::ElementType::Matrix>("transformInCameraSpace");
	layout.AddElement<DynamicConstantBuffer::ElementType::Matrix>("transformInCameraView");

	m_buffer = std::make_shared<NonCachedConstantBuffer>(graphics, layout, targets);
}

void TransformConstantBuffer::Update(Graphics& graphics, Camera& camera)
{
	m_updated = true;

	// matrices[0] transform
	// matrices[1] transform in camera space
	// matrices[2] transform in camera view
	DirectX::XMMATRIX matrices[3] = {};

	matrices[0] = m_pObject->GetTransformMatrix();
	matrices[1] = matrices[0] * camera.GetTransformMatrix();
	matrices[2] = matrices[1] * camera.GetPerspectiveMatrix();

	m_buffer->Update(graphics, matrices, sizeof(matrices));
}

NonCachedConstantBuffer* TransformConstantBuffer::GetBuffer() const
{
	return m_buffer.get();
}

void TransformConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList)
{
	commandList->SetGraphicsConstBufferView(graphics, GetBuffer());
}

void TransformConstantBuffer::BindToRootSignature(Graphics& graphics, RootSignature* rootSignature)
{
	rootSignature->AddConstBufferViewParameter(m_buffer.get());
}