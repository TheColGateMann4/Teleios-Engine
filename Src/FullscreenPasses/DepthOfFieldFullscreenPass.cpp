#include "DepthOfFieldFullscreenPass.h"

#include "Scene/Objects/Camera.h"
#include "Pipeline.h"
#include "TempCommandList.h"

#include "GraphicsResources/GraphicsBuffer.h"

DepthOfFieldFullscreenPass::DepthOfFieldFullscreenPass(Graphics& graphics)
	:
	FullscreenPass(graphics)
{
	m_depthOfFieldPS = std::make_shared<Shader>(graphics, L"PS_DepthOfField", ShaderType::PixelShader);
	m_depthFromScreenCS = Shader::GetBindableResource(graphics, L"CS_GetMiddleDepth", ShaderType::ComputeShader);

	const Camera::Settings defaultCameraSettings = Camera::Settings{};

	// depth of field data
	{
		DynamicConstantBuffer::ConstantBufferLayout layout;
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("focusRange", DynamicConstantBuffer::ImguiFloatData{ true, 0.1f, defaultCameraSettings.FarZ, "%.1f" });
		layout.AddElement<DynamicConstantBuffer::ElementType::Float>("maxBlur", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 50.0f, "%.1f" });

		DynamicConstantBuffer::ConstantBufferData bufferData(layout);
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("focusRange") = 20.0f;
		*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("maxBlur") = 6.0f;

		m_depthOfFieldData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});
	}

	m_DepthBuffer = std::make_shared<GraphicsBuffer>(graphics, 1, sizeof(float), GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void DepthOfFieldFullscreenPass::Draw(Graphics& graphics, Pipeline& pipeline)
{
	// computing depth in the middle of the screen
	{
		GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

		// setting depth buffer state to UAV before computing
		{
			CommandList* commandList = pipeline.GetGraphicCommandList();

			commandList->SetResourceState(graphics, m_DepthBuffer.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}

		{

			TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

			// resource binding and creating stage-specific resources
			{
				computeCommandList.Bind(m_depthFromScreenCS);
				computeCommandList.Bind(m_depthStencilSRV); // t0
				computeCommandList.Bind(m_DepthBufferUAV); // u0

				computeCommandList.Dispatch(graphics);
			}

			graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));
		}
	}

	// darwing depth of field effect
	{
		// setting depth buffer state to SRV before using it in effect shader
		{
			CommandList* commandList = pipeline.GetGraphicCommandList();

			commandList->SetResourceState(graphics, m_DepthBuffer.get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}

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
				tempGraphicsCommandList.Bind(m_DepthBufferSRV); // t2
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
	}
}

void DepthOfFieldFullscreenPass::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	m_depthOfFieldData->InternalInitialize(graphics);

	m_DepthBufferUAV = std::make_shared<UnorderedAccessView>(graphics, m_DepthBuffer.get(), 0);
	m_DepthBufferSRV = std::make_shared<ShaderResourceView>(graphics, m_DepthBuffer.get(), 2);
}

void DepthOfFieldFullscreenPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void DepthOfFieldFullscreenPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{
	m_depthOfFieldData->DrawImguiProperties(graphics);
}