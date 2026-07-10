#pragma once
#include "Includes/CppIncludes.h"

#include "RenderGraphicsStep.h"

#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/OcclusionPrimitives.h"
#include "Graphics/Bindables/RasterizerState.h"
#include "Graphics/Bindables/MaterialBindings.h"

class SceneObject;
class Material;

class RenderGraphicsGeometryStep : public RenderGraphicsStep
{
public:
	RenderGraphicsGeometryStep(SceneObject* sceneObject, const std::string& name);
	RenderGraphicsGeometryStep(SceneObject* sceneObject);
	RenderGraphicsGeometryStep(RenderGraphicsGeometryStep&&) noexcept = default;
	RenderGraphicsGeometryStep(const RenderGraphicsGeometryStep&) = delete;

public:
	void DrawConstantBuffers(Graphics& graphics);

public: // bindable container functions
	void SetBoundingBox(BoundingBox boundingBox);

	const BoundingBox& GetBoundingBox() const;

	SceneObject* GetSceneObject() const;

	void SetMaterial(std::shared_ptr<Material> material);

	Material* GetMaterial() const;

	ObjectRasterizerStateOptions GetRasterizerOptions() const;

	void SetRasterizerOptions(ObjectRasterizerStateOptions rasterizerOptions);

	virtual void Initialize(Graphics& graphics, Pipeline& pipeline) override;

	MaterialBindings* GetMaterialBindings();

private:
	void InitializeMaterialBindings();

private:
	std::shared_ptr<Material> m_material;
	ObjectRasterizerStateOptions m_rasterizerOptions = {};
	std::shared_ptr<MaterialBindings> m_materialBindings;

	BoundingBox m_boundingBox = {};
	SceneObject* m_sceneObject;
};