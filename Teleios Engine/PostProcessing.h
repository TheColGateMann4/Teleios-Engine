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

class Graphics;
class Pipeline;

class PostProcessing
{
private:
	enum class Effect
	{
		None,
		Fog,
		DepthOfField,

		numEffects
	};

public:
	PostProcessing(Graphics& graphics, Pipeline& pipeline);

	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void Update(Graphics& graphics, Pipeline& pipeline);

public:
	void ApplyFog(Graphics& graphics, Pipeline& pipeline);
	void ApplyDepthOfField(Graphics& graphics, Pipeline& pipeline);
	void ApplyNothing(Graphics& graphics, Pipeline& pipeline);

	void Finish(Graphics& graphics, Pipeline& pipeline);

private:
	void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline);
	static const char* EffectToString(Effect effect);
	static bool DrawEffectPicker(const char* label, Effect& currentEffect);

private:
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<InputLayout> m_inputLayout;

	std::shared_ptr<StaticSampler> m_sampler;
	std::shared_ptr<BlendState> m_blendState;
	std::shared_ptr<RasterizerState> m_rasterizerState;
	std::shared_ptr<ViewPort> m_viewPort;
	std::shared_ptr<PrimitiveTechnology> m_topology;

	std::shared_ptr<Shader> m_fogPS;
	std::shared_ptr<Shader> m_depthOfFieldPS;
	std::shared_ptr<Shader> m_fullscreenPS;
	std::shared_ptr<Shader> m_fullscreenVS;
	std::shared_ptr<ShaderResourceViewMultiResource> m_renderTargetSRV;
	std::shared_ptr<ShaderResourceViewMultiResource> m_depthStencilSRV;

	std::shared_ptr<CachedConstantBuffer> m_cameraData;
	std::shared_ptr<CachedConstantBuffer> m_fogData;
	std::shared_ptr<CachedConstantBuffer> m_depthOfFieldData;

	Effect m_currentEffect = Effect::None;
};