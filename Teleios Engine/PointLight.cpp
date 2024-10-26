#include "PointLight.h"
#include "Pipeline.h"

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
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("lightPosition") = position;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("diffuseColor") = color;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic") = 1.8f;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationLinear") = 0.144f;
	*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationConstant") = 0.001f;

	m_lightBuffer = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}});

	pipeline.AddStaticRootResource("lightBuffer", RootSignatureResourceType::type_constBufferView, m_lightBuffer.get());
}

void PointLight::DrawImguiWindow(Graphics& graphics, bool isLayerVisible)
{
	if (!isLayerVisible)
		return;

	if (ImGui::Begin("PointLight"))
	{
		bool changed = false;

		auto checkChanged = [&changed](bool expressionReturn) mutable
			{
				changed = changed || expressionReturn;
			};


		ImGui::Text("Position");
		checkChanged(ImGui::SliderFloat("x##position", &m_position.x, -100.0f, 100.0f));
		checkChanged(ImGui::SliderFloat("y##position", &m_position.y, -100.0f, 100.0f));
		checkChanged(ImGui::SliderFloat("z##position", &m_position.z, -100.0f, 100.0f));

		if(changed)
			*m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("lightPosition") = m_position;

		ImGui::NewLine();

		ImGui::Text("Colors");
		if(ImGui::ColorEdit3("Diffuse", reinterpret_cast<float*>(&m_color)))
		{
			changed = true;
			*m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("diffuseColor") = m_color;
		}

		ImGui::NewLine();

		ImGui::Text("Attenuation");
		checkChanged(ImGui::SliderFloat("quadratic", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationQuadratic"), 1.8f, 0.000002f));
		checkChanged(ImGui::SliderFloat("linear", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationLinear"), 0.7f, 0.00007f));
		checkChanged(ImGui::SliderFloat("constant", m_lightBuffer->GetData().GetValuePointer<DynamicConstantBuffer::ElementType::Float>("attenuationConstant"), 0.001f, 10.0f));

		if (changed)
			m_lightBuffer->Update(graphics);
	}

	ImGui::End();
};

void PointLight::Update()
{

}