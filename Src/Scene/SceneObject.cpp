#include "SceneObject.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Mesh.h"

#include <imgui.h>

SceneObject::SceneObject(SceneObject* pParent)
	:
	m_isChild(pParent != nullptr)
{
	if (pParent != nullptr)
		pParent->AddChild(this);
}

void SceneObject::UpdateBoundingBox()
{
	for (auto& mesh : m_meshes)
		for (auto& technique : mesh.GetTechniques())
			for (auto& step : technique.GetSteps())
				m_boundingBox.Add(step.GetBoundingBox());
}

void SceneObject::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	// initializing modelSceneIndex
	{
		DynamicConstantBuffer::Layout layout;
		layout.Add<DynamicConstantBuffer::ElementType::Uint>("modelSceneIndex");

		layout.GetFinished(DynamicConstantBuffer::Layout::LayoutType::data);

		DynamicConstantBuffer::Data bufferData(layout);
		*bufferData.Get<DynamicConstantBuffer::ElementType::Uint>("modelSceneIndex") = m_sceneIndex;

		m_sceneIndexConstant = std::make_shared<RootSignatureConstants>(bufferData, ResourceTargets{{ShaderVisibilityGraphic::VertexShader, 3}});
	}

	Initialize(graphics, pipeline);

	for (auto& mesh : m_meshes)
		mesh.Initialize(graphics, pipeline);

	// adding modelSceneIndex bind to all meshe's steps
	// specificly after mesh.Initialize() since meshes create implicit techniques there
	for (auto& mesh : m_meshes)
		for (auto& technique : mesh.GetTechniques())
			for (auto& step : technique.GetSteps())
				step.AddBindable(m_sceneIndexConstant);

	UpdateBoundingBox();
}

void SceneObject::SubmitJobs(Renderer& renderer)
{
	for (auto& mesh : m_meshes)
		mesh.SubmitJobs(renderer);
}

void SceneObject::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{
	Update(graphics, pipeline);
}

void SceneObject::InternalAddStaticResources(Pipeline& pipeline)
{
	AddStaticResources(pipeline);
}

void SceneObject::UpdateParentMatrix(DirectX::XMMATRIX parentMatrix)
{
	m_transform.SetParentTransform(parentMatrix);

	for (auto& child : m_children)
		child->UpdateParentMatrix(m_transform.GetWorldTransform());
}

void SceneObject::UpdateLocalTransformIfNeeded()
{
	m_transform.UpdateLocalTransformIfNeeded();
}

void SceneObject::Initialize(Graphics& graphics, Pipeline& pipeline)
{

}

void SceneObject::Draw(Graphics& graphics, Pipeline& pipeline) const
{

}

void SceneObject::Update(Graphics& graphics, Pipeline& pipeline)
{

}

void SceneObject::AddStaticResources(Pipeline& pipeline)
{

}

void SceneObject::DrawHierarchy(SceneObject** selectedObject)
{
	if (m_hideInHierarchy)
		return;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;

	if (*selectedObject == this)
		flags |= ImGuiTreeNodeFlags_Selected;

	if (!hasVisibleChildren())
		flags |= flags |= ImGuiTreeNodeFlags_Leaf;

	const bool nodeExpanded = ImGui::TreeNodeEx(GetDecoratedName().c_str(), flags);

	if (ImGui::IsItemClicked())
	{
		*selectedObject = *selectedObject != this ? this : nullptr;
		
		if(*selectedObject)
			(*selectedObject)->m_selected = false;

		m_selected = true;
	}

	if (nodeExpanded)
	{
		for (auto& child : m_children)
			child->DrawHierarchy(selectedObject);

		ImGui::TreePop();
	}
}

