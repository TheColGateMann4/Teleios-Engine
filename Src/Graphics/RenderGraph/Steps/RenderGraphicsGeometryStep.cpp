#include "RenderGraphicsGeometryStep.h"

#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"

#include "Graphics/Bindables/IndexBuffer.h"
#include "Graphics/Bindables/VertexBuffer.h"
#include "Graphics/Bindables/ConstantBuffer.h"
#include "Graphics/Bindables/Texture.h"
#include "Scene/Material.h"

RenderGraphicsGeometryStep::RenderGraphicsGeometryStep(SceneObject* sceneObject, const std::string& name)
	:
	RenderGraphicsStep(name),
	m_sceneObject(sceneObject)
{

}

RenderGraphicsGeometryStep::RenderGraphicsGeometryStep(SceneObject* sceneObject)
	:
	RenderGraphicsStep(),
	m_sceneObject(sceneObject)
{

}

void RenderGraphicsGeometryStep::DrawConstantBuffers(Graphics& graphics)
{
	const std::vector<CachedConstantBuffer*>& cachedBuffers = m_bindableContainer.GetCachedBuffers();

	for (auto& cachedBuffer : cachedBuffers)
		cachedBuffer->DrawImguiProperties(graphics);
}

void RenderGraphicsGeometryStep::SetBoundingBox(BoundingBox boundingBox)
{
	m_boundingBox = boundingBox;
}

const BoundingBox& RenderGraphicsGeometryStep::GetBoundingBox() const
{
	return m_boundingBox;
}

SceneObject* RenderGraphicsGeometryStep::GetSceneObject() const
{
	return m_sceneObject;
}

void RenderGraphicsGeometryStep::SetMaterial(std::shared_ptr<Material> material)
{
	m_material = std::move(material);
}

Material* RenderGraphicsGeometryStep::GetMaterial() const
{
	return m_material.get();
}

ObjectRasterizerStateOptions RenderGraphicsGeometryStep::GetRasterizerOptions() const
{
	return m_rasterizerOptions;
}

void RenderGraphicsGeometryStep::SetRasterizerOptions(ObjectRasterizerStateOptions rasterizerOptions)
{
	m_rasterizerOptions = rasterizerOptions;
}

void RenderGraphicsGeometryStep::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	RenderGraphicsStep::Initialize(graphics, pipeline);

	InitializeMaterialBindings();
}

MaterialBindings* RenderGraphicsGeometryStep::GetMaterialBindings()
{
	return m_materialBindings.get();
}

void RenderGraphicsGeometryStep::InitializeMaterialBindings()
{
	const auto& textureContainer = m_material ? m_material->GetBindableContainer() : GetBindableContainer();
	const auto& textures = textureContainer.GetTextures();

	if (textures.empty())
		return;

	m_materialBindings = std::make_shared<MaterialBindings>(textures);
	
	AddBindable(m_materialBindings->GetDescriptorHeapBindable());
	AddBindable(m_materialBindings->GetTextureIndexesConstants());
}