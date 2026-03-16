#include "Camera.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "System/Input.h"
#include "Scene/Scene.h"

#include <imgui.h>

CameraBase::CameraBase(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, bool isShadowCamera)
	:
	m_isShadowCamera(isShadowCamera)
{
	m_transform.SetEulerRotation(rotation);
	m_transform.SetPosition(position);
	SetName("Camera");
}

void CameraBase::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	THROW_INTERNAL_ERROR_IF("Camera index was not assigned", m_cameraIndex == -1);

	m_pCameraBuffer = static_cast<CachedConstantBuffer*>(pipeline.GetStaticResource("cameraBuffer").get());
}

void CameraBase::UpdateCameraBuffer()
{
	THROW_INTERNAL_ERROR_IF("Camera buffer wasn't linked", m_pCameraBuffer == nullptr);

	DynamicConstantBuffer::Data& bufferData = m_pCameraBuffer->GetData();
	DynamicConstantBuffer::ArrayData array = bufferData.GetArrayData("cameraBuffers");

	*array.Get<DynamicConstantBuffer::ElementType::Matrix>(m_cameraIndex, "view") = GetViewMatrix();
	*array.Get<DynamicConstantBuffer::ElementType::Matrix>(m_cameraIndex, "projection") = GetPerspectiveMatrix();
}

DirectX::XMMATRIX CameraBase::GetViewMatrix() const
{
	return m_view;
}

DirectX::XMMATRIX CameraBase::GetPerspectiveMatrix() const
{
	return m_perspective;
}

bool CameraBase::ViewChanged() const
{
	return m_viewChanged;
}

bool CameraBase::PerspectiveChanged() const
{
	return m_viewChanged || m_perspectiveChanged;
}

const Camera::Settings* CameraBase::GetSettings() const
{
	return &m_settings;
}

SceneObjectType CameraBase::GetSceneObjectType()
{
	return SceneObjectType::camera;
}

void CameraBase::UpdateDecoratedName()
{
	m_decoratedName = m_originalName;
}

void CameraBase::SetCameraBufferIndex(unsigned int cameraIndex)
{
	m_cameraIndex = cameraIndex;
}

unsigned int CameraBase::GetCameraIndex()
{
	return m_cameraIndex;
}

bool CameraBase::IsShadowCamera() const
{
	return m_isShadowCamera;
}

void CameraBase::SetUpVector(DirectX::XMFLOAT3 up)
{
	m_upVector = up;
}

void CameraBase::SetForwardVector(DirectX::XMFLOAT3 forward)
{
	m_forwardVector = forward;
}

void CameraBase::UpdatePerspectiveMatrix()
{
	m_perspective = DirectX::XMMatrixPerspectiveFovLH(m_settings.FovAngleY, m_settings.AspectRatio, m_settings.NearZ, m_settings.FarZ);

	m_perspectiveChanged = true;
}

void CameraBase::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 startPosition = m_transform.GetPosition();
	DirectX::FXMVECTOR upVector = DirectX::XMLoadFloat3(&m_upVector);
	DirectX::FXMVECTOR forwardVector = DirectX::XMLoadFloat3(&m_forwardVector);
	DirectX::FXMVECTOR cameraPosition = DirectX::XMLoadFloat3(&startPosition);
	DirectX::XMVECTOR lookAtPosition = DirectX::XMVector3Transform(forwardVector, DirectX::XMMatrixRotationQuaternion(m_transform.GetQuaternionRotation()));
	lookAtPosition = DirectX::XMVectorAdd(lookAtPosition, cameraPosition);

	m_view = DirectX::XMMatrixLookAtLH(cameraPosition, lookAtPosition, upVector);

	m_viewChanged = true;
}

Camera::Camera(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, Settings* settings)
	:
	CameraBase(graphics, position, rotation, false)
{
	m_transform.SetEulerRotation(rotation);
	m_transform.SetPosition(position);
	SetName("Camera");

	if (settings != nullptr)
	{
		m_settings = *settings;

		THROW_OBJECT_STATE_ERROR_IF("Member of camera settings was not initialized", m_settings.FovAngleY == 0.0f || m_settings.AspectRatio == 0.0f || m_settings.NearZ == 0.0f || m_settings.FarZ == 0.0f);
		THROW_OBJECT_STATE_ERROR_IF("NearZ can't be the same value as FarZ", m_settings.NearZ == m_settings.FarZ);
	}
	else
	{
		m_settings = {};
		m_settings.AspectRatio = float(graphics.GetWidth()) / float(graphics.GetHeight());
	}

	UpdateViewMatrix();
	UpdatePerspectiveMatrix();
}

