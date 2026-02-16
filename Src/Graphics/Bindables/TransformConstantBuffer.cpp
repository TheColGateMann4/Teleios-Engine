#include "TransformConstantBuffer.h"
#include "Graphics/Core/Graphics.h"
#include "Shaders/TargetShaders.h"
#include "Scene/Objects/Camera.h"
#include "Graphics/Data/DynamicConstantBuffer.h"
#include "Scene/ObjectTransform.h"

TransformConstantBuffer::TransformConstantBuffer(Graphics& graphics, std::vector<TargetSlotAndShader> targets)
	:
	RootParameterBinding(targets)
{
	DynamicConstantBuffer::Layout layout;
	layout.Add<DynamicConstantBuffer::ElementType::Matrix>("transform");

	m_buffer = std::make_shared<NonCachedConstantBuffer>(graphics, layout, targets);
}

void TransformConstantBuffer::SetParentPtr(ObjectTransform* pObjectTransform)
{
	m_pObjectTransform = pObjectTransform;
}

void TransformConstantBuffer::Update(Graphics& graphics, Camera& camera)
{
	DirectX::XMMATRIX transform = m_pObjectTransform->GetWorldTransform();

	m_buffer->Update(graphics, &transform, sizeof(transform));
}

NonCachedConstantBuffer* TransformConstantBuffer::GetBuffer() const
{
	return m_buffer.get();
}

void TransformConstantBuffer::BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target)
{
	commandList->SetGraphicsConstBufferView(graphics, GetBuffer(), target);
}

void TransformConstantBuffer::BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target)
{
	rootSignature->AddConstBufferViewParameter(m_buffer.get(), target);
}

BindableType TransformConstantBuffer::GetBindableType() const
{
	return BindableType::bindable_transformConstantBuffer;
}

RootSignatureBindableType TransformConstantBuffer::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_CBV;
}