void SceneObject::DrawTransformPropeties(Scene& scene)
{
	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	bool transformChanged = false;

	DirectX::XMFLOAT3& position = m_transform.GetPositionLVal();
	//DirectX::XMFLOAT3& rotation = m_transform.GetRotationLVal();
	DirectX::XMFLOAT3& scale = m_transform.GetScaleLVal();

	ImGui::Text("Position");
	checkChanged(transformChanged, ImGui::SliderFloat("x##position", &position.x, -100.0f, 100.0f));
	checkChanged(transformChanged, ImGui::SliderFloat("y##position", &position.y, -100.0f, 100.0f));
	checkChanged(transformChanged, ImGui::SliderFloat("z##position", &position.z, -100.0f, 100.0f));

//	ImGui::NewLine();
//
//	ImGui::Text("Rotation");
//	checkChanged(transformChanged, ImGui::SliderAngle("x##rotation", &rotation.x, -180.0f, 180.0f));
//	checkChanged(transformChanged, ImGui::SliderAngle("y##rotation", &rotation.y, -180.0f, 180.0f));
//	checkChanged(transformChanged, ImGui::SliderAngle("z##rotation", &rotation.z, -180.0f, 180.0f));

	ImGui::NewLine();

	ImGui::Text("Scale");
	checkChanged(transformChanged, ImGui::SliderFloat("x##scale", &scale.x, 0.01f, 10.0f));
	checkChanged(transformChanged, ImGui::SliderFloat("y##scale", &scale.y, 0.01f, 10.0f));
	checkChanged(transformChanged, ImGui::SliderFloat("z##scale", &scale.z, 0.01f, 10.0f));

	m_transform.CheckIsTransformChanged(transformChanged);
}

void SceneObject::DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline)
{

}

void SceneObject::DrawConstantBuffers(Graphics& graphics)
{
	for (auto& mesh : m_meshes)
	{
		ImGui::Separator();

		std::vector<RenderTechnique>& techniques = mesh.GetTechniques();

		for (RenderTechnique& technique : techniques)
		{
			std::string techniqueNodeName = "Technique: " + std::to_string(static_cast<int>(technique.GetType()));

			if (ImGui::TreeNodeEx(
				techniqueNodeName.c_str(),
				ImGuiTreeNodeFlags_DefaultOpen
			))
			{
				std::vector<RenderGraphicsStep>& steps = technique.GetSteps();
				for (RenderGraphicsStep& step : steps)
				{
					std::string stepNodeName = "Step: " + step.GetName();

					if (ImGui::TreeNodeEx(
						stepNodeName.c_str(),
						ImGuiTreeNodeFlags_DefaultOpen
					))
					{
						step.DrawConstantBuffers(graphics);

						ImGui::TreePop();
					}

				}

				ImGui::TreePop();
			}
		}
	}
}

bool SceneObject::isChild() const
{
	return m_isChild;
}

void SceneObject::MakeChild()
{
	m_isChild = this;
}

void SceneObject::HideInHierarchy()
{
	m_hideInHierarchy = true;
}

bool SceneObject::IsVisibleInHierarchy()
{
	return !m_hideInHierarchy;
}

bool SceneObject::hasVisibleChildren()
{
	for (const auto& child : m_children)
		if (child->IsVisibleInHierarchy())
			return true;

	return false;
}

void SceneObject::AddMesh(Mesh& mesh)
{
	m_meshes.push_back(std::move(mesh));
}

void SceneObject::AddChild(SceneObject* object)
{
	object->MakeChild();

	m_children.push_back(object);
}

ObjectTransform* SceneObject::GetTransform()
{
	return &m_transform;
}

const BoundingBox& SceneObject::GetBoundingBox() const
{
	return m_boundingBox;
}

void SceneObject::SetSceneIndex(unsigned int sceneIndex)
{
	m_sceneIndex = sceneIndex;
}

std::string SceneObject::GetName() const
{
	return m_name;
}

void SceneObject::SetName(std::string newName)
{
	m_name = newName;

	UpdateOriginalName();
	UpdateDecoratedName();
}

std::string SceneObject::GetDecoratedName() const
{
	return m_decoratedName;
}

std::string SceneObject::GetOriginalName() const
{
	return m_originalName;
}

unsigned int SceneObject::GetNameIndex() const
{
	return m_nameIndex;
}

void SceneObject::SetNameIndex(unsigned int nameIndex)
{
	m_nameIndex = nameIndex;

	UpdateOriginalName();
	UpdateDecoratedName();
}

void SceneObject::UpdateOriginalName()
{
	m_originalName = m_name;

	if (m_nameIndex > 0)
	{
		m_originalName += " (";
		m_originalName += std::to_string(m_nameIndex);
		m_originalName += ")";
	}
}

void SceneObject::UpdateDecoratedName()
{
	m_decoratedName = m_originalName;
}

SceneObjectType SceneObject::GetSceneObjectType()
{
	return SceneObjectType::unknown;
}