void Camera::UpdateCamera(const Input& input, bool cursorLocked)
{

	if (!m_active)
	{
		UpdateCameraBuffer();
		return;
	}

	if (!m_selected)
	{
		m_viewChanged = false;
		m_perspectiveChanged = false;
	}

	// rotation
	if (cursorLocked)
	{
		POINTS lookOffset = input.GetMouseDelta();
		std::swap(lookOffset.x, lookOffset.y);

		m_viewChanged |= lookOffset.x != 0 || lookOffset.y != 0;

		Look(lookOffset);
	}

	// position
	{
		DirectX::XMFLOAT3 direction = {};
		direction.x = float(input.GetKey(KEY_E)) - float(input.GetKey(KEY_A));
		direction.y = float(input.GetKey(VK_SPACE)) - float(input.GetKey(VK_CONTROL));
		direction.z = float(input.GetKey(KEY_W)) - float(input.GetKey(KEY_S));

		m_viewChanged |= direction.x != 0.0f || direction.y != 0.0f || direction.z != 0.0f;

		Move(direction, input.GetKey(VK_SHIFT));
	}

	if (m_viewChanged)
		UpdateViewMatrix();
}

void Camera::DrawTransformPropeties(Scene& scene)
{
	m_viewChanged = false;
	m_perspectiveChanged = false;

	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	if (ImGui::Button("Active"))
		scene.SetActiveCamera(this);

	DirectX::XMFLOAT3 position = m_transform.GetPosition();

	ImGui::Text("Position");

	bool positionChanged = false;
	checkChanged(positionChanged, ImGui::SliderFloat("x##position", &position.x, -100.0f, 100.0f));
	checkChanged(positionChanged, ImGui::SliderFloat("y##position", &position.y, -100.0f, 100.0f));
	checkChanged(positionChanged, ImGui::SliderFloat("z##position", &position.z, -100.0f, 100.0f));

	ImGui::NewLine();

	ImGui::Text("Rotation");

	bool rotationChanged = false;
	checkChanged(rotationChanged, ImGui::SliderFloat("pitch", &m_pitch, -100.0f, 100.0f));
	checkChanged(rotationChanged, ImGui::SliderFloat("yaw", &m_yaw, -100.0f, 100.0f));

	m_viewChanged = positionChanged || rotationChanged;

	if (positionChanged)
		m_transform.SetPosition(position);

	if (rotationChanged)
		m_transform.SetEulerRotation(position);

	if (m_transform.GetTransformChanged())
	{
		UpdateViewMatrix();
		UpdatePerspectiveMatrix();
	}
}

void Camera::DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline)
{
	bool changed = false;

	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};
	ImGui::Text("Conveniency Options:");
	ImGui::SliderFloat("Sensivity", &m_sensivity, 0.01f, 1.0f, "%.2f");
	ImGui::SliderFloat("Speed", &m_speed, 0.01f, 10.0f, "%.2f");
	ImGui::SliderFloat("FastSpeed", &m_fastSpeed, m_speed, m_speed + 20.0f, "%.1f");

	ImGui::NewLine();

	ImGui::Text("View Options:");
	checkChanged(changed, ImGui::SliderAngle("Fov", &m_settings.FovAngleY, 1.0f, 179.0f));
	checkChanged(changed, ImGui::SliderFloat("AspectRatio", &m_settings.AspectRatio, 0.1f, 2.0f));
	checkChanged(changed, ImGui::SliderFloat("NearZ", &m_settings.NearZ, 0.001f, 100.0f));
	checkChanged(changed, ImGui::SliderFloat("FarZ", &m_settings.FarZ, m_settings.NearZ + 0.01f, 800.0f));

	if (changed)
		UpdatePerspectiveMatrix();
}

void Camera::SetActive(bool active)
{
	m_active = active;
	m_viewChanged = true;
	m_perspectiveChanged = true;

	UpdateDecoratedName();
}

bool Camera::IsActive() const
{
	return m_active;
}

void Camera::UpdateDecoratedName()
{
	m_decoratedName = m_originalName;

	if (m_active)
		m_decoratedName += " (Active)";
}

void Camera::Look(POINTS lookOffset, bool multiplyBySensivity)
{
	static constexpr float almostRightAngle = 0.999f * (_pi / 2.0f);
	static constexpr float halfRotation = _pi;
	const float multipler = (multiplyBySensivity ? m_sensivity : 1.0f);

	m_yaw = GetSlicedValue(m_yaw + lookOffset.y * multipler, halfRotation);
	m_pitch = GetClampedValue(m_pitch + lookOffset.x * multipler, -almostRightAngle, almostRightAngle);

	m_transform.SetEulerRotation(DirectX::XMFLOAT3{ m_pitch, m_yaw, 0.0f });
}

