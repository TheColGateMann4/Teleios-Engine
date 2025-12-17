#include "DepthOfFieldFullscreenPass.h"

#include "Camera.h"
#include "Pipeline.h"
#include "TempCommandList.h"

DepthOfFieldFullscreenPass::DepthOfFieldFullscreenPass(Graphics& graphics)
	:
	FullscreenPass(graphics)
{
	m_depthOfFieldPS = std::make_shared<Shader>(graphics, L"PS_DepthOfField", ShaderType::PixelShader);

	// depth of field data
	{
		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("focusDistance", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("focusRange", DynamicConstantBuffer::ImguiFloatData{ true, 0.1f, defaultCameraSettings.FarZ, "%.1f" });
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("maxBlur", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 50.0f, "%.1f" });

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("focusDistance") = 10.0f;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("focusRange") = 0.5f;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("maxBlur") = 8.0f;

		m_depthOfFieldData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});
	}
}

void DepthOfFieldFullscreenPass::Draw(Graphics& graphics, Pipeline& pipeline)
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
		tempGraphicsCommandList.Bind(m_depthOfFieldData); // b1
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_depthOfFieldPS); // ps
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

void DepthOfFieldFullscreenPass::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	m_depthOfFieldData->InternalInitialize(graphics);
}

void DepthOfFieldFullscreenPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void DepthOfFieldFullscreenPass::DrawImguiPropeties(Graphics & graphics, Pipeline & pipeline)
{
	m_depthOfFieldData->DrawImguiProperties(graphics);
}