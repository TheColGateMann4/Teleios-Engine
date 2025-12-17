#pragma once
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "InputLayout.h"
#include "Shader.h"
#include "ShaderResourceView.h"
#include "ConstantBuffer.h"

class Graphics;
class Pipeline;

class PostProcessing
{
public:
	PostProcessing(Graphics& graphics, Pipeline& pipeline);

	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void Update(Graphics& graphics, Pipeline& pipeline);
public:
	void ApplyTonemapping(Graphics& graphics, Pipeline& pipeline);
	void Finish(Graphics& graphics, const Pipeline& pipeline);

private:
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<InputLayout> m_inputLayout;

	std::shared_ptr<Shader> m_finalPixelShader;
	std::shared_ptr<Shader> m_finalVertexShader;
	std::shared_ptr<ShaderResourceViewMultiResource> m_renderTargetSRV;
	std::shared_ptr<ShaderResourceViewMultiResource> m_depthStencilSRV;

	std::shared_ptr<CachedConstantBuffer> m_cameraData;
	std::shared_ptr<CachedConstantBuffer> m_fogData;
};