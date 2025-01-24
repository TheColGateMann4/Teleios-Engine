#include "ObjectTransform.h"
#include "Camera.h"

void ObjectTransform::SetPosition(DirectX::XMFLOAT3 position)
{
	m_position = position;

	m_localTransformChanged = true;
}

void ObjectTransform::SetQuaternionRotation(DirectX::XMVECTOR rotation)
{
	m_rotation = rotation;

	m_localTransformChanged = true;
}

void ObjectTransform::SetEulerAnglesRotation(DirectX::XMFLOAT3 rotation)
{
	m_rotation = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	m_localTransformChanged = true;
}

void ObjectTransform::SetScale(DirectX::XMFLOAT3 scale)
{
	m_scale = scale;

	m_localTransformChanged = true;
}

DirectX::XMFLOAT3 ObjectTransform::GetPosition() const
{
	return m_position;
}

DirectX::XMVECTOR ObjectTransform::GetRotation() const
{
	return m_rotation;
}

DirectX::XMFLOAT3 ObjectTransform::GetScale() const
{
	return m_scale;
}

DirectX::XMFLOAT3& ObjectTransform::GetPositionLVal()
{
	return m_position;
}

DirectX::XMVECTOR& ObjectTransform::GetRotationLVal()
{
	return m_rotation;
}

DirectX::XMFLOAT3& ObjectTransform::GetScaleLVal()
{
	return m_scale;
}

DirectX::XMMATRIX ObjectTransform::GetWorldTransform() const
{
	return m_worldTransform;
}

void ObjectTransform::SetParentTransform(DirectX::XMMATRIX accumulatedParentTransform)
{
	m_accumulatedParentTransform = accumulatedParentTransform;

	UpdateWorldTransform();

	m_parentTransformChanged = true;
}

std::shared_ptr<TransformConstantBuffer> ObjectTransform::GetTransformConstantBuffer() const
{
	return m_transformConstantBuffer;
}

void ObjectTransform::SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer)
{
	m_transformConstantBuffer = transformConstantBuffer;
	m_transformConstantBuffer->SetParentPtr(this);
}

void ObjectTransform::UpdateTransformBufferIfNeeded(Graphics& graphics, Camera& camera)
{
	if (m_localTransformChanged || m_parentTransformChanged || camera.ViewChanged())
	{
		m_transformConstantBuffer->Update(graphics, camera);

		m_localTransformChanged = false;
		m_parentTransformChanged = false;
	}
}

void ObjectTransform::UpdateLocalTransformIfNeeded()
{
	if (m_localTransformChanged)
		UpdateLocalTransform();
}

void ObjectTransform::CheckIsTransformChanged(bool transformChanged)
{
	m_localTransformChanged |= transformChanged;
}

void ObjectTransform::UpdateLocalTransform()
{
	m_localTransform =
		DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) *
		DirectX::XMMatrixRotationQuaternion(m_rotation) *
		DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	UpdateWorldTransform();
}

void ObjectTransform::UpdateWorldTransform()
{
	m_worldTransform =  m_localTransform * m_accumulatedParentTransform;
}