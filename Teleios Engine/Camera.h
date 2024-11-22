#pragma once
#include "includes/DirectXIncludes.h"
#include "includes/WRLNoWarnings.h"

class Graphics;
class Pipeline;
class Input;

class Camera
{
public:
	struct Settings
	{
		float FovAngleY = _pi / 2;
		float AspectRatio = 1.0f;
		float NearZ = 0.1f;
		float FarZ = 400.0f;
	};

public:
	Camera(Graphics& graphics, Pipeline& pipeline, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, -5.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f }, Settings* settings = nullptr);

public:
	void Update(const Input& input, bool cursorLocked);

	void DrawImguiWindow(bool isLayerVisible);

	DirectX::XMMATRIX GetTransformMatrix() const;

	DirectX::XMMATRIX GetPerspectiveMatrix() const;

	void Look(POINTS lookOffset, bool multiplyBySensivity = true);

	void Move(DirectX::XMFLOAT3 direction, bool isFast = false);

	bool ViewChanged() const;

private:
	void UpdatePerspectiveMatrix();

	static constexpr float GetSlicedValue(float angle, float sliceValue);
	static constexpr float GetClampedValue(float angle, float minAngle, float maxAngle);

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMMATRIX m_perspective;
	Settings m_settings;

	float m_sensivity = 0.02f;
	float m_speed = 0.06f;
	float m_fastSpeed = 0.12f;

	bool m_viewChanged;
};

