#include "PointLight.h"
#include "Pipeline.h"
#include "Camera.h"

#include <imgui.h>

PointLight::PointLight(Graphics& graphics, Pipeline& pipeline, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color)
	:
	m_position(position),
	m_color(color),
	m_model(graphics, m_position, {0.0f, 0.0f, 0.0f}, 0.5f, 21),
	m_transformChanged(true)
{
	AddMesh(&m_model);
	
	DynamicConstantBuffer::ConstantBufferLayout layout;
	layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("lightPosition");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("diffuseColor");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("attenuationLinear");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("attenuationConstant");

	DynamicConstantBuffer::ConstantBufferData bufferData(layout);
	// *bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("lightPosition") = position; // we don't need to set this value since it will be overriden in Update() call by position in camera space
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("diffuseColor") = color;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic") = 0.2f;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationLinear") = 0.04f;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationConstant") = 0.07f;

	m_lightBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}});

	pipeline.AddStaticResource("lightBuffer", m_lightBuffer.get());
}

void PointLight::DrawImguiWindow(Graphics& graphics, bool isLayerVisible)
{
	m_model.DrawImguiWindow(graphics, isLayerVisible);

	if (!isLayerVisible)
		return;

	if (ImGui::Begin("PointLight"))
	{
		bool changed = false;

		auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
			{
				checkValue = checkValue || expressionReturn;
			};

		ImGui::Text("Position");
		checkChanged(m_transformChanged, ImGui::SliderFloat("x##position", &m_position.x, -100.0f, 100.0f));
		checkChanged(m_transformChanged, ImGui::SliderFloat("y##position", &m_position.y, -100.0f, 100.0f));
		checkChanged(m_transformChanged, ImGui::SliderFloat("z##position", &m_position.z, -100.0f, 100.0f));

		ImGui::NewLine();

		ImGui::Text("Diffuse");
		checkChanged(changed, ImGui::ColorEdit3("color##diffuse", reinterpret_cast<float*>(m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("diffuseColor"))));

		ImGui::NewLine();

		ImGui::Text("Attenuation");
		checkChanged(changed, ImGui::SliderFloat("quadratic", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic"), 0.000002f, 1.8f));
		checkChanged(changed, ImGui::SliderFloat("linear", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationLinear"), 0.00007f, 0.7f));
		checkChanged(changed, ImGui::SliderFloat("constant", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationConstant"), 0.001f, 10.0f));

		if(!m_transformChanged && changed)
			m_lightBuffer->Update(graphics);
	}

	ImGui::End();
};

void PointLight::Update(Graphics& graphics, Pipeline& pipeline)
{
	// updating light constant buffer with light position in camera space
	Camera* activeCamera = pipeline.GetCurrentCamera();

	if (m_transformChanged || activeCamera->ViewChanged())
	{
		DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&m_position);
		DirectX::XMVECTOR vResultPosition = DirectX::XMVector3Transform(vPosition, activeCamera->GetTransformMatrix());
		DirectX::XMFLOAT3 resultPosition;

		DirectX::XMStoreFloat3(&resultPosition, vResultPosition);

		*m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("lightPosition") = resultPosition;

		m_lightBuffer->Update(graphics);

		m_transformChanged = false;
	}

	// dragging our model with us
	m_model.SetPosition(m_position);

	m_lightBuffer->UpdateNextFrameResource(graphics); // updating static scene resource
}