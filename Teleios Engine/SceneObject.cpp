#include "SceneObject.h"
#include "Graphics.h"
#include "Pipeline.h"

#include "Drawable.h"

void SceneObject::AddMesh(Drawable* mesh)
{
	m_meshes.push_back(mesh);
}

void SceneObject::AddStaticResources(Pipeline& pipeline)
{

}

void SceneObject::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	Initialize(graphics, pipeline);

	for (auto mesh : m_meshes)
		mesh->Initialize(graphics, pipeline);
}

void SceneObject::Initialize(Graphics& graphics, Pipeline& pipeline)
{

}

void SceneObject::InternalDraw(Graphics& graphics, Pipeline& pipeline) const
{
	Draw(graphics, pipeline);

	for (const auto mesh : m_meshes)
		mesh->DrawDrawable(graphics, pipeline);
}

void SceneObject::Draw(Graphics& graphics, Pipeline& pipeline) const
{

}

void SceneObject::InternalUpdate(Graphics& graphics, Camera& camera, Pipeline& pipeline)
{
	UpdateTransformMatrix(graphics, camera);

	Update(graphics, pipeline);

	for (auto* mesh : m_meshes)
		mesh->InternalUpdate(graphics);
}

void SceneObject::Update(Graphics& graphics, Pipeline& pipeline)
{

}

void SceneObject::DrawImguiWindow(Graphics& graphics, bool isLayerVisible)
{

}

void SceneObject::UpdateTransformMatrix(Graphics& graphics, Camera& camera)
{

	for (auto mesh : m_meshes)
		mesh->UpdateTransformMatrix(graphics, camera);
}