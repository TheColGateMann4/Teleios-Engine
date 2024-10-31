#include "PointLight.h"
#include "Pipeline.h"
#include "Camera.h"

#include <imgui.h>

PointLight::PointLight(Graphics& graphics, Pipeline& pipeline, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 color)
	:
	m_position(position),
	m_color(color)
{
	DynamicConstantBuffer::ConstantBufferLayout layout;
	layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("lightPosition");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("diffuseColor");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("attenuationLinear");
	layout.AddElement<DynamicConstantBuffer::ElementType::Float>("attenuationConstant");

	DynamicConstantBuffer::ConstantBufferData bufferData(layout);
	// *bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("lightPosition") = position; // we don't need to set this value since it will be overriden in Update() call by position in camera space
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("diffuseColor") = color;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic") = 1.8f;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationLinear") = 0.144f;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationConstant") = 0.001f;

	m_lightBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}});

	pipeline.AddStaticResource("lightBuffer", m_lightBuffer.get());
}

void PointLight::DrawImguiWindow(Graphics& graphics, bool isLayerVisible)
{
	if (!isLayerVisible)
		return;

	if (ImGui::Begin("PointLight"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("x##position", &m_position.x, -100.0f, 100.0f);
		ImGui::SliderFloat("y##position", &m_position.y, -100.0f, 100.0f);
		ImGui::SliderFloat("z##position", &m_position.z, -100.0f, 100.0f);

		ImGui::NewLine();

		ImGui::Text("Colors");
		ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("diffuseColor")));

		ImGui::NewLine();

		ImGui::Text("Attenuation");
		ImGui::SliderFloat("quadratic", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic"), 1.8f, 0.000002f);
		ImGui::SliderFloat("linear", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationLinear"), 0.7f, 0.00007f);
		ImGui::SliderFloat("constant", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationConstant"), 0.001f, 10.0f);
	}

	ImGui::End();
};

void PointLight::Update(Graphics& graphics, Pipeline& pipeline)
{
	DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&m_position);
	DirectX::XMVECTOR vResultPosition = DirectX::XMVector3Transform(vPosition, pipeline.GetCurrentCamera()->GetTransformMatrix());
	DirectX::XMFLOAT3 resultPosition;

	DirectX::XMStoreFloat3(&resultPosition, vResultPosition);

	*m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("lightPosition") = resultPosition;

	m_lightBuffer->Update(graphics);
}