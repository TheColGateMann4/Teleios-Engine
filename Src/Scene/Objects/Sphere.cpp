#include "Sphere.h"
#include "Includes/CppIncludes.h"
#include <directxtk12/GeometricPrimitive.h>

#include "Includes/BindablesInclude.h"

#include "Graphics/Data/DynamicVertex.h"

#include "Graphics/RenderGraph/Steps/RenderGraphicsStep.h"
#include "Scene/RenderTechnique.h"

#include <imgui.h>

Sphere::Sphere(Graphics& graphics, DirectX::XMFLOAT3 position, float diameter, size_t tesselation)
	:
	m_initialized(false),
	m_diameter(diameter),
	m_tesselation(tesselation)
{
	SetName("Sphere");
	m_transform.SetPosition(position);

	Mesh modelMesh;

	RenderTechnique technique(RenderJob::JobType::Emissive);
	RenderGraphicsStep step(this);
	{
		DynamicVertex::DynamicVertexLayout vertexLayout;
		vertexLayout.AddElement<DynamicVertex::ElementType::Position>();
		vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();
		vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();

		step.AddBindable(InputLayout::GetResource(graphics, vertexLayout));

		step.AddBindable(Shader::GetResource(graphics, L"PS_LightSource", ShaderType::PixelShader));
		std::vector<const char*> macros = {"NORMAL", "TEXCOORDS"};
		step.AddBindable(Shader::GetResource(graphics, L"VS", ShaderType::VertexShader));
		step.AddBindable(PrimitiveTechnology::GetResource(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE));
		step.AddStaticBindable("lightBuffer");

		// adding root signature constant to indicate locally used point light data
		{
			DynamicConstantBuffer::Layout layout;
			layout.Add<DynamicConstantBuffer::ElementType::Int>("pointLightIndex");

			layout.GetFinished(DynamicConstantBuffer::Layout::LayoutType::data);

			DynamicConstantBuffer::Data bufferData(layout);
			*bufferData.Get<DynamicConstantBuffer::ElementType::Int>("pointLightIndex") = 0;

			m_pointLightIndexConstant = std::make_shared<RootSignatureConstants>(bufferData, ResourceTargets{{ShaderVisibilityGraphic::PixelShader, 1}});

			step.AddBindable(m_pointLightIndexConstant);
		}
	}
	technique.AddStep(std::move(step));
	modelMesh.AddTechnique(std::move(technique));
	AddMesh(modelMesh);
}

void Sphere::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	// upload mesh data to object
	UpdateMesh(graphics, pipeline);
}

void Sphere::SetLightIndex(unsigned int lightIndex)
{
	THROW_INTERNAL_ERROR_IF("Tried to call SetLightIndex on sphere that wasn't meant as light source", !m_pointLightIndexConstant);

	*m_pointLightIndexConstant->GetData().Get<DynamicConstantBuffer::ElementType::Int>("pointLightIndex") = lightIndex;
}

void Sphere::DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline)
{
	bool changed = false;

	auto checkChanged = [&changed](bool expressionReturn) mutable
		{
			changed = changed || expressionReturn;
		};

	static constexpr int minTesselationVal = 3;
	static constexpr int maxTesselationVal = 64;


	ImGui::Text("Mesh Propeties");
	checkChanged(ImGui::SliderFloat("diameter", &m_diameter, 0.1f, 10.0f));
	checkChanged(ImGui::SliderScalar("tesselation", ImGuiDataType_U32, &m_tesselation, &minTesselationVal, &maxTesselationVal));

	if (changed)
		UpdateMesh(graphics, pipeline);
}

void Sphere::UpdateMesh(Graphics& graphics, Pipeline& pipeline)
{
	DirectX::DX12::GeometricPrimitive::VertexCollection vertices = {};
	DirectX::DX12::GeometricPrimitive::IndexCollection indices = {};

	DynamicVertex::DynamicVertexLayout vertexLayout;
	vertexLayout.AddElement<DynamicVertex::ElementType::Position>();
	vertexLayout.AddElement<DynamicVertex::ElementType::Normal>();
	vertexLayout.AddElement<DynamicVertex::ElementType::TextureCoords>();
	vertexLayout.Finish();

	DirectX::DX12::GeometricPrimitive::CreateSphere(vertices, indices, m_diameter, m_tesselation, false);

	Mesh& modelMesh = m_meshes.front();

	RenderGraphicsStep& albedoStep = modelMesh.GetTechnique(RenderJob::JobType::Emissive).GetStep(0);

	if (!m_initialized)
	{
		albedoStep.SetAttributeBufferEntry(VertexBufferEntry::GetResource(graphics, "Sphere", vertices.data(), vertexLayout, vertices.size()));

		albedoStep.SetIndexBufferEntry(IndexBufferEntry::GetResource(graphics, "Sphere", std::move(indices)));

		albedoStep.SetBoundingBox(BoundingBox(m_diameter / 2.0f));

		m_initialized = true;
	}
	else
	{
		// TODO: Add dynamicly adding data to vertexBuffers

		//VertexBuffer* vbuffer = albedoStep.GetBindableContainter().GetVertexBuffer();
		//IndexBuffer* ibuffer = albedoStep.GetBindableContainter().GetIndexBuffer();
		//
		//vbuffer->Update(graphics, vertices.data(), vertices.size(), sizeof(vertices.front()));
		//ibuffer->Update(graphics, indices.data(), indices.size(), sizeof(indices.front()));
		//
		//vbuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
		//ibuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	}
}