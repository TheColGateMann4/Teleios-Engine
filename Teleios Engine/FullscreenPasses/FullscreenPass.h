#pragma once
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "InputLayout.h"

#include "Sampler.h"
#include "BlendState.h"
#include "RasterizerState.h"
#include "ViewPort.h"
#include "PrimitiveTechnology.h"

#include "Shader.h"

#include "ShaderResourceView.h"

#include "ConstantBuffer.h"

class FullscreenPass
{
public:
	FullscreenPass(Graphics& graphics);
	virtual ~FullscreenPass() = default;

	void Initialize(Graphics& graphics, Pipeline& pipeline);
	void Update(Graphics& graphics, Pipeline& pipeline);

	virtual void Draw(Graphics& graphics, Pipeline& pipeline);

public:
	virtual void InternalInitialize(Graphics& graphics, Pipeline& pipeline);
	virtual void InternalUpdate(Graphics& graphics, Pipeline& pipeline);
	virtual void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline);

protected:
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<InputLayout> m_inputLayout;

	std::shared_ptr<StaticSampler> m_sampler;
	std::shared_ptr<BlendState> m_blendState;
	std::shared_ptr<RasterizerState> m_rasterizerState;
	std::shared_ptr<ViewPort> m_viewPort;
	std::shared_ptr<PrimitiveTechnology> m_topology;

	std::shared_ptr<Shader> m_fullscreenPS;
	std::shared_ptr<Shader> m_fullscreenVS;

	std::shared_ptr<ShaderResourceViewMultiResource> m_renderTargetSRV;
	std::shared_ptr<ShaderResourceViewMultiResource> m_depthStencilSRV;

	std::shared_ptr<CachedConstantBuffer> m_cameraData;
};