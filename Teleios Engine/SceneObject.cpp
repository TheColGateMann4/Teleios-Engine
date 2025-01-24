#include "SceneObject.h"
#include "Graphics.h"
#include "Pipeline.h"

#include "Mesh.h"

#include <imgui.h>

SceneObject::SceneObject(SceneObject* pParent)
	:
	m_isChild(pParent != nullptr)
{
	if (pParent != nullptr)
		pParent->AddChild(this);
}

void SceneObject::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	Initialize(graphics, pipeline);

	for (auto& mesh : m_meshes)
		mesh.Initialize(graphics, pipeline);
}

void SceneObject::InternalUpdate(Graphics& graphics, Camera& camera, Pipeline& pipeline)
{
	Update(graphics, pipeline);

	for (auto& mesh : m_meshes)
		mesh.InternalUpdate(graphics, pipeline);
}


void SceneObject::InternalDraw(Graphics& graphics, Pipeline& pipeline) const
{
	Draw(graphics, pipeline);

	for (const auto& mesh : m_meshes)
		mesh.DrawMesh(graphics, pipeline);
}

void SceneObject::InternalAddStaticResources(Pipeline& pipeline)
{
	AddStaticResources(pipeline);
}

void SceneObject::InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& mesh : m_meshes)
		mesh.InitializeGraphicResources(graphics, pipeline);
}

void SceneObject::UpdateParentMatrix(DirectX::XMMATRIX parentMatrix)
{
	m_transform.SetParentTransform(parentMatrix);

	for (auto& child : m_children)
		child->UpdateParentMatrix(m_transform.GetWorldTransform());
}

void SceneObject::UpdateTransformBufferIfNeeded(Graphics& graphics, Camera& camera)
{
	// if object does not have any meshes then we don't need to update transformConstBuffer since no objects will be drawn using it
	if(!m_meshes.empty())
		m_transform.UpdateTransformBufferIfNeeded(graphics, camera);
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

	const bool nodeExpanded = ImGui::TreeNodeEx(m_name.c_str(), flags);

	if (ImGui::IsItemClicked())
		*selectedObject = *selectedObject != this ? this : nullptr;

	if (nodeExpanded)
	{
		for (auto& child : m_children)
			child->DrawHierarchy(selectedObject);

		ImGui::TreePop();
	}
}

void SceneObject::DrawTransformPropeties()
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
		mesh.DrawConstantBuffers(graphics);
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

std::string SceneObject::GetName() const
{
	return m_name;
}

void SceneObject::SetName(std::string newName)
{
	m_name = newName;
}