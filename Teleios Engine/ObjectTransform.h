#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "TransformConstantBuffer.h"

class Graphics;
class Camera;

class ObjectTransform
{
public:
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetQuaternionRotation(DirectX::XMVECTOR rotation);
	void SetEulerAnglesRotation(DirectX::XMFLOAT3 rotation);
	void SetScale(DirectX::XMFLOAT3 scale);

	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMVECTOR GetRotation() const;
	DirectX::XMFLOAT3 GetScale() const;

	// getting lvalue of private member is dirty from design perspective, but we need this for clean imgui workflow
	// for these functions CheckIsTransformChanged() has to be used in order to update matrices
	DirectX::XMFLOAT3& GetPositionLVal();
	DirectX::XMVECTOR& GetRotationLVal();
	DirectX::XMFLOAT3& GetScaleLVal();

public:
	DirectX::XMMATRIX GetWorldTransform() const;
	void SetParentTransform(DirectX::XMMATRIX accumulatedParentTransform);

public:
	void UpdateTransformBufferIfNeeded(Graphics& graphics, Camera& camera);
	void UpdateLocalTransformIfNeeded();
	void CheckIsTransformChanged(bool transformChanged);

private:
	void UpdateLocalTransform();
	void UpdateWorldTransform();

public:
	std::shared_ptr<TransformConstantBuffer> GetTransformConstantBuffer() const;
	void SetTransformConstantBuffer(std::shared_ptr<TransformConstantBuffer> transformConstantBuffer);

private:
	DirectX::XMFLOAT3 m_position = {0.0f, 0.0f, 0.0f};
	DirectX::XMVECTOR m_rotation = DirectX::XMQuaternionIdentity();
	DirectX::XMFLOAT3 m_scale = {1.0f, 1.0f, 1.0f};

	DirectX::XMMATRIX m_worldTransform = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_accumulatedParentTransform = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_localTransform = DirectX::XMMatrixIdentity();

	std::shared_ptr<TransformConstantBuffer> m_transformConstantBuffer = nullptr;

	bool m_localTransformChanged = false;
	bool m_parentTransformChanged = false;
};