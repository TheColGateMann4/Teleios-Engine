#include "PointLight.h"
#include "Graphics/Core/Pipeline.h"
#include "Camera.h"
#include "Sphere.h"
#include "Scene/Scene.h"

#include <imgui.h>

#include "Graphics/Data/DynamicConstantBuffer.h"

PointLight::PointLight(Graphics& graphics, Scene& scene, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color)
	:
	m_color(color),
	m_transformChanged(true),
	m_shadowCamera(graphics, position)
{
	m_transform.SetPosition(position);
	SetName("PointLight");

	// creating Sphere model
	{
		std::shared_ptr<Sphere> sphereModel = std::make_shared<Sphere>(graphics, DirectX::XMFLOAT3{}, 0.5f, 21);

		m_pSphereModel = sphereModel.get();
		AddChild(m_pSphereModel);
		m_pSphereModel->HideInHierarchy();

		scene.AddSceneObject(std::move(sphereModel));
	}
}

void PointLight::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	THROW_INTERNAL_ERROR_IF("Light index was not assigned", m_lightIndex == -1);

	m_pSphereModel->SetLightIndex(m_lightIndex);

	const Camera::Settings* shadowCameraSettings = m_shadowCamera.GetSettings();

	m_pLightBuffer = static_cast<CachedConstantBuffer*>(pipeline.GetStaticResource("lightBuffer").get());

	DynamicConstantBuffer::Data& bufferData = m_pLightBuffer->GetData();
	DynamicConstantBuffer::ArrayData array = bufferData.GetArrayData("lightBuffers");
	*array.Get<DynamicConstantBuffer::ElementType::Float3>(m_lightIndex, "diffuseColor") = m_color;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "attenuationQuadratic") = 0.2f;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "attenuationLinear") = 0.04f;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "attenuationConstant") = 0.07f;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "nearZ") = shadowCameraSettings->NearZ;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "farZ") = shadowCameraSettings->FarZ;

	m_shadowCamera.Initialize(graphics, pipeline);
}

void PointLight::Update(Graphics& graphics, Pipeline& pipeline)
{
	m_shadowCamera.UpdateCameraBuffer();

	m_transformChanged = false;
}

void PointLight::UpdateLight(Graphics& graphics, Scene& scene)
{
	// updating light constant buffer with light position in camera space
	Camera* activeCamera = scene.GetCurrentCamera();

	if (m_transformChanged || activeCamera->ViewChanged())
	{
		auto position = m_transform.GetPosition();

		DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&position);
		DirectX::XMVECTOR vResultPosition = DirectX::XMVector3Transform(vPosition, activeCamera->GetTransformMatrix());
		DirectX::XMFLOAT3 resultPosition;

		DirectX::XMStoreFloat3(&resultPosition, vResultPosition);

		DynamicConstantBuffer::Data& bufferData = m_pLightBuffer->GetData();
		DynamicConstantBuffer::ArrayData array = bufferData.GetArrayData("lightBuffers");
		*array.Get<DynamicConstantBuffer::ElementType::Float3>(m_lightIndex, "lightPosition") = resultPosition;
	}
}

void PointLight::DrawTransformPropeties(Scene& scene)
{
	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	ImGui::Text("Position");

	auto position = m_transform.GetPosition();
	checkChanged(m_transformChanged, ImGui::SliderFloat("x##position", &position.x, -100.0f, 100.0f));
	checkChanged(m_transformChanged, ImGui::SliderFloat("y##position", &position.y, -100.0f, 100.0f));
	checkChanged(m_transformChanged, ImGui::SliderFloat("z##position", &position.z, -100.0f, 100.0f));

	if (m_transformChanged)
	{
		m_transform.SetPosition(position);
		m_shadowCamera.SetPosition(position);
	}
}

void PointLight::DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline)
{
	bool changed = false;

	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	// drawing imgui propeties
	{
		DynamicConstantBuffer::Data& bufferData = m_pLightBuffer->GetData();
		DynamicConstantBuffer::ArrayData array = bufferData.GetArrayData("lightBuffers");

		if (array.DrawImguiProperties(m_lightIndex, false))
			m_pLightBuffer->Update(graphics);
	}

	m_pSphereModel->DrawAdditionalPropeties(graphics, pipeline);
}

SceneObjectType PointLight::GetSceneObjectType()
{
	return SceneObjectType::pointlight;
}

void PointLight::SetLightIndex(unsigned int lightIndex)
{
	m_lightIndex = lightIndex;
}

ShadowCamera* PointLight::GetShadowCamera()
{
	return &m_shadowCamera;
}