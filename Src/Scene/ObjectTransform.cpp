#include "ObjectTransform.h"
#include "Scene/Objects/Camera.h"

#include "Graphics/Core/Graphics.h"

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

void ObjectTransform::SetEulerRotation(DirectX::XMFLOAT3 rotation)
{
	m_rotation = DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	m_localTransformChanged = true;
}

void ObjectTransform::SetScale(DirectX::XMFLOAT3 scale)
{
	m_scale = scale;

	m_localTransformChanged = true;
}

void ObjectTransform::SetFromMatrix(DirectX::XMMATRIX transform, DirectX::XMFLOAT3 basePosition, float scale)
{
	DirectX::XMVECTOR vecPos, vecRotQuat, vecScale;

	bool result = DirectX::XMMatrixDecompose(&vecScale, &vecRotQuat, &vecPos, transform);

	THROW_INTERNAL_ERROR_IF("Couldn't decompose node matrix", !result);

	// 'float scale' is used to scale vertices and positions of objects, used "externally"
	// 'float3 modelScale' is internal model scale which is needed to preserve scene look
	DirectX::XMFLOAT3 modelScale;

	DirectX::XMStoreFloat3(&basePosition, vecPos);
	DirectX::XMStoreFloat3(&modelScale, vecScale);

	basePosition.x *= scale;
	basePosition.y *= scale;
	basePosition.z *= scale;

	SetPosition(basePosition);
	SetQuaternionRotation(vecRotQuat);
	SetScale(modelScale);
}

DirectX::XMFLOAT3 ObjectTransform::GetPosition() const
{
	return m_position;
}

DirectX::XMVECTOR ObjectTransform::GetQuaternionRotation() const
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
	m_parentTransformChanged = !IsEqual(m_accumulatedParentTransform, accumulatedParentTransform);

	m_accumulatedParentTransform = accumulatedParentTransform;

	UpdateWorldTransform();
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

bool ObjectTransform::GetTransformChanged() const
{
	return m_localTransformChanged;
}

void ObjectTransform::SetUpdated()
{
	m_localTransformChanged = false;
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