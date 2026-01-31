#include "Camera.h"
#include "Macros/ErrorMacros.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "System/Input.h"
#include "Scene/Scene.h"

#include <imgui.h>

Camera::Camera(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, Settings* settings)
	:
	m_position(position),
	m_rotation(rotation),
	m_viewChanged(false)
{
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

	UpdatePerspectiveMatrix();
}

void Camera::UpdateCamera(const Input& input, bool cursorLocked)
{
	if (!m_active)
		return;

	// rotation
	if(cursorLocked)
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
}

void Camera::DrawTransformPropeties(Scene& scene)
{
	// we are reseting viewChanged and m_perspectiveChanged values here since its the first function where it can be changed
	m_viewChanged = false;
	m_perspectiveChanged = false;

	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	if (ImGui::Button("Active"))
		scene.SetActiveCamera(this);

	ImGui::Text("Position");
	checkChanged(m_viewChanged, ImGui::SliderFloat("x##position", &m_position.x, -100.0f, 100.0f));
	checkChanged(m_viewChanged, ImGui::SliderFloat("y##position", &m_position.y, -100.0f, 100.0f));
	checkChanged(m_viewChanged, ImGui::SliderFloat("z##position", &m_position.z, -100.0f, 100.0f));

	ImGui::NewLine();

	ImGui::Text("Rotation");
	checkChanged(m_viewChanged, ImGui::SliderAngle("x##rotation", &m_rotation.x, -180.0f, 180.0f));
	checkChanged(m_viewChanged, ImGui::SliderAngle("y##rotation", &m_rotation.y, -180.0f, 180.0f));
	checkChanged(m_viewChanged, ImGui::SliderAngle("z##rotation", &m_rotation.z, -180.0f, 180.0f));
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

DirectX::XMMATRIX Camera::GetTransformMatrix() const
{
	DirectX::FXMVECTOR upVector = {0.0f, 1.0f, 0.0f};
	DirectX::FXMVECTOR forwardVector = {0.0f, 0.0f, 1.0f};
	DirectX::FXMVECTOR cameraPosition = DirectX::XMLoadFloat3(&m_position);
	DirectX::FXMVECTOR cameraRotation = DirectX::XMLoadFloat3(&m_rotation);
	DirectX::XMVECTOR lookAtPosition = DirectX::XMVector3Transform(forwardVector, DirectX::XMMatrixRotationRollPitchYawFromVector(cameraRotation));
	lookAtPosition = DirectX::XMVectorAdd(lookAtPosition, cameraPosition);

	return DirectX::XMMatrixLookAtLH(cameraPosition, lookAtPosition, upVector);
}

DirectX::XMMATRIX Camera::GetPerspectiveMatrix() const
{
	return m_perspective;
}

void Camera::Look(POINTS lookOffset, bool multiplyBySensivity)
{
	static constexpr float almostRightAngle = 0.999f * (_pi / 2.0f);
	static constexpr float halfRotation = _pi;
	float multipler = (multiplyBySensivity ? m_sensivity : 1.0f);

	m_rotation.y = GetSlicedValue(m_rotation.y + lookOffset.y * multipler, halfRotation);
	m_rotation.x = GetClampedValue(m_rotation.x + lookOffset.x * multipler, -almostRightAngle, almostRightAngle);
}

void Camera::Move(DirectX::XMFLOAT3 direction, bool isFast)
{
	float multipler = (isFast ? m_fastSpeed : m_speed);

	direction.x *= multipler;
	direction.y *= multipler;
	direction.z *= multipler;

	DirectX::FXMVECTOR vRotation = DirectX::XMLoadFloat3(&m_rotation);
	DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&m_position);
	DirectX::XMVECTOR vDirection = DirectX::XMLoadFloat3(&direction);

	vDirection = DirectX::XMVector3Transform(vDirection, DirectX::XMMatrixRotationRollPitchYawFromVector(vRotation));

	vPosition = DirectX::XMVectorAdd(vPosition, vDirection);

	DirectX::XMStoreFloat3(&m_position, vPosition);
}

bool Camera::ViewChanged() const
{
	return m_viewChanged;
}

bool Camera::PerspectiveChanged() const
{
	return m_viewChanged || m_perspectiveChanged;
}

const Camera::Settings* Camera::GetSettings() const
{
	return &m_settings;
}

SceneObjectType Camera::GetSceneObjectType()
{
	return SceneObjectType::camera;
}

void Camera::UpdateDecoratedName()
{
	m_decoratedName = m_originalName;

	if (m_active)
		m_decoratedName += " (Active)";
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

void Camera::UpdatePerspectiveMatrix()
{
	m_perspective = DirectX::XMMatrixPerspectiveFovLH(m_settings.FovAngleY, m_settings.AspectRatio, m_settings.NearZ, m_settings.FarZ);

	m_perspectiveChanged = true;
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
	