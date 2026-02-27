#include "DepthOfFieldRenderPass.h"

/*
#include "Scene/Objects/Camera.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/Pipeline.h"
#include "Graphics/Core/TempCommandList.h"

#include "Graphics/Resources/GraphicsBuffer.h"

DepthOfFieldRenderPass::DepthOfFieldRenderPass(Graphics& graphics)
	:
	FullscreenRenderPass(graphics)
{
	m_depthOfFieldPS = std::make_shared<Shader>(graphics, L"PS_DepthOfField", ShaderType::PixelShader);
	m_depthFromScreenCS = Shader::GetResource(graphics, L"CS_GetMiddleDepth", ShaderType::ComputeShader);

	const Camera::Settings defaultCameraSettings = Camera::Settings{};

	// depth of field data
	{
		DynamicConstantBuffer::Layout layout;
		layout.Add<DynamicConstantBuffer::ElementType::Float>("focusRange", DynamicConstantBuffer::ImguiFloatData{ true, 0.1f, defaultCameraSettings.FarZ, "%.1f" });
		layout.Add<DynamicConstantBuffer::ElementType::Float>("maxBlur", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 50.0f, "%.1f" });

		DynamicConstantBuffer::Data bufferData(layout);
		*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("focusRange") = 20.0f;
		*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("maxBlur") = 6.0f;

		m_depthOfFieldData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});
	}

	m_DepthBuffer = std::make_shared<GraphicsBuffer>(graphics, 1, sizeof(float), GraphicsResource::CPUAccess::notavailable, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void FullscreenRenderPass::PreDraw(Graphics& graphics, CommandList* commandList)
{
	// computing depth in the middle of the screen
	{
		GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

		// setting depth buffer state to UAV before computing
		{
			commandList->SetResourceState(graphics, m_DepthBuffer.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}

		{

			TempComputeCommandList computeCommandList(graphics, commandList);

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

	// setting correct states
	{
		// setting depth buffer state to SRV before using it in effect shader
		{
			commandList->SetResourceState(graphics, m_DepthBuffer.get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		}

		{
			GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
			GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

			// changing current backbuffer state to pixel shader resource so we can bind it with SRV
			{
				commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
				commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			}
		}
	}
}

void FullscreenRenderPass::PostDraw(Graphics& graphics, CommandList* commandList)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
	GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

	// changing state of current backbuffer back to render target state
	{
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}
}

void DepthOfFieldRenderPass::InternalInitialize(Graphics& graphics, Pipeline& pipeline)
{
	m_depthOfFieldData->InternalInitialize(graphics);

	m_DepthBufferUAV = std::make_shared<UnorderedAccessView>(graphics, m_DepthBuffer.get(), 0);
	m_DepthBufferSRV = std::make_shared<ShaderResourceView>(graphics, m_DepthBuffer.get(), 2);

	auto renderTarget = graphics.GetSwapChainBuffer();

	m_mesh.AddBindable(m_renderTargetSRV); // t0
	m_mesh.AddBindable(m_depthStencilSRV); // t1
	m_mesh.AddBindable(m_DepthBufferSRV); // t2
	m_mesh.AddBindable(m_cameraData); // b0
	m_mesh.AddBindable(m_depthOfFieldData); // b1
	m_mesh.AddBindable(m_indexBuffer); // ib
	m_mesh.AddBindable(m_vertexBuffer); // vb
	m_mesh.AddBindable(m_depthOfFieldPS); // ps
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

void DepthOfFieldRenderPass::InternalUpdate(Graphics& graphics, Pipeline& pipeline)
{

}

void DepthOfFieldRenderPass::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{
	m_depthOfFieldData->DrawImguiProperties(graphics);
}
*/