#pragma once
#include "includes/CppIncludes.h"
#include "ConstantBuffer.h"

class Pipeline;

class PointLight
{
public:
	PointLight(Graphics& graphics, Pipeline& pipeline, DirectX::XMFLOAT3 position = {-1.5f, 0.0f, -1.5f}, DirectX::XMFLOAT3 color = { 1.0f, 1.0f, 1.0f });

public:
	void DrawImguiWindow(Graphics& graphics, bool isLayerVisible);

	void Update(Graphics& graphics, Pipeline& pipeline);

private:
	std::shared_ptr<CachedConstantBuffer> m_lightBuffer;

	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_color;
};

