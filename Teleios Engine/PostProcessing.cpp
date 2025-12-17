#include "PostProcessing.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"
#include "TempCommandList.h"


#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderTarget.h"
#include "InputLayout.h"
#include "Sampler.h"
#include "ShaderResourceView.h"
#include "UnorderedAccessView.h"
#include "BlendState.h"
#include "PrimitiveTechnology.h"
#include "RasterizerState.h"
#include "ViewPort.h"

#include "ConstantBuffer.h"
#include "Camera.h"

#include <imgui.h>


#include "TempCommandList.h"

PostProcessing::PostProcessing(Graphics& graphics, Pipeline& pipeline)
{
	m_indexBuffer = std::make_shared<IndexBuffer>(graphics, std::vector<unsigned int>{0, 1, 3, 0, 3, 2});

	DynamicVertex::DynamicVertexLayout layout;
	layout.AddElement<DynamicVertex::ElementType::Position>();
	layout.AddElement<DynamicVertex::ElementType::TextureCoords>();

	struct Vertice
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texCoords;
	};

	std::vector<Vertice> vertices = {
		{{ -1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
		{{ -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }}
	};

	m_vertexBuffer = std::make_shared<VertexBuffer>(graphics, vertices.data(), vertices.size(), sizeof(vertices.at(0)));
	m_inputLayout = std::make_shared<InputLayout>(graphics, layout);

	m_finalPixelShader = std::make_shared<Shader>(graphics, L"PS_Fog", ShaderType::PixelShader);
	m_finalVertexShader = std::make_shared<Shader>(graphics, L"VS_Fullscreen", ShaderType::VertexShader);


	m_vertexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);
	m_indexBuffer->BindToCopyPipelineIfNeeded(graphics, pipeline);

	// requesting size for RT and DS SRV's. Each one has special SRV for one frame
	graphics.GetDescriptorHeap().RequestMoreSpace(graphics.GetBufferCount() * 2);

	// initializing our buffer data here since Camera is not present while pipeline is initialized
	{
		const Camera::Settings defaultCameraSettings = Camera::Settings{};

		// camera data
		{

			DynamicConstantBuffer::ConstantBufferLayout layout;
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("nearPlane");
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("farPlane");

			DynamicConstantBuffer::ConstantBufferData bufferData(layout);
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = defaultCameraSettings.NearZ;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = defaultCameraSettings.FarZ;

			m_cameraData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 0}});
		}

		// fog data
		{
			DynamicConstantBuffer::ConstantBufferLayout layout;
			layout.AddElement<DynamicConstantBuffer::ElementType::Float3>("fogColor", DynamicConstantBuffer::ImguiColorData{true});
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogStart", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogEnd", DynamicConstantBuffer::ImguiFloatData{ true, defaultCameraSettings.NearZ, defaultCameraSettings.FarZ, "%.1f" });
			layout.AddElement<DynamicConstantBuffer::ElementType::Float>("fogDensity", DynamicConstantBuffer::ImguiFloatData{ true, 0.001f, 1.0f, "%.3f" });

			DynamicConstantBuffer::ConstantBufferData bufferData(layout);
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float3>("fogColor") = { 0.45f, 0.55f, 0.65f };
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogStart") = 30.0f;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogEnd") = 250.0f;
			*bufferData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("fogDensity") = 0.015f;

			m_fogData = std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}});
		}
	}
}

void PostProcessing::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	m_renderTargetSRV = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetBackBuffer(), 0);
	m_depthStencilSRV = std::make_shared<ShaderResourceViewMultiResource>(graphics, graphics.GetDepthStencil(), 1);

	// Updating camera data
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();
		const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

		DynamicConstantBuffer::ConstantBufferData& cameraData = m_cameraData->GetData();
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
		*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

		m_cameraData->Update(graphics);
	}

	m_fogData->InternalInitialize(graphics);
}

