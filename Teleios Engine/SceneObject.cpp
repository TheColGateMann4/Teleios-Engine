#include "SceneObject.h"
#include "Graphics.h"
#include "Pipeline.h"

#include "Mesh.h"

#include <imgui.h>

void SceneObject::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	Initialize(graphics, pipeline);

	for (auto& mesh : m_meshes)
		mesh.Initialize(graphics, pipeline);

	for (auto& child : m_children)
		child->InternalInitialize(graphics, pipeline);
}

void SceneObject::InternalUpdate(Graphics& graphics, Camera& camera, Pipeline& pipeline)
{
	UpdateTransformMatrix(graphics, camera);

	Update(graphics, pipeline);

	for (auto& mesh : m_meshes)
		mesh.InternalUpdate(graphics);

	for (auto& child : m_children)
		child->InternalUpdate(graphics, camera, pipeline);
}


void SceneObject::InternalDraw(Graphics& graphics, Pipeline& pipeline) const
{
	Draw(graphics, pipeline);

	for (const auto& mesh : m_meshes)
		mesh.DrawMesh(graphics, pipeline);

	for (const auto& child : m_children)
		child->InternalDraw(graphics, pipeline);
}

void SceneObject::InternalAddStaticResources(Pipeline& pipeline)
{
	AddStaticResources(pipeline);

	for (const auto& child : m_children)
		child->InternalAddStaticResources(pipeline);
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
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;

	if (*selectedObject == this)
		flags |= ImGuiTreeNodeFlags_Selected;

	const bool nodeExpanded = ImGui::TreeNodeEx(m_name.c_str(), flags);

	if (ImGui::IsItemClicked())
		*selectedObject = this;


	ImGui::TreePop();
}

void SceneObject::DrawTransformPropeties()
{
#ifdef _DEBUG
	if (m_meshes.size() > 1)
		std::cout << "unhandled meshes in selected object\n";
#endif

	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	bool transformChanged = false;

	Mesh& targetMesh = m_meshes.front();

	DirectX::XMFLOAT3& position = targetMesh.GetPositionLVal();
	DirectX::XMFLOAT3& rotation = targetMesh.GetRotationLVal();

	ImGui::Text("Position");
	checkChanged(transformChanged, ImGui::SliderFloat("x##position", &position.x, -100.0f, 100.0f));
	checkChanged(transformChanged, ImGui::SliderFloat("y##position", &position.y, -100.0f, 100.0f));
	checkChanged(transformChanged, ImGui::SliderFloat("z##position", &position.z, -100.0f, 100.0f));

	ImGui::NewLine();

	ImGui::Text("Rotation");
	checkChanged(transformChanged, ImGui::SliderAngle("x##rotation", &rotation.x, -180.0f, 180.0f));
	checkChanged(transformChanged, ImGui::SliderAngle("y##rotation", &rotation.y, -180.0f, 180.0f));
	checkChanged(transformChanged, ImGui::SliderAngle("z##rotation", &rotation.z, -180.0f, 180.0f));

	targetMesh.SetTransformChanged(transformChanged);
}

void SceneObject::DrawAdditionalPropeties(Graphics& graphics)
{

}

void SceneObject::UpdateTransformMatrix(Graphics& graphics, Camera& camera)
{

	for (auto& mesh : m_meshes)
		mesh.UpdateTransformMatrix(graphics, camera);
}

void SceneObject::AddMesh(Mesh& mesh)
{
	m_meshes.push_back(std::move(mesh));
}

void SceneObject::AddChild(std::shared_ptr<SceneObject> object)
{
	m_children.push_back(object);
}

SceneObject* SceneObject::GetLastChild()
{
	return m_children.back().get();
}

void SceneObject::SetPosition(DirectX::XMFLOAT3 position)
{
	for (auto& mesh : m_meshes)
		mesh.SetPosition(position);
}

std::string SceneObject::GetName() const
{
	return m_name;
}

void SceneObject::SetName(std::string newName)
{
	m_name = newName;
}