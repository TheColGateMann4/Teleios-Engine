#include "PointLight.h"
#include "Graphics/Core/Pipeline.h"
#include "Camera.h"
#include "Sphere.h"
#include "Scene/Scene.h"

#include <imgui.h>

#include "Graphics/Data/DynamicConstantBuffer.h"

PointLight::PointLight(Graphics& graphics, Scene& scene, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color)
	:
	m_position(position),
	m_color(color),
	m_transformChanged(true)
{
	SetName("PointLight");

	// creating Sphere model
	{
		std::shared_ptr<Sphere> sphereModel = std::make_shared<Sphere>(graphics, m_position, DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f }, 0.5f, 21);

		m_pSphereModel = sphereModel.get();
		AddChild(m_pSphereModel);
		m_pSphereModel->HideInHierarchy();

		scene.AddSceneObject(std::move(sphereModel));
	}
}

void PointLight::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_pLightBuffer = static_cast<CachedConstantBuffer*>(pipeline.GetStaticResource("lightBuffer"));

	DynamicConstantBuffer::Data& bufferData = m_pLightBuffer->GetData();
	DynamicConstantBuffer::ArrayData array = bufferData.GetArrayData("lightBuffers");
	*array.Get<DynamicConstantBuffer::ElementType::Float3>(m_lightIndex, "diffuseColor") = m_color;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "attenuationQuadratic") = 0.2f;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "attenuationLinear") = 0.04f;
	*array.Get<DynamicConstantBuffer::ElementType::Float>(m_lightIndex, "attenuationConstant") = 0.07f;
}

void PointLight::Update(Graphics& graphics, Pipeline& pipeline)
{
	// dragging our model with us
	m_pSphereModel->GetTransform()->SetPosition(m_position);
}

void PointLight::UpdateLight(Graphics& graphics, Scene& scene)
{
	// updating light constant buffer with light position in camera space
	Camera* activeCamera = scene.GetCurrentCamera();

	if (m_transformChanged || activeCamera->ViewChanged())
	{
		DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&m_position);
		DirectX::XMVECTOR vResultPosition = DirectX::XMVector3Transform(vPosition, activeCamera->GetTransformMatrix());
		DirectX::XMFLOAT3 resultPosition;

		DirectX::XMStoreFloat3(&resultPosition, vResultPosition);

		DynamicConstantBuffer::Data& bufferData = m_pLightBuffer->GetData();
		DynamicConstantBuffer::ArrayData array = bufferData.GetArrayData("lightBuffers");
		*array.Get<DynamicConstantBuffer::ElementType::Float3>(m_lightIndex, "lightPosition") = resultPosition;

		m_pLightBuffer->Update(graphics);

		m_transformChanged = false;
	}
}

void PointLight::DrawTransformPropeties(Scene& scene)
{
	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	ImGui::Text("Position");
	checkChanged(m_transformChanged, ImGui::SliderFloat("x##position", &m_position.x, -100.0f, 100.0f));
	checkChanged(m_transformChanged, ImGui::SliderFloat("y##position", &m_position.y, -100.0f, 100.0f));
	checkChanged(m_transformChanged, ImGui::SliderFloat("z##position", &m_position.z, -100.0f, 100.0f));
}

void PointLight::DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline)
{
	bool changed = false;

	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	m_pLightBuffer->DrawImguiProperties(graphics);

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