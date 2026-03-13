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

	void UpdateCameraBuffer();

	DirectX::XMMATRIX GetTransformMatrix() const;

	DirectX::XMMATRIX GetPerspectiveMatrix() const;

	bool ViewChanged() const;
	bool PerspectiveChanged() const;

	const Settings* GetSettings() const;

	virtual SceneObjectType GetSceneObjectType() override;

	virtual void UpdateDecoratedName() override;

	void SetCameraIndex(unsigned int cameraIndex);
	unsigned int GetCameraIndex();

	bool IsShadowCamera() const;

protected:
	void UpdatePerspectiveMatrix();

protected:
	CachedConstantBuffer* m_pCameraBuffer = nullptr;
	unsigned int m_cameraIndex = -1;

	DirectX::XMMATRIX m_perspective;
	Settings m_settings;

	bool m_viewChanged;
	bool m_perspectiveChanged;
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

public:
	void SetPosition(DirectX::XMFLOAT3 position);
};