#pragma once
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "InputLayout.h"
#include "Shader.h"
#include "ShaderResourceView.h"

class Graphics;
class Pipeline;

class PostProcessing
{
public:
	PostProcessing(Graphics& graphics, Pipeline& pipeline);

	void ApplyTonemapping(Graphics& graphics, Pipeline& pipeline);
	void Finish(Graphics& graphics, const Pipeline& pipeline);

private:
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<InputLayout> m_inputLayout;

	std::shared_ptr<Shader> m_finalPixelShader;
	std::shared_ptr<Shader> m_finalVertexShader;
	std::vector<std::shared_ptr<ShaderResourceView>> m_renderTargetSRVs;
};