#pragma once
#include "includes/CppIncludes.h"

class Graphics;
class Pipeline;
class Camera;
class Drawable;

class SceneObject
{
public:
	virtual ~SceneObject() = default;

public:
	void AddMesh(Drawable* mesh);
	
	virtual void AddStaticResources(Pipeline& pipeline);
	virtual void Initialize(Graphics& graphics, Pipeline& pipeline);
	
	virtual void Draw(Graphics& graphics, Pipeline& pipeline) const;

	void InternalUpdate(Graphics& graphics, Camera& camera, Pipeline& pipeline);

	virtual void Update(Graphics& graphics, Pipeline& pipeline);

	virtual void DrawImguiWindow(Graphics& graphics, bool isLayerVisible);

	virtual void UpdateTransformMatrix(Graphics& graphics, Camera& camera);

private:
	std::vector<Drawable*> m_meshes;
};