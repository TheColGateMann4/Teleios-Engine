#include "FogFullscreenPass.h"

#include "Camera.h"
#include "Pipeline.h"
#include "TempCommandList.h"

FogFullscreenPass::FogFullscreenPass(Graphics& graphics)
	:
	FullscreenPass(graphics)
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

void FogFullscreenPass::Draw(Graphics& graphics, Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
	GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}

	RenderTarget* renderTarget = graphics.GetSwapChainBuffer();

	TempGraphicsCommandList tempGraphicsCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		tempGraphicsCommandList.Bind(m_renderTargetSRV); // t0
		tempGraphicsCommandList.Bind(m_depthStencilSRV); // t1
		tempGraphicsCommandList.Bind(m_cameraData); // b0
		tempGraphicsCommandList.Bind(m_fogData); // b1
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_fogPS); // ps
		tempGraphicsCommandList.Bind(m_fullscreenVS); // vs
		tempGraphicsCommandList.Bind(m_sampler); // s0
		tempGraphicsCommandList.Bind(m_inputLayout); // il
		tempGraphicsCommandList.Bind(m_blendState); // bs
		tempGraphicsCommandList.Bind(m_rasterizerState); // rs
		tempGraphicsCommandList.Bind(m_topology); // topology
		tempGraphicsCommandList.Bind(m_viewPort); // vp
		tempGraphicsCommandList.Bind(renderTarget); // rt

		tempGraphicsCommandList.DrawIndexed(graphics);
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(tempGraphicsCommandList));

	// changing state of current backbuffer back to render target state
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
}

void FogFullscreenPass::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	m_fogData->InternalInitialize(graphics);
}

void FogFullscreenPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void FogFullscreenPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{
	m_fogData->DrawImguiProperties(graphics);
}