#pragma once
#include "SceneObject.h"

#include "includes/CppIncludes.h"
#include "ConstantBuffer.h"

class Pipeline;

class PointLight : public SceneObject
{
public:
	PointLight(Graphics& graphics, DirectX::XMFLOAT3 position = {-1.5f, 0.0f, -1.5f}, DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f });

public:
	virtual void AddStaticResources(Pipeline& pipeline) override;
	
	virtual void Update(Graphics& graphics, Pipeline& pipeline) override;

	virtual void DrawTransformPropeties() override;

	virtual void DrawAdditionalPropeties(Graphics& graphics, Pipeline& pipeline) override;

private:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_color;
	
	std::shared_ptr<CachedConstantBuffer> m_lightBuffer;
	bool m_transformChanged;
	SceneObject* m_sphereModel;
};

