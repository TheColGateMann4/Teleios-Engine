#pragma once
#include "includes/CppIncludes.h"
#include "Mesh.h"
#include "ObjectTransform.h"

class Graphics;
class Pipeline;
class Camera;

class SceneObject
{
public:
	virtual ~SceneObject() = default;

public:
	void InternalInitialize(Graphics& graphics, Pipeline& pipeline);

	void InternalDraw(Graphics& graphics, Pipeline& pipeline) const;

	void InternalUpdate(Graphics& graphics, Camera& camera, Pipeline& pipeline);

	void InternalAddStaticResources(Pipeline& pipeline);

	void InitializeGraphicResources(Graphics& graphics, Pipeline& pipeline);

	void UpdateParentMatrix(DirectX::XMMATRIX parentMatrix = DirectX::XMMatrixIdentity());

	// this function allows us to update transformCbuffer once per frame
	void UpdateTransformBufferIfNeeded(Graphics& graphics, Camera& camera);

	void UpdateLocalTransformIfNeeded();

private:
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline);

	virtual void Draw(Graphics& graphics, Pipeline& pipeline) const;

	virtual void Update(Graphics& graphics, Pipeline& pipeline);

	virtual void AddStaticResources(Pipeline& pipeline);

public:
	void DrawHierarchy(SceneObject** selectedObject);

	virtual void DrawTransformPropeties();

	virtual void DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline);

	void DrawConstantBuffers(Graphics& graphics);

	virtual void UpdateTransformMatrix(Graphics& graphics, Camera& camera);

protected:
	void AddMesh(Mesh& mesh);

	void AddChild(std::shared_ptr<SceneObject> object);

	SceneObject* GetLastChild();

public:
	ObjectTransform* GetTransform();

	std::string GetName() const;

	void SetName(std::string newName);

protected:
	ObjectTransform m_transform;
	std::vector<Mesh> m_meshes;
	std::vector<std::shared_ptr<SceneObject>> m_children;
	std::string m_name = "unnamed";
};