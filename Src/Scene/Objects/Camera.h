#pragma once
#include "Includes/DirectXIncludes.h"
#include "Includes/WRLNoWarnings.h"

#include "Scene/SceneObject.h"

class Graphics;
class Pipeline;
class Input;

class CameraBase : public SceneObject
{
public:
	struct Settings
	{
		float FovAngleY = _pi / 2;
		float AspectRatio = 1.0f;
		float NearZ = 0.1f;
		float FarZ = 400.0f;
	};

protected:
	CameraBase(Graphics& graphics, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, bool isShadowCamera = false);

public:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	virtual void UpdateCameraBuffer();

	DirectX::XMMATRIX GetViewMatrix() const;

	DirectX::XMMATRIX GetPerspectiveMatrix() const;

	bool ViewChanged() const;
	bool PerspectiveChanged() const;

	const Settings* GetSettings() const;

	virtual SceneObjectType GetSceneObjectType() override;

	virtual void UpdateDecoratedName() override;

	void SetCameraBufferIndex(unsigned int cameraIndex);
	unsigned int GetCameraIndex();

	bool IsShadowCamera() const;

	void SetUpVector(DirectX::XMFLOAT3 up);
	void SetForwardVector(DirectX::XMFLOAT3 forward);

protected:
	void UpdatePerspectiveMatrix();
	void UpdateViewMatrix();

protected:
	CachedConstantBuffer* m_pCameraBuffer = nullptr;
	unsigned int m_cameraIndex = -1;

	DirectX::XMMATRIX m_perspective;
	DirectX::XMMATRIX m_view;
	DirectX::XMFLOAT3 m_upVector = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 m_forwardVector = { 0.0f, 0.0f, 1.0f };
	Settings m_settings;

	bool m_viewChanged = false;
	bool m_perspectiveChanged = false;
	bool m_isShadowCamera;
};

class Camera : public CameraBase
{
public:
	Camera(Graphics& graphics, DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f), DirectX::XMFLOAT3 rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), Settings* settings = nullptr);

public:
	void UpdateCamera(const Input& input, bool cursorLocked);

public:
	virtual void DrawTransformPropeties(Scene& scene) override;
	virtual void DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline) override;

	void SetActive(bool active);
	bool IsActive() const;

	virtual void UpdateDecoratedName() override;

private:
	void Look(POINTS lookOffset, bool multiplyBySensivity = true);
	void Move(DirectX::XMFLOAT3 direction, bool isFast = false);

	static constexpr float GetSlicedValue(float angle, float sliceValue);
	static constexpr float GetClampedValue(float angle, float minAngle, float maxAngle);

private:
	float m_pitch = 0.0f;
	float m_yaw = 0.0f;

	float m_sensivity = 0.02f;
	float m_speed = 0.06f;
	float m_fastSpeed = 0.12f;

	bool m_active = false;
};

class ShadowCamera : public CameraBase
{
	friend class PointLight;

public:
	ShadowCamera(Graphics& graphics, DirectX::XMFLOAT3 position);

	void UpdateCamera();

	virtual void UpdateCameraBuffer() override;

public:
	void SetPosition(DirectX::XMFLOAT3 position);
};