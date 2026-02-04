#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Scene/SceneObject.h"

class Graphics;
class Pipeline;
class Input;

class Camera : public SceneObject
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
	Camera(Graphics& graphics, DirectX::XMFLOAT3 position = { 0.0f, 0.0f, -5.0f }, DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f }, Settings* settings = nullptr);

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	void UpdateCamera(const Input& input, bool cursorLocked);

	void UpdateCameraBuffer();

	virtual void DrawTransformPropeties(Scene& scene) override;

	virtual void DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline) override;

	DirectX::XMMATRIX GetTransformMatrix() const;

	DirectX::XMMATRIX GetPerspectiveMatrix() const;

	void Look(POINTS lookOffset, bool multiplyBySensivity = true);

	void Move(DirectX::XMFLOAT3 direction, bool isFast = false);

	bool ViewChanged() const;
	bool PerspectiveChanged() const;

	const Settings* GetSettings() const;

	virtual SceneObjectType GetSceneObjectType() override;

	virtual void UpdateDecoratedName() override;

	void SetCameraIndex(unsigned int cameraIndex);
	unsigned int GetCameraIndex();

public:
	void SetActive(bool active);

	bool IsActive() const;

private:
	void UpdatePerspectiveMatrix();

	static constexpr float GetSlicedValue(float angle, float sliceValue);
	static constexpr float GetClampedValue(float angle, float minAngle, float maxAngle);

private:
	CachedConstantBuffer* m_pCameraBuffer = nullptr;
	unsigned int m_cameraIndex = -1;

	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMMATRIX m_perspective;
	Settings m_settings;

	float m_sensivity = 0.02f;
	float m_speed = 0.06f;
	float m_fastSpeed = 0.12f;

	bool m_viewChanged;
	bool m_perspectiveChanged;

	bool m_active = false;
};