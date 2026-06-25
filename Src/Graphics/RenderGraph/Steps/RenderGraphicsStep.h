#pragma once
#include "Includes/CppIncludes.h"

#include "RenderStep.h"

#include "Graphics/Core/PipelineState.h"
#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/BindableContainer.h"
#include "Graphics/Core/OcclusionPrimitives.h"
#include "Graphics/Bindables/RasterizerState.h"

class Material;

class RenderGraphicsStep : public RenderStep
{
public:
	RenderGraphicsStep(const std::string& name);
	RenderGraphicsStep();
	RenderGraphicsStep(RenderGraphicsStep&&) noexcept = default;
	RenderGraphicsStep(const RenderGraphicsStep&) = delete;

public:
	void DrawConstantBuffers(Graphics& graphics);

public: // bindable container functions
	void AddStaticBindable(const char* bindableName);

	void SetAttributeBufferEntry(std::shared_ptr<VertexBufferEntry> attributeBufferEntry);

	void SetPositionBufferEntry(std::shared_ptr<VertexBufferEntry> positionBufferEntry);

	void SetIndexBufferEntry(std::shared_ptr<IndexBufferEntry> indexBufferEntry);

	void SetBoundingBox(BoundingBox boundingBox);

	const BoundingBox& GetBoundingBox() const;

	void AddBindable(std::shared_ptr<Bindable> bindable);

	void SetMaterial(std::shared_ptr<Material> material);

	void AddBindable(Bindable* bindable);

	const MeshBindableContainer& GetBindableContainter() const;

	Material* GetMaterial() const;

	ObjectRasterizerStateOptions GetRasterizerOptions() const;

	void SetRasterizerOptions(ObjectRasterizerStateOptions rasterizerOptions);

private:
	MeshBindableContainer m_bindableContainer;
	std::shared_ptr<Material> m_material;
	ObjectRasterizerStateOptions m_rasterizerOptions = {};

	BoundingBox m_boundingBox = {};
};