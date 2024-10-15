#include "Camera.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

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

	m_perspective = DirectX::XMMatrixPerspectiveFovLH(m_settings.FovAngleY, m_settings.AspectRatio, m_settings.NearZ, m_settings.FarZ);
}

void Camera::DrawImguiWindow()
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::SliderFloat("PositionX", &m_position.x, -10.0f, 10.0f);
		ImGui::SliderFloat("PositionY", &m_position.y, -10.0f, 10.0f);
		ImGui::SliderFloat("PositionZ", &m_position.z, -10.0f, 10.0f);

		ImGui::SliderAngle("RotationX", &m_rotation.x, -180.0f, 180.0f);
		ImGui::SliderAngle("RotationY", &m_rotation.y, -180.0f, 180.0f);
		ImGui::SliderAngle("RotationZ", &m_rotation.z, -180.0f, 180.0f);


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