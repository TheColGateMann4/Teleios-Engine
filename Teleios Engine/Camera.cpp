#include "Camera.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"
#include "Input.h"

#include <imgui.h>

Camera::Camera(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, Settings* settings)
	:
	m_position(position),
	m_rotation(rotation)
{
	if (settings != nullptr)
	{
		m_settings = *settings;

		THROW_INTERNAL_ERROR_IF("Member of camera settings was not initialized", m_settings.FovAngleY == 0.0f || m_settings.AspectRatio == 0.0f || m_settings.NearZ == 0.0f || m_settings.FarZ == 0.0f);
		THROW_INTERNAL_ERROR_IF("NearZ can't be the same value as FarZ", m_settings.NearZ == m_settings.FarZ);
	}
	else
	{
		m_settings = {}; 
		m_settings.AspectRatio = float(graphics.GetWidth()) / float(graphics.GetHeight());
	}

	UpdatePerspectiveMatrix();
}

void Camera::Update(const Input& input)
{
	// rotation
	{
		POINTS lookOffset = input.GetMouseDelta();
		std::swap(lookOffset.x, lookOffset.y);

		Look(lookOffset);
	}

	// position
	{
		DirectX::XMFLOAT3 direction = {};
		direction.x = float(input.GetKey(KEY_E)) - float(input.GetKey(KEY_A));
		direction.y = float(input.GetKey(VK_SPACE)) - float(input.GetKey(VK_CONTROL));
		direction.z = float(input.GetKey(KEY_W)) - float(input.GetKey(KEY_S));

		Move(direction, input.GetKey(VK_SHIFT));
	}
}

void Camera::DrawImguiWindow()
{
	if (ImGui::Begin("Camera"))
	{
		bool changed = false;

		auto checkChanged = [&changed](bool expressionReturn) mutable
			{
				changed = changed || expressionReturn;
			};

		ImGui::Text("Position & Rotation:");
		ImGui::SliderFloat("PositionX", &m_position.x, -10.0f, 10.0f);
		ImGui::SliderFloat("PositionY", &m_position.y, -10.0f, 10.0f);
		ImGui::SliderFloat("PositionZ", &m_position.z, -10.0f, 10.0f);

		ImGui::SliderAngle("RotationX", &m_rotation.x, -180.0f, 180.0f);
		ImGui::SliderAngle("RotationY", &m_rotation.y, -180.0f, 180.0f);
		ImGui::SliderAngle("RotationZ", &m_rotation.z, -180.0f, 180.0f);

		ImGui::NewLine();

		ImGui::Text("Conveniency Options:");
		ImGui::SliderFloat("Sensivity", &m_sensivity, 0.01f, 1.0f, "%.2f");
		ImGui::SliderFloat("Speed", &m_speed, 0.01f, 10.0f, "%.2f");
		ImGui::SliderFloat("FastSpeed", &m_fastSpeed, m_speed, m_speed + 20.0f, "%.1f");

		ImGui::NewLine();

		ImGui::Text("View Options:");
		checkChanged(ImGui::SliderAngle("Fov", &m_settings.FovAngleY, 1.0f, 179.0f));
		checkChanged(ImGui::SliderFloat("AspectRatio", &m_settings.AspectRatio, 0.1f, 2.0f));
		checkChanged(ImGui::SliderFloat("NearZ", &m_settings.NearZ, 0.001f, 100.0f));
		checkChanged(ImGui::SliderFloat("FarZ", &m_settings.FarZ, m_settings.NearZ + 0.01f, 800.0f));

		if (changed)
			UpdatePerspectiveMatrix();

		ImGui::End();
	}
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
	float multipler = (multiplyBySensivity ? m_sensivity : 1.0f);

	m_rotation.x += lookOffset.x * multipler;
	m_rotation.y += lookOffset.y * multipler;
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

void Camera::UpdatePerspectiveMatrix()
{
	m_perspective = DirectX::XMMatrixPerspectiveFovLH(m_settings.FovAngleY, m_settings.AspectRatio, m_settings.NearZ, m_settings.FarZ);
}