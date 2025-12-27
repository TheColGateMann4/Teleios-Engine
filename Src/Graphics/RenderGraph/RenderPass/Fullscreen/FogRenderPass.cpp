#include "FogRenderPass.h"

#include "Scene/Objects/Camera.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Core/TempCommandList.h"

FogRenderPass::FogRenderPass(Graphics& graphics)
	:
	FullscreenRenderPass(graphics)
{
	m_fogPS = std::make_shared<Shader>(graphics, L"PS_Fog", ShaderType::PixelShader);

	const Camera::Settings defaultCameraSettings = Camera::Settings{};

	// fog data
	{
		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("fogColor", DynamicConstantBuffer::ImguiColorData{ true });
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogStart", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogEnd", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogDensity", DynamicConstantBuffer::ImguiFloatData{ true, 0.0001f, 1.0f, "%.4f" });

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("fogColor") = { 0.45f, 0.55f, 0.65f };
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogStart") = 30.0f;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogEnd") = 250.0f;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogDensity") = 0.015f;

		m_fogData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});
	}
}

void FogRenderPass::Draw(Graphics& graphics, Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
	GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

	m_mesh.Draw(graphics, pipeline);

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
}

void FogRenderPass::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	m_fogData->InternalInitialize(graphics);

	auto renderTarget = graphics.GetSwapChainBuffer();

	m_mesh.AddBindable(m_renderTargetSRV); // t0
	m_mesh.AddBindable(m_depthStencilSRV); // t1
	m_mesh.AddBindable(m_cameraData); // b0
	m_mesh.AddBindable(m_fogData); // b1
	m_mesh.SetIndexBuffer(m_indexBuffer); // ib
	m_mesh.SetVertexBuffer(m_vertexBuffer); // vb
	m_mesh.AddBindable(m_fogPS); // ps
	m_mesh.AddBindable(m_fullscreenVS); // vs
	m_mesh.AddBindable(m_sampler); // s0
	m_mesh.AddBindable(m_inputLayout); // il
	m_mesh.AddBindable(m_blendState); // bs
	m_mesh.AddBindable(m_rasterizerState); // rs
	m_mesh.AddBindable(m_topology); // topology
	m_mesh.AddBindable(m_viewPort); // vp
	m_mesh.AddBindable(renderTarget); // rt

	m_mesh.Initialize(graphics, pipeline);
}

void FogRenderPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void FogRenderPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{
	m_fogData->DrawImguiProperties(graphics);
}