#pragma once
#include "Includes/CppIncludes.h"
#include "Mesh.h"
#include "ObjectTransform.h"

class Graphics;
class Pipeline;
class Camera;
class Renderer;

class SceneObject
{
public:
	SceneObject(SceneObject* pParent = nullptr);

public:
	virtual ~SceneObject() = default;

public:
	void InternalInitialize(Graphics& graphics, Pipeline& pipeline);

	void SubmitJobs(Renderer& renderer);

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

	bool isChild() const;

	void MakeChild();

	void HideInHierarchy();

	bool IsVisibleInHierarchy();

private:
	bool hasVisibleChildren();

protected:
	void AddMesh(Mesh& mesh);

	void AddChild(SceneObject* object);

public:
	ObjectTransform* GetTransform();

	std::string GetName() const;

	void SetName(std::string newName);

protected:
	ObjectTransform m_transform;
	std::vector<Mesh> m_meshes;
	std::vector<SceneObject*> m_children;
	std::string m_name = "unnamed";
	bool m_isChild;
	bool m_hideInHierarchy = false;
};