void PostProcessing::Update(Graphics& graphics, Pipeline& pipeline)
{
	if (ImGui::Begin("Fog"))
	{
		m_fogData->DrawImguiProperties(graphics);
	}

	ImGui::End();

	// if camera viewmatrix updated then update cameraData cbuffer
	{
		Camera* currentCamera = pipeline.GetCurrentCamera();

		if (currentCamera->ViewChanged())
		{
			const Camera::Settings* currentCameraSettings = currentCamera->GetSettings();

			DynamicConstantBuffer::ConstantBufferData& cameraData = m_cameraData->GetData();
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("nearPlane") = currentCameraSettings->NearZ;
			*cameraData.GetValuePointer<DynamicConstantBuffer::ElementType::Float>("farPlane") = currentCameraSettings->FarZ;

			m_cameraData->Update(graphics);
		}
	}
}

void PostProcessing::ApplyTonemapping(Graphics& graphics, Pipeline& pipeline)
{
	RenderTarget* backBuffer = graphics.GetBackBuffer();

	std::shared_ptr<Shader> computeShader = Shader::GetBindableResource(graphics, L"CS_PostProcess", ShaderType::ComputeShader);

	std::shared_ptr<StaticSampler> sampler = StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, { {ShaderVisibilityGraphic::AllShaders, 0} });

	TempComputeCommandList computeCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		// setting entry states
		//{
		//	CommandList* commandList = pipeline.GetGraphicCommandList();
		//
		//	commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		//}

		//UnorderedAccessView uav(graphics, backBuffer);
		//
		//computeCommandList.Bind(computeShader);
		//computeCommandList.Bind(sampler); // s0
		//computeCommandList.Bind(std::move(uav)); // u0
		//
		//computeCommandList.Dispatch(graphics, graphics.GetWidth(), graphics.GetHeight());
	}

	graphics.GetFrameResourceDeleter()->DeleteResource(graphics, std::move(computeCommandList));
}

void PostProcessing::Finish(Graphics& graphics, const Pipeline& pipeline)
{
	GraphicsTexture* backBuffer = graphics.GetBackBuffer()->GetTexture(graphics);
	GraphicsTexture* depthStencil = graphics.GetDepthStencil()->GetResource(graphics);

	// changing current backbuffer state to pixel shader resource so we can bind it with SRV
	{
		CommandList* commandList = pipeline.GetGraphicCommandList();
		commandList->SetResourceState(graphics, backBuffer, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
		commandList->SetResourceState(graphics, depthStencil, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	}
	
	StaticSampler sampler(graphics);
	BlendState blendState(graphics);
	RasterizerState rasterizerState(graphics);
	PrimitiveTechnology topology(graphics, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	RenderTarget* renderTarget = graphics.GetSwapChainBuffer();
	ViewPort viewPort(graphics);

	TempGraphicsCommandList tempGraphicsCommandList(graphics, pipeline.GetGraphicCommandList());

	{
		tempGraphicsCommandList.Bind(m_renderTargetSRV); // t0
		tempGraphicsCommandList.Bind(m_depthStencilSRV); // t1
		tempGraphicsCommandList.Bind(m_cameraData); // b0
		tempGraphicsCommandList.Bind(m_fogData); // b1
		tempGraphicsCommandList.BindIndexBuffer(m_indexBuffer); // ib
		tempGraphicsCommandList.BindVertexBuffer(m_vertexBuffer); // vb
		tempGraphicsCommandList.Bind(m_finalPixelShader); // ps
		tempGraphicsCommandList.Bind(m_finalVertexShader); // vs
		tempGraphicsCommandList.Bind(std::move(sampler)); // s0
		tempGraphicsCommandList.Bind(m_inputLayout); // il
		tempGraphicsCommandList.Bind(std::move(blendState)); // bs
		tempGraphicsCommandList.Bind(std::move(rasterizerState)); // rs
		tempGraphicsCommandList.Bind(std::move(topology)); // topology
		tempGraphicsCommandList.Bind(std::move(viewPort)); // vp
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