void Camera::Move(DirectX::XMFLOAT3 direction, bool isFast)
{
	float multipler = (isFast ? m_fastSpeed : m_speed);

	direction.x *= multipler;
	direction.y *= multipler;
	direction.z *= multipler;

	DirectX::XMFLOAT3 startPosition = m_transform.GetPosition();

	DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&startPosition);
	DirectX::XMVECTOR vDirection = DirectX::XMLoadFloat3(&direction);

	vDirection = DirectX::XMVector3Transform(vDirection, DirectX::XMMatrixRotationQuaternion(m_transform.GetQuaternionRotation()));

	vPosition = DirectX::XMVectorAdd(vPosition, vDirection);

	DirectX::XMFLOAT3 position;

	DirectX::XMStoreFloat3(&position, vPosition);

	m_transform.SetPosition(position);
}

constexpr float Camera::GetSlicedValue(float angle, float sliceValue)
{
	if (angle >= -sliceValue && angle <= sliceValue)
		return angle;

	float absoluteAngle = std::abs(angle);
	float numPacks = absoluteAngle / sliceValue;
	float sizeOfLastPack = (numPacks - std::floor(numPacks)) * sliceValue;
	float valueOnOtherSide = sliceValue * ((angle < 0) ? 1.0f : -1.0f);
	float lastPackOnOtherSide = sizeOfLastPack * ((angle < 0) ? -1.0f : 1.0f);

	return lastPackOnOtherSide + valueOnOtherSide;
}

constexpr float Camera::GetClampedValue(float angle, float minAngle, float maxAngle)
{
	return angle > maxAngle
		? maxAngle
		: (angle < minAngle ? minAngle : angle);
}

ShadowCamera::ShadowCamera(Graphics& graphics, DirectX::XMFLOAT3 position)
	:
	CameraBase(graphics, position, DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f}, true)
{
	m_settings = {};
	m_settings.FovAngleY = _pi / 2.0f;
	m_settings.AspectRatio = 1.0f;
	m_settings.NearZ = 0.1f;
	m_settings.FarZ = 400.0f;

	UpdateViewMatrix();
	UpdatePerspectiveMatrix();
}

void ShadowCamera::UpdateCameraBuffer()
{
	THROW_INTERNAL_ERROR_IF("Camera buffer wasn't linked", m_pCameraBuffer == nullptr);

	DynamicConstantBuffer::Data& bufferData = m_pCameraBuffer->GetData();
	DynamicConstantBuffer::ArrayData array = bufferData.GetArrayData("cameraBuffers");

	static constexpr float angle = DirectX::XM_PIDIV2;

	struct RotationData
	{
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 up;
		DirectX::XMFLOAT3 forward;
	};

	static constexpr RotationData faceRotations[6] =
	{
		{ { 0.0f,	   angle,	0.0f }, { 0.0f, 1.0f, 0.0f  }, { 0.0f, 0.0f, 1.0f } },
		{ { 0.0f,	   -angle,	0.0f }, { 0.0f, 1.0f, 0.0f  }, { 0.0f, 0.0f, 1.0f } },
		{ { -angle,	  0.0f,	0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { angle,	  0.0f,	0.0f }, { 0.0f, 0.0f, 1.0f },  { 0.0f, 0.0f, 1.0f } },
		{ { 0.0f,	   0.0f,	0.0f }, { 0.0f, 1.0f, 0.0f  }, { 0.0f, 0.0f, 1.0f } },
		{ { 2 * angle, 0.0f,	0.0f }, { 0.0f, 1.0f, 0.0f  }, { 0.0f, 0.0f, 1.0f } }
	};

	for(int i = 0; i < ARRAYSIZE(faceRotations); i++)
	{
		m_transform.SetEulerRotation(faceRotations[i].rotation);
		SetUpVector(faceRotations[i].up);
		SetForwardVector(faceRotations[i].forward);

		UpdateViewMatrix();

		*array.Get<DynamicConstantBuffer::ElementType::Matrix>(m_cameraIndex + i, "view") = GetViewMatrix();
		*array.Get<DynamicConstantBuffer::ElementType::Matrix>(m_cameraIndex + i, "projection") = GetPerspectiveMatrix();
	}
}

void ShadowCamera::SetPosition(DirectX::XMFLOAT3 position)
{
	m_transform.SetPosition(position);
}